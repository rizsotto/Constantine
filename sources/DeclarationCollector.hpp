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

#include <set>

#include <clang/AST/AST.h>

typedef std::set<clang::DeclaratorDecl const *> Variables;
typedef std::set<clang::CXXMethodDecl const *> Methods;

// method to copy variables out from declaration context
Variables GetVariablesFromContext(clang::DeclContext const * const F, bool const WithoutArgs = false);

// method to copy variables out from class declaration
Variables GetVariablesFromRecord(clang::CXXRecordDecl const * const Rec);

// method to copy methods out from class declaration 
Methods GetMethodsFromRecord(clang::CXXRecordDecl const * const Rec);


// method to get refered declarations from the given declaration
Variables GetReferedVariables(clang::DeclaratorDecl const *);

// method to get all member variables and all refered declarations
Variables GetMemberVariablesAndReferences(clang::CXXRecordDecl const * const Rec, clang::DeclContext const * const F);
