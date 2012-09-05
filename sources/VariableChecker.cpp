// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#include "VariableChecker.hpp"
#include "ConstantAnalysis.hpp"

#include <algorithm>
#include <functional>
#include <iterator>
#include <cassert>

#include <clang/AST/AST.h>
#include <clang/AST/RecursiveASTVisitor.h>


namespace {

typedef std::set<clang::VarDecl const *> Variables;

void report(clang::DiagnosticsEngine & DiagEng, clang::VarDecl const * const Decl) {
    std::string Msg;
    Msg += "variable '";
    Msg += Decl->getNameAsString();
    Msg += "' could be declared as const [Medve plugin]";

    unsigned const DiagID = DiagEng.getCustomDiagID(clang::DiagnosticsEngine::Warning, Msg.c_str());
    DiagEng.Report(Decl->getLocStart(), DiagID);
}


class FunctionVisitor : public clang::RecursiveASTVisitor<FunctionVisitor> {
public:
    FunctionVisitor(Variables & In, clang::DiagnosticsEngine * const Diagnostics)
        : clang::RecursiveASTVisitor<FunctionVisitor>()
        , Candidates(In)
        , Changed()
        , Reporter(Diagnostics)
    {}

    static bool IsConst(clang::VarDecl const & D) {
        return (D.getType().getNonReferenceType().isConstQualified());
    }

    void Decide(clang::VarDecl const & Var, bool const Constness) {
        if (Changed.end() == Changed.find(&Var)) {
            if (! IsConst(Var)) {
                if (Constness) {
                    Candidates.insert(&Var);
                } else {
                    Candidates.erase(&Var);
                    Changed.insert(&Var);
                }
            }
        }
    }

    bool VisitFunctionDecl(clang::FunctionDecl const * F) {
        ConstantAnalysis const & Analysis = ConstantAnalysis::AnalyseThis(*(F->getBody()));
        if (Reporter) {
            Analysis.Debug(*Reporter);
        }
        for (clang::DeclContext::decl_iterator It(F->decls_begin()), End(F->decls_end()); It != End; ++It) {
            if (clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl>(*It)) {
                Decide(*VD, ! Analysis.WasChanged(VD));
            }
        }
        return true;
    }

    Variables const & GetResult() const {
        return Candidates;
    }

private:
    Variables & Candidates;

    Variables Changed;
    clang::DiagnosticsEngine * const Reporter;
};

} // namespace anonymous


VariableChecker::VariableChecker(clang::CompilerInstance const & Compiler, bool const Verbose)
    : clang::ASTConsumer()
    , DiagEng(Compiler.getDiagnostics())
    , VerboseAnalysis(Verbose)
{ }

void VariableChecker::HandleTranslationUnit(clang::ASTContext & Ctx) {
    Variables Result;
    FunctionVisitor Collector(Result, (VerboseAnalysis ? &DiagEng : 0));
    Collector.TraverseDecl(Ctx.getTranslationUnitDecl());
    // Print out the results
    std::for_each(Result.begin(), Result.end(), std::bind1st(std::ptr_fun(report), DiagEng));
}
