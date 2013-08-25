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

// Usage extract method implemented in visitor style.
class UsageExtractor
    : public boost::noncopyable
    , public clang::RecursiveASTVisitor<UsageExtractor> {
public:
    UsageExtractor(VariableUsages::UsageRefsMap & Out, clang::QualType const & InType)
        : boost::noncopyable()
        , clang::RecursiveASTVisitor<UsageExtractor>()
        , Results(Out)
        , WorkingType(InType)
    { }

private:
    void ResetType() {
        static clang::QualType const Empty = clang::QualType();

        WorkingType = Empty;
    }

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
        if (auto const D = clang::dyn_cast<clang::DeclaratorDecl const>(Decl->getCanonicalDecl())) {
            auto It = Results.find(D);
            if (Results.end() == It) {
                auto const R = Results.insert(VariableUsages::UsageRefsMap::value_type(D, VariableUsages::UsageRefs()));
                It = R.first;
            }
            VariableUsages::UsageRefs & Ls = It->second;
            Ls.push_back(VariableUsages::UsageRef(WorkingType, Location));
        }
        // reset the state for the next call
        ResetType();
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
    VariableUsages::UsageRefsMap & Results;
    clang::QualType WorkingType;
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

void VariableUsages::AddToResults(clang::Expr const * E, clang::QualType const & Type) {
    clang::Stmt const * const Stmt = E;

    UsageExtractor Visitor(Results, Type);
    Visitor.TraverseStmt(const_cast<clang::Stmt*>(Stmt));
}

void VariableUsages::Report(char const * const M, clang::DiagnosticsEngine & DE) const {
    boost::for_each(Results | boost::adaptors::filtered(IsItFromMainModule()),
        std::bind(DumpUsageMapEntry, std::placeholders::_1, M, std::ref(DE)));
}
