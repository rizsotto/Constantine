// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#include <set>
#include <map>
#include <algorithm>
#include <functional>
#include <iterator>
#include <cassert>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/AST.h>

namespace {

typedef std::set<clang::Stmt const *> Contexts;
typedef std::set<clang::VarDecl const *> Variables;
typedef std::map<clang::Stmt const *, Variables> VariablesByContext;
typedef std::map<clang::VarDecl const *, Contexts> ContextsByVariable;

class ConstantAnalysis : public clang::RecursiveASTVisitor<ConstantAnalysis> {
    Variables NonConstants;
public:
    ConstantAnalysis()
        : NonConstants()
    { }

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
        case clang::BO_ShrAssign:
            if (clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl>(LHSDecl)) {
                NonConstants.insert(VD);
            }
            break;
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
        case clang::UO_AddrOf:
            if (clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl>(D)) {
                NonConstants.insert(VD);
            }
            break;
        default:
            break;
        }
        return true;
    }

    bool VisitDeclStmt(clang::DeclStmt const * const DS) {
        for (clang::DeclStmt::const_decl_iterator I = DS->decl_begin(),
             E = DS->decl_end(); I != E; ++I) {
            checkRefDeclaration(*I);
        }
        return true;
    }

    bool VisitCallExpr(clang::CallExpr const * const CE) {
        for (clang::CallExpr::const_arg_iterator AIt(CE->arg_begin()), AEnd(CE->arg_end()); AIt != AEnd; ++AIt ) {
            insertWhenReferedWithoutCast(*AIt);
        }
        return true;
    }

    bool VisitMemberExpr(clang::MemberExpr const * const CE) {
        clang::Type const * const T = CE->getMemberDecl()->getType().getCanonicalType().getTypePtr();
        if (clang::FunctionProtoType const * const F = T->getAs<clang::FunctionProtoType>()) {
            if (! (F->getTypeQuals() & clang::Qualifiers::Const) ) {
                insertWhenReferedWithoutCast(CE->getBase());
            }
        }
        return true;
    }

    Variables const & getNonConstVariables() const {
        return NonConstants;
    }

private:
    void checkRefDeclaration(clang::Decl const * const Decl) {
        clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl const>(Decl);
        if ((VD) && (VD->getType().getTypePtr()->isReferenceType())) {
            insertWhenReferedWithoutCast(VD->getInit()->IgnoreParenCasts());
        }
    }
    void insertWhenReferedWithoutCast(clang::Expr const * E) {
        if (clang::DeclRefExpr const * const DE = clang::dyn_cast<clang::DeclRefExpr>(E)) {
            if (clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl>(DE->getDecl())) {
                NonConstants.insert(VD);
            }
        }
    }

    static clang::Decl const * getDecl(clang::Expr const * const E) {
        if (clang::DeclRefExpr const * const DR = clang::dyn_cast<clang::DeclRefExpr const>(E)) {
            return DR->getDecl();
        } else if (clang::MemberExpr const * const ME = clang::dyn_cast<clang::MemberExpr>(E)) {
            return getDecl(ME->getBase());
        }
        return 0;
    }

    ConstantAnalysis(ConstantAnalysis const &);
    ConstantAnalysis & operator=(ConstantAnalysis const &);
};

class DeclContextVisitor : public clang::RecursiveASTVisitor<DeclContextVisitor> {
    VariablesByContext Vars;
    ContextsByVariable Ctxs;

public:
    DeclContextVisitor()
        : clang::RecursiveASTVisitor<DeclContextVisitor>()
        , Vars()
        , Ctxs()
    { }

    bool VisitBlockDeck(clang::BlockDecl const * const D) {
        return collectDeclarations(D, D);
    }
    bool VisitFunctionDecl(clang::FunctionDecl const * const D) {
        return collectDeclarations(D, D);
    }
    bool VisitLinkageSpecDecl(clang::LinkageSpecDecl const * const D) {
        return collectDeclarations(D, D);
    }
    bool VisitNamespaceDecl(clang::NamespaceDecl const * const D) {
        return collectDeclarations(D, D);
    }
    bool VisitTagDecl(clang::TagDecl const * const D) {
        return collectDeclarations(D, D);
    }
    bool VisitTranslationUnitDecl(clang::TranslationUnitDecl const * const D) {
        return collectDeclarations(D, D);
    }

    VariablesByContext const & getVariablesByContext() const {
        return Vars;
    }
    ContextsByVariable const & getContextsByVariable() const {
        return Ctxs;
    }

private:
    static bool is_non_const(clang::VarDecl const * const D) {
        return (! D->getType().getNonReferenceType().isConstQualified());
    }
    void insertContextsByVariable(clang::Stmt const * const S, clang::VarDecl const * const D) {
        if (is_non_const(D)) {
            ContextsByVariable::iterator It = Ctxs.find(D);
            if (Ctxs.end() == It) {
                std::pair<ContextsByVariable::iterator, bool> Result = Ctxs.insert(ContextsByVariable::value_type(D, Contexts()));
                assert(Result.second);
                It = Result.first;
                assert(Ctxs.end() != It);
            }
            (*It).second.insert(S);
        }
    }
    void insertVariablesByContext(clang::VarDecl const * const D, clang::Stmt const * const S) {
        if (is_non_const(D)) {
            VariablesByContext::iterator It = Vars.find(S);
            if (Vars.end() == It) {
                std::pair<VariablesByContext::iterator, bool> Result = Vars.insert(VariablesByContext::value_type(S, Variables()));
                assert(Result.second);
                It = Result.first;
                assert(Vars.end() != It);
            }
            (*It).second.insert(D);
        }
    }
    bool collectDeclarations(clang::DeclContext const * const DC, clang::Decl const * const D) {
        for (clang::DeclContext const * Cit(DC); 0 != Cit; Cit = Cit->getParent()) {
            for (clang::DeclContext::decl_iterator Dit(Cit->decls_begin()), End(Cit->decls_end());
                End != Dit; ++Dit) {
                if (clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl const>(*Dit)) {
                    if (D->hasBody()) {
                        clang::Stmt const * const S = D->getBody();
                        insertContextsByVariable(S, VD);
                        insertVariablesByContext(VD, S);
                    }
                }
            }
        }
        return true;
    }

    DeclContextVisitor(DeclContextVisitor const &);
    DeclContextVisitor & operator=(DeclContextVisitor const &);
};

class VariableChecker : public clang::ASTConsumer {
    clang::DiagnosticsEngine & DiagEng;
public:
    VariableChecker(clang::CompilerInstance const & Compiler)
        : clang::ASTConsumer()
        , DiagEng(Compiler.getDiagnostics())
    { }

    void HandleTranslationUnit(clang::ASTContext & Ctx) {
        DeclContextVisitor Collector;
        Collector.TraverseDecl(Ctx.getTranslationUnitDecl());

        VariablesByContext const & Input = Collector.getVariablesByContext();
        VariablesByContext Output;
        std::insert_iterator<VariablesByContext> OutputIt(Output, Output.begin());
        std::transform(Input.begin(), Input.end(), OutputIt, &VariableChecker::analyse);
        ContextsByVariable const & Candidates = Collector.getContextsByVariable();
        ContextsByVariable Result;
        std::insert_iterator<ContextsByVariable> ResultIt(Result, Result.begin());
        std::remove_copy_if(Candidates.begin(), Candidates.end(), ResultIt, std::bind1st(std::ptr_fun(VariableChecker::check), Output));
        std::for_each(Result.begin(), Result.end(), std::bind1st(std::ptr_fun(VariableChecker::report), DiagEng));
    }

private:
    static VariablesByContext::value_type analyse(VariablesByContext::value_type const In) {
        ConstantAnalysis Analysis;
        Analysis.TraverseStmt(const_cast<clang::Stmt*>(In.first));
        return VariablesByContext::value_type(In.first, Analysis.getNonConstVariables());
    }

    static bool check(VariablesByContext const & AllCtxs, ContextsByVariable::value_type It) {
        clang::VarDecl const * Var = It.first;
        Contexts const & Visibles = It.second;
        for (Contexts::const_iterator It(Visibles.begin()), End(Visibles.end());
            End != It; ++It) {
            VariablesByContext::const_iterator CtxIt = AllCtxs.find(*It);
            assert(AllCtxs.end() != CtxIt);
            if (CtxIt->second.count(Var)) {
                return true;
            }
        }
        return false;
    }

    static void report(clang::DiagnosticsEngine & DiagEng, ContextsByVariable::value_type It) {
        char const * const Msg = "variable could be declared as const [Medve plugin]";
        unsigned const DiagID =
            DiagEng.getCustomDiagID(clang::DiagnosticsEngine::Warning, Msg);
        clang::VarDecl const * const Decl = It.first;
        DiagEng.Report(Decl->getLocStart(), DiagID);
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
            ? (clang::ASTConsumer *) new VariableChecker(Compiler)
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
