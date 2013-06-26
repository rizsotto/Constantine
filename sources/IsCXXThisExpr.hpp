// This file is distributed under MIT-LICENSE. See COPYING for details.

#pragma once

#include <boost/noncopyable.hpp>

#include <clang/AST/AST.h>
#include <clang/AST/RecursiveASTVisitor.h>

// These are helper struct/method to figure out was it a member
// method call or a call on a variable.
class IsCXXThisExpr
    : public boost::noncopyable
    , public clang::RecursiveASTVisitor<IsCXXThisExpr> {
public:
    static bool Check(clang::Stmt const * const Stmt) {
        IsCXXThisExpr V;
        V.TraverseStmt(const_cast<clang::Stmt*>(Stmt));
        return V.Found;
    }

    // public visitor method.
    bool VisitCXXThisExpr(clang::CXXThisExpr const *) {
        Found = true;
        return true;
    }

private:
    IsCXXThisExpr()
        : boost::noncopyable()
        , clang::RecursiveASTVisitor<IsCXXThisExpr>()
        , Found(false)
    { }

    bool Found;
};
