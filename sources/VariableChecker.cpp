// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#include "VariableChecker.hpp"
#include "ConstantAnalysis.hpp"
#include "DeclarationCollector.hpp"

#include <algorithm>
#include <functional>
#include <iterator>
#include <cassert>

#include <clang/AST/AST.h>


namespace {

VariablesByContext::value_type analyse(VariablesByContext::value_type const In) {
    ConstantAnalysis Analysis;
    Analysis.TraverseStmt(const_cast<clang::Stmt*>(In.first));
    return VariablesByContext::value_type(In.first, Analysis.getNonConstVariables());
}

bool check(VariablesByContext const & AllCtxs, ContextsByVariable::value_type const It) {
    clang::VarDecl const * const Var = It.first;
    Contexts const & Visibles = It.second;
    for (Contexts::const_iterator It(Visibles.begin()), End(Visibles.end());
        End != It; ++It) {
        VariablesByContext::const_iterator const CtxIt = AllCtxs.find(*It);
        assert(AllCtxs.end() != CtxIt);
        if (CtxIt->second.count(Var)) {
            return true;
        }
    }
    return false;
}

void report(clang::DiagnosticsEngine & DiagEng, ContextsByVariable::value_type const It) {
    char const * const Msg = "variable could be declared as const [Medve plugin]";
    unsigned const DiagID =
        DiagEng.getCustomDiagID(clang::DiagnosticsEngine::Warning, Msg);
    clang::VarDecl const * const Decl = It.first;
    DiagEng.Report(Decl->getLocStart(), DiagID);
}

} // namespace anonymous


VariableChecker::VariableChecker(clang::CompilerInstance const & Compiler)
    : clang::ASTConsumer()
    , DiagEng(Compiler.getDiagnostics())
{ }

void VariableChecker::HandleTranslationUnit(clang::ASTContext & Ctx) {
    DeclarationCollector Collector;
    Collector.TraverseDecl(Ctx.getTranslationUnitDecl());

    VariablesByContext const & Input = Collector.getVariablesByContext();
    VariablesByContext Output;
    std::insert_iterator<VariablesByContext> const OutputIt(Output, Output.begin());
    std::transform(Input.begin(), Input.end(), OutputIt, analyse);
    ContextsByVariable const & Candidates = Collector.getContextsByVariable();
    ContextsByVariable Result;
    std::insert_iterator<ContextsByVariable> const ResultIt(Result, Result.begin());
    std::remove_copy_if(Candidates.begin(), Candidates.end(), ResultIt, std::bind1st(std::ptr_fun(check), Output));
    std::for_each(Result.begin(), Result.end(), std::bind1st(std::ptr_fun(report), DiagEng));
}
