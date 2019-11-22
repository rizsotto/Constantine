add_library(constantine_a STATIC
        DeclarationCollector.hpp
        DeclarationCollector.cpp
        IsCXXThisExpr.hpp
        IsFromMainModule.hpp
        ModuleAnalysis.hpp
        ModuleAnalysis.cpp
        ScopeAnalysis.hpp
        ScopeAnalysis.cpp)

target_include_directories(constantine_a PUBLIC ${CLANG_INCLUDE_DIRS})
target_compile_definitions(constantine_a PUBLIC ${CLANG_DEFINITIONS})
target_compile_features(constantine_a PUBLIC cxx_std_14)
target_compile_options(constantine_a PUBLIC -fno-exceptions -fno-rtti)

set_target_properties(constantine_a PROPERTIES
        LINKER_LANGUAGE CXX
        POSITION_INDEPENDENT_CODE ON)
