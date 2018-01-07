cmake_minimum_required(VERSION 2.8.11)

project(astrochrono)

# Project release version
set(ASTROCHRONO_VERSION_MAJOR 0)
set(ASTROCHRONO_VERSION_MINOR 1)
set(ASTROCHRONO_VERSION_PATCH 0)
set(ASTROCHRONO_VERSION "${ASTROCHRONO_VERSION_MAJOR}.${ASTROCHRONO_VERSION_MINOR}.${ASTROCHRONO_VERSION_PATCH}")

# Add definitions
add_definitions (-std=c++14)
add_definitions (-Wall)
add_definitions (-Werror)

add_library(astrochrono SHARED astrochrono.cc)

# Library versioning
set_target_properties(astrochrono PROPERTIES VERSION ${ASTROCHRONO_VERSION})
# Install library
install(TARGETS astrochrono
    DESTINATION lib)
# Install header files
install (FILES ${PROJECT_SOURCE_DIR}/astrochrono.h
    DESTINATION include)

# Testing
option(ASTROCHRONO_WITH_TESTS "Build ASTROCHRONO unit test programs." ON)

if (ASTROCHRONO_WITH_TESTS)
    enable_testing()

    # finds boost, triggers an error otherwise
    find_package(Boost REQUIRED COMPONENTS unit_test_framework)

    # creates the executable
    add_executable(test_executable test.cc)
    # indicates the include paths
    target_include_directories(test_executable PRIVATE ${Boost_INCLUDE_DIRS})
    # indicates the shared library variant
    target_compile_definitions(test_executable PRIVATE "BOOST_TEST_DYN_LINK=1")
    # indicates the link paths
    target_link_libraries(test_executable astrochrono ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

    # declares a test with our executable
    add_test(NAME basic_test COMMAND test_executable)
endif (ASTROCHRONO_WITH_TESTS)