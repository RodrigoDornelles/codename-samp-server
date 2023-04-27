cmake_minimum_required(VERSION 3.0)
project(samp)
include("AMXConfig")
include("AddSAMPPlugin")

set(CMAKE_SKIP_INSTALL_RULES True)
include_directories("$ENV{SAMPSDK_DIR}")
add_samp_plugin(
    "main"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugins/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugins/main.def"
    "$ENV{SAMPSDK_DIR}/amxplugin.cpp"
)

target_link_libraries("main" "sampgdk")
set_target_properties("main" PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/plugins"
)
