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

#include "DebugModuleAnalysis.hpp"

#include "../libconstantine_a/IsFromMainModule.hpp"
#include "../libconstantine_a/DeclarationCollector.hpp"
#include "../libconstantine_a/ScopeAnalysis.hpp"

#include <memory>

#include <clang/AST/AST.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/Diagnostic.h>


namespace {

// Report function for debug functionality.
template <unsigned N>
void EmitNoteMessage(clang::DiagnosticsEngine & DE, char const (&Message)[N], clang::DeclaratorDecl const * const V) {
    unsigned const Id = DE.getCustomDiagID(clang::DiagnosticsEngine::Note, Message);
    clang::DiagnosticBuilder const DB = DE.Report(V->getBeginLoc(), Id);
    DB << V->getNameAsString();
    DB.setForceEmit();
}

template <unsigned N>
void EmitNoteMessage(clang::DiagnosticsEngine &DE, const char (&Message)[N], UsageRefsMap::value_type const &Var) {
    if (!IsFromMainModule(Var.first))
        return;

    auto const Id = DE.getCustomDiagID(clang::DiagnosticsEngine::Note, Message);
    auto const & Ls = Var.second;
    for (auto const &L : Ls) {
        auto const DB = DE.Report(std::get<1>(L).getBegin(), Id);
        DB << Var.first->getNameAsString();
        DB << std::get<0>(L).getAsString();
        DB.setForceEmit();
    }
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


class DebugFunctionDeclarations
        : public clang::RecursiveASTVisitor<DebugFunctionDeclarations> {
public:
    explicit DebugFunctionDeclarations(clang::DiagnosticsEngine & DE)
            : clang::RecursiveASTVisitor<DebugFunctionDeclarations>()
            , Diagnostics(DE)
    {}

    bool VisitFunctionDecl(clang::FunctionDecl const * const F) {
        if (F->isThisDeclarationADefinition()) {
            EmitNoteMessage(Diagnostics, "function '%0' declared here", F);
        }
        return true;
    }

private:
    clang::DiagnosticsEngine & Diagnostics;
};


class DebugVariableDeclarations
        : public clang::RecursiveASTVisitor<DebugVariableDeclarations> {
public:
    DebugVariableDeclarations()
            : clang::RecursiveASTVisitor<DebugVariableDeclarations>()
            , Results()
    {}

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
        for (auto && Variable: GetVariablesFromContext(F)) {
            Results.insert(Variable);
        }
    }

    void OnCXXMethodDecl(clang::CXXMethodDecl const * const F) {
        for (auto && Variable: GetVariablesFromContext(F, (!CanThisMethodSignatureChange(F)))) {
            Results.insert(Variable);
        }
        clang::CXXRecordDecl const * const Parent = F->getParent();
        auto const Declaration = Parent->hasDefinition() ? Parent->getDefinition() : Parent->getCanonicalDecl();
        for (auto && Variable: GetVariablesFromRecord(Declaration)) {
            Results.insert(Variable);
        }
    }

    void Dump(clang::DiagnosticsEngine & Diagnostics) const {
        for (auto && Result: Results) {
            EmitNoteMessage(Diagnostics, "variable '%0' declared here", Result);
        }
    }

private:
    Variables Results;
};


class DebugVariableUsages
        : public clang::RecursiveASTVisitor<DebugVariableUsages> {
public:
    explicit DebugVariableUsages(clang::DiagnosticsEngine & DE)
            : clang::RecursiveASTVisitor<DebugVariableUsages>()
            , Diagnostics(DE)
    {}

    bool VisitFunctionDecl(clang::FunctionDecl const * const F) {
        if (F->isThisDeclarationADefinition()) {
            ScopeAnalysis const &Analysis = ScopeAnalysis::AnalyseThis(*(F->getBody()));
            Analysis.ForEachReferenced([&](auto const Entry) {
                EmitNoteMessage(Diagnostics, "symbol '%0' was used with type '%1'", Entry);
            });
        }
        return true;
    }

private:
    clang::DiagnosticsEngine & Diagnostics;
};


class DebugVariableChanges
        : public clang::RecursiveASTVisitor<DebugVariableChanges> {
public:
    explicit DebugVariableChanges(clang::DiagnosticsEngine & DE)
            : clang::RecursiveASTVisitor<DebugVariableChanges>()
            , Diagnostics(DE)
    {}

    bool VisitFunctionDecl(clang::FunctionDecl const * const F) {
        if (F->isThisDeclarationADefinition()) {
            ScopeAnalysis const &Analysis = ScopeAnalysis::AnalyseThis(*(F->getBody()));
            Analysis.ForEachChanged([&](auto const Entry) {
                EmitNoteMessage(Diagnostics, "variable '%0' with type '%1' was changed", Entry);
            });
        }
        return true;
    }

private:
    clang::DiagnosticsEngine & Diagnostics;
};

} // namespace anonymous


DebugModuleAnalysis::DebugModuleAnalysis(clang::CompilerInstance const &Compiler, Target const T)
    : clang::ASTConsumer()
    , Reporter(Compiler.getDiagnostics())
    , State(T)
{ }

void DebugModuleAnalysis::HandleTranslationUnit(clang::ASTContext & Ctx) {
    switch (State) {
        case FunctionDeclaration : {
            std::unique_ptr<DebugFunctionDeclarations> Visitor = std::make_unique<DebugFunctionDeclarations>(Reporter);
            Visitor->TraverseDecl(Ctx.getTranslationUnitDecl());
            break;
        }
        case VariableDeclaration: {
            std::unique_ptr<DebugVariableDeclarations> Visitor = std::make_unique<DebugVariableDeclarations>();
            Visitor->TraverseDecl(Ctx.getTranslationUnitDecl());
            Visitor->Dump(Reporter);
            break;
        }
        case VariableChanges: {
            std::unique_ptr<DebugVariableChanges> Visitor = std::make_unique<DebugVariableChanges>(Reporter);
            Visitor->TraverseDecl(Ctx.getTranslationUnitDecl());
            break;
        }
        case VariableUsages : {
            std::unique_ptr<DebugVariableUsages> Visitor = std::make_unique<DebugVariableUsages>(Reporter);
            Visitor->TraverseDecl(Ctx.getTranslationUnitDecl());
            break;
        }
    }
}
