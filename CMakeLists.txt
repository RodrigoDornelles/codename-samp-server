cmake_minimum_required(VERSION 3.14)
project(samp)
include("FetchContent")
include("AMXConfig")
include("AddSAMPPlugin")

set(CXX_STANDARD 17)
set(CMAKE_SKIP_INSTALL_RULES True)
include_directories("$ENV{SAMPSDK_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/plugins")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/_deps/cpp_redis_vendor-src/includes")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/_deps/cpp_redis_vendor-src/tacopie/includes")

FetchContent_Declare(
  cpp_redis_vendor
  GIT_REPOSITORY "https://github.com/cpp-redis/cpp_redis"
  GIT_TAG "bbe38a7f83de943ffcc90271092d689ae02b3489"
  GIT_SUBMODULES "tacopie"
)
FetchContent_MakeAvailable(cpp_redis_vendor)

add_samp_plugin(
    "main"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugins/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugins/manager/map.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugins/singleton/redis.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugins/main.def"
    "$ENV{SAMPSDK_DIR}/amxplugin.cpp"
)

target_compile_options(tacopie PUBLIC -m32)
target_compile_options(cpp_redis PUBLIC -m32)
target_link_libraries("main" "tacopie")
target_link_libraries("main" "cpp_redis")
target_link_libraries("main" "sampgdk")
set_target_properties("main" PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/plugins"
)
