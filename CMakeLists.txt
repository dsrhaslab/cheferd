# ---------------------------------------------------------------------------- #
cmake_minimum_required(VERSION 3.11)
include(FetchContent)

# ---------------------------------------------------------------------------- #

project(cheferd VERSION 1.0.1 DESCRIPTION "Cheferd: A Software-Defined Storage Control Plane")

# ---------------------------------------------------------------------------- #

set (CMAKE_CXX_STANDARD 17)

if(MSVC)
    add_definitions(-D_WIN32_WINNT=0x600)
endif()

find_package(Threads REQUIRED)

# Another way is to use CMake's FetchContent module to clone gRPC at
# configure time. This makes gRPC's source code available to your project,
# similar to a git submodule.
message(STATUS "Using gRPC via add_subdirectory (FetchContent).")
include(FetchContent)
FetchContent_Declare(
        grpc
        GIT_REPOSITORY https://github.com/grpc/grpc.git
        # when using gRPC, you will actually set this to an existing tag, such as
        # v1.25.0, v1.26.0 etc..
        # For the purpose of testing, we override the tag used to the commit
        # that's currently under test.
        GIT_TAG        v1.37.0)
FetchContent_MakeAvailable(grpc)

# Since FetchContent uses add_subdirectory under the hood, we can use
# the grpc targets directly from this build.
set(_PROTOBUF_LIBPROTOBUF libprotobuf)
set(_REFLECTION grpc++_reflection)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
set(_GRPC_GRPCPP grpc++)
if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
endif()

# ---------------------------------------------------------------------------- #

# Proto file
get_filename_component(hw_proto "protos/controllers_grpc_interface.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/controllers_grpc_interface.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/controllers_grpc_interface.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/controllers_grpc_interface.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/controllers_grpc_interface.grpc.pb.h")
add_custom_command(
        OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
        DEPENDS "${hw_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# hw_grpc_proto
add_library(hw_grpc_proto
        ${hw_grpc_srcs}
        ${hw_grpc_hdrs}
        ${hw_proto_srcs}
        ${hw_proto_hdrs})
target_link_libraries(hw_grpc_proto
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF})


# ---------------------------------------------------------------------------- #

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# SET (CMAKE_CXX_COMPILER "/usr/bin/clang++")

# Setup the options that CMake can take in
option(cheferd_INSTALL "Install cheferd's header and library" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(CHEFERD_TESTS "Build Cheferd's unit tests" ON)

# Setup the basic C++ Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fsanitize=address -g")


# Test whether -Wthread-safety is available. See
# https://clang.llvm.org/docs/ThreadSafetyAnalysis.html
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wthread-safety HAVE_CLANG_THREAD_SAFETY)

# ---------------------------------------------------------------------------- #
# cheferd


# Library for the cheferd control plane (use with tests)

if (BUILD_SHARED_LIBS)
    message(STATUS "Compiling shared libraries ...")
    add_library(cheferd SHARED "")
else()
    message(STATUS "Compiling static libraries ...")
    add_library(cheferd STATIC "")
endif(BUILD_SHARED_LIBS)

target_compile_options(cheferd PRIVATE ${warn_opts})

set_target_properties(
        cheferd
        PROPERTIES
        # CXX_VISIBILITY_PRESET hidden
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(cheferd PUBLIC include)

if (HAVE_CLANG_THREAD_SAFETY)
    target_compile_options(cheferd PUBLIC -Wthread-safety)
endif (HAVE_CLANG_THREAD_SAFETY)

target_sources(
        cheferd
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include/cheferd/controller/control_application.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/controller/core_control_application.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/controller/local_control_application.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/controller/controller.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/controller/system_admin.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/networking/connection_manager.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/networking/core_connection_manager.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/networking/local_connection_manager.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/networking/interface_definitions.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/networking/paio_interface.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/networking/local_interface.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/networking/southbound_interface.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/networking/stage_response/stage_response.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/networking/stage_response/stage_response_ack.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/networking/stage_response/stage_response_handshake.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/networking/stage_response/stage_response_stats_global.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/networking/stage_response/stage_response_stats.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/networking/stage_response/stage_response_stat.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/session/session.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/session/data_plane_session.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/session/handshake_session.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/session/local_controller_session.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/session/policy_generator.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/utils/context_propagation_definitions.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/utils/logging.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/utils/options.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/utils/rules_file_parser.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/utils/command_line_parser.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/utils/config_file_parser.hpp
        ${PROJECT_SOURCE_DIR}/include/cheferd/utils/status.hpp
)

target_sources(
        cheferd
        PRIVATE
        src/controller/core_control_application.cpp
        src/controller/local_control_application.cpp
        src/controller/controller.cpp
        src/controller/controller_exec.cpp
        src/controller/system_admin.cpp
        src/networking/local_connection_manager.cpp
        src/networking/core_connection_manager.cpp
        src/networking/paio_interface.cpp
        src/networking/local_interface.cpp
        src/networking/stage_response/stage_response.cpp
        src/networking/stage_response/stage_response_ack.cpp
        src/networking/stage_response/stage_response_handshake.cpp
        src/networking/stage_response/stage_response_stats_global.cpp
        src/networking/stage_response/stage_response_stats.cpp
        src/networking/stage_response/stage_response_stat.cpp
        src/session/session.cpp
        src/session/handshake_session.cpp
        src/session/data_plane_session.cpp
        src/session/local_controller_session.cpp
        src/session/policy_generator.cpp
        src/utils/logging.cpp
        src/utils/rules_file_parser.cpp
        src/utils/command_line_parser.cpp
        src/utils/config_file_parser.cpp
        src/utils/status.cpp
)


# ---------------------------------------------------------------------------- #
# > gflags --- commandline flag processing library

if(${CMAKE_VERSION} VERSION_LESS "3.24.0")
    FetchContent_Declare(gflags
            URL             https://github.com/gflags/gflags/archive/refs/tags/v2.2.2.tar.gz
            UPDATE_COMMAND  ""
            INSTALL_COMMAND ""
            )
else ()
    FetchContent_Declare(gflags
            URL             https://github.com/gflags/gflags/archive/refs/tags/v2.2.2.tar.gz
            UPDATE_COMMAND  ""
            INSTALL_COMMAND ""
            DOWNLOAD_EXTRACT_TIMESTAMP NEW
            )
endif ()

FetchContent_MakeAvailable(gflags)
target_link_libraries(cheferd gflags)

# ---------------------------------------------------------------------------- #
# spdlog

FetchContent_Declare(spdlog
        URL https://github.com/gabime/spdlog/archive/v1.8.1.tar.gz
        UPDATE_COMMAND ""
        INSTALL_COMMAND ""
        )

FetchContent_MakeAvailable(spdlog)
target_link_libraries(cheferd spdlog
        hw_grpc_proto
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF}
        )

# ---------------------------------------------------------------------------- #
# asio

FetchContent_Declare(asio
        URL https://github.com/chriskohlhoff/asio/archive/asio-1-18-0.tar.gz
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        UPDATE_COMMAND ""
        INSTALL_COMMAND ""
        )

FetchContent_MakeAvailable(asio)
target_include_directories(cheferd PUBLIC ${asio_SOURCE_DIR}/asio/include)

# ---------------------------------------------------------------------------- #
# yaml

FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG yaml-cpp-0.6.3
)
FetchContent_MakeAvailable(yaml-cpp)
target_link_libraries(cheferd yaml-cpp)

# ---------------------------------------------------------------------------- #

# ---------------------------------------------------------------------------- #
# tests


if (CHEFERD_TESTS)
    message(STATUS "Compiling CHEFERD tests ...")
    enable_testing()

    function(cheferd_test test_file exec_file)
        get_filename_component(test_target_name "${exec_file}" NAME_WE)

        add_executable("${test_target_name}" "")
        target_sources("${test_target_name}"
                PRIVATE
                "${test_file}"
                )

        target_compile_options("${test_target_name}" PRIVATE ${warn_opts})
        target_link_libraries("${test_target_name}" cheferd)

        add_test(NAME "${test_target_name}" COMMAND "${test_target_name}")
    endfunction(cheferd_test)
    #cheferd_test("tests/cheferd_controller_flags_test.cpp" "controller_flags_test")
    cheferd_test("tests/cheferd_housekeeping_rules_file_parser_test.cpp" "housekeeping_rules_file_parser_test")
    cheferd_test("tests/cheferd_control_rules_file_parser_test.cpp" "control_rules_file_parser_test")
    cheferd_test("tests/cheferd_local_controller_connection_test.cpp" "local_controller_connection_test")
    cheferd_test("tests/cheferd_data_plane_connection_test.cpp" "data_plane_connection_test")
    cheferd_test("tests/cheferd_collect_statistics_test.cpp" "collect_statistics_test")

endif (CHEFERD_TESTS)



# ---------------------------------------------------------------------------- #
# install

add_executable("cheferd_exec" "")
target_sources("cheferd_exec"
        PRIVATE
        src/controller/controller_exec.cpp
        )

target_compile_options(cheferd_exec PRIVATE ${warn_opts})
target_link_libraries(cheferd_exec cheferd)

if (cheferd_INSTALL)
    message(STATUS "Installing libcheferd ...")
    include(GNUInstallDirs)
    install(TARGETS cheferd
            EXPORT cheferdTargets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )
    install(DIRECTORY include/cheferd TYPE INCLUDE)
endif (cheferd_INSTALL)
