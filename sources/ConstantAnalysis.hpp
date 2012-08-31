// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#ifndef _constant_analysis_hpp_
#define _constant_analysis_hpp_

#include <set>

#include <clang/AST/AST.h>


// This class tracks the usage of variables in a statement body to see
// if they are never written to, implying that they constant.
class ConstantAnalysis {
public:
    static ConstantAnalysis AnalyseThis(clang::Stmt const &);

    bool WasChanged(clang::VarDecl const *) const;
    bool WasReferenced(clang::VarDecl const *) const;

private:
    typedef std::set<clang::VarDecl const *> Variables;

    Variables Changed;
    Variables Used;
};

#endif // _constant_analysis_hpp_
