/*  Copyright (C) 2012-2014  László Nagy
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

#pragma once

#include <clang/AST/AST.h>

#include <tuple>
#include <list>
#include <map>


// Collect variable usages. One variable could have been used multiple
// times with different constness of the given type.

typedef std::tuple<clang::QualType, clang::SourceRange> UsageRef;
typedef std::list<UsageRef> UsageRefs;
typedef std::map<clang::DeclaratorDecl const *, UsageRefs> UsageRefsMap;

void Register(
        UsageRefsMap & Results,
        clang::Expr const * const Stmt,
        clang::QualType const & Type = clang::QualType());


struct IsItFromMainModule {
    bool operator()(clang::Decl const * const D) const {
        auto const & SM = D->getASTContext().getSourceManager();
        return SM.isInMainFile(D->getLocation());
    }
    bool operator()(UsageRefsMap::value_type const & Var) const {
        return this->operator()(Var.first);
    }
};

template <unsigned N>
void DumpUsageMapEntry(UsageRefsMap::value_type const & Var
           , char const (&Message)[N]
           , clang::DiagnosticsEngine & DE);

template <unsigned N>
void DumpUsageMapEntry(UsageRefsMap::value_type const & Var
           , char const (&Message)[N]
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
