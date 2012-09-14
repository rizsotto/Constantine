// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#ifndef _ScopeAnalysis_hpp_
#define _ScopeAnalysis_hpp_

#include <list>
#include <map>

#include <clang/AST/AST.h>
#include <clang/Basic/Diagnostic.h>


// This class tracks the usage of variables in a statement body to see
// if they are never written to, implying that they constant.
class ScopeAnalysis {
public:
    typedef std::list<clang::SourceRange>  Locations;
    typedef std::map<clang::VarDecl const *, Locations> Variables;

public:
    static ScopeAnalysis AnalyseThis(clang::Stmt const &);

    bool WasChanged(clang::VarDecl const *) const;
    bool WasReferenced(clang::VarDecl const *) const;

    void DebugChanged(clang::DiagnosticsEngine &) const;
    void DebugReferenced(clang::DiagnosticsEngine &) const;

private:
    Variables Changed;
    Variables Used;
};

#endif // _ScopeAnalysis_hpp_
