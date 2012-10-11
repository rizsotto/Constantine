// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#ifndef _ScopeAnalysis_hpp_
#define _ScopeAnalysis_hpp_

#include <utility>
#include <list>
#include <map>

#include <clang/AST/AST.h>
#include <clang/Basic/Diagnostic.h>


// This class tracks the usage of variables in a statement body to see
// if they are never written to, implying that they constant.
class ScopeAnalysis {
public:
    typedef std::pair<clang::QualType, clang::SourceRange> UsageRef;
    typedef std::list<UsageRef> UsageRefs;
    typedef std::map<clang::DeclaratorDecl const *, UsageRefs> UsageRefsMap;

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

#endif // _ScopeAnalysis_hpp_
