cmake_minimum_required(VERSION 3.20)
project(ditto_backend CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")

find_package(Drogon REQUIRED)
find_package(OpenSSL REQUIRED)

# Prefer system-installed nlohmann_json (brew install nlohmann-json) so
# jwt-cpp doesn't try to fetch its own copy (which bundles an old CMakeLists).
find_package(nlohmann_json QUIET)

include(FetchContent)
FetchContent_Declare(
    jwt-cpp
    GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
    GIT_TAG        v0.7.0
)
set(JWT_BUILD_EXAMPLES       OFF CACHE BOOL "" FORCE)
set(JWT_BUILD_TESTS          OFF CACHE BOOL "" FORCE)
# Tell jwt-cpp to use the system nlohmann_json rather than fetching its own.
set(JWT_EXTERNAL_PICOJSON    ON  CACHE BOOL "" FORCE)
set(JWT_DISABLE_PICOJSON     ON  CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(jwt-cpp)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(ditto_backend ${SOURCES})

target_include_directories(ditto_backend PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(ditto_backend PRIVATE
    Drogon::Drogon
    OpenSSL::SSL
    OpenSSL::Crypto
    jwt-cpp::jwt-cpp
)
