// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#include "ConstantAnalysis.hpp"

#include <algorithm>
#include <functional>
#include <iterator>

#include <clang/AST/RecursiveASTVisitor.h>

namespace {

bool is_non_const(clang::VarDecl const * const D) {
    return (! (D->getType().getNonReferenceType().isConstQualified()));
}

bool is_reference(clang::VarDecl const * const D) {
    return (D->getType().getTypePtr()->isReferenceType());
}

// FIXME: extract clang::Decl from an clang::Expr, but only if
// it is variable access or member access. Don't we have a better
// name for it? And shall it be that specific, although the name
// is so generic?
clang::Decl const * getDecl(clang::Expr const * const E) {
    if (clang::DeclRefExpr const * const DR = clang::dyn_cast<clang::DeclRefExpr const>(E)) {
        return DR->getDecl();
    } else if (clang::MemberExpr const * const ME = clang::dyn_cast<clang::MemberExpr>(E)) {
        return getDecl(ME->getBase());
    }
    return 0;
}


class VerboseVariableCollector {
protected:
    typedef std::set<clang::VarDecl const *> Variables;

public:
    VerboseVariableCollector(Variables & Out)
        : Results(Out)
    { }

protected:
    inline
    void AddToResults(clang::Decl const * const D) {
        if (clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl>(D)) {
            Results.insert(VD);
        }
    }

private:
    Variables & Results;
};

// Collect all variables which were mutated in the given scope.
// (The scope is given by the TraverseStmt method.)
class VariableChangeCollector
    : public VerboseVariableCollector
    , public clang::RecursiveASTVisitor<VariableChangeCollector> {
public:
    VariableChangeCollector(Variables & Out)
        : VerboseVariableCollector(Out)
        , clang::RecursiveASTVisitor<VariableChangeCollector>()
    { }

    // Assignments are mutating variables.
    bool VisitBinaryOperator(clang::BinaryOperator const * const BO) {
        clang::Decl const * const LHSDecl =
            getDecl(BO->getLHS()->IgnoreParenCasts());
        if (!LHSDecl)
            return true;

        switch (BO->getOpcode()) {
        case clang::BO_Assign: {
            clang::Decl const * const RHSDecl =
                getDecl(BO->getRHS()->IgnoreParenCasts());
            if (LHSDecl == RHSDecl) {
                break;
            }
        }
        case clang::BO_AddAssign:
        case clang::BO_SubAssign:
        case clang::BO_MulAssign:
        case clang::BO_DivAssign:
        case clang::BO_AndAssign:
        case clang::BO_OrAssign:
        case clang::BO_XorAssign:
        case clang::BO_ShlAssign:
        case clang::BO_ShrAssign:
            AddToResults(LHSDecl);
            break;
        default:
            break;
        }
        return true;
    }

    // Some operator does mutate variables.
    bool VisitUnaryOperator(clang::UnaryOperator const * const UO) {
        clang::Decl const * const D =
            getDecl(UO->getSubExpr()->IgnoreParenCasts());
        if (!D)
            return true;

        switch (UO->getOpcode()) {
        case clang::UO_PostDec:
        case clang::UO_PostInc:
        case clang::UO_PreDec:
        case clang::UO_PreInc:
        // FIXME: Address-Of ruin the whole pointer business...
        case clang::UO_AddrOf:
            AddToResults(D);
            break;
        default:
            break;
        }
        return true;
    }

    // FIXME: This is the case when a new variable is declared and
    // the current might be marked as non-const if the new variable
    // also not const. This should be represented as dependency graph
    // and not neccessary would mean change on the variable.
    bool VisitDeclStmt(clang::DeclStmt const * const DS) {
        clang::DeclGroupRef const & DG = DS->getDeclGroup();
        for (clang::DeclGroupRef::const_iterator It(DG.begin()), End(DG.end()); It != End; ++It) {
            checkRefDeclaration(*It);
        }
        return true;
    }

    // Variables potentially mutated when you pass by-pointer or by-reference.
    bool VisitCallExpr(clang::CallExpr const * const CE) {
        for (clang::CallExpr::const_arg_iterator AIt(CE->arg_begin()), AEnd(CE->arg_end()); AIt != AEnd; ++AIt ) {
            insertWhenReferedWithoutCast(*AIt);
        }
        return true;
    }

    // Variables are mutated if non-const member function called.
    bool VisitMemberExpr(clang::MemberExpr const * const CE) {
        clang::Type const * const T = CE->getMemberDecl()->getType().getCanonicalType().getTypePtr();
        if (clang::FunctionProtoType const * const F = T->getAs<clang::FunctionProtoType>()) {
            if (! (F->getTypeQuals() & clang::Qualifiers::Const) ) {
                insertWhenReferedWithoutCast(CE);
            }
        }
        return true;
    }

private:
    inline
    void checkRefDeclaration(clang::Decl const * const Decl) {
        if (clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl const>(Decl)) {
            if (is_reference(VD) && is_non_const(VD)) {
                insertWhenReferedWithoutCast(VD->getInit()->IgnoreParenCasts());
            }
        }
    }

    // FIXME: explain it with clang AST examples.
    inline
    void insertWhenReferedWithoutCast(clang::Expr const * const E) {
        if (clang::Decl const * const D = getDecl(E)) {
            AddToResults(D);
        }
    }
};

// Collect all variables which were accessed in the given scope.
// (The scope is given by the TraverseStmt method.)
class VariableAccessCollector
    : public VerboseVariableCollector
    , public clang::RecursiveASTVisitor<VariableAccessCollector> {
public:
    VariableAccessCollector(Variables & Out)
        : VerboseVariableCollector(Out)
        , clang::RecursiveASTVisitor<VariableAccessCollector>()
    { }

    // Variable access is a usage of the variable.
    bool VisitDeclRefExpr(clang::DeclRefExpr const * const DRE) {
        AddToResults(DRE->getDecl());
        return true;
    }

    // Member access is a usage of the class.
    bool VisitMemberExpr(clang::MemberExpr const * const ME) {
        AddToResults(ME->getMemberDecl());
        return true;
    }
};

} // namespace anonymous

ConstantAnalysis ConstantAnalysis::AnalyseThis(clang::Stmt const & Stmt) {
    ConstantAnalysis Result;
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

bool ConstantAnalysis::WasChanged(clang::VarDecl const * const Decl) const {
    return (Changed.end() != Changed.find(Decl));
}

bool ConstantAnalysis::WasReferenced(clang::VarDecl const * const Decl) const {
    return (Used.end() != Used.find(Decl));
}

