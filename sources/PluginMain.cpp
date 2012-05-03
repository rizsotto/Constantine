// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#include <clang/Basic/LangOptions.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/AST.h>
#include <clang/AST/Stmt.h>
#include <clang/Frontend/CompilerInstance.h>
#include <llvm/Support/raw_ostream.h>

namespace {

void verboseDump(clang::Decl const * const Decl) {
    clang::LangOptions const LO;
    clang::PrintingPolicy PP(LO);
    PP.Dump = true;

    Decl->print(llvm::errs(), PP);
}

class VariableVisitor : public clang::ASTConsumer
                      , public clang::RecursiveASTVisitor<VariableVisitor> {
public:
    VariableVisitor()
        : clang::ASTConsumer()
        , clang::RecursiveASTVisitor<VariableVisitor>()
    { }

    virtual void HandleTranslationUnit(clang::ASTContext & ctx) {
        TraverseDecl(ctx.getTranslationUnitDecl());
    }

    // method arguments
    bool VisitParmVarDecl(clang::ParmVarDecl const * const Decl) {
        return VisitFilteredVarDecl(Decl);
    }

    // top level variable declarations
    bool VisitVarDecl(clang::VarDecl const * const Decl) {
        return (Decl->isFileVarDecl() && (!Decl->hasExternalStorage()))
            ? VisitFilteredVarDecl(Decl)
            : true;
    }

    bool VisitFilteredVarDecl(clang::VarDecl const * const Decl) {
        verboseDump(Decl);
        return true;
    }
};

class NullConsumer : public clang::ASTConsumer {
public:
    NullConsumer()
        : clang::ASTConsumer()
    { }
};

class MedvePlugin : public clang::PluginASTAction {
    static bool isCPlusPlus(clang::CompilerInstance const & Compiler) {
        clang::LangOptions const Opts = Compiler.getLangOpts();

        return (Opts.CPlusPlus) || (Opts.CPlusPlus0x);
    }
    clang::ASTConsumer * CreateASTConsumer(clang::CompilerInstance & Compiler,
                                           llvm::StringRef) {
        return isCPlusPlus(Compiler)
            ? (clang::ASTConsumer *) new VariableVisitor()
            : (clang::ASTConsumer *) new NullConsumer();
    }

    bool ParseArgs(clang::CompilerInstance const &,
                   std::vector<std::string> const &) {
        return true;
    }
};

} // namespace anonymous

static clang::FrontendPluginRegistry::Add<MedvePlugin>
    MedvePluginRegistry("medve", "suggest const usage");
