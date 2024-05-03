cmake_minimum_required(VERSION 3.10)

project(helmholtz)

find_package (margot_heel_generator REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
margot_heel_generate_interface("${CMAKE_CURRENT_SOURCE_DIR}/margot.json")

add_subdirectory(naive)
add_subdirectory(fpga)

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif("${isSystemDir}" STREQUAL "-1")

add_library(helmholtz SHARED helmholtz.h helmholtz.cpp)
add_library(helmholtz::helmholtz ALIAS helmholtz)

target_include_directories(helmholtz PUBLIC $<BUILD_INTERFACE:${HEADER_DIR}>)
target_link_libraries(helmholtz PUBLIC $<BUILD_INTERFACE:margot::margot_heel_interface>)

target_include_directories(helmholtz PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/naive>
)
target_link_libraries(helmholtz PUBLIC $<BUILD_INTERFACE:naive>)

target_include_directories(helmholtz PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/fpga>
)
target_link_libraries(helmholtz PUBLIC $<BUILD_INTERFACE:fpga>)

target_compile_features(helmholtz PUBLIC cxx_std_17)
set_target_properties(helmholtz PROPERTIES PUBLIC_HEADER "helmholtz.h")

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

install(TARGETS helmholtz naive fpga
    EXPORT helmholtzTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helmholtz
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helmholtz
)

set(CONFIG_INSTALL_PATH "${CMAKE_INSTALL_LIBDIR}/cmake")
write_basic_package_version_file(
    helmholtzConfigVersion.cmake
    VERSION "1.0"
    COMPATIBILITY ExactVersion
)
install(
    EXPORT helmholtzTargets
    FILE helmholtzTargets.cmake
    NAMESPACE helmholtz::
    DESTINATION ${CONFIG_INSTALL_PATH}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/config.cmake.in helmholtzConfig.cmake @ONLY)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/helmholtzConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/helmholtzConfigVersion.cmake
    DESTINATION ${CONFIG_INSTALL_PATH}
)
