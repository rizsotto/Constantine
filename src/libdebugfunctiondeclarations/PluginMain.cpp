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

#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/ASTConsumer.h>


namespace {

    // Do nothing, just enjoy the non C++ code.
    class NullConsumer : public clang::ASTConsumer {
    public:
        NullConsumer() : clang::ASTConsumer() {}
    };

    // The function declaration debug plugin...
    class Plugin : public clang::PluginASTAction {
    public:
        Plugin() : clang::PluginASTAction() {}

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
                   ? std::unique_ptr<clang::ASTConsumer>(new ModuleAnalysis(C, FuncionDeclaration))
                   : std::unique_ptr<clang::ASTConsumer>(new NullConsumer());
        }

        // ..:: Entry point for plugins ::..
        bool ParseArgs(clang::CompilerInstance const &,
                       std::vector<std::string> const &Args) override {
            return true;
        }
    };

} // namespace anonymous

static clang::FrontendPluginRegistry::Add<Plugin>
    Register("constantine", "suggest const usage");
