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

#include "../libconstantine_a/DeclarationCollector.hpp"
#include "../libconstantine_a/ScopeAnalysis.hpp"

#include <functional>
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

bool IsJustAMethod(clang::CXXMethodDecl const * const F) {
    return
        (F->isUserProvided())
    &&  (! F->isVirtual())
    &&  (! F->isCopyAssignmentOperator())
    &&  (nullptr == clang::dyn_cast<clang::CXXConstructorDecl const>(F))
    &&  (nullptr == clang::dyn_cast<clang::CXXConversionDecl const>(F))
    &&  (nullptr == clang::dyn_cast<clang::CXXDestructorDecl const>(F));
}


// Base class for analysis. Implement function declaration visitor, which visit
// functions only once. The traversal algorithm is calling all methods, which is
// not desired. In case of a CXXMethodDecl, it was calling the VisitFunctionDecl
// and the VisitCXXMethodDecl as well. This dispatching is reworked in this class.
class ModuleVisitor
    : public clang::RecursiveASTVisitor<ModuleVisitor> {
public:
    typedef std::unique_ptr<ModuleVisitor> Ptr;

    static ModuleVisitor::Ptr CreateVisitor(Target);

    virtual ~ModuleVisitor() = default;

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
    void OnFunctionDecl(clang::FunctionDecl const * const F) override {
        Functions.insert(F);
    }

    void OnCXXMethodDecl(clang::CXXMethodDecl const * const F) override {
        Functions.insert(F);
    }

    void Dump(clang::DiagnosticsEngine & DE) const override {
        for (auto && Function: Functions) {
            EmitNoteMessage(DE, "function '%0' declared here", Function);
        }
    }

protected:
    std::set<clang::FunctionDecl const *> Functions;
};


class DebugVariableDeclarations
    : public ModuleVisitor {
private:
    void OnFunctionDecl(clang::FunctionDecl const * const F) override {
        for (auto && Variable: GetVariablesFromContext(F)) {
            Results.insert(Variable);
        }
    }

    void OnCXXMethodDecl(clang::CXXMethodDecl const * const F) override {
        for (auto && Variable: GetVariablesFromContext(F, (! IsJustAMethod(F)))) {
            Results.insert(Variable);
        }
        clang::CXXRecordDecl const * const Parent = F->getParent();
        auto const Declaration = Parent->hasDefinition() ? Parent->getDefinition() : Parent->getCanonicalDecl();
        for (auto && Variable: GetVariablesFromRecord(Declaration)) {
            Results.insert(Variable);
        }
    }

    void Dump(clang::DiagnosticsEngine & DE) const override {
        for (auto && Result: Results) {
            EmitNoteMessage(DE, "variable '%0' declared here", Result);
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

    void Dump(clang::DiagnosticsEngine & DE) const override {
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

    void Dump(clang::DiagnosticsEngine & DE) const override {
        for (auto && Function: Functions) {
            ReportVariableUsage(DE, Function);
        }
    }
};


ModuleVisitor::Ptr ModuleVisitor::CreateVisitor(Target const State) {
    switch (State) {
    case FunctionDeclaration :
        return ModuleVisitor::Ptr( new DebugFunctionDeclarations() );
    case VariableDeclaration :
        return ModuleVisitor::Ptr( new DebugVariableDeclarations() );
    case VariableChanges:
        return ModuleVisitor::Ptr( new DebugVariableChanges() );
    case VariableUsages :
        return ModuleVisitor::Ptr( new DebugVariableUsages() );
    }
}

} // namespace anonymous


DebugModuleAnalysis::DebugModuleAnalysis(clang::CompilerInstance const &Compiler, Target const T)
    : clang::ASTConsumer()
    , Reporter(Compiler.getDiagnostics())
    , State(T)
{ }

void DebugModuleAnalysis::HandleTranslationUnit(clang::ASTContext & Ctx) {
    ModuleVisitor::Ptr const V = ModuleVisitor::CreateVisitor(State);
    V->TraverseDecl(Ctx.getTranslationUnitDecl());
    V->Dump(Reporter);
}
