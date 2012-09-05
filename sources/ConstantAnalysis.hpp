// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#ifndef _constant_analysis_hpp_
#define _constant_analysis_hpp_

#include <list>
#include <map>

#include <clang/AST/AST.h>
#include <clang/Basic/Diagnostic.h>


// This class tracks the usage of variables in a statement body to see
// if they are never written to, implying that they constant.
class ConstantAnalysis {
public:
    typedef std::list<clang::SourceRange>  Locations;
    typedef std::map<clang::VarDecl const *, Locations> Variables;

public:
    static ConstantAnalysis AnalyseThis(clang::Stmt const &);

    bool WasChanged(clang::VarDecl const *) const;
    bool WasReferenced(clang::VarDecl const *) const;

    void Debug(clang::DiagnosticsEngine &) const;

private:
    Variables Changed;
    Variables Used;
};

#endif // _constant_analysis_hpp_
