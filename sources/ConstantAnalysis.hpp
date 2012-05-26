// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#ifndef _constant_analysis_hpp_
#define _constant_analysis_hpp_

#include <set>

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/AST.h>


typedef std::set<clang::VarDecl const *> Variables;

class ConstantAnalysis : public clang::RecursiveASTVisitor<ConstantAnalysis> {
public:
    ConstantAnalysis();

    bool VisitBinaryOperator(clang::BinaryOperator const *);
    bool VisitUnaryOperator(clang::UnaryOperator const *);
    bool VisitDeclStmt(clang::DeclStmt const *);
    bool VisitCallExpr(clang::CallExpr const *);
    bool VisitMemberExpr(clang::MemberExpr const *);

    Variables const & getNonConstVariables() const;

private:
    void checkRefDeclaration(clang::Decl const *);
    void insertWhenReferedWithoutCast(clang::Expr const *);

private:
    Variables NonConstants;

private:
    ConstantAnalysis(ConstantAnalysis const &);
    ConstantAnalysis & operator=(ConstantAnalysis const &);
};

#endif // _constant_analysis_hpp_
