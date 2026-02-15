cmake_minimum_required(VERSION 3.10)
project(crust)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(crust INTERFACE)
target_include_directories(crust INTERFACE src/)

enable_testing()

# Add GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

# Find Python interpreter for code generation
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Generate sumtype headers from *.enum.json specs
# Output directory: tests/ (where specs live)
set(ENUM_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/tests")
file(GLOB ENUM_SPECS "${CMAKE_SOURCE_DIR}/tests/*.enum.json")
set(ENUM_HEADERS "")
foreach(spec ${ENUM_SPECS})
  string(REGEX REPLACE "\\.enum\\.json$" ".enum.hpp" header "${spec}")
  list(APPEND ENUM_HEADERS "${header}")
  
  add_custom_command(
    OUTPUT "${header}"
    COMMAND ${Python3_EXECUTABLE}
            "${CMAKE_SOURCE_DIR}/tools/gen_sumtype.py"
            "${spec}"
            "${header}"
    DEPENDS "${spec}"
    COMMENT "Generating ${header}"
    VERBATIM
  )
endforeach()

# Custom target to generate all enum headers (not built by default)
add_custom_target(gen_enum DEPENDS ${ENUM_HEADERS})

# Define test executable
file(GLOB TEST_SOURCES "tests/*.cpp")
add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests gtest_main crust)

# Make tests depend on the generated headers
add_dependencies(tests gen_enum)
target_include_directories(tests PRIVATE "${ENUM_OUTPUT_DIR}")

add_test(NAME tests COMMAND tests)
