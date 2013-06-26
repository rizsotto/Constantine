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
