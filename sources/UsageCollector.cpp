// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#include "UsageCollector.hpp"
#include "IsCXXThisExpr.hpp"

#include <boost/bind.hpp>
#include <boost/range.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/algorithm/for_each.hpp>

namespace {

// Usage extract method implemented in visitor style.
class UsageExtractor
    : public clang::RecursiveASTVisitor<UsageExtractor> {
public:
    typedef ScopeAnalysis::UsageRef UsageRef;
    typedef std::pair<clang::DeclaratorDecl const *, UsageRef> Usage;

    static Usage GetUsage(clang::Expr const & Expr) {
        Usage Result(0, UsageRef());
        {
            clang::Stmt const * const Stmt = &Expr;

            UsageExtractor Visitor(Result);
            Visitor.TraverseStmt(const_cast<clang::Stmt*>(Stmt));
        }
        Result.second.second = Expr.getSourceRange();
        return Result;
    }

private:
    UsageExtractor(Usage & In)
        : clang::RecursiveASTVisitor<UsageExtractor>()
        , Result(In)
    { }

    void SetType(clang::QualType const & In) {
        static clang::QualType const Empty = clang::QualType();
        clang::QualType & Current = Result.second.first;

        if (Empty != Current) {
            return;
        }
        if (Empty == In) {
            return;
        }
        Current = In;
    }

    void SetVariable(clang::Decl const * const Decl) {
        static clang::DeclaratorDecl const * const Empty = 0;
        clang::DeclaratorDecl const * & Current = Result.first;

        if (Empty != Current) {
            return;
        }
        clang::DeclaratorDecl const * const In =
            clang::dyn_cast<clang::DeclaratorDecl const>(Decl->getCanonicalDecl());
        if (Empty == In) {
            return;
        }
        Current = In;
    }

public:
    // public visitor method.
    bool VisitDeclRefExpr(clang::DeclRefExpr const * const Expr) {
        SetVariable(Expr->getDecl());
        SetType(Expr->getType());
        return true;
    }

    bool VisitCastExpr(clang::CastExpr const * const Expr) {
        SetType(Expr->getType());
        return true;
    }

    bool VisitUnaryOperator(clang::UnaryOperator const * const Expr) {
        switch (Expr->getOpcode()) {
        case clang::UO_AddrOf:
        case clang::UO_Deref:
            SetType(Expr->getType());
        default:
            ;
        }
        return true;
    }

    bool VisitMemberExpr(clang::MemberExpr const * const Expr) {
        if (IsCXXThisExpr::Check(Expr->getBase())) {
            SetVariable(Expr->getMemberDecl());
            SetType(Expr->getType());
        }
        return true;
    }

private:
    Usage & Result;
};

void AddToUsageMap(ScopeAnalysis::UsageRefsMap & Results, UsageExtractor::Usage const & U) {
    if (clang::DeclaratorDecl const * const VD = U.first) {
        ScopeAnalysis::UsageRefsMap::iterator It = Results.find(VD);
        if (Results.end() == It) {
            std::pair<ScopeAnalysis::UsageRefsMap::iterator, bool> const R =
                Results.insert(ScopeAnalysis::UsageRefsMap::value_type(VD, ScopeAnalysis::UsageRefs()));
            It = R.first;
        }
        ScopeAnalysis::UsageRefs & Ls = It->second;
        Ls.push_back(U.second);
    }
}

// helper method not to be so verbose.
struct IsItFromMainModule {
    bool operator()(clang::Decl const * const D) const {
        clang::SourceManager const & SM = D->getASTContext().getSourceManager();
        return SM.isFromMainFile(D->getLocation());
    }
    bool operator()(ScopeAnalysis::UsageRefsMap::value_type const & Var) const {
        return this->operator()(Var.first);
    }
};

void DumpUsageMapEntry( ScopeAnalysis::UsageRefsMap::value_type const & Var
           , char const * const Message
           , clang::DiagnosticsEngine & DE) {
    unsigned const Id = DE.getCustomDiagID(clang::DiagnosticsEngine::Note, Message);
    ScopeAnalysis::UsageRefs const & Ls = Var.second;
    for (ScopeAnalysis::UsageRefs::const_iterator It(Ls.begin()), End(Ls.end()); It != End; ++It) {
        clang::DiagnosticBuilder const DB = DE.Report(It->second.getBegin(), Id);
        DB << Var.first->getNameAsString();
        DB << It->first.getAsString();
        DB.setForceEmit();
    }
}

} // namespace anonymous


UsageCollector::UsageCollector(ScopeAnalysis::UsageRefsMap & Out)
    : Results(Out)
{ }

UsageCollector::~UsageCollector()
{ }

void UsageCollector::AddToResults(clang::Expr const * const Stmt, clang::QualType const & Type) {
    UsageExtractor::Usage U = UsageExtractor::GetUsage(*Stmt);
    if (! Type.isNull()) {
        U.second.first = Type;
    }
    return AddToUsageMap(Results, U);
}

void UsageCollector::Report(char const * const M, clang::DiagnosticsEngine & DE) const {
    boost::for_each(Results | boost::adaptors::filtered(IsItFromMainModule()),
        boost::bind(DumpUsageMapEntry, _1, M, boost::ref(DE)));
}
