#include <tuple>
#include <algorithm>
#include <iomanip>
#include <thread>
#include "markovDrawer.hpp"
#include "performanceModel.hpp"
#include "dataSource.hpp"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "implot.h"
#include "imgui_impl_sdlrenderer.h"
#include "csvReader.hpp"
#include <iostream>
#include <SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif


#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

static SDL_Point toDraw[dataResolution];
static GenericSource drawnData;
static MarkovSource model(8,3);

int scaleLoadToCoor(float load, float maxLoad, int highCoor, int lowCoor){
  int constant = lowCoor;
  float factor = (float) (highCoor - lowCoor) / maxLoad;
  return (int) std::round(factor * load + constant);
}

void getLine(int width, int height){

  float scaling = (float) width / dataResolution;
    for(int i = 0; i < dataResolution; i++){
      SDL_Point pt = {(int) std::floor(scaling * i), height-loads[i]};
      toDraw[i] = pt;
    }
}

void markovDrawer::dataDrawingWindow(){

  int w, h;
  SDL_GetWindowSize(window, &w, &h);
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize({(float)w, 50.f});

  ImGui::Begin("Data Drawing", &showDrawingWindow);

  if(ImGui::Button("Stop")) showDrawingWindow = false;
  ImGui::SameLine();
  if(ImGui::Button("Reset")) for(auto& x : loads) x = 0;
  ImGui::SameLine();
  ImGui::InputFloat("Maximum Load", &maxLoad);
  ImGui::SameLine();
  if(ImGui::Button("Commit Data")){for(auto l : loads){
    float load = h - l;

    if(load > h - 40){ drawnData.addPoint(0, true); continue;} //On screen point is in loading zone

    float factor = maxLoad / (150 - h + chargingCutoff);
    float constant = maxLoad - (150.f * maxLoad / (150.f + chargingCutoff - h));

    drawnData.addPoint(factor * load + constant, false);
  }}
  ImGui::SameLine();
  if(ImGui::Button("Create Markov")) model.matchToData(drawnData);
  ImGui::SameLine();
  if(ImGui::Button("Sample Markov")){
    for(auto& l : loads){
      auto[load, charge] = model.sample(1);
      l = h - scaleLoadToCoor(load, maxLoad, 150, h-chargingCutoff);
    }
  }

  ImGui::SameLine();

  int xPos, yPos;
  int buttons = SDL_GetMouseState(&xPos, &yPos);

  if(buttons & SDL_BUTTON_LMASK && yPos > 149){

    xPos = (int) std::floor((float) dataResolution * xPos / w);

    yPos = h - yPos;
    //loads[xPos] = 50 + h - yPos;
    for(int i = std::max(0, xPos - 700); i < std::min(xPos+701, dataResolution); i++ ){

      float modifier = 1.f / (0.005 * std::abs(i - xPos)  + 1);
      loads[i] = (1.f - modifier) * loads[i] + modifier * yPos;

    }
  }
  ImGui::End();



  SDL_SetRenderDrawColor(SDLRenderer, 0, 122, 0, 200);
  SDL_SetRenderDrawBlendMode(SDLRenderer, SDL_BLENDMODE_BLEND);
  SDL_Rect chargeZone{0, h-chargingCutoff, w, chargingCutoff};
  SDL_RenderFillRect(SDLRenderer, &chargeZone);




  SDL_SetRenderDrawColor(SDLRenderer, 122, 122, 122, 200);
  int step = (h-chargingCutoff-150) / 4;
  for(int i = 150; i < h-chargingCutoff; i+=step){
    SDL_RenderDrawLine(SDLRenderer, 0, i, w, i);
  }

  SDL_SetRenderDrawColor(SDLRenderer, 122, 0, 0, 255);
  SDL_RenderDrawLine(SDLRenderer, 0, 150, w, 150);

  getLine(w,h);

  SDL_SetRenderDrawColor(SDLRenderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
  SDL_RenderDrawLines(SDLRenderer, toDraw, dataResolution);

}

void markovDrawer::MainWindow(){
  int w;
  int h;
  SDL_GetWindowSize(window, &w, &h);
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize({(float)w,(float)h});

  const char* windowName = dbg ? " DBG Engaged##window" : "##window";
  ImGui::Begin(windowName, &running, ImGuiWindowFlags_AlwaysAutoResize);

  if(ImGui::Button("Toggle Debug")) dbg = !dbg;
  ImGui::SameLine();
  if(ImGui::Button("Draw Data")) showDrawingWindow = true;

  ImGui::End();
}

markovDrawer::markovDrawer(){
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0){
    std::cerr << "SDL initialisation has failed with: " << SDL_GetError() << std::endl;
    return;
  }

    // Create window with graphics context

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    window = SDL_CreateWindow("Boats", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, window_flags);
    SDLRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);


    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, SDLRenderer);
    ImGui_ImplSDLRenderer_Init(SDLRenderer);

    running = true;

}

void markovDrawer::loop(){

  while(running){


    SDL_Event ev;
    while (SDL_PollEvent(&ev)){
      ImGui_ImplSDL2_ProcessEvent(&ev);
      if(ev.type == SDL_QUIT) running = false;
      if (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_CLOSE && ev.window.windowID == SDL_GetWindowID(window)) running = false;
    }



    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();

    ImGui::NewFrame();

    SDL_SetRenderDrawColor(SDLRenderer, (Uint8)(clear_color.x * 0), (Uint8)(clear_color.y * 0), (Uint8)(clear_color.z * 0), (Uint8)(clear_color.w * 255));
    SDL_RenderClear(SDLRenderer);

    if(!showDrawingWindow) MainWindow();
    if(showDrawingWindow) dataDrawingWindow();

    ImGui::Render();

    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(SDLRenderer);
  }
}

markovDrawer::~markovDrawer(){
  ImGui_ImplSDLRenderer_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyRenderer(SDLRenderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
