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
#include <llvm/Support/raw_ostream.h>
#include <llvm/ADT/SmallSet.h>

namespace {

class ConstantAnalysis : public clang::RecursiveASTVisitor<ConstantAnalysis> {
    static size_t const VARDECL_SET_SIZE = 256;
    typedef llvm::SmallPtrSet<clang::VarDecl const*, VARDECL_SET_SIZE> VarDeclSet;
    VarDeclSet NonConstants;
public:
    ConstantAnalysis(clang::Stmt * const Stmt)
        : NonConstants()
    {
        TraverseStmt(Stmt);
    }

    static clang::Decl const * getDecl(clang::Expr const * E) {
        if (clang::DeclRefExpr const * DR = clang::dyn_cast<clang::DeclRefExpr const>(E))
            return DR->getDecl();
        else
            return 0;
    }

    bool VisitBinaryOperator(clang::BinaryOperator const * const BO) {
        clang::Decl const * const LHSDecl =
            getDecl(BO->getLHS()->IgnoreParenCasts());
        if (!LHSDecl)
            return true;

        switch (BO->getOpcode()) {
        case clang::BO_Assign: {
            clang::Decl const * const RHSDecl =
                getDecl(BO->getRHS()->IgnoreParenCasts());
            if (LHSDecl == RHSDecl) {
                break;
            }
        }
        case clang::BO_AddAssign:
        case clang::BO_SubAssign:
        case clang::BO_MulAssign:
        case clang::BO_DivAssign:
        case clang::BO_AndAssign:
        case clang::BO_OrAssign:
        case clang::BO_XorAssign:
        case clang::BO_ShlAssign:
        case clang::BO_ShrAssign: {
            clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl>(LHSDecl);
            if (VD) {
                NonConstants.insert(VD);
            }
            break;
        }
        default:
            break;
        }
        return true;
    }

    bool VisitUnaryOperator(clang::UnaryOperator const * const UO) {
        clang::Decl const * const D =
            getDecl(UO->getSubExpr()->IgnoreParenCasts());
        if (!D)
            return true;

        switch (UO->getOpcode()) {
        case clang::UO_PostDec:
        case clang::UO_PostInc:
        case clang::UO_PreDec:
        case clang::UO_PreInc:
        case clang::UO_AddrOf: {
            clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl>(D);
            if (VD) {
                NonConstants.insert(VD);
            }
            break;
        }
        default:
            break;
        }
        return true;
    }

    void checkRefDeclaration(clang::Decl const * const Decl) {
        clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl const>(Decl);
        if ((VD) && (VD->getType().getTypePtr()->isReferenceType())) {
            if (clang::Decl const * const D = getDecl(VD->getInit()->IgnoreParenCasts())) {
                if (clang::VarDecl const * const RefVD = clang::dyn_cast<clang::VarDecl const>(D)) {
                    NonConstants.insert(RefVD);
                }
            }
        }
    }

    bool VisitDeclStmt(clang::DeclStmt const * const DS) {
        for (clang::DeclStmt::const_decl_iterator I = DS->decl_begin(),
             E = DS->decl_end(); I != E; ++I) {
            checkRefDeclaration(*I);
        }
        return true;
    }

    bool isPseudoConstant(const clang::VarDecl *VD) const {
        return ! NonConstants.count(VD);
    }
};

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
        if (Decl->getType().isConstQualified()) {
            return true;
        }

        clang::DeclContext const * const Ctx = Decl->getParentFunctionOrMethod();
        assert(Ctx);

        clang::FunctionDecl const * const Function =
            llvm::dyn_cast<clang::FunctionDecl const>(Ctx);
        assert(Function);
        assert(Function->hasBody());

        //Function->getBody()->dump();
        ConstantAnalysis Checker(Function->getBody());
        if (Checker.isPseudoConstant(Decl)) {
            reportPseudoConst(Decl);
        }
        return true;
    }

private:
    void report(clang::VarDecl const * const Decl, char const * const msg) {
        unsigned const DiagID =
            DiagEng.getCustomDiagID(clang::DiagnosticsEngine::Warning, msg);
        DiagEng.Report(Decl->getLocStart(), DiagID);
    }
    void reportPseudoConst(clang::VarDecl const * const Decl) {
        report(Decl, "variable could be declared as const [Medve plugin]");
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
