cmake_minimum_required(VERSION 3.16)
project(ProgressiveMeshes LANGUAGES C CXX)


# ---------------------------
# Compiler settings
# ---------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------
# Build options
# ---------------------------
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# ---------------------------
# Source files
# ---------------------------
file(GLOB_RECURSE SRC_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# GLAD is C, add explicitly
set(GLAD_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/lib/glad/src/glad.c
)

# ---------------------------
# Find packages
# ---------------------------
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

# ---------------------------
# Executable
# ---------------------------
add_executable(${PROJECT_NAME} ${SRC_FILES} ${GLAD_SRC})

# ---------------------------
# Include directories
# ---------------------------
target_include_directories(${PROJECT_NAME}
	PRIVATE
	include
	${CMAKE_CURRENT_SOURCE_DIR}/lib/glad/include
)

# ---------------------------
# Link libraries
# ---------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE
	glfw
	OpenGL::GL
)

# ---------------------------
# Output folders
# ---------------------------
set_target_properties(${PROJECT_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin
)

# ---------------------------
# Copy data folder to bin after build
# ---------------------------
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
)