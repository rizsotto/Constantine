// This file is distributed under MIT-LICENSE. See COPYING for details.

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
