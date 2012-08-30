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

typedef std::map<clang::Stmt const *, ConstantAnalysis::ConstPtr> AnalyzersByContext;

inline
AnalyzersByContext::value_type analyse(VariablesByContext::value_type const & In) {
    return AnalyzersByContext::value_type(In.first, ConstantAnalysis::AnalyseThis(*In.first));
}

bool pseudo_constness_check(ContextsByVariable::value_type const & It, AnalyzersByContext const & Analyzers) {
    clang::VarDecl const * const Var = It.first;
    Contexts const & Visibles = It.second;
    // go through all scopes where Var is visible
    for (Contexts::const_iterator CtxIt(Visibles.begin()), CtxEnd(Visibles.end());
        CtxEnd != CtxIt; ++CtxIt) {
        // check the scope related analysis
        AnalyzersByContext::const_iterator const AnalyzerIt = Analyzers.find(*CtxIt);
        assert(Analyzers.end() != AnalyzerIt);
        ConstantAnalysis const & Analyzer = *(AnalyzerIt->second);
        // pseudo const only iff it was not changed any of the scopes
        if (Analyzer.WasChanged(Var)) {
            return false;
        }
    }
    return true;
}

void report(clang::DiagnosticsEngine & DiagEng, clang::VarDecl const * const Decl) {
    std::string Msg;
    Msg += "variable '";
    Msg += Decl->getNameAsString();
    Msg += "' could be declared as const [Medve plugin]";

    unsigned const DiagID = DiagEng.getCustomDiagID(clang::DiagnosticsEngine::Warning, Msg.c_str());
    DiagEng.Report(Decl->getLocStart(), DiagID);
}

} // namespace anonymous


VariableChecker::VariableChecker(clang::CompilerInstance const & Compiler)
    : clang::ASTConsumer()
    , DiagEng(Compiler.getDiagnostics())
{ }

void VariableChecker::HandleTranslationUnit(clang::ASTContext & Ctx) {
    // Collect declarations and their scopes
    DeclarationCollector Collector;
    Collector.TraverseDecl(Ctx.getTranslationUnitDecl());
    // Run analysis on all scopes
    AnalyzersByContext Analyzers;
    {
        VariablesByContext const & Input = Collector.getVariablesByContext();
        std::insert_iterator<AnalyzersByContext> const AnalyzersIt(Analyzers, Analyzers.begin());
        std::transform(Input.begin(), Input.end(), AnalyzersIt, analyse);
    }
    // Get variables where analyses were diagnose pseudo constness
    Variables Result;
    {
        ContextsByVariable const & Candidates = Collector.getContextsByVariable();
        for (ContextsByVariable::const_iterator It(Candidates.begin()), End(Candidates.end()); It != End; ++It) {
            if (pseudo_constness_check(*It, Analyzers)) {
                Result.insert(It->first);
            }
        }
    }
    // Print out the results
    std::for_each(Result.begin(), Result.end(), std::bind1st(std::ptr_fun(report), DiagEng));
}
