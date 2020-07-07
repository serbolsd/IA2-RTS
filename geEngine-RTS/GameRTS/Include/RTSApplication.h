#pragma once

#include <lua.hpp>

#include "RTSTexture.h"
#include "RTSWorld.h"

using namespace geEngineSDK;

namespace sf{
  class RenderWindow;
  class Font;
}

class RTSApplication
{
 public:
  RTSApplication();
  virtual ~RTSApplication();

  int32
  run();

  sf::RenderWindow*
  getRenderWindow() {
    return m_window;
  }

  RTSWorld*
  getWorld() {
    return &m_gameWorld;
  }

  float getFPS() const {
    return m_framesPerSecond;
  }
  void
  setZoomToView(const float& _zoom);

 protected:

 private:
   void
   initSystems();

   bool
   initLuaScriptSystem();

   void
   initGUI();

   void
   destroySystems();

   void
   gameLoop();

   void
   postInit();

   void
   postDestroy();

   void
   updateFrame();
   
   void
   renderFrame();

 private:
   sf::RenderWindow* m_window;
   sf::View m_windoView;
   sf::Vector2f m_OriginalViewWindSize;
   float m_currentZoom = 0.1f;
   sf::Font* m_arialFont;
   RTSWorld m_gameWorld;

   float m_fpsTimer;
   float m_fpsCounter;
   float m_framesPerSecond;

   lua_State* m_luaState;

   String m_windowsName = "";
   bool m_bFullScreen;
};
