// Copyright 2011 by Laszlo Nagy [see file LICENSE.TXT]

#include "clang/Basic/SourceLocation.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/AST.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/Support/raw_ostream.h"

namespace
{

class ASTConsumer : public clang::ASTConsumer
{
    clang::SourceManager const & m_sm;

public:
    ASTConsumer(clang::SourceManager const & sm)
        : m_sm (sm)
    { }

    virtual bool HandleTopLevelDecl(clang::DeclGroupRef gd)
    {
        for (clang::DeclGroupRef::iterator it = gd.begin(), e = gd.end(); it != e; ++it)
        {
            clang::Decl const * const current = *it;
            if (clang::NamedDecl const * found = clang::dyn_cast<clang::NamedDecl>(current) )
            {
                clang::SourceLocation const & loc = found->getLocation();
                llvm::errs() << "top-level-decl: \"" << found->getName() << "\" at ";
                loc.print (llvm::errs(), m_sm);
                llvm::errs() << "\n";
            }
        }
        return true;
    }
};

class FindConstCandidateAction : public clang::PluginASTAction
{
protected:
    clang::ASTConsumer * CreateASTConsumer(clang::CompilerInstance & compiler, llvm::StringRef)
    {
        return new ASTConsumer(compiler.getSourceManager());
    }

    bool ParseArgs(clang::CompilerInstance const & compiler,
                   std::vector<std::string> const & args)
    {
        for (unsigned i = 0, e = args.size(); i != e; ++i)
        {
            llvm::errs() << "PrintFunctionNames arg = " << args[i] << "\n";
            // Example error handling.
            if (args[i] == "-an-error")
            {
                clang::DiagnosticsEngine & D = compiler.getDiagnostics();
                unsigned DiagID = D.getCustomDiagID(
                                      clang::DiagnosticsEngine::Error, "invalid argument '" + args[i] + "'");
                D.Report(DiagID);
                return false;
            }
        }
        if (args.size() && args[0] == "help")
        {
            PrintHelp(llvm::errs() );
        }
        return true;
    }
    void PrintHelp(llvm::raw_ostream & ros)
    {
        ros << "Help for PrintFunctionNames plugin goes here\n";
    }
};

}

static clang::FrontendPluginRegistry::Add<FindConstCandidateAction>
    x("find-const-candidate", "suggests declaration to be const");
