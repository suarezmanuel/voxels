# --- START OF FILE CMakeLists.txt ---

# --- 1. Basic Project Setup ---
cmake_minimum_required(VERSION 3.10)
project(OpenGLCube C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

find_package(TBB REQUIRED)

# --- 2. Define Executable (common part) ---
add_executable(VoxelCube src/main.cpp src/stb_impl.cpp extern/glad/glad.c extern/perlin/perlin.cpp)

set(IMGUI_SOURCES
    extern/imgui/imgui.cpp
    extern/imgui/imgui_demo.cpp
    extern/imgui/imgui_draw.cpp
    extern/imgui/imgui_tables.cpp
    extern/imgui/imgui_widgets.cpp
    extern/imgui/backends/imgui_impl_glfw.cpp
    extern/imgui/backends/imgui_impl_opengl3.cpp
)

add_library(ImGui_Lib STATIC ${IMGUI_SOURCES})

target_include_directories(ImGui_Lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/imgui/backends
)

# --- 3. Set Up Common Include Directories ---
target_include_directories(VoxelCube PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/extern
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/stb_image
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/stb_image_write
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/perlin
)

# --- 4. Platform-Specific Configuration ---
if(APPLE)
    message(STATUS "Configuring for macOS")

    set(GLFW_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw-3.4.bin.MACOS/include)
    target_include_directories(ImGui_Lib PUBLIC ${GLFW_INCLUDE_DIR})

    set_target_properties(VoxelCube PROPERTIES MACOSX_BUNDLE ON)
    target_include_directories(VoxelCube PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw-3.4.bin.MACOS/include)
    target_link_directories(VoxelCube PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw-3.4.bin.MACOS/lib-universal)
    find_package(OpenGL REQUIRED)
    target_link_libraries(VoxelCube PRIVATE
        glfw3
        TBB::tbb
        ImGui_Lib
        ${OPENGL_LIBRARIES}
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )

    add_custom_command(TARGET VoxelCube POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:VoxelCube>/../Resources/shaders
        COMMENT "Copying shaders to app bundle."
    )

    add_custom_command(TARGET VoxelCube POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/textures
        $<TARGET_FILE_DIR:VoxelCube>/../Resources/textures
        COMMENT "Copying textures to app bundle."
    )

    # add_custom_command(TARGET VoxelCube POST_BUILD
    #     COMMAND ${CMAKE_COMMAND} -E copy_directory
    #     ${CMAKE_CURRENT_SOURCE_DIR}/extern/imgui
    #     $<TARGET_FILE_DIR:VoxelCube>/../Resources/imgui
    #     COMMENT "Copying imgui to app bundle."
    # )
    
elseif(WIN32)
    message(STATUS "Configuring for Windows")
    target_include_directories(VoxelCube PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw-3.4.bin.WIN64/include)
    target_link_directories(VoxelCube PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw-3.4.bin.WIN64/lib-vc2022)
    find_package(OpenGL REQUIRED)
    target_link_libraries(VoxelCube PRIVATE glfw3 ${OPENGL_LIBRARIES})

# --- UPDATED SECTION FOR LINUX ---
elseif(UNIX AND NOT APPLE)
    message(STATUS "Configuring for Linux")

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW3 REQUIRED glfw3)            # uses pkg-config

    find_package(OpenGL REQUIRED)

    target_include_directories(ImGui_Lib PUBLIC ${GLFW3_INCLUDE_DIRS})
    target_include_directories(VoxelCube PRIVATE
        ${GLFW3_INCLUDE_DIRS}
    )

    target_link_libraries(VoxelCube PRIVATE
        ${GLFW3_LIBRARIES}      # should expand to -lglfw
        ${OPENGL_LIBRARIES}     # typically -lGL
        ${CMAKE_DL_LIBS}        # -ldl
        TBB::tbb
        ImGui_Lib
        pthread                 # -lpthread
        X11                     # -lX11
        m                       # -lm
    )

# -----------------------------

else()
    message(FATAL_ERROR "Unsupported platform. This CMakeLists.txt is configured for macOS, Windows, and Linux.")
endif()

