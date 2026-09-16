cmake_minimum_required(VERSION 3.24)
project(BondAutomationTraining VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(bond_core
    src/application/application.cpp
    src/simulation/simulation.cpp
    src/training/campaign.cpp
    src/persistence/progress_store.cpp
    src/daily/daily_exercise.cpp
)
target_include_directories(bond_core PUBLIC include)
target_compile_definitions(bond_core PUBLIC BOND_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
if(MSVC)
  target_compile_options(bond_core PRIVATE /W4 /permissive-)
else()
  target_compile_options(bond_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(bond_training src/main.cpp)
target_link_libraries(bond_training PRIVATE bond_core)

include(CTest)
if(BUILD_TESTING)
  add_executable(bond_core_tests tests/core_tests.cpp)
  target_link_libraries(bond_core_tests PRIVATE bond_core)
  add_test(NAME bond_core_tests COMMAND bond_core_tests)
endif()
