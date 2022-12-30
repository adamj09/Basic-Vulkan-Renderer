# Includes .env.cmake file (list of includes)
include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
cmake_minimum_required (VERSION 3.23.2)

# Sets the name variable for the project
set(NAME renderer)

# Build type, set to "Debug" for debugging and "Release" for full release build with all optimizations
set(CMAKE_BUILD_TYPE Debug) 

# Sets the compiler being used for the project
message(STATUS "Using ${CMAKE_GENERATOR}")
if (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
    if (NOT MINGW_PATH) 
      message(FATAL_ERROR "MINGW_PATH not set in .env.cmake")
    endif()
    set(USE_MINGW "True")
    set(CMAKE_C_COMPILER ${MINGW_PATH}/bin/gcc.exe)
    set(CMAKE_CXX_COMPILER  ${MINGW_PATH}/bin/g++.exe)
endif()

# Project creation
project(${NAME} VERSION 0.1.0)

# Uses Vulkan path in env.cmake to target specific version
if (DEFINED VULKAN_SDK_PATH)
  message(STATUS "Using Vulkan path specified in .env.cmake file")
  set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
  set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib")
  set(Vulkan_FOUND "True")
else() # If Vulkan path is not found in .env.cmake if will fall back to cmake's find_package() function
  message(STATUS "Using find_package function to find vulkan...")
  find_package(Vulkan REQUIRED)
  message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
endif()
if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!") # Throws error if could not find Vulkan
else()
    message(STATUS "Using Vulkan lib at: ${Vulkan_LIBRARIES}") # If you get this message it successfully found and is including Vulkan
endif()

# All C++ files are added here
file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/source/*.cpp
    ${PROJECT_SOURCE_DIR}/source/*.hpp
)

# Creates executable (.exe file)
add_executable(${PROJECT_NAME} ${SOURCES})

# Specifies what C++ standard to compile
set_property(TARGET ${PROJECT_NAME} PROPERTY C++20)

# Links libraries differently based on platform
if(WIN32)
    message(STATUS "Creating build for Windows")
    # If using MINGW compiler, include it
    if (USE_MINGW)
        target_include_directories(${PROJECT_NAME} PUBLIC
            ${MINGW_PATH}/include
        )
        target_link_directories(${PROJECT_NAME} PUBLIC
            ${MINGW_PATH}/lib
        )
    endif()
    # Include all other libraries
    target_include_directories(${PROJECT_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/source
        ${Vulkan_INCLUDE_DIRS}
        ${GLFW_PATH}
        ${GLM_PATH}
        ${TINY_OBJ_PATH}
        ${STB_MASTER_PATH}
    )

    target_link_directories(${PROJECT_NAME} PUBLIC
        ${Vulkan_LIBRARIES}
        ${GLFW_LIB_PATH}
    )
 
    target_link_libraries(${PROJECT_NAME} PUBLIC 
        glfw3 vulkan-1
    )
elseif(UNIX)
    message(STATUS "Creating build for Unix")
    
    target_include_directories(${PROJECT_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/source
        ${Vulkan_INCLUDE_DIRS}
        ${GLFW_PATH}
        ${GLM_PATH}
    )
    target_link_libraries(${PROJECT_NAME} PUBLIC 
        ${Vulkan_LIBRARIES}
    )
endif()

# GLSL Shader Compilation
file(GLOB_RECURSE GLSL_SOURCES
    ${PROJECT_SOURCE_DIR}/source/shaders/*.frag
    ${PROJECT_SOURCE_DIR}/source/shaders/*.vert
)

foreach(GLSL ${GLSL_SOURCES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_SOURCE_DIR}/source/spirv_shaders/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_SOURCE_DIR}/source/spirv_shaders/"
        COMMAND ${GLSLC} -c ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)

add_dependencies(${PROJECT_NAME} Shaders)