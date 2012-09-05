// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#include "ConstantAnalysis.hpp"

#include <algorithm>
#include <functional>
#include <iterator>

#include <boost/bind.hpp>

#include <clang/AST/RecursiveASTVisitor.h>

namespace {

bool is_non_const(clang::VarDecl const * const D) {
    return (! (D->getType().getNonReferenceType().isConstQualified()));
}

bool is_reference(clang::VarDecl const * const D) {
    return (D->getType().getTypePtr()->isReferenceType());
}

// FIXME: extract clang::Decl from an clang::Expr, but only if
// it is variable access or member access. Don't we have a better
// name for it? And shall it be that specific, although the name
// is so generic?
clang::Decl const * getDecl(clang::Expr const * const Stmt) {
    if (clang::DeclRefExpr const * const DR = clang::dyn_cast<clang::DeclRefExpr const>(Stmt)) {
        return DR->getDecl();
    } else if (clang::MemberExpr const * const ME = clang::dyn_cast<clang::MemberExpr const>(Stmt)) {
        return getDecl(ME->getBase());
    }
    return 0;
}

// Collect named variables and do emit diagnostic messages for tests.
class VerboseVariableCollector {
public:
    VerboseVariableCollector(ConstantAnalysis::Variables & Out)
        : Results(Out)
    { }

protected:
    void AddToResults(clang::Decl const * const D, clang::SourceRange const & Location) {
        if (clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl const>(D)) {
            ConstantAnalysis::Variables::iterator It = Results.find(VD);
            if (Results.end() == It) {
                std::pair<ConstantAnalysis::Variables::iterator, bool> R =
                    Results.insert(ConstantAnalysis::Variables::value_type(VD, ConstantAnalysis::Locations()));
                It = R.first;
            }
            ConstantAnalysis::Locations & Ls = It->second;
            Ls.push_back(Location);
        }
    }

private:
    ConstantAnalysis::Variables & Results;
};

// Collect all variables which were mutated in the given scope.
// (The scope is given by the TraverseStmt method.)
class VariableChangeCollector
    : public VerboseVariableCollector
    , public clang::RecursiveASTVisitor<VariableChangeCollector> {
public:
    VariableChangeCollector(ConstantAnalysis::Variables & Out)
        : VerboseVariableCollector(Out)
        , clang::RecursiveASTVisitor<VariableChangeCollector>()
    { }

    // Assignments are mutating variables.
    bool VisitBinaryOperator(clang::BinaryOperator const * const Stmt) {
        clang::Decl const * const LHSDecl =
            getDecl(Stmt->getLHS()->IgnoreParenCasts());
        if (!LHSDecl)
            return true;

        switch (Stmt->getOpcode()) {
        case clang::BO_Assign: {
            clang::Decl const * const RHSDecl =
                getDecl(Stmt->getRHS()->IgnoreParenCasts());
            if (LHSDecl == RHSDecl) {
                break;
            }
        }
        case clang::BO_AddAssign:
        case clang::BO_SubAssign:
        case clang::BO_MulAssign:
        case clang::BO_DivAssign:
        case clang::BO_AndAssign:
        case clang::BO_OrAssign:
        case clang::BO_XorAssign:
        case clang::BO_ShlAssign:
        case clang::BO_ShrAssign:
            AddToResults(LHSDecl, Stmt->getSourceRange());
            break;
        default:
            break;
        }
        return true;
    }

    // Some operator does mutate variables.
    bool VisitUnaryOperator(clang::UnaryOperator const * const Stmt) {
        clang::Decl const * const D =
            getDecl(Stmt->getSubExpr()->IgnoreParenCasts());
        if (!D)
            return true;

        switch (Stmt->getOpcode()) {
        case clang::UO_PostDec:
        case clang::UO_PostInc:
        case clang::UO_PreDec:
        case clang::UO_PreInc:
        // FIXME: Address-Of ruin the whole pointer business...
        case clang::UO_AddrOf:
            AddToResults(D, Stmt->getSourceRange());
            break;
        default:
            break;
        }
        return true;
    }

    // FIXME: This is the case when a new variable is declared and
    // the current might be marked as non-const if the new variable
    // also not const. This should be represented as dependency graph
    // and not neccessary would mean change on the variable.
    bool VisitDeclStmt(clang::DeclStmt const * const Stmt) {
        clang::DeclGroupRef const & DG = Stmt->getDeclGroup();
        for (clang::DeclGroupRef::const_iterator It(DG.begin()), End(DG.end()); It != End; ++It) {
            checkRefDeclaration(*It);
        }
        return true;
    }

    // Variables potentially mutated when you pass by-pointer or by-reference.
    bool VisitCallExpr(clang::CallExpr const * const Stmt) {
        for (clang::CallExpr::const_arg_iterator AIt(Stmt->arg_begin()), AEnd(Stmt->arg_end()); AIt != AEnd; ++AIt ) {
            insertWhenReferedWithoutCast(*AIt);
        }
        return true;
    }

    // Variables are mutated if non-const member function called.
    bool VisitMemberExpr(clang::MemberExpr const * const Stmt) {
        clang::Type const * const T = Stmt->getMemberDecl()->getType().getCanonicalType().getTypePtr();
        if (clang::FunctionProtoType const * const F = T->getAs<clang::FunctionProtoType>()) {
            if (! (F->getTypeQuals() & clang::Qualifiers::Const) ) {
                insertWhenReferedWithoutCast(Stmt);
            }
        }
        return true;
    }

private:
    inline
    void checkRefDeclaration(clang::Decl const * const Decl) {
        if (clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl const>(Decl)) {
            if (is_reference(VD) && is_non_const(VD)) {
                insertWhenReferedWithoutCast(VD->getInit()->IgnoreParenCasts());
            }
        }
    }

    // FIXME: explain it with clang AST examples.
    inline
    void insertWhenReferedWithoutCast(clang::Expr const * const Stmt) {
        if (clang::Decl const * const D = getDecl(Stmt)) {
            AddToResults(D, Stmt->getSourceRange());
        }
    }
};

// Collect all variables which were accessed in the given scope.
// (The scope is given by the TraverseStmt method.)
class VariableAccessCollector
    : public VerboseVariableCollector
    , public clang::RecursiveASTVisitor<VariableAccessCollector> {
public:
    VariableAccessCollector(ConstantAnalysis::Variables & Out)
        : VerboseVariableCollector(Out)
        , clang::RecursiveASTVisitor<VariableAccessCollector>()
    { }

    // Variable access is a usage of the variable.
    bool VisitDeclRefExpr(clang::DeclRefExpr const * const Stmt) {
        AddToResults(Stmt->getDecl(), Stmt->getSourceRange());
        return true;
    }

    // Member access is a usage of the class.
    bool VisitMemberExpr(clang::MemberExpr const * const Stmt) {
        AddToResults(Stmt->getMemberDecl(), Stmt->getSourceRange());
        return true;
    }
};

std::string CreateMessage(clang::VarDecl const * const D, char const * const Msg) {
    std::string Result;
    Result += "variable '";
    Result += D->getNameAsString();
    Result += "' ";
    Result += Msg;
    return Result;
}

void ReportEach(clang::DiagnosticsEngine & Diagnostic, clang::SourceRange const & Location, std::string const & Message) {
    // FIXME: set level to Note. Currently only Warning and above are printed on the cosole.
    unsigned const Id = Diagnostic.getCustomDiagID(clang::DiagnosticsEngine::Warning, Message);
    clang::DiagnosticBuilder Reporter = Diagnostic.Report(Location.getBegin(), Id);
    Reporter.AddSourceRange(clang::CharSourceRange::getTokenRange(Location));
}

void Report(ConstantAnalysis::Variables::value_type const & It, clang::DiagnosticsEngine & Diagnostic, char const * const Msg) {
    std::string const & Message = CreateMessage(It.first, Msg);
    ConstantAnalysis::Locations const & Ls = It.second;

    std::for_each(Ls.begin(), Ls.end(), boost::bind(ReportEach, boost::ref(Diagnostic), _1, boost::cref(Message)));
}

} // namespace anonymous

ConstantAnalysis ConstantAnalysis::AnalyseThis(clang::Stmt const & Stmt) {
    ConstantAnalysis Result;
    {
        VariableChangeCollector Visitor(Result.Changed);
        Visitor.TraverseStmt(const_cast<clang::Stmt*>(&Stmt));
    }
    {
        VariableAccessCollector Visitor(Result.Used);
        Visitor.TraverseStmt(const_cast<clang::Stmt*>(&Stmt));
    }
    return Result;
}

bool ConstantAnalysis::WasChanged(clang::VarDecl const * const Decl) const {
    return (Changed.end() != Changed.find(Decl));
}

bool ConstantAnalysis::WasReferenced(clang::VarDecl const * const Decl) const {
    return (Used.end() != Used.find(Decl));
}

void ConstantAnalysis::Debug(clang::DiagnosticsEngine & DiagEng) const {
    std::for_each(Changed.begin(), Changed.end(),
        boost::bind(Report, _1, boost::ref(DiagEng), "was changed"));
    std::for_each(Used.begin(), Used.end(),
        boost::bind(Report, _1, boost::ref(DiagEng), "was used"));
}
