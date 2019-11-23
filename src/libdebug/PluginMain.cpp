/*  Copyright (C) 2012-2014  László Nagy
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

#include "libconstantine_a/ModuleAnalysis.hpp"

#include <memory>

#include <llvm/Support/CommandLine.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/ASTConsumer.h>


namespace {

    char const * const plugin_name = "constantine";

    // The debug plugin...
    class Plugin : public clang::PluginASTAction {
    public:
        Plugin()
        : clang::PluginASTAction()
        , Debug(PseudoConstness)
        {}

        Plugin(Plugin const &) = delete;

        Plugin &operator=(Plugin const &) = delete;

    private:
        // Decide whether the compiler was invoked as C++ compiler or not.
        static bool IsCPlusPlus(clang::CompilerInstance const &Compiler) {
            clang::LangOptions const &Opts = Compiler.getLangOpts();
            return Opts.CPlusPlus;
        }

        // ..:: Entry point for plugins ::..
        std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &C, llvm::StringRef) override {
            return IsCPlusPlus(C)
                   ? std::unique_ptr<clang::ASTConsumer>(new ModuleAnalysis(C, Debug))
                   : std::make_unique<clang::ASTConsumer>();
        }

        // ..:: Entry point for plugins ::..
        bool ParseArgs(clang::CompilerInstance const &,
                       std::vector<std::string> const &Args) override {
            std::vector<char const *> ArgPtrs;
            {
                // make llvm::cl::ParseCommandLineOptions happy
                ArgPtrs.push_back(plugin_name);
                for (auto && Arg : Args) {
                    ArgPtrs.push_back(Arg.c_str());
                }
            }
            {
                static llvm::cl::opt<Target> const
                    DebugParser("mode",
                        llvm::cl::desc("Set the debugging level for Constantine plugin:"),
                        llvm::cl::values(
                            clEnumVal(FunctionDeclaration, "Enable function detection"),
                            clEnumVal(VariableDeclaration, "Enable variables detection"),
                            clEnumVal(VariableChanges, "Enable variable change detection"),
                            clEnumVal(VariableUsages, "Enable variable usage detection")),
                        llvm::cl::Required);

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
        Register(plugin_name, "debug constantine plugin functionality");
