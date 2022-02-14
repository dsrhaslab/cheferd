# ---------------------------------------------------------------------------- #

cmake_minimum_required(VERSION 3.11)
include(FetchContent)

# ---------------------------------------------------------------------------- #

project(cheferd VERSION 1.0.0 DESCRIPTION "Shepherd: A Software-Defined Storage Control Plane")

# ---------------------------------------------------------------------------- #
# include(../cmake/common.cmake)
cmake_minimum_required(VERSION 3.5.1)

set (CMAKE_CXX_STANDARD 17)

if(MSVC)
    add_definitions(-D_WIN32_WINNT=0x600)
endif()

find_package(Threads REQUIRED)

if(GRPC_AS_SUBMODULE)
    # One way to build a projects that uses gRPC is to just include the
    # entire gRPC project tree via "add_subdirectory".
    # This approach is very simple to use, but the are some potential
    # disadvantages:
    # * it includes gRPC's CMakeLists.txt directly into your build script
    #   without and that can make gRPC's internal setting interfere with your
    #   own build.
    # * depending on what's installed on your system, the contents of submodules
    #   in gRPC's third_party/* might need to be available (and there might be
    #   additional prerequisites required to build them). Consider using
    #   the gRPC_*_PROVIDER options to fine-tune the expected behavior.
    #
    # A more robust approach to add dependency on gRPC is using
    # cmake's ExternalProject_Add (see cmake_externalproject/CMakeLists.txt).

    # Include the gRPC's cmake build (normally grpc source code would live
    # in a git submodule called "third_party/grpc", but this example lives in
    # the same repository as gRPC sources, so we just look a few directories up)
    add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/grpc EXCLUDE_FROM_ALL)
    message(STATUS "Using gRPC via add_subdirectory.")

    # After using add_subdirectory, we can now use the grpc targets directly from
    # this build.
    set(_PROTOBUF_LIBPROTOBUF libprotobuf)
    set(_REFLECTION grpc++_reflection)
    if(CMAKE_CROSSCOMPILING)
        find_program(_PROTOBUF_PROTOC protoc)
    else()
        set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
    endif()
    set(_GRPC_GRPCPP grpc++)
    if(CMAKE_CROSSCOMPILING)
        find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
    else()
        set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
    endif()
elseif(GRPC_FETCHCONTENT)
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
            GIT_TAG        vGRPC_TAG_VERSION_OF_YOUR_CHOICE)
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
else()
    # This branch assumes that gRPC and all its dependencies are already installed
    # on this system, so they can be located by find_package().

    # Find Protobuf installation
    # Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
    set(protobuf_MODULE_COMPATIBLE TRUE)
    find_package(Protobuf CONFIG REQUIRED)
    message(STATUS "Using protobuf ${Protobuf_VERSION}")

    set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
    set(_REFLECTION gRPC::grpc++_reflection)
    if(CMAKE_CROSSCOMPILING)
        find_program(_PROTOBUF_PROTOC protoc)
    else()
        set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
    endif()

    # Find gRPC installation
    # Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
    find_package(gRPC CONFIG REQUIRED)
    message(STATUS "Using gRPC ${gRPC_VERSION}")

    set(_GRPC_GRPCPP gRPC::grpc++)
    if(CMAKE_CROSSCOMPILING)
        find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
    else()
        set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
    endif()
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
find_package(Boost REQUIRED COMPONENTS program_options)

include_directories(${Boost_INCLUDE_DIR})
# ---------------------------------------------------------------------------- #

# ---------------------------------------------------------------------------- #

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# SET (CMAKE_CXX_COMPILER "/usr/bin/clang++")

# Setup the options that CMake can take in
option(SHEPHERD_INSTALL "Install Shepherd's header and library" ON)
option(SHEPHERD_BUILD_TESTS "Build Shepherd's unit tests" ON)

# Setup the basic C++ Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Test whether -Wthread-safety is available. See
# https://clang.llvm.org/docs/ThreadSafetyAnalysis.html
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wthread-safety HAVE_CLANG_THREAD_SAFETY)

# ---------------------------------------------------------------------------- #
# shepherd

# Executable for the Shepherd control plane (use without tests)
add_executable(cheferd "" )

# Library for the Shepherd control plane (use with tests)
# add_library(shepherd-library "")

# set_target_properties(
#     shepherd-library
#     PROPERTIES
#     VERSION ${PROJECT_VERSION}
#     SOVERSION ${PROJECT_VERSION_MAJOR}
# )

target_include_directories(cheferd PRIVATE include)

if (HAVE_CLANG_THREAD_SAFETY)
    target_compile_options(cheferd PUBLIC -Wthread-safety)
endif (HAVE_CLANG_THREAD_SAFETY)

target_sources(
        cheferd
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include/shepherd/controller/control_application.hpp
        ${PROJECT_SOURCE_DIR}/include/shepherd/controller/core_control_application.hpp
        ${PROJECT_SOURCE_DIR}/include/shepherd/controller/local_control_application.hpp
        ${PROJECT_SOURCE_DIR}/include/shepherd/controller/controller.hpp
        ${PROJECT_SOURCE_DIR}/include/shepherd/controller/system_admin.hpp
        ${PROJECT_SOURCE_DIR}/include/shepherd/networking/connection_manager.hpp
        ${PROJECT_SOURCE_DIR}/include/shepherd/networking/interface_definitions.hpp
        ${PROJECT_SOURCE_DIR}/include/shepherd/networking/paio_interface.hpp
        ${PROJECT_SOURCE_DIR}/include/shepherd/networking/local_interface.hpp
        ${PROJECT_SOURCE_DIR}/include/shepherd/networking/southbound_interface.hpp
        include/shepherd/networking/stage_response/stage_response.hpp
        include/shepherd/networking/stage_response/stage_response_ack.hpp
        include/shepherd/networking/stage_response/stage_response_handshake.hpp
        include/shepherd/networking/stage_response/stage_response_stats_entity.hpp
        include/shepherd/networking/stage_response/stage_response_stats_global.hpp
        include/shepherd/networking/stage_response/stage_response_stats.hpp
        include/shepherd/networking/stage_response/stage_response_stat.hpp
        ${PROJECT_SOURCE_DIR}/include/shepherd/session/data_plane_session.hpp
        ${PROJECT_SOURCE_DIR}/include/shepherd/session/local_controller_session.hpp
        ${PROJECT_SOURCE_DIR}/include/shepherd/session/policy_generator.hpp
        ${PROJECT_SOURCE_DIR}/include/shepherd/utils/context_propagation_definitions.hpp
        ${PROJECT_SOURCE_DIR}/include/shepherd/utils/logging.hpp
        ${PROJECT_SOURCE_DIR}/include/shepherd/utils/options.hpp
        ${PROJECT_SOURCE_DIR}/include/shepherd/utils/rules_file_parser.hpp
        ${PROJECT_SOURCE_DIR}/include/shepherd/utils/status.hpp
)

target_sources(
        cheferd
        PRIVATE
        src/controller/core_control_application.cpp
        src/controller/local_control_application.cpp
        src/controller/controller.cpp
        src/controller/controller_exec.cpp
        src/controller/system_admin.cpp
        src/networking/connection_manager.cpp
        src/networking/paio_interface.cpp
        src/networking/local_interface.cpp
        src/networking/stage_response/stage_response.cpp
        src/networking/stage_response/stage_response_ack.cpp
        src/networking/stage_response/stage_response_handshake.cpp
        src/networking/stage_response/stage_response_stats_entity.cpp
        src/networking/stage_response/stage_response_stats_global.cpp
        src/networking/stage_response/stage_response_stats.cpp
        src/networking/stage_response/stage_response_stat.cpp
        src/session/data_plane_session.cpp
        src/session/local_controller_session.cpp
        src/session/policy_generator.cpp
        src/utils/logging.cpp
        src/utils/rules_file_parser.cpp
        src/utils/status.cpp
)

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
        ${Boost_LIBRARIES}
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
target_include_directories(cheferd PRIVATE ${asio_SOURCE_DIR}/asio/include)

# ---------------------------------------------------------------------------- #
# tests

# if (SHEPHERD_BUILD_TESTS)
#     enable_testing()
#
#     function(shepherd_test test_file)
#         get_filename_component(test_target_name "${test_file}" NAME_WE)

#         add_executable("${test_target_name}" "")
#         target_sources("${test_target_name}"
#             PRIVATE
#             "${test_file}"
#             )

#         target_link_libraries("${test_target_name}" shepherd)

#         add_test(NAME "${test_target_name}" COMMAND "${test_target_name}")
#     endfunction(shepherd_test)

#     shepherd_test("tests/shepherd_rules_file_parser_test.cpp")

# endif (SHEPHERD_BUILD_TESTS)

# ---------------------------------------------------------------------------- #
# install

if (SHEPHERD_INSTALL)
    include(GNUInstallDirs)
    install(TARGETS cheferd
            EXPORT shepherdTargets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )
endif (SHEPHERD_INSTALL)




