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
    char const * const Message = "variable '%0' could be declared as const [Medve plugin]";

    unsigned const DiagID = DiagEng.getCustomDiagID(clang::DiagnosticsEngine::Warning, Message);
    clang::DiagnosticBuilder Reporter = DiagEng.Report(Decl->getLocStart(), DiagID);
    Reporter << Decl->getNameAsString();
}


class FunctionVisitor : public clang::RecursiveASTVisitor<FunctionVisitor> {
public:
    FunctionVisitor(clang::DiagnosticsEngine & Diagnostics, bool const InDebugChanges, bool const InDebugUsages)
        : clang::RecursiveASTVisitor<FunctionVisitor>()
        , Candidates()
        , Changed()
        , Reporter(Diagnostics)
        , DebugChanges(InDebugChanges)
        , DebugUsages(InDebugUsages)
    {}

    bool VisitFunctionDecl(clang::FunctionDecl const * F) {
        ConstantAnalysis const & Analysis = ConstantAnalysis::AnalyseThis(*(F->getBody()));
        if (DebugChanges) {
            Analysis.DebugChanged(Reporter);
        }
        if (DebugUsages) {
            Analysis.DebugReferenced(Reporter);
        }
        for (clang::DeclContext::decl_iterator It(F->decls_begin()), End(F->decls_end()); It != End; ++It) {
            if (clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl>(*It)) {
                Decide(*VD, ! Analysis.WasChanged(VD));
            }
        }
        return true;
    }

    void GenerateReports() {
        std::for_each(Candidates.begin(), Candidates.end(),
            std::bind1st(std::ptr_fun(report), Reporter));
    }

private:
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

private:
    Variables Candidates;

    Variables Changed;
    clang::DiagnosticsEngine & Reporter;

    bool const DebugChanges;
    bool const DebugUsages;
};

} // namespace anonymous


VariableChecker::VariableChecker(clang::CompilerInstance const & Compiler, bool const InDebugChanges, bool const InDebugUsages)
    : clang::ASTConsumer()
    , Reporter(Compiler.getDiagnostics())
    , DebugChanges(InDebugChanges)
    , DebugUsages(InDebugUsages)
{ }

void VariableChecker::HandleTranslationUnit(clang::ASTContext & Ctx) {
    FunctionVisitor Collector(Reporter, DebugChanges, DebugUsages);
    Collector.TraverseDecl(Ctx.getTranslationUnitDecl());
    // generate reports only if we don't debug anything
    if ((! DebugChanges) && (! DebugUsages)) {
        Collector.GenerateReports();
    }
}
