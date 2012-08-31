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

clang::Decl const * getDecl(clang::Expr const * const E) {
    if (clang::DeclRefExpr const * const DR = clang::dyn_cast<clang::DeclRefExpr const>(E)) {
        return DR->getDecl();
    } else if (clang::MemberExpr const * const ME = clang::dyn_cast<clang::MemberExpr>(E)) {
        return getDecl(ME->getBase());
    }
    return 0;
}

typedef std::set<clang::VarDecl const *> Variables;

class StatementVisitor : public clang::RecursiveASTVisitor<StatementVisitor> {
public:
    StatementVisitor(Variables & In)
    : clang::RecursiveASTVisitor<StatementVisitor>()
    , Changed(In)
    { }

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
            if (clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl>(LHSDecl)) {
                Changed.insert(VD);
            }
            break;
        default:
            break;
        }
        return true;
    }

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
        case clang::UO_AddrOf:
            if (clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl>(D)) {
                Changed.insert(VD);
            }
            break;
        default:
            break;
        }
        return true;
    }

    bool VisitDeclStmt(clang::DeclStmt const * const DS) {
        for (clang::DeclStmt::const_decl_iterator I = DS->decl_begin(),
             E = DS->decl_end(); I != E; ++I) {
            checkRefDeclaration(*I);
        }
        return true;
    }

    bool VisitCallExpr(clang::CallExpr const * const CE) {
        for (clang::CallExpr::const_arg_iterator AIt(CE->arg_begin()), AEnd(CE->arg_end()); AIt != AEnd; ++AIt ) {
            insertWhenReferedWithoutCast(*AIt);
        }
        return true;
    }

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
    void checkRefDeclaration(clang::Decl const * const Decl) {
        if (clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl const>(Decl)) {
            if (is_reference(VD) && is_non_const(VD)) {
                insertWhenReferedWithoutCast(VD->getInit()->IgnoreParenCasts());
            }
        }
    }

    void insertWhenReferedWithoutCast(clang::Expr const * const E) {
        if (clang::Decl const * const D = getDecl(E)) {
            if (clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl>(D)) {
                Changed.insert(VD);
            }
        }
    }

private:
    Variables & Changed;

private:
    StatementVisitor(StatementVisitor const &);
    StatementVisitor & operator=(StatementVisitor const &);
};

} // namespace anonymous

ConstantAnalysis ConstantAnalysis::AnalyseThis(clang::Stmt const & Stmt) {
    ConstantAnalysis Result;

    StatementVisitor Visitor(Result.Changed);
    Visitor.TraverseStmt(const_cast<clang::Stmt*>(&Stmt));

    return Result;
}

bool ConstantAnalysis::WasChanged(clang::VarDecl const * const Decl) const {
    return (Changed.end() != Changed.find(Decl));
}

bool ConstantAnalysis::WasReferenced(clang::VarDecl const * const Decl) const {
    return (Used.end() != Used.find(Decl));
}

