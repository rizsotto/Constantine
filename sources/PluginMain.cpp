// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#include "ModuleAnalysis.hpp"

#include <iterator>

#include "llvm/Support/CommandLine.h"

#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/ASTConsumer.h>

#include <boost/functional.hpp>
#include <boost/range/algorithm/transform.hpp>


namespace {

// Do nothing, just enjoy the non C++ code.
class NullConsumer : public clang::ASTConsumer {
public:
    NullConsumer()
        : clang::ASTConsumer()
    { }
};

static char const * const plugin_name = "constantine";

// The const analyser plugin... Implements the neccessary interface
// to be a plugin. Parse command line arguments and dispatch the
// real work to other classes.
class Plugin
    : public boost::noncopyable
    , public clang::PluginASTAction {
public:
    Plugin()
        : boost::noncopyable()
        , clang::PluginASTAction()
        , Debug(PseudoConstness)
    { }

private:
    // Decide wheater the compiler was invoked as C++ compiler or not.
    static bool IsCPlusPlus(clang::CompilerInstance const & Compiler) {
        clang::LangOptions const Opts = Compiler.getLangOpts();
        return (Opts.CPlusPlus) || (Opts.CPlusPlus0x);
    }

    // ..:: Entry point for plugins ::..
    clang::ASTConsumer * CreateASTConsumer(clang::CompilerInstance & C, llvm::StringRef) {
        return IsCPlusPlus(C)
            ? (clang::ASTConsumer *) new ModuleAnalysis(C, Debug)
            : (clang::ASTConsumer *) new NullConsumer();
    }

    // ..:: Entry point for plugins ::..
    bool ParseArgs(clang::CompilerInstance const &,
                   std::vector<std::string> const & Args) {
        std::vector<char const *> ArgPtrs;
        {
            // make llvm::cl::ParseCommandLineOptions happy
            ArgPtrs.push_back(plugin_name);

            boost::transform(Args,
                std::back_inserter(ArgPtrs),
                boost::mem_fun_ref(&std::string::c_str));
        }
        {
            static llvm::cl::opt<Target> const
                DebugParser("debug-constantine",
                    llvm::cl::desc("Set the debugging level for Medve plugin:"),
                    llvm::cl::init(PseudoConstness),
                    llvm::cl::values(
                        clEnumVal(FuncionDeclaration, "Enable function detection"),
                        clEnumVal(VariableDeclaration, "Enable variables detection"),
                        clEnumVal(VariableChanges, "Enable variable change detection"),
                        clEnumVal(VariableUsages, "Enable variable usage detection"),
                        clEnumValEnd));

            llvm::cl::ParseCommandLineOptions(ArgPtrs.size(), &ArgPtrs.front());

            Debug = DebugParser;
        }
        return true;
    }

private:
    Target Debug;
};

} // namespace anonymous

static clang::FrontendPluginRegistry::Add<Plugin>
    Register(plugin_name, "suggest const usage");
