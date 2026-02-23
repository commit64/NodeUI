add_library(core STATIC)
add_library(MyProject::core ALIAS core)

target_sources(core PRIVATE .
)

target_include_directories(core PUBLIC .)