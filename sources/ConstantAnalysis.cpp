// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#include "ConstantAnalysis.hpp"

#include <algorithm>
#include <functional>
#include <iterator>

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

} // namespace anonymous


ConstantAnalysis::ConstantAnalysis()
: clang::RecursiveASTVisitor<ConstantAnalysis>()
, NonConstants()
{ }

bool ConstantAnalysis::VisitBinaryOperator(clang::BinaryOperator const * const BO) {
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
            NonConstants.insert(VD);
        }
        break;
    default:
        break;
    }
    return true;
}

bool ConstantAnalysis::VisitUnaryOperator(clang::UnaryOperator const * const UO) {
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
            NonConstants.insert(VD);
        }
        break;
    default:
        break;
    }
    return true;
}

bool ConstantAnalysis::VisitDeclStmt(clang::DeclStmt const * const DS) {
    for (clang::DeclStmt::const_decl_iterator I = DS->decl_begin(),
         E = DS->decl_end(); I != E; ++I) {
        checkRefDeclaration(*I);
    }
    return true;
}

bool ConstantAnalysis::VisitCallExpr(clang::CallExpr const * const CE) {
    for (clang::CallExpr::const_arg_iterator AIt(CE->arg_begin()), AEnd(CE->arg_end()); AIt != AEnd; ++AIt ) {
        insertWhenReferedWithoutCast(*AIt);
    }
    return true;
}

bool ConstantAnalysis::VisitMemberExpr(clang::MemberExpr const * const CE) {
    clang::Type const * const T = CE->getMemberDecl()->getType().getCanonicalType().getTypePtr();
    if (clang::FunctionProtoType const * const F = T->getAs<clang::FunctionProtoType>()) {
        if (! (F->getTypeQuals() & clang::Qualifiers::Const) ) {
            insertWhenReferedWithoutCast(CE);
        }
    }
    return true;
}

Variables const & ConstantAnalysis::getNonConstVariables() const {
    return NonConstants;
}

void ConstantAnalysis::checkRefDeclaration(clang::Decl const * const Decl) {
    if (clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl const>(Decl)) {
        if (is_reference(VD) && is_non_const(VD)) {
            insertWhenReferedWithoutCast(VD->getInit()->IgnoreParenCasts());
        }
    }
}

void ConstantAnalysis::insertWhenReferedWithoutCast(clang::Expr const * const E) {
    if (clang::Decl const * const D = getDecl(E)) {
        if (clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl>(D)) {
            NonConstants.insert(VD);
        }
    }
}
