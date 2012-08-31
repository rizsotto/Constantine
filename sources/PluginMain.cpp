// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#include "VariableChecker.hpp"

#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/ASTConsumer.h>

namespace {

// Do nothing, just enjoy the non C++ code.
class NullConsumer : public clang::ASTConsumer {
public:
    NullConsumer()
        : clang::ASTConsumer()
    { }
};

class Plugin : public clang::PluginASTAction {
private:
    static bool IsCPlusPlus(clang::CompilerInstance const & Compiler) {
        clang::LangOptions const Opts = Compiler.getLangOpts();
        return (Opts.CPlusPlus) || (Opts.CPlusPlus0x);
    }

    clang::ASTConsumer * CreateASTConsumer(clang::CompilerInstance & Compiler, llvm::StringRef) {
        return IsCPlusPlus(Compiler)
            ? (clang::ASTConsumer *) new VariableChecker(Compiler)
            : (clang::ASTConsumer *) new NullConsumer();
    }

    bool ParseArgs(clang::CompilerInstance const &,
                   std::vector<std::string> const &) {
        return true;
    }
};

} // namespace anonymous

static clang::FrontendPluginRegistry::Add<Plugin>
    Register("medve", "suggest const usage");
