// This file is distributed under MIT-LICENSE. See COPYING for details.

#include "UsageCollector.hpp"
#include "IsCXXThisExpr.hpp"

#include <boost/bind.hpp>
#include <boost/range.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/algorithm/for_each.hpp>

namespace {

// Usage extract method implemented in visitor style.
class UsageExtractor
    : public boost::noncopyable
    , public clang::RecursiveASTVisitor<UsageExtractor> {
public:
    UsageExtractor(ScopeAnalysis::UsageRefsMap & Out, clang::QualType const & InType)
        : boost::noncopyable()
        , clang::RecursiveASTVisitor<UsageExtractor>()
        , Results(Out)
        , WorkingType(InType)
    { }

private:
    void SetType(clang::QualType const & In) {
        static clang::QualType const Empty = clang::QualType();

        if (Empty != WorkingType) {
            return;
        }
        if (Empty == In) {
            return;
        }
        WorkingType = In;
    }

    void AddToUsageMap(clang::ValueDecl const * const Decl,
                       clang::QualType const & Type,
                       clang::SourceRange const & Location) {
        SetType(Type);
        if (clang::DeclaratorDecl const * const D =
                clang::dyn_cast<clang::DeclaratorDecl const>(Decl->getCanonicalDecl())) {
            ScopeAnalysis::UsageRefsMap::iterator It = Results.find(D);
            if (Results.end() == It) {
                std::pair<ScopeAnalysis::UsageRefsMap::iterator, bool> const R =
                    Results.insert(ScopeAnalysis::UsageRefsMap::value_type(D, ScopeAnalysis::UsageRefs()));
                It = R.first;
            }
            ScopeAnalysis::UsageRefs & Ls = It->second;
            Ls.push_back(ScopeAnalysis::UsageRef(WorkingType, Location));
        }
        WorkingType = clang::QualType();
    }

public:
    // public visitor method.
    bool VisitCastExpr(clang::CastExpr const * const E) {
        SetType(E->getType());
        return true;
    }

    bool VisitUnaryOperator(clang::UnaryOperator const * const E) {
        switch (E->getOpcode()) {
        case clang::UO_AddrOf:
        case clang::UO_Deref:
            SetType(E->getType());
        default:
            ;
        }
        return true;
    }

    bool VisitDeclRefExpr(clang::DeclRefExpr const * const E) {
        AddToUsageMap(E->getDecl(), E->getType(), E->getSourceRange());
        return true;
    }

    bool VisitMemberExpr(clang::MemberExpr const * const E) {
        AddToUsageMap(E->getMemberDecl(), E->getType(), E->getSourceRange());
        return true;
    }

private:
    ScopeAnalysis::UsageRefsMap & Results;
    clang::QualType WorkingType;
};

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
    : boost::noncopyable()
    , Results(Out)
{ }

UsageCollector::~UsageCollector()
{ }

void UsageCollector::AddToResults(clang::Expr const * E, clang::QualType const & Type) {
    clang::Stmt const * const Stmt = E;

    UsageExtractor Visitor(Results, Type);
    Visitor.TraverseStmt(const_cast<clang::Stmt*>(Stmt));
}

void UsageCollector::Report(char const * const M, clang::DiagnosticsEngine & DE) const {
    boost::for_each(Results | boost::adaptors::filtered(IsItFromMainModule()),
        boost::bind(DumpUsageMapEntry, _1, M, boost::ref(DE)));
}
