// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#ifndef _variable_checker_hpp_
#define _variable_checker_hpp_

#include <clang/AST/ASTConsumer.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/CompilerInstance.h>

#include <boost/noncopyable.hpp>

enum Target
    { FuncionDeclaration
    , Arguments
    , LocalVariables
    , MemberVariables
    , VariableChanges
    , VariableUsages
    , PseudoConstness
    };

// FIXME: Variable checker is a very wrong name for this class.
// It does run the pseudo const analysis on the given translation unit.
class VariableChecker : public boost::noncopyable, public clang::ASTConsumer {
public:
    VariableChecker(clang::CompilerInstance const &, Target);

    void HandleTranslationUnit(clang::ASTContext &);

private:
    clang::DiagnosticsEngine & Reporter;
    Target const State;
};

#endif // _variable_checker_hpp_
