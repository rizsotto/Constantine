// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#include "DeclarationCollector.hpp"

namespace {

void GetVariablesFromRecord(clang::CXXRecordDecl const & Rec, Variables & Out) {
    for (clang::RecordDecl::field_iterator It(Rec.field_begin()), End(Rec.field_end()); It != End; ++It ) {
        if (clang::FieldDecl const * const D = clang::dyn_cast<clang::FieldDecl const>(*It)) {
            Out.insert(D);
        }
    }
}

// sytax given by clang api
bool GetVariablesFromRecord(clang::CXXRecordDecl const * const Rec, void * State) {
    Variables & Out = *((Variables*)State);
    GetVariablesFromRecord(*Rec, Out);
    return true;
}

void GetMethodsFromRecord(clang::CXXRecordDecl const & Rec, Methods & Out) {
    for (clang::CXXRecordDecl::method_iterator It(Rec.method_begin()), End(Rec.method_end()); It != End; ++It) {
        if (clang::CXXMethodDecl const * const D = clang::dyn_cast<clang::CXXMethodDecl const>(*It)) {
            Out.insert(D->getCanonicalDecl());
        }
    }
}

// sytax given by clang api
bool GetMethodsFromRecord(clang::CXXRecordDecl const * const Rec, void * State) {
    Methods & Out = *((Methods*)State);
    GetMethodsFromRecord(*Rec, Out);
    return true;
}

// Strip away parentheses and casts we don't care about.
clang::Expr const * StripExpr(clang::Expr const * E) {
    while (E) {
        if (clang::ParenExpr const * const Paren = clang::dyn_cast<clang::ParenExpr const>(E)) {
            E = Paren->getSubExpr();
            continue;
        }
        if (clang::ImplicitCastExpr const * const ICE = clang::dyn_cast<clang::ImplicitCastExpr const>(E)) {
            E = ICE->getSubExpr();
            continue;
        }
        if (clang::UnaryOperator const * const UnOp = clang::dyn_cast<clang::UnaryOperator const>(E)) {
            E = UnOp->getSubExpr();
            continue;
        }
        if (clang::MaterializeTemporaryExpr const * const M = clang::dyn_cast<clang::MaterializeTemporaryExpr const>(E)) {
            E = M->GetTemporaryExpr();
            continue;
        }
        if (clang::ArraySubscriptExpr const * const ASE = clang::dyn_cast<clang::ArraySubscriptExpr const>(E)) {
            E = ASE->getBase();
            continue;
        }
        break;
    }
    return E;
}

// Dig into member variable access to register the outer variable
clang::ValueDecl const * DigIntoMemberExpr(clang::MemberExpr const * E) {
    while (E) {
        clang::Expr const * const Child = E->getBase();
        if (clang::MemberExpr const * const Candidate = clang::dyn_cast<clang::MemberExpr const>(Child)) {
            E = Candidate;
            continue;
        }
        break;
    }
    return E->getMemberDecl();
}

clang::DeclaratorDecl const * ReferedTo(clang::DeclaratorDecl const * const D) {
    // check is it reference or pointer type
    {
        clang::QualType const & T = D->getType();
        if (! ((*T).isReferenceType() || (*T).isPointerType())) {
            return 0;
        }
    }
    // check is it refer to a variable
    if (clang::VarDecl const * const V = clang::dyn_cast<clang::VarDecl const>(D)) {
        // get the initialization expression
        if (clang::Expr const * const E = StripExpr(V->getInit())) {
            clang::ValueDecl const * RefVal = 0;

            if (clang::DeclRefExpr const * const RE = clang::dyn_cast<clang::DeclRefExpr const>(E)) {
                RefVal = RE->getDecl();
            } else if (clang::MemberExpr const * const ME = clang::dyn_cast<clang::MemberExpr const>(E)) {
                RefVal = DigIntoMemberExpr(ME);
            }

            if (RefVal) {
                return clang::dyn_cast<clang::DeclaratorDecl const>(RefVal);
            }
        }
    }
    return 0;
}


} // namespace anonymous

Variables GetVariablesFromContext(clang::DeclContext const * const F, bool const WithoutArgs) {
    Variables Result;
    for (clang::DeclContext::decl_iterator It(F->decls_begin()), End(F->decls_end()); It != End; ++It ) {
        if (clang::VarDecl const * const D = clang::dyn_cast<clang::VarDecl const>(*It)) {
            if (! (WithoutArgs && (clang::dyn_cast<clang::ParmVarDecl const>(D)))) {
                Result.insert(D);
            }
        }
    }
    return Result;
}

Variables GetVariablesFromRecord(clang::CXXRecordDecl const * const Rec) {
    Variables Result;
    GetVariablesFromRecord(*Rec, Result);
    Rec->forallBases(GetVariablesFromRecord, &Result);
    return Result;
}

Methods GetMethodsFromRecord(clang::CXXRecordDecl const * const Rec) {
    Methods Result;
    GetMethodsFromRecord(*Rec, Result);
    Rec->forallBases(GetMethodsFromRecord, &Result);
    return Result;
}

Variables GetReferedVariables(clang::DeclaratorDecl const * D) {
    Variables Result;
    while ((D = ReferedTo(D))) {
        Result.insert(D);
    }
    return Result;
}

Variables GetMemberVariablesAndReferences(clang::CXXRecordDecl const * const Rec, clang::DeclContext const * const F) {
    Variables Members = GetVariablesFromRecord(Rec);
    Variables const & Locals = GetVariablesFromContext(F);
    for (Variables::const_iterator LoIt(Locals.begin()), LoEnd(Locals.end()); LoIt != LoEnd; ++LoIt) {
        Variables const & Refs = GetReferedVariables(*LoIt);
        for (Variables::const_iterator ReIt(Refs.begin()), ReEnd(Refs.end()); ReIt != ReEnd; ++ReIt) {
            if (Members.count(*ReIt)) {
                Members.insert(Refs.begin(), Refs.end());
                Members.insert(*LoIt);
                break;
            }
        }
    }
    return Members;
}
