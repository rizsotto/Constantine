// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#ifndef _ModuleAnalysis_hpp_
#define _ModuleAnalysis_hpp_

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

// It runs the pseudo const analysis on the given translation unit.
class ModuleAnalysis : public boost::noncopyable, public clang::ASTConsumer {
public:
    ModuleAnalysis(clang::CompilerInstance const &, Target);

    void HandleTranslationUnit(clang::ASTContext &);

private:
    clang::DiagnosticsEngine & Reporter;
    Target const State;
};

#endif // _ModuleAnalysis_hpp_
