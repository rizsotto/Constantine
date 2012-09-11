// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#include "VariableChecker.hpp"
#include "ConstantAnalysis.hpp"

#include <algorithm>
#include <functional>
#include <iterator>
#include <cassert>

#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/range.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/algorithm/transform.hpp>

#include <clang/AST/AST.h>
#include <clang/AST/RecursiveASTVisitor.h>


namespace {

// Report function for pseudo constness analysis.
void ReportVariablePseudoConstness(clang::DiagnosticsEngine & DE, clang::VarDecl const * const Decl) {
    static char const * const Message = "variable '%0' could be declared as const [Medve plugin]";
    unsigned const DiagID = DE.getCustomDiagID(clang::DiagnosticsEngine::Warning, Message);
    clang::DiagnosticBuilder DB = DE.Report(Decl->getLocStart(), DiagID);
    DB << Decl->getNameAsString();
}

// Report function for debug functionality.
void ReportVariableDeclaration(clang::DiagnosticsEngine & DE, clang::VarDecl const * const Decl) {
    static char const * const Message = "variable '%0' declared here";
    unsigned const DiagID = DE.getCustomDiagID(clang::DiagnosticsEngine::Note, Message);
    clang::DiagnosticBuilder DB = DE.Report(Decl->getLocStart(), DiagID);
    DB << Decl->getNameAsString();
    DB.setForceEmit();
}

void ReportFunctionDeclaration(clang::DiagnosticsEngine & DE, clang::FunctionDecl const * const F) {
    static char const * const Message = "function '%0' declared here";
    unsigned const Id = DE.getCustomDiagID(clang::DiagnosticsEngine::Note, Message);
    clang::DiagnosticBuilder DB = DE.Report(F->getSourceRange().getBegin(), Id);
    DB << F->getNameAsString();
    DB.setForceEmit();
}


typedef std::set<clang::VarDecl const *> Variables;


// Pseudo constness analysis try to detect what variable can be declare as const.
// This analysis runs through multiple scopes. We need to store the state of the
// ongoing analysis. Once the variable was changed can't be const.
class PseudoConstnessAnalysisState : public boost::noncopyable {
public:
    PseudoConstnessAnalysisState()
        : boost::noncopyable()
        , Candidates()
        , Changed()
    { }

    void Eval( ConstantAnalysis const & Analysis, clang::VarDecl const * const Decl) {
        if (Analysis.WasChanged(Decl)) {
            Candidates.erase(Decl);
            Changed.insert(Decl);
        } else if (Analysis.WasReferenced(Decl)) {
            if (Changed.end() == Changed.find(Decl)) {
                if (! IsConst(*Decl)) {
                    Candidates.insert(Decl);
                }
            }
        }
    }

    void GenerateReports(clang::DiagnosticsEngine & DE) {
        boost::for_each(Candidates, boost::bind(ReportVariablePseudoConstness, boost::ref(DE), _1));
    }

private:
    static bool IsConst(clang::VarDecl const & D) {
        return (D.getType().getNonReferenceType().isConstQualified());
    }

private:
    Variables Candidates;
    Variables Changed;
};


// Represents a function declaration. This class try to add new methods
// to clang::FunctionDecl. That type is part of an inheritance tree.
// To add functions to it, here is a duplication of the hierarchy.
struct FunctionWrapper : public boost::noncopyable {
protected:
    virtual clang::FunctionDecl const * GetFunctionDecl() const = 0;

    typedef std::set<clang::VarDecl const *> VarDeclSet;
    virtual VarDeclSet GetArguments() const = 0;
    virtual VarDeclSet GetLocals() const = 0;
    virtual VarDeclSet GetMembers() const = 0;

public:
    // Debug functionality
    virtual void DumpFuncionDeclaration(clang::DiagnosticsEngine &) const;
    virtual void DumpArguments(clang::DiagnosticsEngine &) const;
    virtual void DumpLocalVariables(clang::DiagnosticsEngine &) const;
    virtual void DumpMemberVariables(clang::DiagnosticsEngine &) const;

    // Analysis functionality
    virtual void DumpVariableChanges(clang::DiagnosticsEngine &) const;
    virtual void DumpVariableUsages(clang::DiagnosticsEngine &) const;
    virtual void CheckPseudoConstness(PseudoConstnessAnalysisState &) const;
};

void FunctionWrapper::DumpFuncionDeclaration(clang::DiagnosticsEngine & DE) const {
    clang::FunctionDecl const * F = GetFunctionDecl();
    ReportFunctionDeclaration(DE, F);
}

void FunctionWrapper::DumpArguments(clang::DiagnosticsEngine & DE) const {
    boost::for_each(GetArguments(), boost::bind(ReportVariableDeclaration, DE, _1));
}

void FunctionWrapper::DumpLocalVariables(clang::DiagnosticsEngine & DE) const {
    boost::for_each(GetLocals(), boost::bind(ReportVariableDeclaration, DE, _1));
}

void FunctionWrapper::DumpMemberVariables(clang::DiagnosticsEngine & DE) const {
    boost::for_each(GetMembers(), boost::bind(ReportVariableDeclaration, DE, _1));
}

void FunctionWrapper::DumpVariableChanges(clang::DiagnosticsEngine & DE) const {
    clang::FunctionDecl const * F = GetFunctionDecl();
    ConstantAnalysis const & Analysis = ConstantAnalysis::AnalyseThis(*(F->getBody()));
    Analysis.DebugChanged(DE);
}

void FunctionWrapper::DumpVariableUsages(clang::DiagnosticsEngine & DE) const {
    clang::FunctionDecl const * F = GetFunctionDecl();
    ConstantAnalysis const & Analysis = ConstantAnalysis::AnalyseThis(*(F->getBody()));
    Analysis.DebugReferenced(DE);
}

void FunctionWrapper::CheckPseudoConstness(PseudoConstnessAnalysisState & State) const {
    clang::FunctionDecl const * F = GetFunctionDecl();
    ConstantAnalysis const & Analysis = ConstantAnalysis::AnalyseThis(*(F->getBody()));

    boost::for_each(GetArguments(),
        boost::bind(&PseudoConstnessAnalysisState::Eval, &State, boost::cref(Analysis), _1));
    boost::for_each(GetLocals(),
        boost::bind(&PseudoConstnessAnalysisState::Eval, &State, boost::cref(Analysis), _1));
    boost::for_each(GetMembers(),
        boost::bind(&PseudoConstnessAnalysisState::Eval, &State, boost::cref(Analysis), _1));
}

class FunctionDeclWrapper : public FunctionWrapper {
public:
    FunctionDeclWrapper(clang::FunctionDecl const * const F)
        : FunctionWrapper()
        , Function(F)
    { }

protected:
    struct CastVarDecl {
        clang::VarDecl const * operator()(clang::Decl const * const Decl) const {
            return clang::dyn_cast<clang::VarDecl const>(Decl);
        }
    };

    clang::FunctionDecl const * GetFunctionDecl() const {
        return Function;
    }

    VarDeclSet GetArguments() const {
        VarDeclSet Result;
        boost::copy(
            boost::make_iterator_range(Function->param_begin(), Function->param_end()),
            std::insert_iterator<VarDeclSet>(Result, Result.begin()));
        return Result;
    }

    VarDeclSet GetLocals() const {
        VarDeclSet Result;
        boost::transform(
              boost::make_iterator_range(Function->decls_begin(), Function->decls_end())
            , std::insert_iterator<VarDeclSet>(Result, Result.begin())
            , CastVarDecl()
        );
        Result.erase(0);
        return Result;
    }

    VarDeclSet GetMembers() const {
        return VarDeclSet();
    }

private:
    clang::FunctionDecl const * const Function;
};


// This class collect function declarations and create wrapped classes around them.
class FunctionCollector : public clang::RecursiveASTVisitor<FunctionCollector> {
public:
    void DumpFuncionDeclaration(clang::DiagnosticsEngine & DE) const {
        boost::for_each(Functions,
            boost::bind(&FunctionWrapper::DumpFuncionDeclaration, _1, boost::ref(DE)));
    }

    void DumpArguments(clang::DiagnosticsEngine & DE) const {
        boost::for_each(Functions,
            boost::bind(&FunctionWrapper::DumpArguments, _1, boost::ref(DE)));
    }

    void DumpLocalVariables(clang::DiagnosticsEngine & DE) const {
        boost::for_each(Functions,
            boost::bind(&FunctionWrapper::DumpLocalVariables, _1, boost::ref(DE)));
    }

    void DumpMemberVariables(clang::DiagnosticsEngine & DE) const {
        boost::for_each(Functions,
            boost::bind(&FunctionWrapper::DumpMemberVariables, _1, boost::ref(DE)));
    }

    void DumpVariableChanges(clang::DiagnosticsEngine & DE) const {
        boost::for_each(Functions,
            boost::bind(&FunctionWrapper::DumpVariableChanges, _1, boost::ref(DE)));
    }

    void DumpVariableUsages(clang::DiagnosticsEngine & DE) const {
        boost::for_each(Functions,
            boost::bind(&FunctionWrapper::DumpVariableUsages, _1, boost::ref(DE)));
    }

    void DumpPseudoConstness(clang::DiagnosticsEngine & DE) const {
        PseudoConstnessAnalysisState State;
        boost::for_each(Functions,
            boost::bind(&FunctionWrapper::CheckPseudoConstness, _1, boost::ref(State)));
        State.GenerateReports(DE);
    }

    // this is for visitor pattern
    bool VisitFunctionDecl(clang::FunctionDecl const * F) {
        Functions.insert( FunctionWrapperPtr(new FunctionDeclWrapper(F)) );
        return true;
    }

private:
    typedef boost::shared_ptr<FunctionWrapper> FunctionWrapperPtr;
    typedef std::set<FunctionWrapperPtr> FunctionWrapperPtrs;

    FunctionWrapperPtrs Functions;
};

} // namespace anonymous


VariableChecker::VariableChecker(clang::CompilerInstance const & Compiler, bool const InDebugChanges, bool const InDebugUsages)
    : clang::ASTConsumer()
    , Reporter(Compiler.getDiagnostics())
    , DebugChanges(InDebugChanges)
    , DebugUsages(InDebugUsages)
{ }

void VariableChecker::HandleTranslationUnit(clang::ASTContext & Ctx) {
    FunctionCollector Collector;
    Collector.TraverseDecl(Ctx.getTranslationUnitDecl());

    if (DebugChanges) {
        Collector.DumpVariableChanges(Reporter);
    } else if (DebugUsages) {
        Collector.DumpVariableUsages(Reporter);
    } else {
        Collector.DumpPseudoConstness(Reporter);
    }
}
