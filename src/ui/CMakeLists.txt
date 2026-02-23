add_library(ui STATIC)
add_library(MyProject::ui ALIAS ui)

target_sources(ui PRIVATE
    gvlayout.h gvlayout.cpp
    graphicsitem.h graphicsitem.cpp
)

target_include_directories(ui PUBLIC .)
target_link_libraries(ui PUBLIC Qt6::Widgets)