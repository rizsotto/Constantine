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

