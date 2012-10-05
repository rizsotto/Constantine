// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#include "ModuleAnalysis.hpp"
#include "ScopeAnalysis.hpp"

#include <iterator>
#include <map>

#include <clang/AST/AST.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/range.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/algorithm/transform.hpp>


namespace {

// Report function for pseudo constness analysis.
void ReportVariablePseudoConstness(clang::DiagnosticsEngine & DE, clang::DeclaratorDecl const * const V) {
    static char const * const Message = "variable '%0' could be declared as const";
    unsigned const Id = DE.getCustomDiagID(clang::DiagnosticsEngine::Warning, Message);
    clang::DiagnosticBuilder DB = DE.Report(V->getLocStart(), Id);
    DB << V->getNameAsString();
}

// Report function for debug functionality.
void ReportVariableDeclaration(clang::DiagnosticsEngine & DE, clang::DeclaratorDecl const * const V) {
    static char const * const Message = "variable '%0' declared here";
    unsigned const Id = DE.getCustomDiagID(clang::DiagnosticsEngine::Note, Message);
    clang::DiagnosticBuilder DB = DE.Report(V->getLocStart(), Id);
    DB << V->getNameAsString();
    DB.setForceEmit();
}

void ReportFunctionDeclaration(clang::DiagnosticsEngine & DE, clang::FunctionDecl const * const F) {
    static char const * const Message = "function '%0' declared here";
    unsigned const Id = DE.getCustomDiagID(clang::DiagnosticsEngine::Note, Message);
    clang::DiagnosticBuilder DB = DE.Report(F->getSourceRange().getBegin(), Id);
    DB << F->getNameAsString();
    DB.setForceEmit();
}


typedef std::set<clang::DeclaratorDecl const *> Variables;


// Pseudo constness analysis detects what variable can be declare as const.
// This analysis runs through multiple scopes. We need to store the state of
// the ongoing analysis. Once the variable was changed can't be const.
class PseudoConstnessAnalysisState : public boost::noncopyable {
public:
    PseudoConstnessAnalysisState()
        : boost::noncopyable()
        , Candidates()
        , Changed()
    { }

    void Eval(ScopeAnalysis const & Analysis, clang::DeclaratorDecl const * const V) {
        if (Analysis.WasChanged(V)) {
            Candidates.erase(V);
            Changed.insert(V);
        } else if (Analysis.WasReferenced(V)) {
            if (Changed.end() == Changed.find(V)) {
                if (! IsConst(*V)) {
                    Candidates.insert(V);
                }
            }
        }
    }

    void GenerateReports(clang::DiagnosticsEngine & DE) const {
        boost::for_each(Candidates,
            boost::bind(ReportVariablePseudoConstness, boost::ref(DE), _1));
    }

private:
    static bool IsConst(clang::DeclaratorDecl const & D) {
        return (D.getType().getNonReferenceType().isConstQualified());
    }

private:
    Variables Candidates;
    Variables Changed;
};


// method to copy variables out from declaration context
Variables GetVariablesFromContext(clang::DeclContext const * const F, bool const WithoutArgs = false) {
    Variables Result;
    for (clang::DeclContext::decl_iterator It(F->decls_begin()), End(F->decls_end()); It != End; ++It ) {
        if (clang::VarDecl const * const D = clang::dyn_cast<clang::VarDecl const>(*It)) {
            if (! (WithoutArgs && (clang::dyn_cast<clang::ParmVarDecl const>(D)))) {
                Result.insert(D);
            }
        }
    }
    return Result;
}

// method to copy variables out from class declaration 
Variables GetVariablesFromRecord(clang::RecordDecl const * const F) {
    Variables Result;
    for (clang::RecordDecl::field_iterator It(F->field_begin()), End(F->field_end()); It != End; ++It ) {
        if (clang::FieldDecl const * const D = clang::dyn_cast<clang::FieldDecl const>(*It)) {
            Result.insert(D);
        }
    }
    return Result;
}


// Base class for analysis. Implement function declaration visitor, which visit
// functions only once. The traversal algorithm is calling all methods, which is
// not desired. In case of a CXXMethodDecl, it was calling the VisitFunctionDecl
// and the VisitCXXMethodDecl as well. This dispatching is reworked in this class.
class ModuleVisitor
    : public boost::noncopyable
    , public clang::RecursiveASTVisitor<ModuleVisitor> {
public:
    typedef boost::shared_ptr<ModuleVisitor> Ptr;
    static ModuleVisitor::Ptr CreateVisitor(Target);

    virtual ~ModuleVisitor()
    { }

public:
    // public visitor method.
    bool VisitFunctionDecl(clang::FunctionDecl const * F) {
        if (clang::CXXMethodDecl const * const D = clang::dyn_cast<clang::CXXMethodDecl const>(F)) {
            return OnCXXMethodDecl(D);
        }
        return OnFunctionDecl(F);
    }

public:
    // interface methods with different visibilities.
    virtual void Dump(clang::DiagnosticsEngine &) const = 0;

protected:
    virtual bool OnFunctionDecl(clang::FunctionDecl const *) = 0;
    virtual bool OnCXXMethodDecl(clang::CXXMethodDecl const *) = 0;
};


class DebugFunctionDeclarations
    : public ModuleVisitor {
protected:
    bool OnFunctionDecl(clang::FunctionDecl const * const F) {
        if (F->isThisDeclarationADefinition()) {
            Functions.insert(F);
        }
        return true;
    }

    bool OnCXXMethodDecl(clang::CXXMethodDecl const * const F) {
        if (F->isThisDeclarationADefinition()) {
            Functions.insert(F);
        }
        return true;
    }

    void Dump(clang::DiagnosticsEngine & DE) const {
        boost::for_each(Functions,
            boost::bind(ReportFunctionDeclaration, boost::ref(DE), _1));
    }

protected:
    std::set<clang::FunctionDecl const *> Functions;
};


class DebugVariableDeclarations
    : public ModuleVisitor {
private:
    bool OnFunctionDecl(clang::FunctionDecl const * const F) {
        if (F->isThisDeclarationADefinition()) {
            boost::copy(GetVariablesFromContext(F),
                std::insert_iterator<Variables>(Result, Result.begin()));
        }
        return true;
    }

    bool OnCXXMethodDecl(clang::CXXMethodDecl const * const F) {
        if (F->isThisDeclarationADefinition()) {
            boost::copy(GetVariablesFromContext(F, F->isVirtual()),
                std::insert_iterator<Variables>(Result, Result.begin()));
            boost::copy(GetVariablesFromRecord(F->getParent()->getCanonicalDecl()),
                std::insert_iterator<Variables>(Result, Result.begin()));
        }
        return true;
    }

    void Dump(clang::DiagnosticsEngine & DE) const {
        boost::for_each(Result,
            boost::bind(ReportVariableDeclaration, boost::ref(DE), _1));
    }

private:
    Variables Result;
};


class DebugVariableUsages
    : public DebugFunctionDeclarations {
private:
    static void ReportVariableUsage(clang::DiagnosticsEngine & DE, clang::FunctionDecl const * F) {
        ScopeAnalysis const & Analysis = ScopeAnalysis::AnalyseThis(*(F->getBody()));
        Analysis.DebugReferenced(DE);
    }

    void Dump(clang::DiagnosticsEngine & DE) const {
        boost::for_each(Functions,
            boost::bind(ReportVariableUsage, boost::ref(DE), _1));
    }
};


class DebugVariableChanges
    : public DebugFunctionDeclarations {
private:
    static void ReportVariableUsage(clang::DiagnosticsEngine & DE, clang::FunctionDecl const * F) {
        ScopeAnalysis const & Analysis = ScopeAnalysis::AnalyseThis(*(F->getBody()));
        Analysis.DebugChanged(DE);
    }

    void Dump(clang::DiagnosticsEngine & DE) const {
        boost::for_each(Functions,
            boost::bind(ReportVariableUsage, boost::ref(DE), _1));
    }
};


class AnalyseVariableUsage
    : public ModuleVisitor {
private:
    bool OnFunctionDecl(clang::FunctionDecl const * const F) {
        if (F->isThisDeclarationADefinition()) {
            ScopeAnalysis const & Analysis = ScopeAnalysis::AnalyseThis(*(F->getBody()));
            boost::for_each(GetVariablesFromContext(F),
                boost::bind(&PseudoConstnessAnalysisState::Eval, &State, boost::cref(Analysis), _1));
        }
        return true;
    }

    bool OnCXXMethodDecl(clang::CXXMethodDecl const * const F) {
        if (F->isThisDeclarationADefinition()) {
            ScopeAnalysis const & Analysis = ScopeAnalysis::AnalyseThis(*(F->getBody()));
            boost::for_each(GetVariablesFromContext(F, F->isVirtual()),
                boost::bind(&PseudoConstnessAnalysisState::Eval, &State, boost::cref(Analysis), _1));
            boost::for_each(GetVariablesFromRecord(F->getParent()->getCanonicalDecl()),
                boost::bind(&PseudoConstnessAnalysisState::Eval, &State, boost::cref(Analysis), _1));
        }
        return true;
    }

    void Dump(clang::DiagnosticsEngine & DE) const {
        State.GenerateReports(DE);
    }

private:
    PseudoConstnessAnalysisState State;
};


ModuleVisitor::Ptr ModuleVisitor::CreateVisitor(Target const State) {
    switch (State) {
    case FuncionDeclaration :
        return ModuleVisitor::Ptr( new DebugFunctionDeclarations() );
    case VariableDeclaration :
        return ModuleVisitor::Ptr( new DebugVariableDeclarations() );
    case VariableChanges:
        return ModuleVisitor::Ptr( new DebugVariableChanges() );
    case VariableUsages :
        return ModuleVisitor::Ptr( new DebugVariableUsages() );
    case PseudoConstness :
        return ModuleVisitor::Ptr( new AnalyseVariableUsage() );
    }
}

} // namespace anonymous


ModuleAnalysis::ModuleAnalysis(clang::CompilerInstance const & Compiler, Target T)
    : boost::noncopyable()
    , clang::ASTConsumer()
    , Reporter(Compiler.getDiagnostics())
    , State(T)
{ }

void ModuleAnalysis::HandleTranslationUnit(clang::ASTContext & Ctx) {
    ModuleVisitor::Ptr V = ModuleVisitor::CreateVisitor(State);
    V->TraverseDecl(Ctx.getTranslationUnitDecl());
    V->Dump(Reporter);
}
