# --- START OF FILE CMakeLists.txt ---

# --- 1. Basic Project Setup ---
cmake_minimum_required(VERSION 3.10)
project(OpenGLCube C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 2. Define Executable (common part) ---
# We define the sources here. The BUNDLE option will be added later for Mac.
add_executable(VoxelCube main.cpp extern/glad/glad.c)

# --- 3. Set Up Common Include Directories ---
# These paths are needed for ALL platforms.
target_include_directories(VoxelCube PRIVATE
    # Path for GLAD and KHR. The #include is <glad/glad.h>, so we point to the folder containing 'glad'.
    ${CMAKE_CURRENT_SOURCE_DIR}/extern

    # Path for GLM. The #include is <glm/glm.hpp>, so we point to the folder containing the *first* 'glm'.
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/glm
)

# --- 4. Platform-Specific Configuration ---
if(APPLE)
    message(STATUS "Configuring for macOS")
    
    # Add the MACOSX_BUNDLE property to make it a proper .app
    set_target_properties(VoxelCube PROPERTIES MACOSX_BUNDLE ON)

    # Add platform-specific include and library paths for GLFW
    target_include_directories(VoxelCube PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw-3.4.bin.MACOS/include)
    target_link_directories(VoxelCube PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw-3.4.bin.MACOS/lib-universal)

    # Find and link required libraries and frameworks
    find_package(OpenGL REQUIRED)
    target_link_libraries(VoxelCube PRIVATE
        glfw3
        ${OPENGL_LIBRARIES}
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )

elseif(WIN32)
    message(STATUS "Configuring for Windows")

    # Add platform-specific include and library paths for GLFW
    target_include_directories(VoxelCube PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw-3.4.bin.WIN64/include)
    target_link_directories(VoxelCube PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw-3.4.bin.WIN64/lib-vc2022)

    # Find and link required libraries
    find_package(OpenGL REQUIRED)
    target_link_libraries(VoxelCube PRIVATE glfw3 ${OPENGL_LIBRARIES})

# --- NEW SECTION FOR LINUX ---
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