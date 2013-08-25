/*  Copyright (C) 2012, 2013  László Nagy
    This file is part of Constantine.

    Constantine implements pseudo const analysis.

    Constantine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Constantine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "VariableUsages.hpp"

#include <functional>
#include <boost/range.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/algorithm/for_each.hpp>

namespace {

static clang::QualType const NoType = clang::QualType();
static clang::SourceRange const NoRange = clang::SourceRange();

// Usage extract method implemented in visitor style.
class UsageExtractor
    : public boost::noncopyable
    , public clang::RecursiveASTVisitor<UsageExtractor> {
public:
    UsageExtractor(VariableUsages::UsageRefsMap & Out, clang::QualType const & InType)
        : boost::noncopyable()
        , clang::RecursiveASTVisitor<UsageExtractor>()
        , Results(Out)
        , State(InType, NoRange)
    { }

private:
    void Capture(clang::Expr const * const E) {
        // do nothing if it already has type info
        if (std::get<0>(State) != NoType)
            return;

        State = std::make_tuple(E->getType(), E->getSourceRange());
    }

    void RegisterUsage(clang::ValueDecl const * const Decl,
                       clang::SourceRange const & Location) {
        std::get<1>(State) = Location;
        if (auto const D = clang::dyn_cast<clang::DeclaratorDecl const>(Decl->getCanonicalDecl())) {
            auto It = Results.find(D);
            if (Results.end() == It) {
                auto const R = Results.insert(VariableUsages::UsageRefsMap::value_type(D, VariableUsages::UsageRefs()));
                It = R.first;
            }
            VariableUsages::UsageRefs & Ls = It->second;
            Ls.push_back(State);
        }
        // reset the state for the next call
        State = std::make_tuple(NoType, NoRange);
    }

public:
    // public visitor method.
    bool VisitCastExpr(clang::CastExpr const * const E) {
        Capture(E);
        return true;
    }

    bool VisitUnaryOperator(clang::UnaryOperator const * const E) {
        switch (E->getOpcode()) {
        case clang::UO_AddrOf:
        case clang::UO_Deref:
            Capture(E);
        default:
            ;
        }
        return true;
    }

    bool VisitDeclRefExpr(clang::DeclRefExpr const * const E) {
        Capture(E);
        RegisterUsage(E->getDecl(), E->getSourceRange());
        return true;
    }

    bool VisitMemberExpr(clang::MemberExpr const * const E) {
        Capture(E);
        RegisterUsage(E->getMemberDecl(), E->getSourceRange());
        return true;
    }

private:
    VariableUsages::UsageRefsMap & Results;
    VariableUsages::UsageRef State;
};

// helper method not to be so verbose.
struct IsItFromMainModule {
    bool operator()(clang::Decl const * const D) const {
        auto const & SM = D->getASTContext().getSourceManager();
        return SM.isFromMainFile(D->getLocation());
    }
    bool operator()(VariableUsages::UsageRefsMap::value_type const & Var) const {
        return this->operator()(Var.first);
    }
};

void DumpUsageMapEntry( VariableUsages::UsageRefsMap::value_type const & Var
           , char const * const Message
           , clang::DiagnosticsEngine & DE) {
    auto const Id = DE.getCustomDiagID(clang::DiagnosticsEngine::Note, Message);
    auto const & Ls = Var.second;
    for (auto const &L : Ls) {
        auto const DB = DE.Report(std::get<1>(L).getBegin(), Id);
        DB << Var.first->getNameAsString();
        DB << std::get<0>(L).getAsString();
        DB.setForceEmit();
    }
}

} // namespace anonymous


VariableUsages::VariableUsages(VariableUsages::UsageRefsMap & Out)
    : boost::noncopyable()
    , Results(Out)
{ }

VariableUsages::~VariableUsages()
{ }

void VariableUsages::Register(clang::Expr const * E, clang::QualType const & Type) {
    clang::Stmt const * const Stmt = E;

    UsageExtractor Visitor(Results, Type);
    Visitor.TraverseStmt(const_cast<clang::Stmt*>(Stmt));
}

void VariableUsages::Report(char const * const M, clang::DiagnosticsEngine & DE) const {
    boost::for_each(Results | boost::adaptors::filtered(IsItFromMainModule()),
        std::bind(DumpUsageMapEntry, std::placeholders::_1, M, std::ref(DE)));
}
