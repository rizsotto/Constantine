// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#include "DeclarationCollector.hpp"

#include <boost/bind.hpp>
#include <boost/range.hpp>
#include <boost/range/algorithm/transform.hpp>

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
        if (clang::CastExpr const * const CE = clang::dyn_cast<clang::CastExpr const>(E)) {
            E = CE->getSubExpr();
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

std::set<clang::Expr const *> CollectRefereeExpr(clang::Expr const * const E) {
    std::set<clang::Expr const *> Result;

    std::set<clang::Expr const *> Works;
    Works.insert(E);

    while (! Works.empty()) {
        clang::Expr const * const Current = *(Works.begin());
        Works.erase(Works.begin());

        if (clang::Expr const * const Stripped = StripExpr(Current)) {
            if (clang::dyn_cast<clang::DeclRefExpr const>(Stripped)) {
                Result.insert(Stripped);
            } else if (clang::MemberExpr const * ME = clang::dyn_cast<clang::MemberExpr const>(Stripped)) {
                // Dig into member variable access to register the outer variable
                while (ME) {
                    clang::Expr const * const Child = ME->getBase();
                    if (clang::MemberExpr const * const Candidate = clang::dyn_cast<clang::MemberExpr const>(Child)) {
                        ME = Candidate;
                        continue;
                    }
                    break;
                }
                Result.insert(ME);
            } else if (clang::AbstractConditionalOperator const * const ACO = clang::dyn_cast<clang::AbstractConditionalOperator const>(Stripped)) {
                Works.insert(ACO->getTrueExpr());
                Works.insert(ACO->getFalseExpr());
            }
        }
    }
    return Result;
}

clang::DeclaratorDecl const * GetDeclarationFromExpr(clang::Expr const * const E) {
    clang::ValueDecl const * RefVal = 0;

    if (clang::DeclRefExpr const * const DRE = clang::dyn_cast<clang::DeclRefExpr const>(E)) {
        RefVal = DRE->getDecl();
    } else if (clang::MemberExpr const * const ME = clang::dyn_cast<clang::MemberExpr const>(E)) {
        RefVal = ME->getMemberDecl();
    }

    if (RefVal) {
        return clang::dyn_cast<clang::DeclaratorDecl const>(RefVal);
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

Variables GetReferedVariables(clang::DeclaratorDecl const * const D) {
    Variables Result;

    Variables Works;
    Works.insert(D);

    while (! Works.empty()) {
        // get the current element
        clang::DeclaratorDecl const * const Current = *(Works.begin());
        Works.erase(Works.begin());
        // current element goes into results
        if (Current) {
            Result.insert(Current);
        } else {
            continue;
        }
        // check is it reference or pointer type
        {
            clang::QualType const & T = Current->getType();
            if (! ((*T).isReferenceType() || (*T).isPointerType())) {
                continue;
            }
        }
        // check is it refer to a variable
        if (clang::VarDecl const * const V = clang::dyn_cast<clang::VarDecl const>(Current)) {
            // get the initialization expression
            std::set<clang::Expr const *> const & Es = CollectRefereeExpr(V->getInit());
            boost::transform(Es, std::inserter(Works, Works.begin()), &GetDeclarationFromExpr);
        }
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
                break;
            }
        }
    }
    return Members;
}
