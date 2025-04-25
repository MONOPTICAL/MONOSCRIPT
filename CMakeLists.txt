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

  # СТАНДАРНАЯ БИБЛИОТЕКА
  # ----------------------
  file(GLOB D_SOURCES "${CMAKE_SOURCE_DIR}/mono/*.d") # Получаем все .d файлы в папке std
  set(D_LIB ${CMAKE_BINARY_DIR}/libm_std.so) # Путь к .so библиотеке

  # Создаём .so библиотеку из .d файлов
  add_custom_command(
      OUTPUT ${D_LIB}
      COMMAND dmd -shared -fPIC -of=${D_LIB} ${D_SOURCES}
      DEPENDS ${D_SOURCES}
      COMMENT "Building D shared library: ${D_LIB}"
  )
  add_custom_target(build_dlib ALL DEPENDS ${D_LIB}) # Создаём цель для сборки библиотеки
  add_definitions(-DSTDLIB_SO_PATH="${D_LIB}") # Путь к .so библиотеке
  add_definitions(-DSTDLIB_TOML_PATH="${CMAKE_SOURCE_DIR}/mono/mono.toml") # Путь к toml файлу
  # ----------------------

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
    src/runtime/ASTGen.cpp
    src/runtime/CodeGenContext.cpp
    src/runtime/codegen/Declarations.cpp
    src/runtime/codegen/Arrays.cpp
    src/runtime/codegen/Statements.cpp
)

# Добавляем библиотеку D
add_dependencies(ms build_dlib) 

target_link_libraries(ms PRIVATE ${llvm_libs})