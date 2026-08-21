cmake_minimum_required(VERSION 3.22)
project(RenderOptimizer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(MOD_ID "render_optimizer" CACHE STRING "")
set(MOD_NAME "Render Optimizer" CACHE STRING "")
set(MOD_AUTHOR "Leo" CACHE STRING "")
set(MOD_VERSION "0.2.0" CACHE STRING "")
set(MOD_LIBRARY_NAME "RenderOptimizer" CACHE STRING "")
set(MOD_MINECRAFT_VERSIONS "[\"1.26.*\"]" CACHE STRING "")
set(MOD_ICON "" CACHE STRING "")

include(FetchContent)

FetchContent_Declare(
  preloader_android
  GIT_REPOSITORY https://github.com/LiteLDev/preloader-android.git
  GIT_TAG 0.2.2
  SOURCE_SUBDIR cmake/preloader-sdk
)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_Declare(
  boost_pfr
  GIT_REPOSITORY https://github.com/boostorg/pfr.git
  GIT_TAG boost-1.88.0
)
FetchContent_Declare(
  magic_enum
  GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
  GIT_TAG v0.9.7
)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 11.2.0
)
FetchContent_Declare(
  bedrocktools_sdk
  GIT_REPOSITORY https://github.com/redleo1570/BedrockTools.git
  GIT_TAG main
)

set(FMT_DOC OFF CACHE BOOL "" FORCE)
set(FMT_TEST OFF CACHE BOOL "" FORCE)
set(FMT_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(
  preloader_android nlohmann_json boost_pfr magic_enum fmt
)
FetchContent_GetProperties(bedrocktools_sdk)
if(NOT bedrocktools_sdk_POPULATED)
  FetchContent_Populate(bedrocktools_sdk)
endif()
set(BEDROCKTOOLS_INCLUDE_DIR "${bedrocktools_sdk_SOURCE_DIR}/include")

FetchContent_GetProperties(preloader_android)
set(LEVI_PRELOADER_INCLUDE_DIR "${preloader_android_SOURCE_DIR}/include")

if(NOT ANDROID)
  message(STATUS "Configure Android with the workflow or Android NDK.")
  return()
endif()

if(NOT CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
  message(FATAL_ERROR "Render Optimizer V2 currently targets arm64-v8a only.")
endif()

if(NOT DEFINED LEVI_PACKAGE_CONFIG_DIR)
  set(LEVI_PACKAGE_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/config" CACHE PATH "")
endif()

if(NOT EXISTS "${LEVI_PACKAGE_CONFIG_DIR}/config.json")
  message(FATAL_ERROR "Missing config/config.json")
endif()
if(NOT EXISTS "${LEVI_PACKAGE_CONFIG_DIR}/config.schema.json")
  message(FATAL_ERROR "Missing config/config.schema.json")
endif()

if(NOT TARGET preloader)
  add_subdirectory("${preloader_android_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/_preloader_android")
endif()

add_library(${MOD_LIBRARY_NAME} SHARED
  src/main.cpp
  src/mod/Config.cpp
  src/mod/RenderOptimizer.cpp
)

target_include_directories(${MOD_LIBRARY_NAME} PRIVATE
  src
  "${LEVI_PRELOADER_INCLUDE_DIR}"
  "${BEDROCKTOOLS_INCLUDE_DIR}"
)

target_compile_options(${MOD_LIBRARY_NAME} PRIVATE
  -fvisibility=hidden
  -fvisibility-inlines-hidden
)

target_link_options(${MOD_LIBRARY_NAME} PRIVATE
  "-Wl,-z,max-page-size=16384"
)

target_link_libraries(${MOD_LIBRARY_NAME} PRIVATE
  preloader
  nlohmann_json::nlohmann_json
  Boost::pfr
  magic_enum::magic_enum
  fmt::fmt
  log
  dl
)

set(LEVI_PACKAGE_ROOT "${CMAKE_CURRENT_BINARY_DIR}/package")
set(LEVI_PACKAGE_MOD_DIR "${LEVI_PACKAGE_ROOT}/${MOD_ID}")
set(LEVI_PACKAGE_FILE "${CMAKE_CURRENT_BINARY_DIR}/${MOD_ID}-${MOD_VERSION}-${CMAKE_ANDROID_ARCH_ABI}.levipack")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/manifest.json.in"
  "${CMAKE_CURRENT_BINARY_DIR}/manifest.json"
  @ONLY
)

add_custom_target(levi_package
  COMMAND "${CMAKE_COMMAND}" -E rm -rf "${LEVI_PACKAGE_ROOT}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${LEVI_PACKAGE_MOD_DIR}"
  COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${MOD_LIBRARY_NAME}>" "${LEVI_PACKAGE_MOD_DIR}/lib${MOD_LIBRARY_NAME}.so"
  COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_BINARY_DIR}/manifest.json" "${LEVI_PACKAGE_MOD_DIR}/manifest.json"
  COMMAND "${CMAKE_COMMAND}" -E copy_directory "${LEVI_PACKAGE_CONFIG_DIR}" "${LEVI_PACKAGE_MOD_DIR}/config"
  COMMAND "${CMAKE_COMMAND}" -E chdir "${LEVI_PACKAGE_ROOT}" "${CMAKE_COMMAND}" -E tar "cf" "${LEVI_PACKAGE_FILE}" --format=zip "${MOD_ID}"
  DEPENDS ${MOD_LIBRARY_NAME}
  VERBATIM
)
