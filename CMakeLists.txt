# --- START OF FILE CMakeLists.txt ---

# --- 1. Basic Project Setup ---
cmake_minimum_required(VERSION 3.10)
project(OpenGLCube C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 2. Define Executable (common part) ---
add_executable(VoxelCube main.cpp extern/glad/glad.c)

# --- 3. Set Up Common Include Directories ---
target_include_directories(VoxelCube PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/extern
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/glm
)

# --- 4. Platform-Specific Configuration ---
if(APPLE)
    message(STATUS "Configuring for macOS")
    set_target_properties(VoxelCube PROPERTIES MACOSX_BUNDLE ON)
    target_include_directories(VoxelCube PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw-3.4.bin.MACOS/include)
    target_link_directories(VoxelCube PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw-3.4.bin.MACOS/lib-universal)
    find_package(OpenGL REQUIRED)
    target_link_libraries(VoxelCube PRIVATE
        glfw3
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
    
elseif(WIN32)
    message(STATUS "Configuring for Windows")
    target_include_directories(VoxelCube PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw-3.4.bin.WIN64/include)
    target_link_directories(VoxelCube PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw-3.4.bin.WIN64/lib-vc2022)
    find_package(OpenGL REQUIRED)
    target_link_libraries(VoxelCube PRIVATE glfw3 ${OPENGL_LIBRARIES})

# --- UPDATED SECTION FOR LINUX ---
elseif(UNIX AND NOT APPLE)
    message(STATUS "Configuring for Linux")

    # On Linux, we find system-wide libraries using find_package.
    # This is the standard approach.
    find_package(glfw3 REQUIRED)
    find_package(OpenGL REQUIRED)

    # We link against the "imported targets" that find_package provides.
    # This automatically handles include directories and library paths.
    target_link_libraries(VoxelCube PRIVATE
        glfw3::glfw
        OpenGL::GL
    )
# -----------------------------

else()
    message(FATAL_ERROR "Unsupported platform. This CMakeLists.txt is configured for macOS, Windows, and Linux.")
endif()
