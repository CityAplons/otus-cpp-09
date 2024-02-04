cmake_minimum_required(VERSION 3.10)

set(PROJECT_REVISION 1 CACHE INTERNAL "Project revision (CI)")
project(bulk-async
    VERSION 0.0.${PROJECT_REVISION}
    DESCRIPTION "General CMake template for C++ projects by N.Mikhailovskiy"
    HOMEPAGE_URL "https://github.com/CityAplons/cmake-multiplatform-template"
    LANGUAGES CXX
)

set_property(GLOBAL PROPERTY G_CXX_STANDARD 20)
set_property(GLOBAL PROPERTY G_CXX_STANDARD_REQUIRED ON)

# Include utility functions
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(GNUInstallDirs)
include(compiler)
include(test)

# Transparent versioning
include_directories(cmake/version)
configure_file(cmake/version/version.h.in version.h)
configure_file(cmake/version/VERSION.in VERSION)
add_library(version cmake/version/project.cpp)
target_include_directories(version
    PRIVATE "${CMAKE_BINARY_DIR}"
)

# Describe build targets
add_library(async SHARED src/async/async.cpp)
set_target_properties(async PROPERTIES PUBLIC_HEADER "src/async/async.hpp")

add_executable(bulk src/main.cpp)
target_link_libraries(bulk PRIVATE
    version
)

add_executable(client src/async/client/main.cpp)
target_link_libraries(client
    async
)

set_compile_options(async)
set_compile_options(bulk)
set_compile_options(client)
set_compile_options(version)

install(TARGETS bulk client
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(TARGETS async
    ARCHIVE DESTINATION ${CMAKE_INSTALL_BINDIR}/async
    LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}/async
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_BINDIR}/async
)

# Describe unit tests
enable_testing()
add_unit_test(test/test_version.cpp version)

# Packaging
option(MAKE_PACKAGE "Add 'package' and 'package_source' build targets for packages" ON)

if(MAKE_PACKAGE)
    add_subdirectory(cmake/packaging)
endif()