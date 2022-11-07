#ifndef MARKOVAPPLICATION_HPP
#define MARKOVAPPLICATION_HPP

#include <vector>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"

#include "performanceModel.hpp"

#include <stdio.h>
#include <SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

static constexpr int dataResolution = 60*60*24;
static int loads[dataResolution];
class markovDrawer {
private:
  SDL_Window* window;
  SDL_GLContext gl_context;
  SDL_Renderer* SDLRenderer = nullptr;

  ImGuiIO io;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  int modelSize = 3;
  performanceModel* Model = nullptr;

  bool dbg = false;
  bool running = false;

  bool showDrawingWindow = false;

  float maxLoad = 800;
  float chargingCutoff = 40;

  void dataDrawingWindow();
  void MainWindow();
public:
  void loop();
  markovDrawer ();
  virtual ~markovDrawer ();
};


#endif
