set(SYMBOL_FILE PrintFunctionNames.exports)

set (CLANG_LIBS
	libclang
	clangFrontend
	clangAST
	clangAnalysis
	clangBasic
	clangCodeGen
	clangDriver
	clangFrontendTool
	clangIndex
	clangLex
	clangParse
	clangRewrite
	clangSema
	clangSerialization
	clangStaticAnalyzerCheckers
	clangStaticAnalyzerCore
	clangStaticAnalyzerFrontend
)

set (USER_LIBS
	pthread
)

add_clang_plugin(PrintFunctionNames PrintFunctionNames.cpp)

set_target_properties(PrintFunctionNames PROPERTIES
	LINKER_LANGUAGE CXX
	PREFIX "")
