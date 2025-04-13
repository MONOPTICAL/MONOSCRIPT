cmake_minimum_required(VERSION 3.24) 
project(ms) 

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
  analysis
  transformutils
  instcombine
  instrumentation
  ipo
  vectorize
  scalaropts
  objcarcopts
  passes
  native
  asmprinter
  mcjit
  executionengine
)

add_executable(ms # Create an executable target named ms
    src/main.cpp 
    src/lexer/Lexer.cpp 
    src/lexer/ParsingFunctions.cpp 
    src/parser/Logic.cpp 
    src/parser/Parser.cpp 
    src/parser/Parsers.cpp
    src/runtime/ASTGen.cpp
    src/runtime/CodeGenContext.cpp
    src/runtime/codegen/Declarations.cpp
    src/runtime/codegen/Arrays.cpp
)

llvm_map_components_to_libnames(llvm_libs support core irreader)

target_link_libraries(ms ${llvm_libs})