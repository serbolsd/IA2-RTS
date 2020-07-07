#include <gePrerequisitesUtil.h>

#if GE_PLATFORM == GE_PLATFORM_WIN32
# include <Win32/geWin32Windows.h>
#endif

#include <geRTTIPlainField.h>
#include <geException.h>
#include <geMath.h>

#include <geCrashHandler.h>
#include <geDynLibManager.h>
#include <geFileSystem.h>
#include <geTime.h>
#include <geUnicode.h>

#include <SFML/Graphics.hpp>

#include <commdlg.h>
#include <imgui.h>
#include <imgui-sfml.h>


#include "RTSConfig.h"
#include "RTSApplication.h"
#include "RTSTiledMap.h"
#include "RTSFunctionsCoords.h"

void
loadMapFromFile(RTSApplication* pApp);

void
mainMenu(RTSApplication* pApp);

float
g_cameraZoom = 1.0f;

static const char*
g_AddList[]{ "Start","End","Water","Grass","Marsh","Obstacle", "Tree" };
static int 
g_AddOption;
static string g_AddOptionPreviw = g_AddList[0];
static const char*
g_PathList[]{ "Depth","Breadth","Best","Dijkstra","A*"};
static int
g_PathOption=1;
static string g_PathFindingOptionPreviw = g_PathList[1];
static const char*
g_EuristicList[]{ "Euclidian","Square"};
static int 
g_EuristicOption;
static string g_EuristicOptionPreviw = g_EuristicList[0];

static float g_TimeToPaintTail = 0.3f;
static float g_euristicRelevande = 1.0f;
static float g_costRelevande = 1.0f;

int
setResolution(lua_State* l) {
  int32 width = (int32)lua_tointeger(l, 1);
  int32 height = (int32)lua_tointeger(l, 2);

  GameOptions::s_Resolution.x = width;
  GameOptions::s_Resolution.y = height;

  return 0;
}

int
setWindowName(lua_State* l) {
  GameOptions::s_windowName = (String)lua_tostring(l, 1);
  return 0;
}

int
setWindowStyle(lua_State* l) {
  bool WindowStyle = (bool)lua_toboolean(l, 1);

  GameOptions::s_bFullScreen = WindowStyle;
  return 0;
}

int 
setMapMovementSpeed(lua_State* l) {
  float speedx = (float)lua_tonumber(l, 1);
  float speedy = (float)lua_tonumber(l, 2);

  GameOptions::s_MapMovementSpeed.x = speedx;
  GameOptions::s_MapMovementSpeed.y = speedy;

  return 0;
}

int
setCameraPosition(lua_State* l) {
  int32 posx = (int32)lua_tointeger(l, 1);
  int32 posy = (int32)lua_tointeger(l, 2);

  GameOptions::s_CameraPosition.x = posx;
  GameOptions::s_CameraPosition.y = posy;

  return 0;
}

int 
setMapShowGrid(lua_State* l) {
  bool showGrid = (bool)lua_toboolean(l, 1);

  GameOptions::s_MapShowGrid = showGrid;

  return 0;
}

int
setMapGridColor(lua_State* l) {
  int32 r = (int32)lua_tointeger(l, 1);
  int32 g = (int32)lua_tointeger(l, 2);
  int32 b = (int32)lua_tointeger(l, 3);
  int32 a = (int32)lua_tointeger(l, 4);

  GameOptions::s_MapGridColor.r = r;
  GameOptions::s_MapGridColor.g = g;
  GameOptions::s_MapGridColor.b = b;
  GameOptions::s_MapGridColor.a = a;

  return 0;
}

int 
setMapSize(lua_State* l) {
  int sizeX = (int)lua_tointeger(l, 1);
  int sizeY = (int)lua_tointeger(l, 2);

  GameOptions::s_MapSizeX = sizeX;
  GameOptions::s_MapSizeY = sizeY;

  return 0;
}

int
setIfMapIsISO(lua_State* l) {
  bool mapIsISO = (bool)lua_toboolean(l, 1);

  GameOptions::s_MapIsIsometric = mapIsISO;

  return 0;
}

int
setTileSize(lua_State* l) {
  int sizeX = (int)lua_tointeger(l, 1);
  int sizeY = (int)lua_tointeger(l, 2);

  GameOptions::s_TileSizeX = sizeX;
  GameOptions::s_TileSizeY = sizeY;


    GameOptions::TILEHALFSIZE = Vector2I((int32)GameOptions::s_TileSizeX >> 1, (int32)GameOptions::s_TileSizeY >> 1);

    GameOptions::BITSHFT_TILESIZE = Vector2I(
      Math::countTrailingZeros(GameOptions::s_TileSizeX),
      Math::countTrailingZeros(GameOptions::s_TileSizeY)
    );

    GameOptions::BITSFHT_TILEHALFSIZE = Vector2I(GameOptions::BITSHFT_TILESIZE.x - 1,
      GameOptions::BITSHFT_TILESIZE.y - 1);

  return 0;
}

RTSApplication::RTSApplication()
  : m_window(nullptr),
    m_fpsTimer(0.0f),
    m_fpsCounter(0.0f),
    m_framesPerSecond(0.0f)
{}

RTSApplication::~RTSApplication() {}

int32
RTSApplication::run() {

  CrashHandler::startUp();
  DynLibManager::startUp();
  Time::startUp();
  GameOptions::startUp();


  __try {
    initSystems();
    gameLoop();
    destroySystems();
  }
  __except (g_crashHandler().reportCrash(GetExceptionInformation())) {
    PlatformUtility::terminate(true);
  }

  GameOptions::shutDown();
  Time::shutDown();
  DynLibManager::shutDown();
  CrashHandler::shutDown();

  return 0;
}

void RTSApplication::setZoomToView(const float& _zoom)
{
  if (_zoom==m_currentZoom)
  {
    return;
  }
  
  sf::Vector2f newViewSize = m_OriginalViewWindSize / _zoom;//The zoom is made from the original resolution
  Vector2I camPos = { static_cast<int32>(newViewSize.x), static_cast<int32>(newViewSize.y - (175 / _zoom)) };
  GameOptions::s_Resolution = { static_cast<int32>(newViewSize.x), static_cast<int32>(newViewSize.y - (175 / _zoom)) };
  
  m_windoView.setSize(newViewSize);
  m_windoView.setCenter(float(camPos.x/2), newViewSize.y/2);//set look at for the new resolution
  m_window->setView(m_windoView);
  m_gameWorld.getTiledMap()->setEnd(camPos.x, camPos.y);//set the new end resolution for redner tiles
  //m_gameWorld.getTiledMap()->setCameraStartPosition(camPos.x, camPos.y);//set the new end resolution for redner tiles
  m_currentZoom = _zoom;

}

void
RTSApplication::initSystems() {

  if (!initLuaScriptSystem())
  {
    std::cout << "couldn´t initialize config with lua\n";
  }

  if (nullptr != m_window) {  //Window already initialized
    return; //Shouldn't do anything
  }

  //Create the application window
  m_window = ge_new<sf::RenderWindow>(sf::VideoMode(GameOptions::s_Resolution.x,
                                      GameOptions::s_Resolution.y),
                                      GameOptions::s_windowName.c_str(),
                                      GameOptions::s_bFullScreen ? sf::Style::Fullscreen : sf::Style::Default);
  if (nullptr == m_window) {
    GE_EXCEPT(InvalidStateException, "Couldn't create Application Window");
  }
  m_window->setMouseCursorVisible(false);
  //get the data of view of the window
  m_windoView = m_window->getView();
  m_OriginalViewWindSize = m_windoView.getSize();

  m_arialFont = ge_new<sf::Font>();
  if (nullptr == m_arialFont) {
    GE_EXCEPT(InvalidStateException, "Couldn't create a Font");
  }
  
  
  if (!m_arialFont->loadFromFile("Fonts/arial.ttf")) {
    GE_EXCEPT(FileNotFoundException, "Arial font not found");
  }

  //m_window->setVerticalSyncEnabled(true);

  initGUI();
}

bool RTSApplication::initLuaScriptSystem()
{
  m_luaState = luaL_newstate();
  if (nullptr == m_luaState)
  {
    return false;
  }

  luaL_openlibs(m_luaState);

  lua_register(m_luaState, "setResolution", &setResolution);
  lua_register(m_luaState, "setWindowsName", &setWindowName);
  lua_register(m_luaState, "setWindowStyle", &setWindowStyle);
  lua_register(m_luaState, "setMapMovementSpeed", &setMapMovementSpeed);
  lua_register(m_luaState, "setCameraPosition", &setCameraPosition);
  lua_register(m_luaState, "setMapShowGrid", &setMapShowGrid);
  lua_register(m_luaState, "setMapGridColor", &setMapGridColor);
  lua_register(m_luaState, "setMapSize", &setMapSize);
  lua_register(m_luaState, "setIfMapIsISO", &setIfMapIsISO);
  lua_register(m_luaState, "setTileSize", &setTileSize);

  luaL_dofile(m_luaState, "config.lua");
  luaL_dostring(m_luaState, "ConfigGame()");

  return true;
}

void
RTSApplication::initGUI() {
  ImGui::SFML::Init(*m_window);
}

void
RTSApplication::destroySystems() {
  ImGui::SFML::Shutdown();

  if (nullptr != m_window) {
    m_window->close();
    ge_delete(m_window);
  }

  if (nullptr != m_arialFont) {
    ge_delete(m_arialFont);
  }
}

void
RTSApplication::gameLoop() {
  if (nullptr == m_window) {  //Windows not yet initialized
    return; //Shouldn't do anything
  }

  postInit();

  sf::Event event;
  while (m_window->isOpen()) {
    while (m_window->pollEvent(event)) {
      ImGui::CreateContext();
      ImGuiIO& io = ImGui::GetIO();
      if (io.WantCaptureMouse)
      {
        m_window->setMouseCursorVisible(true);
        continue;
      }
      else
      {
        m_window->setMouseCursorVisible(false);
      }
      ImGui::SFML::ProcessEvent(event);
      
      if (event.type == sf::Event::Closed) {
        m_window->close();
      }
      if (event.type == sf::Event::MouseButtonPressed)
      {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
          switch (g_AddOption)
          {
          case 0:
            m_gameWorld.getTiledMap()->selecetStartTail();
            break;
          case 1:
            m_gameWorld.getTiledMap()->selecetFinalTail();
            break;
          case 6:
            m_gameWorld.getTiledMap()->addTree();
          default:
            break;
          }
        }
      }
    }
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)&& g_AddOption>1 && g_AddOption < 6)
    {
      m_gameWorld.getTiledMap()->setTileType(g_AddOption - 2);
    }
    
    g_time()._update();
    ge_frame_mark();
    updateFrame();
    renderFrame();
    ge_frame_clear();
  }

  postDestroy();
}

void
RTSApplication::updateFrame() {
  float deltaTime = g_time().getFrameDelta();
  
  m_fpsTimer += deltaTime;
  if (1.0f < m_fpsTimer) {
    m_framesPerSecond = m_fpsCounter;
    m_fpsCounter = 0.0f;
    m_fpsTimer = 0.0f;
  }
  m_fpsCounter += 1.0f;

  //Update the interface
  sf::Clock myClock;
  ImGui::SFML::Update(*m_window, myClock.getElapsedTime());

  //Begin the menu 
  mainMenu(this);

  //Check for camera movement
  Vector2 axisMovement(FORCE_INIT::kForceInitToZero);
  Vector2I mousePosition;
  mousePosition.x = sf::Mouse::getPosition(*m_window).x;
  mousePosition.y = sf::Mouse::getPosition(*m_window).y;

  if (0 == mousePosition.x ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
    if (GameOptions::s_MapIsIsometric)
    {
      axisMovement += Vector2(-1.f, 1.f);
    }
    else
    {
      axisMovement += Vector2(-1.f, 0.f);
    }
  }
  if (GameOptions::s_Resolution.x -1 == mousePosition.x ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
    if (GameOptions::s_MapIsIsometric)
    {
      axisMovement += Vector2(1.f, -1.f);
    }
    else
    {
      axisMovement += Vector2(1.f, 0.f);
    }
  }
  if (0 == mousePosition.y ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::W) ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
    if (GameOptions::s_MapIsIsometric)
    {
      axisMovement += Vector2(-1.f, -1.f);
    }
    else
    {
      axisMovement += Vector2(0.f, -1.f);
    }
  }
  if (GameOptions::s_Resolution.y - 1 == mousePosition.y ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::S) ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
    if (GameOptions::s_MapIsIsometric)
    {
      axisMovement += Vector2(1.f, 1.f);
    }
    else
    {
      axisMovement += Vector2(0.f, 1.f);
    }
  }

  axisMovement *= GameOptions::s_MapMovementSpeed * deltaTime;

  m_gameWorld.getTiledMap()->moveCamera(axisMovement.x, axisMovement.y);

  //Update the world
  m_gameWorld.update(deltaTime);
}

void
RTSApplication::renderFrame() {
  m_window->clear(sf::Color::Blue);

  m_gameWorld.render();

  ImGui::SFML::Render(*m_window);

  /*
  sf::Text text;
  text.setPosition(0.f, 30.f);
  text.setFont(*m_arialFont);
  text.setCharacterSize(24);
  text.setFillColor(sf::Color::Red);
  text.setString( toString(1.0f/g_time().getFrameDelta()).c_str() );
  m_window->draw(text);
  */
  m_window->display();
}

void
RTSApplication::postInit() {
  m_gameWorld.init(m_window);
  m_gameWorld.updateResolutionData();
}

void
RTSApplication::postDestroy() {
  m_gameWorld.destroy();
}



void
loadMapFromFile(RTSApplication* pApp) {
  OPENFILENAMEW ofn = { 0 };

  WString fileName;
  fileName.resize(MAX_PATH);
  bool bMustLoad = false;

  Path currentDirectory = FileSystem::getWorkingDirectoryPath();

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = nullptr;
  ofn.lpstrDefExt = L".bmp";
  ofn.lpstrFilter = L"Bitmap File\0*.BMP\0All\0*.*\0";
  ofn.lpstrInitialDir = L"Maps\\";
  ofn.lpstrFile = &fileName[0];
  ofn.lpstrFile[0] = '\0';
  ofn.nMaxFile = MAX_PATH;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

  if (GetOpenFileNameW(&ofn)) {
    if (fileName.size() > 0) {
      bMustLoad = true;
    }
  }

  SetCurrentDirectoryW(UTF8::toWide(currentDirectory.toString()).c_str());

  if (bMustLoad) {
    pApp->getWorld()->getTiledMap()->loadFromImageFile(pApp->getRenderWindow(),
                                                       toString(fileName));
  }
}

void
mainMenu(RTSApplication* pApp) {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("Map")) {
      if (ImGui::MenuItem("Load...", "CTRL+O")) {
        loadMapFromFile(pApp);
      }
      if (ImGui::MenuItem("Save...", "CTRL+S")) {

      }
      ImGui::Separator();

      if (ImGui::MenuItem("Quit", "CTRL+Q")) {
        pApp->getRenderWindow()->close();
      }

      ImGui::EndMenu();
    }
    
    ImGui::EndMainMenuBar();
  }

  ImGui::Begin("Game Options");
  {
    ImGui::Text("Framerate: %f", pApp->getFPS());

    ImGui::SliderFloat("Map movement speed X",
      &GameOptions::s_MapMovementSpeed.x,
      0.0f,
      10240.0f);
    ImGui::SliderFloat("Map movement speed Y",
      &GameOptions::s_MapMovementSpeed.y,
      0.0f,
      10240.0f);

    ImGui::Checkbox("Show grid", &GameOptions::s_MapShowGrid);
   
    ImGui::SliderFloat("Zoom",
      &g_cameraZoom,
      0.3f,
      2.0f);
    pApp->setZoomToView(g_cameraZoom);

    
    if (ImGui::BeginCombo("ADD", g_AddOptionPreviw.c_str()))
    {
      g_AddOptionPreviw = "";
      ImGui::ListBox("ADD", &g_AddOption, g_AddList, IM_ARRAYSIZE(g_AddList));
      g_AddOptionPreviw = g_AddList[g_AddOption];
      ImGui::EndCombo();
    }

    if (ImGui::BeginCombo("Path Finding type", g_PathFindingOptionPreviw.c_str()))
    {
      g_PathFindingOptionPreviw = "";
      ImGui::ListBox("Parh Finding", &g_PathOption, g_PathList, IM_ARRAYSIZE(g_PathList));
      g_PathFindingOptionPreviw = g_PathList[g_PathOption];
      pApp->getWorld()->getTiledMap()->setPathFinding(g_PathOption);
      ImGui::EndCombo();
    }

    
    if (ImGui::BeginCombo("Euristic", g_EuristicOptionPreviw.c_str()))
    {
      g_EuristicOptionPreviw = "";
      ImGui::ListBox("Euristic", &g_EuristicOption, g_EuristicList, IM_ARRAYSIZE(g_EuristicList));
      g_EuristicOptionPreviw = g_EuristicList[g_EuristicOption];
      pApp->getWorld()->getTiledMap()->setEuristic(g_EuristicOption);
      ImGui::EndCombo();
    }

    ImGui::SliderFloat("Time to paint Tails", &g_TimeToPaintTail, 0.0f, 1.0f);
    pApp->getWorld()->getTiledMap()->setTimeToPaintNextTail(g_TimeToPaintTail);
    
    if (ImGui::Button("startSearch"))
    {
      pApp->getWorld()->getTiledMap()->startPathFinding();
    }

    ImGui::SliderFloat("Euristic Relevance", &g_euristicRelevande, 0.0f, 2.0f);
    pApp->getWorld()->getTiledMap()->setEuristicRelevance(g_euristicRelevande);

    ImGui::SliderFloat("Cost Relevance", &g_costRelevande, 0.1f, 2.0f);
    pApp->getWorld()->getTiledMap()->setCostRelevance(g_costRelevande);
  }
  ImGui::End();
}
