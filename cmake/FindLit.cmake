# output:
#   LIT_FOUND
#   LIT_EXECUTABLE

find_program(LIT_EXECUTABLE
  NAMES lit llvm-lit
  PATHS ENV LLVM_PATH)
if(LIT_EXECUTABLE)
  set(LIT_FOUND 1)
  message(STATUS "lit found : ${LIT_EXECUTABLE}")
endif()
