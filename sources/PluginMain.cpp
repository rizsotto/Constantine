// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#include "VariableChecker.hpp"

#include <iterator>

#include "llvm/Support/CommandLine.h"

#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/ASTConsumer.h>

#include <boost/functional.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <boost/range/iterator_range.hpp>


namespace {

// Do nothing, just enjoy the non C++ code.
class NullConsumer : public clang::ASTConsumer {
public:
    NullConsumer()
        : clang::ASTConsumer()
    { }
};

class Plugin : public clang::PluginASTAction {
public:
    Plugin()
        : clang::PluginASTAction()
        , FlagDebugChangeDetection("debug-change-detection",
            llvm::cl::desc("Enable debug output for variable changes [Medve plugin]"),
            llvm::cl::init(false))
        , FlagDebugUsageDetection("debug-usage-detection",
            llvm::cl::desc("Enable debug output for variable usages [Medve plugin]"),
            llvm::cl::init(false))
    { }

private:
    static bool IsCPlusPlus(clang::CompilerInstance const & Compiler) {
        clang::LangOptions const Opts = Compiler.getLangOpts();
        return (Opts.CPlusPlus) || (Opts.CPlusPlus0x);
    }

    clang::ASTConsumer * CreateASTConsumer(clang::CompilerInstance & Compiler, llvm::StringRef) {
        return IsCPlusPlus(Compiler)
            ? (clang::ASTConsumer *) new VariableChecker(Compiler, FlagDebugChangeDetection, FlagDebugUsageDetection)
            : (clang::ASTConsumer *) new NullConsumer();
    }

    bool ParseArgs(clang::CompilerInstance const &,
                   std::vector<std::string> const & Args) {
        std::vector<char const *> ArgPtrs;
        {
            // make llvm::cl::ParseCommandLineOptions happy
            static char const * const prg_name = "medve";
            ArgPtrs.push_back(prg_name);
        }

        boost::transform( boost::make_iterator_range(Args.begin(), Args.end())
                        , std::back_inserter(ArgPtrs)
                        , boost::mem_fun_ref(&std::string::c_str));

        llvm::cl::ParseCommandLineOptions(ArgPtrs.size(), &ArgPtrs.front());

        return true;
    }

private:
    llvm::cl::opt<bool> FlagDebugChangeDetection;
    llvm::cl::opt<bool> FlagDebugUsageDetection;
};

} // namespace anonymous

static clang::FrontendPluginRegistry::Add<Plugin>
    Register("medve", "suggest const usage");
