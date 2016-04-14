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

#include "ScopeAnalysis.hpp"
#include "IsCXXThisExpr.hpp"
#include "IsFromMainModule.hpp"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/Diagnostic.h>

#include <functional>


namespace {

static clang::QualType const NoType = clang::QualType();
static clang::SourceRange const NoRange = clang::SourceRange();

// Usage extract method implemented in visitor style.
class UsageExtractor
    : public clang::RecursiveASTVisitor<UsageExtractor> {
public:
    UsageExtractor(UsageRefsMap & Out, clang::QualType const & InType)
        : clang::RecursiveASTVisitor<UsageExtractor>()
        , Results(Out)
        , State(InType, NoRange)
    { }

    UsageExtractor(UsageExtractor const &) = delete;
    UsageExtractor & operator=(UsageExtractor const &) = delete;

private:
    void Capture(clang::Expr const * const E) {
        // do nothing if it already has type info
        if (std::get<0>(State) != NoType)
            return;

        State = std::make_tuple(E->getType(), E->getSourceRange());
    }

    void RegisterUsage(clang::ValueDecl const * const Decl,
                       clang::SourceRange const & Location) {
        std::get<1>(State) = Location;
        if (auto const D = clang::dyn_cast<clang::DeclaratorDecl const>(Decl->getCanonicalDecl())) {
            auto It = Results.find(D);
            if (Results.end() == It) {
                auto const R = Results.insert(UsageRefsMap::value_type(D, UsageRefs()));
                It = R.first;
            }
            UsageRefs & Ls = It->second;
            Ls.push_back(State);
        }
        // reset the state for the next call
        State = std::make_tuple(NoType, NoRange);
    }

public:
    // public visitor method.
    bool VisitCastExpr(clang::CastExpr const * const E) {
        Capture(E);
        return true;
    }

    bool VisitUnaryOperator(clang::UnaryOperator const * const E) {
        switch (E->getOpcode()) {
        case clang::UO_AddrOf:
        case clang::UO_Deref:
            Capture(E);
        default:
            ;
        }
        return true;
    }

    bool VisitDeclRefExpr(clang::DeclRefExpr const * const E) {
        Capture(E);
        RegisterUsage(E->getDecl(), E->getSourceRange());
        return true;
    }

    bool VisitMemberExpr(clang::MemberExpr const * const E) {
        Capture(E);
        RegisterUsage(E->getMemberDecl(), E->getSourceRange());
        return true;
    }

private:
    UsageRefsMap & Results;
    UsageRef State;
};

void Register(UsageRefsMap & Results,
              clang::Expr const * E,
              clang::QualType const & Type = clang::QualType()) {
    clang::Stmt const * const Stmt = E;

    UsageExtractor Visitor(Results, Type);
    Visitor.TraverseStmt(const_cast<clang::Stmt*>(Stmt));
}

template <unsigned N>
void DumpUsageMapEntry(UsageRefsMap::value_type const & Var
           , char const (&Message)[N]
           , clang::DiagnosticsEngine & DE) {
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


// Collect all variables which were mutated in the given scope.
// (The scope is given by the TraverseStmt method.)
class VariableChangeCollector
    : public clang::RecursiveASTVisitor<VariableChangeCollector> {
public:
    VariableChangeCollector(UsageRefsMap & Out)
        : clang::RecursiveASTVisitor<VariableChangeCollector>()
        , Results(Out)
    { }

public:
    // Assignments are mutating variables.
    bool VisitBinaryOperator(clang::BinaryOperator const * const Stmt) {
        if (Stmt->isAssignmentOp()) {
            Register(Results, Stmt->getLHS());
        }
        return true;
    }

    // Inc/Dec-rement operator does mutate variables.
    bool VisitUnaryOperator(clang::UnaryOperator const * const Stmt) {
        if (Stmt->isIncrementDecrementOp()) {
            Register(Results, Stmt->getSubExpr());
        }
        return true;
    }

    // Arguments potentially mutated when you pass by-pointer or by-reference.
    bool VisitCXXConstructExpr(clang::CXXConstructExpr const * const Stmt) {
        auto const F = Stmt->getConstructor();
        // check the function parameters one by one
        auto const Args = std::min(Stmt->getNumArgs(), F->getNumParams());
        for (auto It = 0u; It < Args; ++It) {
            auto const P = F->getParamDecl(It);
            if (IsNonConstReferenced(P->getType())) {
                Register(Results, Stmt->getArg(It), (*(P->getType())).getPointeeType());
            }
        }
        return true;
    }

    // Arguments potentially mutated when you pass by-pointer or by-reference.
    bool VisitCallExpr(clang::CallExpr const * const Stmt) {
        // some function call is a member call and has 'this' as a first
        // argument, which is not checked here.
        auto const Offset = HasThisAsFirstArgument(Stmt) ? 1 : 0;

        if (auto const F = Stmt->getDirectCallee()) {
            // check the function parameters one by one
            auto const Args = std::min(Stmt->getNumArgs(), F->getNumParams());
            for (auto It = 0u; It < Args; ++It) {
                auto const P = F->getParamDecl(It);
                if (IsNonConstReferenced(P->getType())) {
                    assert(It + Offset <= Stmt->getNumArgs());
                    Register(Results, Stmt->getArg(It + Offset),
                                 (*(P->getType())).getPointeeType());
                }
            }
        }
        return true;
    }

    // Objects are mutated when non const member call happen.
    bool VisitCXXMemberCallExpr(clang::CXXMemberCallExpr const * const Stmt) {
        if (auto const MD = Stmt->getMethodDecl()) {
            if ((! MD->isConst()) && (! MD->isStatic())) {
                Register(Results, Stmt->getImplicitObjectArgument());
            }
        }
        return true;
    }

    // Objects are mutated when non const operator called.
    bool VisitCXXOperatorCallExpr(clang::CXXOperatorCallExpr const * const Stmt) {
        // the implimentation relies on that here the first argument
        // is the 'this', while it was not the case with CXXMethodDecl.
        if (auto const F = Stmt->getDirectCallee()) {
            if (auto const MD = clang::dyn_cast<clang::CXXMethodDecl const>(F)) {
                if ((! MD->isConst()) && (! MD->isStatic()) && (0 < Stmt->getNumArgs())) {
                    Register(Results, Stmt->getArg(0));
                }
            }
        }
        return true;
    }

    // Placement new change change the pre allocated memory.
    bool VisitCXXNewExpr(clang::CXXNewExpr const * const Stmt) {
        auto const Args = Stmt->getNumPlacementArgs();
        for (auto It = 0u; It < Args; ++It) {
            // FIXME: not all placement argument are mutating.
            Register(Results, Stmt->getPlacementArg(It));
        }
        return true;
    }

private:
    static bool IsNonConstReferenced(clang::QualType const & Decl) {
        return
            ((*Decl).isReferenceType() || (*Decl).isPointerType())
            && (! (*Decl).getPointeeType().isConstQualified());
    }

    static bool HasThisAsFirstArgument(clang::CallExpr const * const Stmt) {
        return
            (clang::dyn_cast<clang::CXXOperatorCallExpr const>(Stmt)) &&
            (Stmt->getDirectCallee()) &&
            (clang::dyn_cast<clang::CXXMethodDecl const>(Stmt->getDirectCallee()));
    }

private:
    UsageRefsMap & Results;
};

// Collect all variables which were accessed in the given scope.
// (The scope is given by the TraverseStmt method.)
class VariableAccessCollector
    : public clang::RecursiveASTVisitor<VariableAccessCollector> {
public:
    VariableAccessCollector(UsageRefsMap & Out)
        : clang::RecursiveASTVisitor<VariableAccessCollector>()
        , Results(Out)
    { }

public:
    bool VisitDeclRefExpr(clang::DeclRefExpr const * const Stmt) {
        Register(Results, Stmt);
        return true;
    }

    bool VisitMemberExpr(clang::MemberExpr * const Stmt) {
        if (IsCXXThisExpr::Check(Stmt)) {
            Register(Results, Stmt);
        }
        return true;
    }

private:
    UsageRefsMap & Results;
};

} // namespace anonymous

ScopeAnalysis ScopeAnalysis::AnalyseThis(clang::Stmt const & Stmt) {
    ScopeAnalysis Result;
    {
        VariableChangeCollector Visitor(Result.Changed);
        Visitor.TraverseStmt(const_cast<clang::Stmt*>(&Stmt));
    }
    {
        VariableAccessCollector Visitor(Result.Used);
        Visitor.TraverseStmt(const_cast<clang::Stmt*>(&Stmt));
    }
    return Result;
}

bool ScopeAnalysis::WasChanged(clang::DeclaratorDecl const * const Decl) const {
    return (Changed.end() != Changed.find(Decl));
}

bool ScopeAnalysis::WasReferenced(clang::DeclaratorDecl const * const Decl) const {
    return (Used.end() != Used.find(Decl));
}

void ScopeAnalysis::DebugChanged(clang::DiagnosticsEngine & DE) const {
    for (auto const Entry : Changed) {
        DumpUsageMapEntry(Entry, "variable '%0' with type '%1' was changed", DE);
    }
}

void ScopeAnalysis::DebugReferenced(clang::DiagnosticsEngine & DE) const {
    for (auto const Entry : Used) {
        DumpUsageMapEntry(Entry, "symbol '%0' was used with type '%1'", DE);
    }
}
