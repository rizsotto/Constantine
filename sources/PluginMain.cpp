// Copyright 2011 by Laszlo Nagy [see file LICENSE.TXT]

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/AST.h"
#include "clang/AST/Stmt.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/Support/raw_ostream.h"

#include <queue>
#include <iterator>

namespace {

bool isBuiltin(clang::NamedDecl const * Decl) {
    clang::IdentifierInfo const * const Id = Decl->getIdentifier();
    return (Id) && (Id->isStr("__va_list_tag") || Id->isStr("__builtin_va_list"));
}

class ExplicitCastFinder : public clang::ASTConsumer
{
    clang::DiagnosticsEngine & DiagEng;
public:
    ExplicitCastFinder(clang::DiagnosticsEngine & DE)
        : DiagEng(DE)
    { }

    void findConstCast(clang::Stmt const * const Current) const {
        typedef std::deque<const clang::Stmt *> StmtQueue;
        typedef std::back_insert_iterator<StmtQueue> StmtQueueInserter;

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
                unsigned const DiagID =
                    DiagEng.getCustomDiagID(clang::DiagnosticsEngine::Warning,
                                            "explicit cast found");
                DiagEng.Report(Head->getLocStart(), DiagID);
                break;
            }
            default:
                std::copy(Head->child_begin(), Head->child_end(), WorkListInserter);
                break;
            }
        }
    }

    virtual bool HandleTopLevelDecl(clang::DeclGroupRef Decls) {
        for (clang::DeclGroupRef::iterator It = Decls.begin(), End = Decls.end(); It != End; ++It)
        {
            if (clang::NamedDecl const * const Current = clang::dyn_cast<clang::NamedDecl>(*It))
            {
                if ((! isBuiltin(Current)) && Current->hasBody())
                {
                    findConstCast(Current->getBody());
                }
            }
        }
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
