// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

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
                         , public clang::RecursiveASTVisitor<ExplicitCastFinder>
{
    clang::DiagnosticsEngine & DiagEng;
public:
    ExplicitCastFinder(clang::DiagnosticsEngine & DE)
        : DiagEng(DE)
    { }

    virtual bool HandleTopLevelDecl(clang::DeclGroupRef Decls) {
        for (clang::DeclGroupRef::iterator It = Decls.begin(), End = Decls.end(); It != End; ++It)
        {
            if (clang::NamedDecl * Current = clang::dyn_cast<clang::NamedDecl>(*It))
            {
                TraverseDecl(Current);
            }
        }
        return true;
    }

    bool VisitExplicitCastExpr(clang::ExplicitCastExpr * Decl) {
        unsigned const DiagID =
            DiagEng.getCustomDiagID(clang::DiagnosticsEngine::Warning,
                                    "explicit cast found");
        DiagEng.Report(Decl->getLocStart(), DiagID);
        return true;
    }

};

class ExplicitCastPlugin : public clang::PluginASTAction
{
protected:
    clang::ASTConsumer * CreateASTConsumer(clang::CompilerInstance & Compiler,
                                           llvm::StringRef) {
        return new ExplicitCastFinder(Compiler.getDiagnostics());
    }

    bool ParseArgs(clang::CompilerInstance const &,
                   std::vector<std::string> const &) {
        return true;
    }
    void PrintHelp(llvm::raw_ostream & Ros) {
        Ros << "List all explicit casts from given module\n";
    }
};

}

static clang::FrontendPluginRegistry::Add<ExplicitCastPlugin>
    X("find-explicit-casts", "look for explicit casts");
