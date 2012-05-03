// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#include <cassert>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/AST.h>
#include <clang/AST/Stmt.h>
#include <clang/Analysis/Analyses/PseudoConstantAnalysis.h>
#include <llvm/Support/raw_ostream.h>

namespace {

class VariableVisitor : public clang::ASTConsumer
                      , public clang::RecursiveASTVisitor<VariableVisitor> {
    clang::DiagnosticsEngine & DiagEng;
public:
    VariableVisitor(clang::CompilerInstance const & Compiler)
        : clang::ASTConsumer()
        , clang::RecursiveASTVisitor<VariableVisitor>()
        , DiagEng(Compiler.getDiagnostics())
    { }

    virtual void HandleTranslationUnit(clang::ASTContext & ctx) {
        TraverseDecl(ctx.getTranslationUnitDecl());
    }

    // method arguments
    bool VisitParmVarDecl(clang::ParmVarDecl const * const Decl) {
        clang::DeclContext const * const Ctx = Decl->getParentFunctionOrMethod();
        assert(Ctx);

        clang::FunctionDecl const * const Function =
            llvm::dyn_cast<clang::FunctionDecl const>(Ctx);
        assert(Function);
        assert(Function->hasBody());

        clang::PseudoConstantAnalysis Checker(Function->getBody());
        return (! Checker.wasReferenced(Decl))
            ? reportNotUsed(Decl)
            : (Checker.isPseudoConstant(Decl))
                ? reportPseudoConst(Decl)
                : true;
    }

private:
    bool report(clang::VarDecl const * const Decl, char const * const msg) {
        unsigned const DiagID =
            DiagEng.getCustomDiagID(clang::DiagnosticsEngine::Warning, msg);
        DiagEng.Report(Decl->getLocStart(), DiagID);

        return false;
    }
    bool reportNotUsed(clang::VarDecl const * const Decl) {
        return report(Decl, "variable declared, but not used [Medve plugin]");
    }
    bool reportPseudoConst(clang::VarDecl const * const Decl) {
        return report(Decl, "variable could be declared as const [Medve plugin]");
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
            ? (clang::ASTConsumer *) new VariableVisitor(Compiler)
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
