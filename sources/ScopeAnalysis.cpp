/*  Copyright (C) 2012, 2013  László Nagy
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

#include <clang/AST/RecursiveASTVisitor.h>

#include <boost/range.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/algorithm/for_each.hpp>

namespace {

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
        for (auto It = 0; It < Args; ++It) {
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
            for (auto It = 0; It < Args; ++It) {
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
        for (auto It = 0; It < Args; ++It) {
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

public:
    void Report(clang::DiagnosticsEngine & DE) const {
        char const * const M = "variable '%0' with type '%1' was changed";
        boost::for_each(Results | boost::adaptors::filtered(IsItFromMainModule()),
            std::bind(DumpUsageMapEntry, std::placeholders::_1, M, std::ref(DE)));
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

public:
    void Report(clang::DiagnosticsEngine & DE) const {
        char const * const M = "symbol '%0' was used with type '%1'";
        boost::for_each(Results | boost::adaptors::filtered(IsItFromMainModule()),
            std::bind(DumpUsageMapEntry, std::placeholders::_1, M, std::ref(DE)));
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
    ScopeAnalysis Copy = *this;
    {
        VariableChangeCollector const Visitor(Copy.Changed);
        Visitor.Report(DE);
    }
}

void ScopeAnalysis::DebugReferenced(clang::DiagnosticsEngine & DE) const {
    ScopeAnalysis Copy = *this;
    {
        VariableAccessCollector const Visitor(Copy.Used);
        Visitor.Report(DE);
    }
}
