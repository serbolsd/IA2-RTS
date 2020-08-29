function ConfigGame()

  Resolution_x = 1366;
  Resolution_y = 768;
  
  windowName = "LuaTest";
  fullWindow = false;

  MapMovementSpeed_x = 1024;
  MapMovementSpeed_y = 1024;
  CameraPosition_x = 0;
  CameraPosition_y = 0;
  
  
  MapShowGrid = false;
  MapGridColor_r = 255;
  MapGridColor_g = 0;
  MapGridColor_b = 0;
  MapGridColor_a = 255;
  
  MapSize_rows = 256;
  MapSize_columns = 256;

  MAP_IS_ISOMETRIC =  false;
  ISO_TILE_SIZE_X=64;
  ISO_TILE_SIZE_Y=32;
  
  NORMAL_TILE_SIZE_X=64;
  NORMAL_TILE_SIZE_Y=64;

  setResolution(Resolution_x, Resolution_y);
  setWindowsName(windowName);
  setWindowStyle(fullWindow);
  setMapMovementSpeed(MapMovementSpeed_x,MapMovementSpeed_y);
  setCameraPosition(CameraPosition_x,CameraPosition_y);
  setMapShowGrid(MapShowGrid);
  setMapGridColor(MapGridColor_r,MapGridColor_g,MapGridColor_b,MapGridColor_a);
  setMapSize(MapSize_rows, MapSize_columns);
  setIfMapIsISO(MAP_IS_ISOMETRIC);
  --setTileSize(ISO_TILE_SIZE_X,ISO_TILE_SIZE_Y);
  setTileSize(NORMAL_TILE_SIZE_X,NORMAL_TILE_SIZE_Y);
end




