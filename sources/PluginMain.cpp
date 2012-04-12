// Copyright 2011 by Laszlo Nagy [see file LICENSE.TXT]

#include "clang/Basic/SourceLocation.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/AST.h"
#include "clang/AST/Stmt.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/Support/raw_ostream.h"

#include <queue>
#include <iterator>

namespace
{

class ASTConsumer : public clang::ASTConsumer
{
public:
    ASTConsumer()
    { }

    static bool isBuiltin(clang::NamedDecl const * decl)
    {
        clang::IdentifierInfo const * const id = decl->getIdentifier();
        return (id) && (id->isStr("__va_list_tag") || id->isStr("__builtin_va_list"));
    }

    static void findConstCast(clang::Stmt const * Current) {
        std::deque<const clang::Stmt *> WorkList;
        std::back_insert_iterator<std::deque<const clang::Stmt *> > Inserter(WorkList);

        *Inserter++ = Current;

        while (! WorkList.empty()) {
            clang::Stmt const * const Head = WorkList.front();
            WorkList.pop_front();

            switch (Head->getStmtClass()) {
            case clang::Stmt::BlockExprClass: {
                clang::BlockExpr const * const B = clang::cast<clang::BlockExpr>(Head);
                *Inserter++ = B->getBody();
                break;
            }
            case clang::Stmt::CStyleCastExprClass:
            case clang::Stmt::CXXFunctionalCastExprClass:
            case clang::Stmt::CXXConstCastExprClass:
            case clang::Stmt::CXXDynamicCastExprClass:
            case clang::Stmt::CXXReinterpretCastExprClass:
            case clang::Stmt::CXXStaticCastExprClass: {
                Head->dump();
                break;
            }
            default:
                std::copy(Head->child_begin(), Head->child_end(), Inserter);
                break;
            }
        }
    }

    static void dump(clang::Decl const * const Current) {
        clang::PrintingPolicy DumpPolicy(Current->getASTContext().getLangOpts());
        DumpPolicy.Dump = true;
        Current->print(llvm::errs(), DumpPolicy);
    }

    virtual bool HandleTopLevelDecl(clang::DeclGroupRef gd) {
        for (clang::DeclGroupRef::iterator It = gd.begin(), e = gd.end(); It != e; ++It)
        {
            if (clang::NamedDecl const * const Current = clang::dyn_cast<clang::NamedDecl>(*It))
            {
                if (! isBuiltin(Current))
                {
                    dump(Current);

                    if (Current->hasBody())
                    {
                        findConstCast(Current->getBody());
                    }
                }
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
        return new ASTConsumer();
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
