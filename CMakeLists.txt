cmake_minimum_required(VERSION 3.25)
project(reflect_router LANGUAGES CXX)

# --- 1. Die Library (Header-only Teil) ---
# Wir erstellen ein Interface-Target, das alle nötigen Flags für Reflection mitbringt
add_library(reflect_router_lib INTERFACE)

# Den include-Pfad so setzen, dass man #include <reflect_router/core.hpp> schreiben kann
target_include_directories(reflect_router_lib INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/reflect_router/include
)

# Die experimentellen Flags direkt an die Library binden
# So erbt jedes Target, das gegen diese Lib linkt, automatisch die Reflection-Power
target_compile_options(reflect_router_lib INTERFACE
    -freflection
)

# --- 2. Das Haupt-Programm ---
add_executable(reflect_router_app reflect_router/src/main.cpp)

# Gegen unsere Library linken (erbt Header-Pfade und Compile-Flags)
target_link_libraries(reflect_router_app PRIVATE reflect_router_lib)

# C++ Standard festlegen (26 oder 2c, je nachdem was der Bloomberg-Clang erkennt)
set_target_properties(reflect_router_app PROPERTIES
    CXX_STANDARD 26
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_link_directories(reflect_router_app PRIVATE 
    /opt/p2996/clang/lib/x86_64-unknown-linux-gnu
)