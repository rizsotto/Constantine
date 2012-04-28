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

class ExplicitCastFinder : public clang::ASTConsumer
                         , public clang::RecursiveASTVisitor<ExplicitCastFinder> {
public:
    ExplicitCastFinder(clang::DiagnosticsEngine & DE)
        : clang::ASTConsumer()
        , clang::RecursiveASTVisitor<ExplicitCastFinder>()
        , Diagnostics(DE)
    { }

    bool HandleTopLevelDecl(clang::DeclGroupRef Decls) {
        for (clang::DeclGroupRef::iterator It = Decls.begin(), End = Decls.end(); It != End; ++It) {
            if (clang::NamedDecl * Current = clang::dyn_cast<clang::NamedDecl>(*It)) {
                TraverseDecl(Current);
            }
        }
        return true;
    }

    bool VisitExplicitCastExpr(clang::ExplicitCastExpr * Decl) {
        unsigned const Id =
            Diagnostics.getCustomDiagID(clang::DiagnosticsEngine::Warning,
                                    "explicit cast found");
        Diagnostics.Report(Decl->getLocStart(), Id);
        return true;
    }
private:
    clang::DiagnosticsEngine & Diagnostics;
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
            ? (clang::ASTConsumer *) new ExplicitCastFinder(Compiler.getDiagnostics())
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
