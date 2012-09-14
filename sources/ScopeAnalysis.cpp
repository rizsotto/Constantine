// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#include "ScopeAnalysis.hpp"

#include <algorithm>
#include <functional>
#include <iterator>

#include <boost/bind.hpp>

#include <clang/AST/RecursiveASTVisitor.h>

namespace {

// Collect named variables and do emit diagnostic messages for tests.
class VerboseVariableCollector {
public:
    VerboseVariableCollector(ScopeAnalysis::Variables & Out)
        : Results(Out)
    { }

protected:
    void AddToResults(clang::Decl const * const D, clang::SourceRange const & Location) {
        if (clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl const>(D)) {
            ScopeAnalysis::Variables::iterator It = Results.find(VD);
            if (Results.end() == It) {
                std::pair<ScopeAnalysis::Variables::iterator, bool> R =
                    Results.insert(ScopeAnalysis::Variables::value_type(VD, ScopeAnalysis::Locations()));
                It = R.first;
            }
            ScopeAnalysis::Locations & Ls = It->second;
            Ls.push_back(Location);
        }
    }

private:
    ScopeAnalysis::Variables & Results;
};

// Collect all variables which were mutated in the given scope.
// (The scope is given by the TraverseStmt method.)
class VariableChangeCollector
    : public VerboseVariableCollector
    , public clang::RecursiveASTVisitor<VariableChangeCollector> {
public:
    VariableChangeCollector(ScopeAnalysis::Variables & Out)
        : VerboseVariableCollector(Out)
        , clang::RecursiveASTVisitor<VariableChangeCollector>()
    { }

    // Assignments are mutating variables.
    bool VisitBinaryOperator(clang::BinaryOperator const * const Stmt) {
        clang::Decl const * const LHSDecl =
            GetDeclaration(Stmt->getLHS()->IgnoreParenCasts());
        if (!LHSDecl)
            return true;

        switch (Stmt->getOpcode()) {
        case clang::BO_Assign:
        case clang::BO_MulAssign:
        case clang::BO_DivAssign:
        case clang::BO_RemAssign:
        case clang::BO_AddAssign:
        case clang::BO_SubAssign:
        case clang::BO_ShlAssign:
        case clang::BO_ShrAssign:
        case clang::BO_AndAssign:
        case clang::BO_XorAssign:
        case clang::BO_OrAssign:
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
            GetDeclaration(Stmt->getSubExpr()->IgnoreParenCasts());
        if (!D)
            return true;

        switch (Stmt->getOpcode()) {
        case clang::UO_PostInc:
        case clang::UO_PostDec:
        case clang::UO_PreInc:
        case clang::UO_PreDec:
            AddToResults(D, Stmt->getSourceRange());
            break;
        default:
            break;
        }
        return true;
    }

    // Variables potentially mutated when you pass by-pointer or by-reference.
    bool VisitCallExpr(clang::CallExpr const * const Stmt) {
        std::for_each(Stmt->arg_begin(), Stmt->arg_end(),
            boost::bind(&VariableChangeCollector::AddToResultsWhenReferedWithoutCast, boost::ref(this), _1));
        return true;
    }

    // Variables are mutated if non-const member function called.
    bool VisitMemberExpr(clang::MemberExpr const * const Stmt) {
        clang::Type const * const T = Stmt->getMemberDecl()->getType().getCanonicalType().getTypePtr();
        if (clang::FunctionProtoType const * const F = T->getAs<clang::FunctionProtoType>()) {
            if (! (F->getTypeQuals() & clang::Qualifiers::Const) ) {
                AddToResultsWhenReferedWithoutCast(Stmt);
            }
        }
        return true;
    }

private:
    /*** source looks like this ***
        $ cat ../show.cpp

        void by_pointer(int *) { }

        void by_const_pointer(int const *) { }

        void by_ref(int &) { }

        void by_const_ref(int const &) { }

        void by_value(int) { }

        void test() {
            int k = 0;
            int * kptr = &k;

            by_pointer(kptr);
            by_const_pointer(kptr);
            by_ref(k);
            by_const_ref(k);
            by_value(k);
        }

        *** AST looks like this ***
        $ clang++ -cc1 -ast-dump ../show.cpp

        void test() (CompoundStmt 0x40d3c48 <./show.cpp:12:13, line:21:1>
          (DeclStmt 0x40d3558 <line:13:5, col:14>
            0x40d34e0 "int k =
              (IntegerLiteral 0x40d3538 <col:13> 'int' 0)")
          (DeclStmt 0x40d3630 <line:14:5, col:20>
            0x40d3590 "int *kptr =
              (UnaryOperator 0x40d3610 <col:18, col:19> 'int *' prefix '&'
                (DeclRefExpr 0x40d35e8 <col:19> 'int' lvalue Var 0x40d34e0 'k' 'int'))")
          (CallExpr 0x40d3730 <line:16:5, col:20> 'void'
            (ImplicitCastExpr 0x40d3718 <col:5> 'void (*)(int *)' <FunctionToPointerDecay>
              (DeclRefExpr 0x40d36c8 <col:5> 'void (int *)' lvalue Function 0x40a6ca0 'by_pointer' 'void (int *)'))
            (ImplicitCastExpr 0x40d3760 <col:16> 'int *' <LValueToRValue>
              (DeclRefExpr 0x40d36a0 <col:16> 'int *' lvalue Var 0x40d3590 'kptr' 'int *')))
          (CallExpr 0x40d3860 <line:17:5, col:26> 'void'
            (ImplicitCastExpr 0x40d3848 <col:5> 'void (*)(const int *)' <FunctionToPointerDecay>
              (DeclRefExpr 0x40d37f8 <col:5> 'void (const int *)' lvalue Function 0x40d2e10 'by_const_pointer' 'void (const int *)'))
            (ImplicitCastExpr 0x40d38a8 <col:22> 'const int *' <NoOp>
              (ImplicitCastExpr 0x40d3890 <col:22> 'int *' <LValueToRValue>
                (DeclRefExpr 0x40d37d0 <col:22> 'int *' lvalue Var 0x40d3590 'kptr' 'int *'))))
          (CallExpr 0x40d39b0 <line:18:5, col:13> 'void'
            (ImplicitCastExpr 0x40d3998 <col:5> 'void (*)(int &)' <FunctionToPointerDecay>
              (DeclRefExpr 0x40d3940 <col:5> 'void (int &)' lvalue Function 0x40d2fd0 'by_ref' 'void (int &)'))
            (DeclRefExpr 0x40d3918 <col:12> 'int' lvalue Var 0x40d34e0 'k' 'int'))
          (CallExpr 0x40d3ad0 <line:19:5, col:19> 'void'
            (ImplicitCastExpr 0x40d3ab8 <col:5> 'void (*)(const int &)' <FunctionToPointerDecay>
              (DeclRefExpr 0x40d3a60 <col:5> 'void (const int &)' lvalue Function 0x40d3190 'by_const_ref' 'void (const int &)'))
            (ImplicitCastExpr 0x40d3b00 <col:18> 'const int' lvalue <NoOp>
              (DeclRefExpr 0x40d3a38 <col:18> 'int' lvalue Var 0x40d34e0 'k' 'int')))
          (CallExpr 0x40d3c00 <line:20:5, col:15> 'void'
            (ImplicitCastExpr 0x40d3be8 <col:5> 'void (*)(int)' <FunctionToPointerDecay>
              (DeclRefExpr 0x40d3b98 <col:5> 'void (int)' lvalue Function 0x40d3320 'by_value' 'void (int)'))
            (ImplicitCastExpr 0x40d3c30 <col:14> 'int' <LValueToRValue>
              (DeclRefExpr 0x40d3b70 <col:14> 'int' lvalue Var 0x40d34e0 'k' 'int'))))
     */
    inline
    void AddToResultsWhenReferedWithoutCast(clang::Expr const * const Stmt) {
        if (clang::Decl const * const D = GetDeclaration(Stmt)) {
            AddToResults(D, Stmt->getSourceRange());
        }
    }

    // FIXME: extract clang::Decl from an clang::Expr, but only if
    // it is variable access or member access. Don't we have a better
    // name for it? And shall it be that specific, although the name
    // is so generic?
    static clang::Decl const * GetDeclaration(clang::Expr const * const Stmt) {
        if (clang::DeclRefExpr const * const DR = clang::dyn_cast<clang::DeclRefExpr const>(Stmt)) {
            return DR->getDecl();
        } else if (clang::MemberExpr const * const ME = clang::dyn_cast<clang::MemberExpr const>(Stmt)) {
            return GetDeclaration(ME->getBase());
        }
        return 0;
    }
};

// Collect all variables which were accessed in the given scope.
// (The scope is given by the TraverseStmt method.)
class VariableAccessCollector
    : public VerboseVariableCollector
    , public clang::RecursiveASTVisitor<VariableAccessCollector> {
public:
    VariableAccessCollector(ScopeAnalysis::Variables & Out)
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

void Report( ScopeAnalysis::Variables::value_type const & Var
           , char const * const Message
           , clang::DiagnosticsEngine & DE) {
    unsigned const Id = DE.getCustomDiagID(clang::DiagnosticsEngine::Note, Message);
    ScopeAnalysis::Locations const & Ls = Var.second;
    for (ScopeAnalysis::Locations::const_iterator It(Ls.begin()), End(Ls.end()); It != End; ++It) {
        clang::DiagnosticBuilder DB = DE.Report(It->getBegin(), Id);
        DB << Var.first->getNameAsString();
        DB.AddSourceRange(clang::CharSourceRange::getTokenRange(*It));
        DB.setForceEmit();
    }
}

} // namespace anonymous

ScopeAnalysis ScopeAnalysis::AnalyseThis(clang::Stmt const & Stmt) {
    ScopeAnalysis Result;
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

bool ScopeAnalysis::WasChanged(clang::VarDecl const * const Decl) const {
    return (Changed.end() != Changed.find(Decl));
}

bool ScopeAnalysis::WasReferenced(clang::VarDecl const * const Decl) const {
    return (Used.end() != Used.find(Decl));
}

void ScopeAnalysis::DebugChanged(clang::DiagnosticsEngine & DE) const {
    std::for_each(Changed.begin(), Changed.end(),
        boost::bind(Report, _1, "variable '%0' was changed", boost::ref(DE)));
}

void ScopeAnalysis::DebugReferenced(clang::DiagnosticsEngine & DE) const {
    std::for_each(Used.begin(), Used.end(),
        boost::bind(Report, _1, "variable '%0' was used", boost::ref(DE)));
}
