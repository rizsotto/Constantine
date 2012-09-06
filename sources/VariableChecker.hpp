// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#ifndef _variable_checker_hpp_
#define _variable_checker_hpp_

#include <clang/AST/ASTConsumer.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/CompilerInstance.h>


class VariableChecker : public clang::ASTConsumer {
public:
    VariableChecker(clang::CompilerInstance const &, bool DebugChanges, bool DebugUsages);

    void HandleTranslationUnit(clang::ASTContext &);

private:
    clang::DiagnosticsEngine & Reporter;
    bool const DebugChanges;
    bool const DebugUsages;

private:
    VariableChecker(VariableChecker const &);
    VariableChecker & operator=(VariableChecker const &);
};

#endif // _variable_checker_hpp_
