cmake_minimum_required(VERSION 3.10)
project(hyper_bandchip VERSION 0.10 LANGUAGES C CXX)

find_package(SDL2 REQUIRED)

set(CURRENT_RENDERER "OpenGL ES 3.0" CACHE STRING "Renderer to build with.")
set_property(CACHE CURRENT_RENDERER PROPERTY STRINGS "OpenGL 3.3;OpenGL ES 3.0;Direct3D 11")

if (CURRENT_RENDERER STREQUAL "OpenGL ES 3.0")
	set(CURRENT_RENDERER_SOURCE_DIR "opengles3")
	set(CURRENT_RENDERER_DEFINE RENDERER_OPENGLES3)
	set(CURRENT_RENDERER_LIBRARIES GL GLEW)
endif()

set(MACHINE_CORE_FILES
	src/bc_chip8.cpp
	src/bc_schip.cpp
	src/bc_xochip.cpp
	src/bc_hyperchip64.cpp)

add_executable(hyper_bandchip src/${CURRENT_RENDERER_SOURCE_DIR}/renderer.cpp src/hyper_bandchip.cpp src/machine.cpp ${MACHINE_CORE_FILES} src/audio.cpp)
target_include_directories(hyper_bandchip PUBLIC "${PROJECT_BINARY_DIR}/include" "${SDL2_INCLUDE_DIRS}")
target_compile_definitions(hyper_bandchip PUBLIC ${CURRENT_RENDERER_DEFINE})
target_compile_features(hyper_bandchip PRIVATE cxx_std_20)
target_link_libraries(hyper_bandchip SDL2 ${CURRENT_RENDERER_LIBRARIES})
