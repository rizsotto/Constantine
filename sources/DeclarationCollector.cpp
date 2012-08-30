// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#include "DeclarationCollector.hpp"

#include <algorithm>
#include <functional>
#include <iterator>
#include <cassert>

namespace {

bool is_non_const(clang::VarDecl const * const D) {
    return (! (D->getType().getNonReferenceType().isConstQualified()));
}

} // namespace anonymous


DeclarationCollector::DeclarationCollector()
: clang::RecursiveASTVisitor<DeclarationCollector>()
, Vars()
, Ctxs()
{ }

bool DeclarationCollector::VisitBlockDeck(clang::BlockDecl const * const D) {
    return collectDeclarations(D, D);
}
bool DeclarationCollector::VisitFunctionDecl(clang::FunctionDecl const * const D) {
    return collectDeclarations(D, D);
}
bool DeclarationCollector::VisitLinkageSpecDecl(clang::LinkageSpecDecl const * const D) {
    return collectDeclarations(D, D);
}
bool DeclarationCollector::VisitNamespaceDecl(clang::NamespaceDecl const * const D) {
    return collectDeclarations(D, D);
}
bool DeclarationCollector::VisitTagDecl(clang::TagDecl const * const D) {
    return collectDeclarations(D, D);
}
bool DeclarationCollector::VisitTranslationUnitDecl(clang::TranslationUnitDecl const * const D) {
    return collectDeclarations(D, D);
}

VariablesByContext const & DeclarationCollector::getVariablesByContext() const {
    return Vars;
}
ContextsByVariable const & DeclarationCollector::getContextsByVariable() const {
    return Ctxs;
}

void DeclarationCollector::insertContextsByVariable(clang::Stmt const * const S, clang::VarDecl const * const D) {
    if (is_non_const(D)) {
        ContextsByVariable::iterator It = Ctxs.find(D);
        if (Ctxs.end() == It) {
            std::pair<ContextsByVariable::iterator, bool> const Result = Ctxs.insert(ContextsByVariable::value_type(D, Contexts()));
            assert(Result.second);
            It = Result.first;
            assert(Ctxs.end() != It);
        }
        (*It).second.insert(S);
    }
}

void DeclarationCollector::insertVariablesByContext(clang::VarDecl const * const D, clang::Stmt const * const S) {
    if (is_non_const(D)) {
        VariablesByContext::iterator It = Vars.find(S);
        if (Vars.end() == It) {
            std::pair<VariablesByContext::iterator, bool> const Result = Vars.insert(VariablesByContext::value_type(S, Variables()));
            assert(Result.second);
            It = Result.first;
            assert(Vars.end() != It);
        }
        (*It).second.insert(D);
    }
}

bool DeclarationCollector::collectDeclarations(clang::DeclContext const * const DC, clang::Decl const * const D) {
    for (clang::DeclContext const * Cit(DC); 0 != Cit; Cit = Cit->getParent()) {
        for (clang::DeclContext::decl_iterator Dit(Cit->decls_begin()), End(Cit->decls_end());
            End != Dit; ++Dit) {
            if (clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl const>(*Dit)) {
                if (D->hasBody()) {
                    clang::Stmt const * const S = D->getBody();
                    insertContextsByVariable(S, VD);
                    insertVariablesByContext(VD, S);
                }
            }
        }
    }
    return true;
}
