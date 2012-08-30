// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#ifndef _constant_analysis_hpp_
#define _constant_analysis_hpp_

#include <set>
#include <boost/shared_ptr.hpp>

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/AST.h>


typedef std::set<clang::VarDecl const *> Variables;

// This class tracks the usage of variables in a statement body to see
// if they are never written to, implying that they constant.
class ConstantAnalysis : public clang::RecursiveASTVisitor<ConstantAnalysis> {
public:
    typedef boost::shared_ptr<ConstantAnalysis const> ConstPtr;
    static ConstPtr AnalyseThis(clang::Stmt const &);

    bool WasChanged(clang::VarDecl const *) const;
    bool WasReferenced(clang::VarDecl const *) const;

    bool VisitBinaryOperator(clang::BinaryOperator const *);
    bool VisitUnaryOperator(clang::UnaryOperator const *);
    bool VisitDeclStmt(clang::DeclStmt const *);
    bool VisitCallExpr(clang::CallExpr const *);
    bool VisitMemberExpr(clang::MemberExpr const *);

private:
    void checkRefDeclaration(clang::Decl const *);
    void insertWhenReferedWithoutCast(clang::Expr const *);

private:
    Variables Changed;
    Variables Used;

private:
    ConstantAnalysis();

    ConstantAnalysis(ConstantAnalysis const &);
    ConstantAnalysis & operator=(ConstantAnalysis const &);
};

#endif // _constant_analysis_hpp_
