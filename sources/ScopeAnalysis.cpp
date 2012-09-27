// Copyright 2012 by Laszlo Nagy [see file MIT-LICENSE]

#include "ScopeAnalysis.hpp"

#include <algorithm>
#include <functional>
#include <iterator>

#include <boost/bind.hpp>

#include <clang/AST/RecursiveASTVisitor.h>

namespace {

// Usage extract method implemented in visitor style.
class UsageExtractor
    : public clang::RecursiveASTVisitor<UsageExtractor> {
public:
    // Represents the used variable and the type of usage.
    struct Usage {
        clang::VarDecl const * Variable;
        clang::QualType Type;
    };

    static Usage GetUsage(clang::Stmt const & Stmt) {
        Usage Result = { 0, clang::QualType() };
        {
            UsageExtractor Visitor(Result);
            Visitor.TraverseStmt(const_cast<clang::Stmt*>(&Stmt));
        }
        return Result;
    }

protected:
    UsageExtractor(Usage & In)
        : clang::RecursiveASTVisitor<UsageExtractor>()
        , Result(In)
    { }

public:
    bool VisitDeclRefExpr(clang::DeclRefExpr const * const D) {
        if (clang::VarDecl const * const VD = clang::dyn_cast<clang::VarDecl const>(D->getDecl())) {
            Result.Variable = VD;
            Result.Type = D->getType();
        }
        return true;
    }

protected:
    Usage & Result;
};

// Collect named variables and do emit diagnostic messages for tests.
class UsageRefCollector {
public:
    UsageRefCollector(ScopeAnalysis::UsageRefsMap & Out)
        : Results(Out)
    { }

protected:
    void AddToResults(clang::Decl const * const D, clang::SourceRange const & L) {
        UsageExtractor::Usage U =
            { clang::dyn_cast<clang::VarDecl const>(D), clang::QualType() };

        return AddToResults(U, L);
    }

    void AddToResults(UsageExtractor::Usage const & U, clang::SourceRange const & L) {
        if (clang::VarDecl const * const VD = U.Variable) {
            ScopeAnalysis::UsageRefsMap::iterator It = Results.find(VD);
            if (Results.end() == It) {
                std::pair<ScopeAnalysis::UsageRefsMap::iterator, bool> R =
                    Results.insert(ScopeAnalysis::UsageRefsMap::value_type(VD, ScopeAnalysis::UsageRefs()));
                It = R.first;
            }
            ScopeAnalysis::UsageRefs & Ls = It->second;
            Ls.push_back(ScopeAnalysis::UsageRef(U.Type, L));
        }
    }

private:
    ScopeAnalysis::UsageRefsMap & Results;
};

// Collect all variables which were mutated in the given scope.
// (The scope is given by the TraverseStmt method.)
class VariableChangeCollector
    : public UsageRefCollector
    , public clang::RecursiveASTVisitor<VariableChangeCollector> {
public:
    VariableChangeCollector(ScopeAnalysis::UsageRefsMap & Out)
        : UsageRefCollector(Out)
        , clang::RecursiveASTVisitor<VariableChangeCollector>()
    { }

    // Assignments are mutating variables.
    bool VisitBinaryOperator(clang::BinaryOperator const * const Stmt) {
        UsageExtractor::Usage const & U =
            UsageExtractor::GetUsage(*(Stmt->getLHS()));

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
            AddToResults(U, Stmt->getSourceRange());
            break;
        default:
            break;
        }
        return true;
    }

    // Some operator does mutate variables.
    bool VisitUnaryOperator(clang::UnaryOperator const * const Stmt) {
        UsageExtractor::Usage const & U =
            UsageExtractor::GetUsage(*(Stmt->getSubExpr()));

        switch (Stmt->getOpcode()) {
        case clang::UO_PostInc:
        case clang::UO_PostDec:
        case clang::UO_PreInc:
        case clang::UO_PreDec:
            AddToResults(U, Stmt->getSourceRange());
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
        UsageExtractor::Usage const & U =
            UsageExtractor::GetUsage(*(Stmt));

        AddToResults(U, Stmt->getSourceRange());
    }
};

// Collect all variables which were accessed in the given scope.
// (The scope is given by the TraverseStmt method.)
class VariableAccessCollector
    : public UsageRefCollector
    , public clang::RecursiveASTVisitor<VariableAccessCollector> {
public:
    VariableAccessCollector(ScopeAnalysis::UsageRefsMap & Out)
        : UsageRefCollector(Out)
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

void Report( ScopeAnalysis::UsageRefsMap::value_type const & Var
           , char const * const Message
           , clang::DiagnosticsEngine & DE) {
    unsigned const Id = DE.getCustomDiagID(clang::DiagnosticsEngine::Note, Message);
    ScopeAnalysis::UsageRefs const & Ls = Var.second;
    for (ScopeAnalysis::UsageRefs::const_iterator It(Ls.begin()), End(Ls.end()); It != End; ++It) {
        clang::DiagnosticBuilder DB = DE.Report(It->second.getBegin(), Id);
        DB << Var.first->getNameAsString();
        DB << It->first.getAsString();
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
        boost::bind(Report, _1, "variable '%0' with type '%1' was changed", boost::ref(DE)));
}

void ScopeAnalysis::DebugReferenced(clang::DiagnosticsEngine & DE) const {
    std::for_each(Used.begin(), Used.end(),
        boost::bind(Report, _1, "variable '%0' was used", boost::ref(DE)));
}
