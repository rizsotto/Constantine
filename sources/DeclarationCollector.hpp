// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#ifndef _DeclarationCollector_hpp_
#define _DeclarationCollector_hpp_

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

#endif // _DeclarationCollector_hpp_
