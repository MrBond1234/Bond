cmake_minimum_required(VERSION 3.24)
project(BondAutomationTraining VERSION 1.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
  # A local package must not require the Debug CRT simply because it was built from the IDE.
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_library(bond_core
    src/application/application.cpp
    src/application/automation_protocol.cpp
    src/simulation/simulation.cpp
    src/training/campaign.cpp
    src/training/evaluation.cpp
    src/execution/execution.cpp
    src/localization/localizer.cpp
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

add_executable(bond_training WIN32 src/main.cpp)
target_link_libraries(bond_training PRIVATE bond_core)
target_compile_definitions(bond_training PRIVATE BOND_EXECUTION_WORKER_PATH="$<TARGET_FILE:bond_execution_worker>")

add_executable(bond_execution_worker src/execution/worker_main.cpp)
target_compile_definitions(bond_core PRIVATE BOND_EXECUTION_WORKER_PATH="$<TARGET_FILE:bond_execution_worker>")
if(WIN32)
  target_link_libraries(bond_execution_worker PRIVATE advapi32)
endif()
add_dependencies(bond_training bond_execution_worker)
add_custom_command(TARGET bond_training POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/data" "$<TARGET_FILE_DIR:bond_training>/data"
  COMMENT "Copying training data beside the local application")

include(CTest)
if(BUILD_TESTING)
  add_executable(bond_core_tests tests/core_tests.cpp)
  target_link_libraries(bond_core_tests PRIVATE bond_core)
  target_compile_definitions(bond_core_tests PRIVATE BOND_EXECUTION_WORKER_PATH="$<TARGET_FILE:bond_execution_worker>")
  add_dependencies(bond_core_tests bond_execution_worker)
  add_test(NAME bond_core_tests COMMAND bond_core_tests)
endif()

install(TARGETS bond_training bond_execution_worker RUNTIME DESTINATION .)
install(DIRECTORY data/ DESTINATION data)
install(FILES README.md DESTINATION .)
install(DIRECTORY docs/ DESTINATION docs)

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "Bond-Automation-Training")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
include(CPack)
