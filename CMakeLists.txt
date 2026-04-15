cmake_minimum_required(VERSION 3.14)
project(FurBBS CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(LINK_WITH_SHARED_LIB "link shared library" OFF)

include(GNUInstallDirs)

set(TRPC_ROOT $ENV{TRPC_ROOT})
if(NOT TRPC_ROOT)
    set(TRPC_ROOT "/usr/local/trpc-cpp")
endif()

set(CMAKE_MODULE_PATH "${TRPC_ROOT}/cmake;${CMAKE_MODULE_PATH}")
include(trpc)

find_package(PostgreSQL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread filesystem)
find_package(yaml-cpp REQUIRED)
find_package(spdlog REQUIRED)
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src
    ${PostgreSQL_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
)

file(GLOB_RECURSE SRC_FILES
    "${PROJECT_SOURCE_DIR}/src/*.cc"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/common/security.cc"
    "${PROJECT_SOURCE_DIR}/src/common/content_filter.cc"
    "${PROJECT_SOURCE_DIR}/src/common/captcha_verifier.cc"
    "${PROJECT_SOURCE_DIR}/src/middleware/*.cc"
    "${PROJECT_SOURCE_DIR}/src/repository/customization_repository.cc"
    "${PROJECT_SOURCE_DIR}/src/service_impl/customization_service.h"
    "${PROJECT_SOURCE_DIR}/src/repository/economy_repository.cc"
    "${PROJECT_SOURCE_DIR}/src/repository/economy_repository.h"
    "${PROJECT_SOURCE_DIR}/src/service_impl/economy_service.h"
)

file(GLOB_RECURSE PROTO_FILES
    "${PROJECT_SOURCE_DIR}/proto/*.proto"
)

trpc_compile_proto(
    PROTO_FILES ${PROTO_FILES}
    OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/generated
    OUTPUT_SOURCES PROTO_SRCS
    OUTPUT_HEADERS PROTO_HDRS
)

include_directories(${PROJECT_BINARY_DIR}/generated)

add_executable(furbbs_server
    ${SRC_FILES}
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

target_link_libraries(furbbs_server
    trpc
    ${PostgreSQL_LIBRARIES}
    ${Boost_LIBRARIES}
    CURL::libcurl
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    yaml-cpp
    pq
    pqxx
    fmt
    jwt-cpp
    OpenSSL::SSL
    OpenSSL::Crypto
)

install(TARGETS furbbs_server DESTINATION ${CMAKE_INSTALL_BINDIR})
