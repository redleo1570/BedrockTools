cmake_minimum_required(VERSION 3.22)
project(RenderOptimizer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MOD_ID "render_optimizer" CACHE STRING "")
set(MOD_NAME "Render Optimizer" CACHE STRING "")
set(MOD_AUTHOR "Leo" CACHE STRING "")
set(MOD_VERSION "0.1.0" CACHE STRING "")
set(MOD_LIBRARY_NAME "RenderOptimizer" CACHE STRING "")
set(MOD_MINECRAFT_VERSIONS "[\"1.21.*\"]" CACHE STRING "")

# The official LeviLauncher template fetches the public preloader-android SDK.
include(FetchContent)
FetchContent_Declare(
  preloader_android
  GIT_REPOSITORY https://github.com/LiteLDev/preloader-android.git
  GIT_TAG v0.2.2
)
FetchContent_MakeAvailable(preloader_android)

add_library(${MOD_LIBRARY_NAME} SHARED
  src/main.cpp
  src/mod/RenderOptimizer.cpp
  src/mod/Config.cpp
)

target_include_directories(${MOD_LIBRARY_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${MOD_LIBRARY_NAME} PRIVATE
  preloader_android
  log
)

set_target_properties(${MOD_LIBRARY_NAME} PROPERTIES
  OUTPUT_NAME "${MOD_LIBRARY_NAME}"
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/manifest.json.in
  ${CMAKE_CURRENT_BINARY_DIR}/manifest.json
  @ONLY
)

add_custom_target(levi_package ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/package
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${MOD_LIBRARY_NAME}> ${CMAKE_CURRENT_BINARY_DIR}/package/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/manifest.json ${CMAKE_CURRENT_BINARY_DIR}/package/
  DEPENDS ${MOD_LIBRARY_NAME}
)
