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

#include "VariableUsages.hpp"

#include <utility>
#include <list>
#include <map>

#include <clang/AST/AST.h>
#include <clang/Basic/Diagnostic.h>


// This class tracks the usage of variables in a statement body to see
// if they are never written to, implying that they constant.
class ScopeAnalysis {
public:
    static ScopeAnalysis AnalyseThis(clang::Stmt const &);

    bool WasChanged(clang::DeclaratorDecl const *) const;
    bool WasReferenced(clang::DeclaratorDecl const *) const;

    void DebugChanged(clang::DiagnosticsEngine &) const;
    void DebugReferenced(clang::DiagnosticsEngine &) const;

private:
    UsageRefsMap Changed;
    UsageRefsMap Used;
};
