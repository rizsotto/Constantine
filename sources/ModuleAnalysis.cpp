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

#include <functional>
#include <iterator>
#include <map>
#include <memory>

#include <clang/AST/AST.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/Diagnostic.h>


namespace {

// Report function for pseudo constness analysis.
template <unsigned N>
void EmitWarningMessage(clang::DiagnosticsEngine & DE, char const (&Message)[N], clang::DeclaratorDecl const * const V) {
    unsigned const Id =
        DE.getCustomDiagID(clang::DiagnosticsEngine::Warning, Message);
    clang::DiagnosticBuilder const DB = DE.Report(V->getLocStart(), Id);
    DB << V->getNameAsString();
    DB.setForceEmit();
}

void ReportVariablePseudoConstness(clang::DiagnosticsEngine & DE, clang::DeclaratorDecl const * const V) {
    EmitWarningMessage(DE, "variable '%0' could be declared as const", V);
}

void ReportFunctionPseudoConstness(clang::DiagnosticsEngine & DE, clang::DeclaratorDecl const * const V) {
    EmitWarningMessage(DE, "function '%0' could be declared as const", V);
}

void ReportFunctionPseudoStaticness(clang::DiagnosticsEngine & DE, clang::DeclaratorDecl const * const V) {
    EmitWarningMessage(DE, "function '%0' could be declared as static", V);
}

// Report function for debug functionality.
template <unsigned N>
void EmitNoteMessage(clang::DiagnosticsEngine & DE, char const (&Message)[N], clang::DeclaratorDecl const * const V) {
    unsigned const Id = DE.getCustomDiagID(clang::DiagnosticsEngine::Note, Message);
    clang::DiagnosticBuilder const DB = DE.Report(V->getLocStart(), Id);
    DB << V->getNameAsString();
    DB.setForceEmit();
}

void ReportVariableDeclaration(clang::DiagnosticsEngine & DE, clang::DeclaratorDecl const * const V) {
    EmitNoteMessage(DE, "variable '%0' declared here", V);
}

void ReportFunctionDeclaration(clang::DiagnosticsEngine & DE, clang::DeclaratorDecl const * const V) {
    EmitNoteMessage(DE, "function '%0' declared here", V);
}


bool IsJustAMethod(clang::CXXMethodDecl const * const F) {
    return
        (F->isUserProvided())
    &&  (! F->isVirtual())
    &&  (! F->isCopyAssignmentOperator())
    &&  (0 == clang::dyn_cast<clang::CXXConstructorDecl const>(F))
    &&  (0 == clang::dyn_cast<clang::CXXConversionDecl const>(F))
    &&  (0 == clang::dyn_cast<clang::CXXDestructorDecl const>(F));
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
            for (auto && Variable: GetReferedVariables(V)) {
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
                ReportVariablePseudoConstness(DE, Variable);
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


// Base class for analysis. Implement function declaration visitor, which visit
// functions only once. The traversal algorithm is calling all methods, which is
// not desired. In case of a CXXMethodDecl, it was calling the VisitFunctionDecl
// and the VisitCXXMethodDecl as well. This dispatching is reworked in this class.
class ModuleVisitor
    : public clang::RecursiveASTVisitor<ModuleVisitor> {
public:
    typedef std::unique_ptr<ModuleVisitor> Ptr;
    static ModuleVisitor::Ptr CreateVisitor(Target);

    virtual ~ModuleVisitor()
    { }

public:
    // public visitor method.
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

public:
    // interface methods with different visibilities.
    virtual void Dump(clang::DiagnosticsEngine &) const = 0;

protected:
    virtual void OnFunctionDecl(clang::FunctionDecl const *) = 0;
    virtual void OnCXXMethodDecl(clang::CXXMethodDecl const *) = 0;
};


class DebugFunctionDeclarations
    : public ModuleVisitor {
protected:
    void OnFunctionDecl(clang::FunctionDecl const * const F) {
        Functions.insert(F);
    }

    void OnCXXMethodDecl(clang::CXXMethodDecl const * const F) {
        Functions.insert(F);
    }

    void Dump(clang::DiagnosticsEngine & DE) const {
        for (auto && Function: Functions) {
            ReportFunctionDeclaration(DE, Function);
        }
    }

protected:
    std::set<clang::FunctionDecl const *> Functions;
};


class DebugVariableDeclarations
    : public ModuleVisitor {
private:
    void OnFunctionDecl(clang::FunctionDecl const * const F) {
        for (auto && Variable: GetVariablesFromContext(F)) {
            Results.insert(Variable);
        }
    }

    void OnCXXMethodDecl(clang::CXXMethodDecl const * const F) {
        for (auto && Variable: GetVariablesFromContext(F, (! IsJustAMethod(F)))) {
            Results.insert(Variable);
        }
        clang::CXXRecordDecl const * const Parent = F->getParent();
        auto const Declaration = Parent->hasDefinition() ? Parent->getDefinition() : Parent->getCanonicalDecl();
        for (auto && Variable: GetVariablesFromRecord(Declaration)) {
            Results.insert(Variable);
        }
    }

    void Dump(clang::DiagnosticsEngine & DE) const {
        for (auto && Result: Results) {
            ReportVariableDeclaration(DE, Result);
        }
    }

private:
    Variables Results;
};


class DebugVariableUsages
    : public DebugFunctionDeclarations {
private:
    static void ReportVariableUsage(clang::DiagnosticsEngine & DE, clang::FunctionDecl const * const F) {
        ScopeAnalysis const & Analysis = ScopeAnalysis::AnalyseThis(*(F->getBody()));
        Analysis.DebugReferenced(DE);
    }

    void Dump(clang::DiagnosticsEngine & DE) const {
        for (auto && Function: Functions) {
            ReportVariableUsage(DE, Function);
        }
    }
};


class DebugVariableChanges
    : public DebugFunctionDeclarations {
private:
    static void ReportVariableUsage(clang::DiagnosticsEngine & DE, clang::FunctionDecl const * const F) {
        ScopeAnalysis const & Analysis = ScopeAnalysis::AnalyseThis(*(F->getBody()));
        Analysis.DebugChanged(DE);
    }

    void Dump(clang::DiagnosticsEngine & DE) const {
        for (auto && Function: Functions) {
            ReportVariableUsage(DE, Function);
        }
    }
};


class AnalyseVariableUsage
    : public ModuleVisitor {
private:
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
        for (auto && Variable: GetVariablesFromContext(F, (! IsJustAMethod(F)))) {
            State.Eval(Analysis, Variable);
        }
        for (auto && Variable: MemberVariables) {
            State.Eval(Analysis, Variable);
        }
        // then check the method itself.
        if ((! F->isVirtual()) &&
            (! F->isStatic()) &&
            F->isUserProvided() &&
            IsJustAMethod(F)
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
                ReportFunctionPseudoConstness(DE, Candidate);
            }
        }
        for (auto && Candidate: StaticCandidates) {
            if (IsFromMainModule(Candidate)) {
                ReportFunctionPseudoStaticness(DE, Candidate);
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


ModuleAnalysis::ModuleAnalysis(clang::CompilerInstance const & Compiler, Target const T)
    : clang::ASTConsumer()
    , Reporter(Compiler.getDiagnostics())
    , State(T)
{ }

void ModuleAnalysis::HandleTranslationUnit(clang::ASTContext & Ctx) {
    ModuleVisitor::Ptr const V = ModuleVisitor::CreateVisitor(State);
    V->TraverseDecl(Ctx.getTranslationUnitDecl());
    V->Dump(Reporter);
}
