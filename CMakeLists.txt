cmake_minimum_required(VERSION 3.20)
project(TurboEmulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(SOURCES
    src/main.cpp
    src/core/hypervisor.cpp
    src/core/snapshot_manager.cpp
    src/guest/guest_manager.cpp
    src/guest/virtio_serial.cpp
    src/guest/shared_storage.cpp
    src/guest/apk_installer.cpp
    src/guest/microg_manager.cpp
    src/graphics/renderer.cpp
    src/graphics/angle_bridge.cpp
    src/input/input_manager.cpp
    src/input/aiming_engine.cpp
    src/input/gyro_filter.cpp
    src/ui/ui_manager.cpp
    src/ui/keymapper_overlay.cpp
    src/ui/game_presets.cpp
)

set(HEADERS
    include/turbo_common.hpp
    src/core/hypervisor.hpp
    src/graphics/renderer.hpp
    src/input/input_manager.hpp
    src/ui/ui_manager.hpp
)

add_executable(TurboEmulator ${SOURCES} ${HEADERS})

if(WIN32)
    target_link_libraries(TurboEmulator PRIVATE
        WinHvPlatform
        d3d11
        dxgi
        d3dcompiler
        user32
        gdi32
    )
    if(MINGW OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_link_options(TurboEmulator PRIVATE -static -static-libgcc -static-libstdc++)
    endif()
endif()
