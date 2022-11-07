cmake_minimum_required(VERSION 3.10)

message(STATUS "vcpkg at $ENV{VCPKG_HOME}")
set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_HOME}) ## Set VCPKG_HOME to point to vcpkg.cmake

project(BOATS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
  set(BUILD_MATLAB)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})


find_package(GSL REQUIRED)


set(SOURCE_FILES dataSource.cpp performanceModel.cpp controlStrategy.cpp simulator.hpp event.cpp components.cpp)
add_executable(boats  main.cpp ${SOURCE_FILES})
target_link_libraries(boats ${CMAKE_DL_LIBS} GSL::gsl GSL::gslcblas)

find_package(SDL2 CONFIG)
if(SDL2_FOUND)
  set(IMGUI_SOURCE ./imgui/imgui.cpp ./imgui/imgui_draw.cpp ./imgui/imgui_tables.cpp ./imgui/imgui_widgets.cpp ./imgui/backends/imgui_impl_sdl.cpp ./imgui/backends/imgui_impl_sdlrenderer.cpp)
  set(IMPLOT_SOURCE ./implot/implot.cpp ./implot/implot_items.cpp)
  add_executable(markovDrawer  markovMain.cpp markovDrawer.cpp ${SOURCE_FILES} ${IMGUI_SOURCE} ${IMPLOT_SOURCE})
  set(IMGUI_PATH "./imgui")
  target_include_directories(markovDrawer PUBLIC ${IMGUI_PATH} ${IMGUI_PATH}/backends  ${SDL2_INCLUDE_DIRS} ./implot)
  target_link_libraries(markovDrawer ${SDL2_LIBRARIES} ${CMAKE_DL_LIBS} ${OPENGL_LIBRARIES})
endif()

