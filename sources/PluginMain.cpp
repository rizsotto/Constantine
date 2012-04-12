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
#include <algorithm>

namespace
{

struct WriteDiagnostic {
    void operator()(clang::Stmt const * const Current) const {
        Current->dump();
    }
};

class ASTConsumer : public clang::ASTConsumer
{
public:
    ASTConsumer()
    { }

    static bool isBuiltin(clang::NamedDecl const * Decl) {
        clang::IdentifierInfo const * const Id = Decl->getIdentifier();
        return (Id) && (Id->isStr("__va_list_tag") || Id->isStr("__builtin_va_list"));
    }

    typedef std::deque<const clang::Stmt *> StmtQueue;
    typedef std::back_insert_iterator<StmtQueue> StmtQueueInserter;

    static StmtQueueInserter & findConstCast(clang::Stmt const * const Current, StmtQueueInserter & Result) {
        StmtQueue WorkList;
        StmtQueueInserter WorkListInserter(WorkList);

        *WorkListInserter++ = Current;

        while (! WorkList.empty()) {
            clang::Stmt const * const Head = WorkList.front();
            WorkList.pop_front();

            switch (Head->getStmtClass()) {
            case clang::Stmt::BlockExprClass: {
                clang::BlockExpr const * const B = clang::cast<clang::BlockExpr>(Head);
                *WorkListInserter++ = B->getBody();
                break;
            }
            case clang::Stmt::CStyleCastExprClass:
            case clang::Stmt::CXXFunctionalCastExprClass:
            case clang::Stmt::CXXConstCastExprClass:
            case clang::Stmt::CXXDynamicCastExprClass:
            case clang::Stmt::CXXReinterpretCastExprClass:
            case clang::Stmt::CXXStaticCastExprClass: {
                *Result++ = Head;
                break;
            }
            default:
                std::copy(Head->child_begin(), Head->child_end(), WorkListInserter);
                break;
            }
        }
        return Result;
    }

    static void dump(clang::Decl const * const Current) {
        clang::PrintingPolicy DumpPolicy(Current->getASTContext().getLangOpts());
        DumpPolicy.Dump = true;
        Current->print(llvm::errs(), DumpPolicy);
    }

    virtual bool HandleTopLevelDecl(clang::DeclGroupRef Decls) {
        StmtQueue Casts;
        StmtQueueInserter CastInserter(Casts);

        for (clang::DeclGroupRef::iterator It = Decls.begin(), e = Decls.end(); It != e; ++It)
        {
            if (clang::NamedDecl const * const Current = clang::dyn_cast<clang::NamedDecl>(*It))
            {
                if (! isBuiltin(Current))
                {
                    //dump(Current);

                    if (Current->hasBody())
                    {
                        CastInserter = findConstCast(Current->getBody(), CastInserter);
                    }
                }
            }
        }
        WriteDiagnostic Reporter;
        std::for_each(Casts.begin(), Casts.end(), Reporter);

        return true;
    }
};

class FindConstCandidateAction : public clang::PluginASTAction
{
protected:
    clang::ASTConsumer * CreateASTConsumer(clang::CompilerInstance &, llvm::StringRef) {
        return new ASTConsumer();
    }

    bool ParseArgs(clang::CompilerInstance const & Compiler,
                   std::vector<std::string> const & Args) {
        for (unsigned i = 0, e = Args.size(); i != e; ++i) {
            llvm::errs() << "PrintFunctionNames arg = " << Args[i] << "\n";
            // Example error handling.
            if (Args[i] == "-an-error") {
                clang::DiagnosticsEngine & D = Compiler.getDiagnostics();
                unsigned DiagID = D.getCustomDiagID(
                                      clang::DiagnosticsEngine::Error, "invalid argument '" + Args[i] + "'");
                D.Report(DiagID);
                return false;
            }
        }
        if (Args.size() && Args[0] == "help") {
            PrintHelp(llvm::errs() );
        }
        return true;
    }
    void PrintHelp(llvm::raw_ostream & Ros) {
        Ros << "Help for PrintFunctionNames plugin goes here\n";
    }
};

}

static clang::FrontendPluginRegistry::Add<FindConstCandidateAction>
    x("find-const-candidate", "suggests declaration to be const");
