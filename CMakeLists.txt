cmake_minimum_required(VERSION 3.24) 
project(ms) 
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Врубаем определения LLVM чтобы билдил нормально
add_definitions(${LLVM_DEFINITIONS})

# Добавить пути к заголовочным файлам LLVM
include_directories(${LLVM_INCLUDE_DIRS})

# Добавить пути к библиотекам LLVM
link_directories(${LLVM_LIBRARY_DIRS})

# Ну а тут список компонентов LLVM
llvm_map_components_to_libnames(llvm_libs
  core
  support
  irreader
  executionengine
  mcjit             
  orcjit           
  target            
  # X86 
  # -------
  X86CodeGen
  X86AsmParser
  X86Desc
  X86Info
  # -------
  analysis
  transformutils
  instcombine
  instrumentation
  ipo
  vectorize
  scalaropts
  objcarcopts
  passes
  asmprinter        
)

add_executable(ms # Create an executable target named ms
    src/main.cpp 
    src/lexer/Lexer.cpp 
    src/lexer/ParsingFunctions.cpp 
    src/parser/Logic.cpp 
    src/parser/Parser.cpp 
    src/parser/Parsers.cpp
    src/visitors/Register.cpp
    src/visitors/BuiltIn.cpp
    src/visitors/TypeSymbolVisitor/Expressions.cpp
    src/visitors/TypeSymbolVisitor/Statements.cpp
    src/visitors/TypeSymbolVisitor/Types.cpp
    src/visitors/TypeSymbolVisitor/Utils.cpp
    src/visitors/TypeSymbolVisitor/NotFinished.cpp
    src/visitors/TypeSymbolVisitor/StructClass.cpp
    src/visitors/TypeSymbolVisitor/Operators.cpp
    src/linker/Linker.cpp
#   src/runtime/ASTGen.cpp
#   src/runtime/CodeGenContext.cpp
#   src/runtime/codegen/Declarations.cpp
#   src/runtime/codegen/Arrays.cpp
#   src/runtime/codegen/Statements.cpp
)

target_link_libraries(ms PRIVATE ${llvm_libs})