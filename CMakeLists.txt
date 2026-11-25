cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" OFF)
option(ENABLE_QT "Use Qt functionality" OFF)

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_package(Qt6 COMPONENTS Widgets Core)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets)
  target_compile_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header -Wno-comma>
  )
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON
  )
endif()

target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
    src/plugin-main.c
    src/white-noise-source.c
    src/white-noise-source.h
    src/white-noise-video-source.c
    src/white-noise-video-source.h
)

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})

# Stack protection for the plugin. MSVC enables /GS by default; it is stated here to make the setting explicit.
if(MSVC)
  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /GS)
else()
  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -fstack-protector-strong)
endif()

# Versions of the OBS sources and the prebuilt dependencies that the tests load from .deps. They must match
# dependencies.obs-studio.version and dependencies.prebuilt.version in buildspec.json, because the folders under
# .deps are named after them.
set(TEST_OBS_VERSION "31.1.1" CACHE STRING "OBS Studio version in buildspec.json")
set(TEST_DEPS_VERSION "2025-07-11" CACHE STRING "Prebuilt obs-deps version in buildspec.json")
set(
  _test_path
  "PATH=path_list_prepend:${CMAKE_CURRENT_SOURCE_DIR}/.deps/bin/64bit;PATH=path_list_prepend:${CMAKE_CURRENT_SOURCE_DIR}/.deps/obs-deps-${TEST_DEPS_VERSION}-x64/bin"
)

# Scenario test that loads the plugin into libobs and checks the audio it outputs (tests/audio-test.cpp).
# The test is always built on Windows. On Linux, the CI workflow (audio-test.yaml) turns it on.
option(ENABLE_AUDIO_TESTS "Build the audio scenario test on Linux" OFF)
set(AUDIO_TEST_SCENARIOS default-level level-change)
# cpu-usage checks that the generator thread does not busy-wait. Only the Windows libobs busy-waits in
# os_sleepto_ns, so the scenario runs on Windows only and audio-test.yaml does not list it for Ubuntu.
if(WIN32)
  list(APPEND AUDIO_TEST_SCENARIOS cpu-usage)
endif()
if(WIN32 OR ENABLE_AUDIO_TESTS)
  add_executable(audio-test tests/audio-test.cpp)
  target_compile_features(audio-test PRIVATE cxx_std_17)
  target_link_libraries(audio-test PRIVATE OBS::libobs)
  add_dependencies(audio-test ${CMAKE_PROJECT_NAME})
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(X11 REQUIRED)
    target_link_libraries(audio-test PRIVATE X11::X11)
  endif()
endif()

# Render test that loads the plugin into libobs with graphics and checks the pixels of the video source
# (tests/render-test.cpp, tests/render-fixture.hpp, and tests/obs-integration-env.hpp).
# The test is always built on Windows. On Linux, the CI workflow (render-test.yaml) turns it on.
option(ENABLE_RENDER_TESTS "Build the render test on Linux" OFF)
# Keep this list equal to SCENARIOS in render-test.yaml
set(RENDER_TEST_SCENARIOS noise-stats changes-each-frame custom-size)
if(WIN32 OR ENABLE_RENDER_TESTS)
  add_executable(render-test tests/render-test.cpp)
  target_compile_features(render-test PRIVATE cxx_std_17)
  target_link_libraries(render-test PRIVATE OBS::libobs)
  add_dependencies(render-test ${CMAKE_PROJECT_NAME})
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(X11 REQUIRED)
    target_link_libraries(render-test PRIVATE X11::X11)
  endif()
endif()

if(WIN32)
  enable_testing()
  # libobs and its dependencies are loaded from .deps
  foreach(_scenario IN LISTS AUDIO_TEST_SCENARIOS)
    add_test(
      NAME audio-${_scenario}
      COMMAND audio-test "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>" "${CMAKE_CURRENT_SOURCE_DIR}/data" ${_scenario}
    )
    set_tests_properties(audio-${_scenario} PROPERTIES ENVIRONMENT_MODIFICATION "${_test_path}")
  endforeach()

  # Registers every render scenario for Direct3D 11 and OpenGL. Build the graphics modules from the OBS build tree
  # first: cmake --build .deps/obs-studio-<version>/build_x64 --config Release --target libobs-d3d11 libobs-opengl
  # CI builds only libobs-d3d11 and runs the d3d11 tests (ctest -R d3d11).
  set(_obs_source "${CMAKE_CURRENT_SOURCE_DIR}/.deps/obs-studio-${TEST_OBS_VERSION}")
  set(_graphics_d3d11 "${_obs_source}/build_x64/libobs-d3d11/Release/libobs-d3d11.dll")
  set(_graphics_opengl "${_obs_source}/build_x64/libobs-opengl/Release/libobs-opengl.dll")
  foreach(_backend IN ITEMS d3d11 opengl)
    foreach(_scenario IN LISTS RENDER_TEST_SCENARIOS)
      set(_test_name render-${_backend}-${_scenario})
      add_test(
        NAME ${_test_name}
        COMMAND
          render-test "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>" "${CMAKE_CURRENT_SOURCE_DIR}/data"
          "${_graphics_${_backend}}" "${_obs_source}/libobs/data" ${_scenario}
      )
      set_tests_properties(${_test_name} PROPERTIES ENVIRONMENT_MODIFICATION "${_test_path}")
    endforeach()
  endforeach()
endif()
