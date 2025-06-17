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

    target_include_directories(VoxelCube PRIVATE
        ${GLFW3_INCLUDE_DIRS}
    )

    target_link_libraries(VoxelCube PRIVATE
        ${GLFW3_LIBRARIES}      # should expand to -lglfw
        ${OPENGL_LIBRARIES}     # typically -lGL
        ${CMAKE_DL_LIBS}        # -ldl
        pthread                 # -lpthread
        X11                     # -lX11
        m                       # -lm
    )
# -----------------------------

else()
    message(FATAL_ERROR "Unsupported platform. This CMakeLists.txt is configured for macOS, Windows, and Linux.")
endif()