// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#ifndef _declaration_collector_hpp_
#define _declaration_collector_hpp_

#include <set>
#include <map>

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/AST.h>


typedef std::set<clang::Stmt const *> Contexts;
typedef std::set<clang::VarDecl const *> Variables;
typedef std::map<clang::Stmt const *, Variables> VariablesByContext;
typedef std::map<clang::VarDecl const *, Contexts> ContextsByVariable;

class DeclarationCollector : public clang::RecursiveASTVisitor<DeclarationCollector> {
public:
    DeclarationCollector();

    bool VisitBlockDeck(clang::BlockDecl const *);
    bool VisitFunctionDecl(clang::FunctionDecl const *);
    bool VisitLinkageSpecDecl(clang::LinkageSpecDecl const *);
    bool VisitNamespaceDecl(clang::NamespaceDecl const *);
    bool VisitTagDecl(clang::TagDecl const *);
    bool VisitTranslationUnitDecl(clang::TranslationUnitDecl const *);

    VariablesByContext const & getVariablesByContext() const;
    ContextsByVariable const & getContextsByVariable() const;

private:
    void insertContextsByVariable(clang::Stmt const *, clang::VarDecl const *);
    void insertVariablesByContext(clang::VarDecl const *, clang::Stmt const *);
    bool collectDeclarations(clang::DeclContext const *, clang::Decl const *);

private:
    VariablesByContext Vars;
    ContextsByVariable Ctxs;

private:
    DeclarationCollector(DeclarationCollector const &);
    DeclarationCollector & operator=(DeclarationCollector const &);
};

#endif // _declaration_collector_hpp_
