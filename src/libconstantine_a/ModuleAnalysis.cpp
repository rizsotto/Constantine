/*  Copyright (C) 2012-2014  László Nagy
    This file is part of Constantine.

    Constantine implements pseudo const analysis.

    Constantine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Constantine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ModuleAnalysis.hpp"

#include "DeclarationCollector.hpp"
#include "ScopeAnalysis.hpp"
#include "IsCXXThisExpr.hpp"
#include "IsFromMainModule.hpp"

#include <map>
#include <memory>

#include <clang/AST/AST.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/Diagnostic.h>


namespace {

// Report function for pseudo constness analysis.
template <unsigned N>
void EmitWarningMessage(clang::DiagnosticsEngine & DE, char const (&Message)[N], clang::DeclaratorDecl const * const V) {
    unsigned const Id = DE.getCustomDiagID(clang::DiagnosticsEngine::Warning, Message);
    clang::DiagnosticBuilder const DB = DE.Report(V->getBeginLoc(), Id);
    DB << V->getNameAsString();
    DB.setForceEmit();
}

bool CanThisMethodSignatureChange(clang::CXXMethodDecl const * const F) {
    return
        (F->isUserProvided())
    &&  (! F->isVirtual())
    &&  (! F->isCopyAssignmentOperator())
    &&  (nullptr == clang::dyn_cast<clang::CXXConstructorDecl const>(F))
    &&  (nullptr == clang::dyn_cast<clang::CXXConversionDecl const>(F))
    &&  (nullptr == clang::dyn_cast<clang::CXXDestructorDecl const>(F));
}


// Pseudo constness analysis detects what variable can be declare as const.
// This analysis runs through multiple scopes. We need to store the state of
// the ongoing analysis. Once the variable was changed can't be const.
class PseudoConstnessAnalysisState {
public:
    PseudoConstnessAnalysisState()
        : Candidates()
        , Changed()
    { }

    PseudoConstnessAnalysisState(PseudoConstnessAnalysisState const &) = delete;
    PseudoConstnessAnalysisState & operator=(PseudoConstnessAnalysisState const &) = delete;


    void Eval(ScopeAnalysis const & Analysis, clang::DeclaratorDecl const * const V) {
        if (Analysis.WasChanged(V)) {
            for (auto && Variable: GetReferredVariables(V)) {
                RegisterChange(Variable);
            }
        } else if (Changed.end() == Changed.find(V)) {
            if (! IsConst(*V)) {
                Candidates.insert(V);
            }
        }
    }

    void GenerateReports(clang::DiagnosticsEngine & DE) const {
        for (auto && Variable: Candidates) {
            if (IsFromMainModule(Variable)) {
                EmitWarningMessage(DE, "variable '%0' could be declared as const", Variable);
            }
        }
    }

private:
    static bool IsConst(clang::DeclaratorDecl const & D) {
        return (D.getType().getNonReferenceType().isConstQualified());
    }

    void RegisterChange(clang::DeclaratorDecl const * const V) {
        Candidates.erase(V);
        Changed.insert(V);
    }

private:
    Variables Candidates;
    Variables Changed;
};


class PseudoConstnessAnalysis
    : public clang::RecursiveASTVisitor<PseudoConstnessAnalysis> {
public:
    // Implement function declaration visitor, which visit functions only once.
    // The traversal algorithm is calling all methods, which is not desired.
    // In case of a CXXMethodDecl, it was calling the VisitFunctionDecl and
    // the VisitCXXMethodDecl as well. This dispatching is reworked in this class.
    bool VisitFunctionDecl(clang::FunctionDecl const * const F) {
        if (! (F->isThisDeclarationADefinition()))
            return true;

        if (auto const D = clang::dyn_cast<clang::CXXMethodDecl const>(F)) {
            OnCXXMethodDecl(D);
        } else {
            OnFunctionDecl(F);
        }
        return true;
    }

    void OnFunctionDecl(clang::FunctionDecl const * const F) {
        ScopeAnalysis const & Analysis = ScopeAnalysis::AnalyseThis(*(F->getBody()));
        for (auto && Variable: GetVariablesFromContext(F)) {
            State.Eval(Analysis, Variable);
        }
    }

    void OnCXXMethodDecl(clang::CXXMethodDecl const * const F) {
        clang::CXXRecordDecl const * const Parent = F->getParent();
        clang::CXXRecordDecl const * const RecordDecl =
            Parent->hasDefinition() ? Parent->getDefinition() : Parent->getCanonicalDecl();
        Variables const MemberVariables = GetMemberVariablesAndReferences(RecordDecl, F);
        // check variables first,
        ScopeAnalysis const & Analysis = ScopeAnalysis::AnalyseThis(*(F->getBody()));
        for (auto && Variable: GetVariablesFromContext(F, (!CanThisMethodSignatureChange(F)))) {
            State.Eval(Analysis, Variable);
        }
        for (auto && Variable: MemberVariables) {
            State.Eval(Analysis, Variable);
        }
        // then check the method itself.
        if ((! F->isVirtual()) &&
            (! F->isStatic()) &&
            F->isUserProvided() &&
                CanThisMethodSignatureChange(F)
        ) {
            Methods const MemberFunctions = GetMethodsFromRecord(RecordDecl);
            for (auto && Variable: MemberVariables) {
                if (Analysis.WasChanged(Variable)) {
                    return;
                }
            }
            for (auto && Function: MemberFunctions) {
                if (IsMutatingMethod(Function) && Analysis.WasReferenced(Function)) {
                    return;
                }
            }
            // if it looks const, it might be even static..
            bool NotMutateMember = ! IsCXXThisExpr::Check(F->getBody());
            for (auto && Variable: MemberVariables) {
                if (Analysis.WasReferenced(Variable)) {
                    NotMutateMember = false;
                }
            }
            for (auto && Function : MemberFunctions) {
                if (IsMemberMethod(Function) && Analysis.WasReferenced(Function)) {
                    NotMutateMember = false;
                }
            }
            if (NotMutateMember) {
                StaticCandidates.insert(F);
            } else if (! F->isConst()) {
                ConstCandidates.insert(F);
            }
        }
    }

    void Dump(clang::DiagnosticsEngine & DE) const {
        State.GenerateReports(DE);
        for (auto && Candidate: ConstCandidates) {
            if (IsFromMainModule(Candidate)) {
                EmitWarningMessage(DE, "function '%0' could be declared as const", Candidate);
            }
        }
        for (auto && Candidate: StaticCandidates) {
            if (IsFromMainModule(Candidate)) {
                EmitWarningMessage(DE, "function '%0' could be declared as static", Candidate);
            }
        }
    }

private:
    inline
    static bool IsMutatingMethod(clang::CXXMethodDecl const * const F) {
        return (! F->isStatic()) && (! F->isConst());
    }

    inline
    static bool IsMemberMethod(clang::CXXMethodDecl const * const F) {
        return (! F->isStatic());
    }

private:
    PseudoConstnessAnalysisState State;
    Methods ConstCandidates;
    Methods StaticCandidates;
};

} // namespace anonymous


ModuleAnalysis::ModuleAnalysis(clang::CompilerInstance const &Compiler)
    : clang::ASTConsumer()
    , Reporter(Compiler.getDiagnostics())
{ }

void ModuleAnalysis::HandleTranslationUnit(clang::ASTContext & Ctx) {
    std::unique_ptr<PseudoConstnessAnalysis> Visitor = std::make_unique<PseudoConstnessAnalysis>();
    Visitor->TraverseDecl(Ctx.getTranslationUnitDecl());
    Visitor->Dump(Reporter);
}
