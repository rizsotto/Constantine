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

#pragma once

#include <boost/noncopyable.hpp>
#include <clang/AST/AST.h>


// Collect variable usages. One variable could have been used multiple
// times with different constness of the given type.
class UsageCollector
    : public boost::noncopyable {
public:
    typedef std::pair<clang::QualType, clang::SourceRange> UsageRef;
    typedef std::list<UsageRef> UsageRefs;
    typedef std::map<clang::DeclaratorDecl const *, UsageRefs> UsageRefsMap;

protected:
    UsageCollector(UsageRefsMap & Out);
    virtual ~UsageCollector();

    void AddToResults(
            clang::Expr const * const Stmt,
            clang::QualType const & Type = clang::QualType());

    void Report(char const * const Message, clang::DiagnosticsEngine &) const;

private:
    UsageRefsMap & Results;
};
