include("AMXConfig")
include("AddSAMPPlugin")

set(CMAKE_SKIP_INSTALL_RULES True)
include_directories("$ENV{SAMPSDK_DIR}")
add_samp_plugin(
    "main"
    "plugins/main.cpp"
    "plugins/main.def"
    "$ENV{SAMPSDK_DIR}/amxplugin.cpp"
)

target_link_libraries("main" "sampgdk")
