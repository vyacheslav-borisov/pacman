#include "stdafx.h"
#include "editor.h"

#include "pacman_game_screens.h"
#include "game_resources.h"
#include "game_events.h"
#include "game_objects.h"
#include "standart_widgets.h"
#include "fader_layer.h"

using namespace Pegas;

/*****************************************************************************************************************
	Editor
*****************************************************************************************************************/


enum EditorButtons
{
	k_editorButtonSave = 1,
	k_editorButtonLoad
};

EditorLayer::EditorLayer(IPlatformContext* context): GUILayer(k_layerEditor, false)
{
	m_context = context;	
}

void EditorLayer::create(IPlatformContext* context)
{
	GUILayer::create(context);

	TheEventMgr.addEventListener(this, Event_GUI_ButtonClick::k_type);

	CURCOORD canvasWidth = GrafManager::getInstance().getCanvasWidth();
	CURCOORD canvasHeight = GrafManager::getInstance().getCanvasHeight();

	//maze sprite setup
	CURCOORD scaledMazeWidth, scaledMazeHeight;
	scaledMazeWidth = canvasWidth - 10;
	scaledMazeHeight = (CURCOORD)((k_mazeHeight * scaledMazeWidth * 1.0) / k_mazeWidth);

	m_maze._texture = k_textureMaze;
	m_maze._top = k_topBarHeight;
	m_maze._left = 5;
	m_maze._width = scaledMazeWidth;
	m_maze._height = scaledMazeHeight;

	//work zone line
	m_workZone_fromX = 0;
	m_workZone_fromY = scaledMazeHeight + k_topBarHeight + k_bottomBarHeight;
	m_workZone_toX = canvasWidth;
	m_workZone_toY = m_workZone_fromY;

	//tile grid setup
	m_debugDrawFlags = TileGrid::k_debugDrawGrid | TileGrid::k_debugDrawObstacles;

	m_tileGrid.create(30, 33);
	m_tileGrid.setArea(m_maze._left, m_maze._top, m_maze._width, m_maze._height);

	/*
		enum Tiles
	{
		k_tilePill = 0,
		k_tileWall,
		k_tileSuperPill,
		k_tilePacman,
		k_tileDoor,
		k_tileTunnel,
		k_tileBlinky,
		k_tilePinky,
		k_tileInky,
		k_tileClyde,
		k_tileBonus,
		k_tilePreyGoalNode,
		k_tileBlinkyGoalNode,
		k_tilePinkyGoalNode,
		k_tileInkyGoalNode,
		k_tileClydeGoalNode
	};
	*/

	m_tileGrid.addTileDesc(TileDesc(k_texturePillTile, false));//k_tilePill
	m_tileGrid.addTileDesc(TileDesc(0, true, k_collisionGroupDefault));//k_tileWall
	m_tileGrid.addTileDesc(TileDesc(k_textureSuperPillTile, false));//k_tileSuperPill

	m_tileGrid.addTileDesc(TileDesc(0, false));//k_tilePacman
	m_tileGrid.addTileDesc(TileDesc(0, true, k_collisionGroupDoor, 0xffffffff));//k_tileDoor
	m_tileGrid.addTileDesc(TileDesc(k_textureTunnelEditorTile, false));//k_tileTunnel

	m_tileGrid.addTileDesc(TileDesc(0, false));//k_tileBlinky
	m_tileGrid.addTileDesc(TileDesc(0, false));//k_tilePinky
	m_tileGrid.addTileDesc(TileDesc(0, false));//k_tileInky
	m_tileGrid.addTileDesc(TileDesc(0, false));//k_tileClyde
	m_tileGrid.addTileDesc(TileDesc(0, false));//k_tileBonus

	m_tileGrid.addTileDesc(TileDesc(0, true, k_collisionGroupGoalNode, 0xff00ff00));//k_tilePreyGoalNode
	m_tileGrid.addTileDesc(TileDesc(0, true, k_collisionGroupGoalNode, 0xff0000ff));//k_tileBlinkyGoalNode
	m_tileGrid.addTileDesc(TileDesc(0, true, k_collisionGroupGoalNode, 0xff0000ff));//k_tilePinkyGoalNode
	m_tileGrid.addTileDesc(TileDesc(0, true, k_collisionGroupGoalNode, 0xff0000ff));//k_tileInkyGoalNode
	m_tileGrid.addTileDesc(TileDesc(0, true, k_collisionGroupGoalNode, 0xff0000ff));//k_tileClydeGoalNode

	m_showSprites = false;

	SpriteParameters staticSprite;
	staticSprite._left = staticSprite._top = 0;
	staticSprite._width = m_tileGrid.getCellWidth();
	staticSprite._height = m_tileGrid.getCellHeight();

	m_staticSprites[k_tilePacman] = staticSprite;
	m_staticSprites[k_tilePacman]._texture = k_texturePacmanStaticSprite;

	m_staticSprites[k_tileBlinky] = staticSprite;
	m_staticSprites[k_tileBlinky]._texture = k_textureBlinkyStaticSprite;

	m_staticSprites[k_tilePinky] = staticSprite;
	m_staticSprites[k_tilePinky]._texture = k_texturePinkyStaticSprite;

	m_staticSprites[k_tileInky] = staticSprite;
	m_staticSprites[k_tileInky]._texture = k_textureInkyStaticSprite;

	m_staticSprites[k_tileClyde] = staticSprite;
	m_staticSprites[k_tileClyde]._texture = k_textureClydeStaticSprite;

	m_staticSprites[k_tileBonus] = staticSprite;
	m_staticSprites[k_tileBonus]._texture = k_textureBonusStaticSprite;

	m_staticSprites[k_tileTunnel] = staticSprite;
	m_staticSprites[k_tileTunnel]._texture = k_textureTunnelStaticSprite;
		
	m_currentEditorMode = k_editorMode_None;
	m_gridPanStep = 0.2;
	m_cellSizingStep = 0.05;
	m_minRows = m_minColumns = 5;	

	//editor buttons;
	float buttonLeft = 5;
	float buttonTop = m_workZone_fromY + 5;
	float buttonWidth = 100;
	float buttonHeight = 30;

	ButtonWidget* saveButton = new ButtonWidget(k_editorButtonSave);
	saveButton->setPosition(buttonLeft, buttonTop);
	saveButton->setSize(buttonWidth, buttonHeight);
	saveButton->setCaption(_text("save"));

	addWidget(WidgetPtr(saveButton));

	buttonTop+= buttonHeight + 10;

	ButtonWidget* loadButton = new ButtonWidget(k_editorButtonLoad);
	loadButton->setPosition(buttonLeft, buttonTop);
	loadButton->setSize(buttonWidth, buttonHeight);
	loadButton->setCaption(_text("load"));

	addWidget(WidgetPtr(loadButton));

	m_statusTextParams._color = 0xffffffff;
	m_statusTextParams._font = k_fontMain;
	m_statusTextParams._left = buttonLeft + buttonWidth + 30;
	m_statusTextParams._top =  buttonTop;
}

void EditorLayer::destroy(IPlatformContext* context)
{
	GUILayer::destroy(context);

	TheEventMgr.removeEventListener(this);
}

void EditorLayer::handleEvent(EventPtr evt)
{
	if(evt->getType() == Event_GUI_ButtonClick::k_type)
	{
		Event_GUI_ButtonClick* pEvent = evt->cast<Event_GUI_ButtonClick>();
		
		if(pEvent->m_button->getID() == k_editorButtonSave)
		{
			int32 canvasWidth = (int32)GrafManager::getInstance().getCanvasWidth();
			int32 canvasHeight = (int32)GrafManager::getInstance().getCanvasHeight();

			tchar fileName[256];
#ifdef		_UNICODE
			wsprintf(fileName, _text("maze_%d_%d.map"), canvasWidth, canvasHeight);  
#else
			sprintf(fileName, _text("maze_%d_%d.map"), canvasWidth, canvasHeight);
#endif
			
			ISerializer* fileStream = m_context->createFile(fileName);
			if(fileStream)
			{
				OSUtils::getInstance().debugOutput("saving...");

				m_tileGrid.save(*fileStream);
				
				(*fileStream) << (int32)m_staticSprites.size();
				for(std::map<TILEID, SpriteParameters>::iterator it = m_staticSprites.begin(); 
					it != m_staticSprites.end(); ++it)
				{
					(*fileStream) << it->first;
					(*fileStream) << it->second._left;
					(*fileStream) << it->second._top;
					(*fileStream) << it->second._width;
					(*fileStream) << it->second._height;
				}

				m_context->closeFile(fileStream);

				OSUtils::getInstance().debugOutput("saving complete");
			}else
			{
				OSUtils::getInstance().debugOutput("can not create file");
			}
		}

		if(pEvent->m_button->getID() == k_editorButtonLoad)
		{
			int32 canvasWidth = (int32)GrafManager::getInstance().getCanvasWidth();
			int32 canvasHeight = (int32)GrafManager::getInstance().getCanvasHeight();

			tchar fileName[256];
#ifdef		_UNICODE
			wsprintf(fileName, _text("maze_%d_%d.map"), canvasWidth, canvasHeight);  
#else
			sprintf(fileName, _text("maze_%d_%d.map"), canvasWidth, canvasHeight);
#endif
			ISerializer* fileStream = m_context->openFile(fileName, FileOpenMode::k_readOnly);
			if(fileStream)
			{
				OSUtils::getInstance().debugOutput("loading...");

				m_tileGrid.load(*fileStream);
				
				int32 numSprites;
				TILEID id;
				CURCOORD left, top, width, height;
				
				(*fileStream) >> numSprites;
				for(int32 i = 0; i < numSprites; i++)
				{
					(*fileStream) >> id;
					(*fileStream) >> left;
					(*fileStream) >> top;
					(*fileStream) >> width;
					(*fileStream) >> height;

					if(m_staticSprites.count(id))
					{
						m_staticSprites[id]._left = left;
						m_staticSprites[id]._top = top;
						m_staticSprites[id]._width = width;
						m_staticSprites[id]._height = height;
					}
				}//for(int32 i = 0; i < numSprites; i++)				

				m_context->closeFile(fileStream);

				OSUtils::getInstance().debugOutput("loading complete");
			}else
			{
				OSUtils::getInstance().debugOutput("can not open file");
			}
		}
	}
}

void EditorLayer::onKeyDown(KeyCode key, KeyFlags flags)
{
	GUILayer::onKeyDown(key, flags);

	if(key == IKeyboardController::k_keyCodeESCAPE)
	{
		ProcessPtr process(new Fadein());
		ProcessPtr process2(new ChangeStateTask(m_context, k_stateMainMenu));

		process->attachNext(process2);
		m_context->attachProcess(process);		
	}

	if(key == IKeyboardController::k_keyCodeSPACE)
	{
		m_currentEditorMode = k_editorMode_GridPan;
	}

	if(key == IKeyboardController::k_keyCodeCTRL)
	{
		m_currentEditorMode = k_editorMode_GridCellsTweak;
	}

	if(key == IKeyboardController::k_keyCode_C)
	{
		m_currentEditorMode = k_editorMode_GridCellSizing;
	}

	if(key == IKeyboardController::k_keyCodeUP
		|| key == IKeyboardController::k_keyCodeDOWN
		|| key == IKeyboardController::k_keyCodeLEFT
		|| key == IKeyboardController::k_keyCodeRIGHT)
	{
		if(m_currentEditorMode == k_editorMode_GridPan)
		{
			CURCOORD left, top, width, height;
			m_tileGrid.getArea(left, top, width, height);

			if(key == IKeyboardController::k_keyCodeUP)
			{
				top-= m_gridPanStep;
			}
			if(key == IKeyboardController::k_keyCodeDOWN)
			{
				top+= m_gridPanStep;
			}
			if(key == IKeyboardController::k_keyCodeLEFT)
			{
				left-= m_gridPanStep;
			}
			if(key == IKeyboardController::k_keyCodeRIGHT)
			{
				left+= m_gridPanStep;
			}			

			m_tileGrid.setArea(left, top, width, height);
		}

		if(m_currentEditorMode == k_editorMode_GridCellSizing)
		{
			CURCOORD currentCellSize = m_tileGrid.getCellWidth();

			if(key == IKeyboardController::k_keyCodeUP)
			{
				currentCellSize+= m_cellSizingStep;				
			}
			if(key == IKeyboardController::k_keyCodeDOWN)
			{
				currentCellSize-= m_cellSizingStep;
				if(currentCellSize < 5) currentCellSize = 5;				
			}
			
			m_tileGrid.setCellSize(currentCellSize, currentCellSize);
		}

		if(m_currentEditorMode == k_editorMode_GridCellsTweak)
		{
			int32 numRows = m_tileGrid.getNumRows();
			int32 numColumns = m_tileGrid.getNumColumns();

			if(key == IKeyboardController::k_keyCodeUP)
			{
				numRows++;
			}
			if(key == IKeyboardController::k_keyCodeDOWN)
			{
				numRows--;

				if(numRows < m_minRows) 
					numRows = m_minRows;
			}
			if(key == IKeyboardController::k_keyCodeLEFT)
			{
				numColumns--;

				if(numColumns < m_minColumns) 
					numColumns = m_minColumns;
			}
			if(key == IKeyboardController::k_keyCodeRIGHT)
			{
				numColumns++;				
			}

			m_tileGrid.create(numRows, numColumns);
		}

		if(m_currentEditorMode == k_editorMode_TilesPlacement && m_showSprites)
		{
			if(key == IKeyboardController::k_keyCodeUP)
			{
				m_staticSprites[m_currentTile]._height += m_cellSizingStep;
				m_staticSprites[m_currentTile]._width += m_cellSizingStep;	
			}

			if(key == IKeyboardController::k_keyCodeDOWN)
			{
				m_staticSprites[m_currentTile]._height -= m_cellSizingStep;
				m_staticSprites[m_currentTile]._width -= m_cellSizingStep;

				if(m_staticSprites[m_currentTile]._height < 5)
				{
					m_staticSprites[m_currentTile]._height = 5;
				}

				if(m_staticSprites[m_currentTile]._width < 5)
				{
					m_staticSprites[m_currentTile]._width = 5;
				}
			}

			std::list<Vector3> tileCoords;
			m_tileGrid.getTiles(m_currentTile, tileCoords, true);
			if(tileCoords.size() > 0)
			{
				m_staticSprites[m_currentTile]._left = tileCoords.front()._x - (m_staticSprites[m_currentTile]._width * 0.5);
				m_staticSprites[m_currentTile]._top = tileCoords.front()._y - (m_staticSprites[m_currentTile]._height * 0.5);
			}
		}
	}

	if(key == IKeyboardController::k_keyCode_Q && !(flags & k_keyFlagRepeat))
	{
		if(m_debugDrawFlags & TileGrid::k_debugDrawGrid)
			m_debugDrawFlags &= (~TileGrid::k_debugDrawGrid);
		else
			m_debugDrawFlags |= TileGrid::k_debugDrawGrid;
	}

	if(key == IKeyboardController::k_keyCode_A && !(flags & k_keyFlagRepeat))
	{
		if(m_debugDrawFlags & TileGrid::k_debugDrawObstacles)
			m_debugDrawFlags &= (~TileGrid::k_debugDrawObstacles);
		else
			m_debugDrawFlags |= TileGrid::k_debugDrawObstacles;
	}

	if(key == IKeyboardController::k_keyCode_S && !(flags & k_keyFlagRepeat))
	{
		m_showSprites = !m_showSprites;
	}

	if(key == IKeyboardController::k_keyCode_1 && !(flags & k_keyFlagRepeat))
	{
		if(m_currentEditorMode == k_editorMode_None)
		{
			m_currentEditorMode = k_editorMode_TilesPlacement;
			m_currentTile = k_tilePill;
			m_status = _text("current mode: tiles placement");
		}

		if(m_currentEditorMode == k_editorMode_GoalNodePlacement)
		{
			m_currentTile = k_tilePreyGoalNode;
			m_status+= _text("[prey goal]");
		}
	}

	if(key == IKeyboardController::k_keyCode_2 && !(flags & k_keyFlagRepeat))
	{
		if(m_currentEditorMode == k_editorMode_None)
		{
			m_currentEditorMode = k_editorMode_TilesPlacement;
			m_currentTile = k_tileWall;
			m_status = _text("current mode: wall obstacle placement");
		}

		if(m_currentEditorMode == k_editorMode_GoalNodePlacement)
		{
			m_currentTile = k_tileBlinkyGoalNode;
			m_status+= _text("[blinky goal]");
		}
	}

	if(key == IKeyboardController::k_keyCode_3 && !(flags & k_keyFlagRepeat))
	{
		if(m_currentEditorMode == k_editorMode_None)
		{
			m_currentEditorMode = k_editorMode_TilesPlacement;
			m_currentTile = k_tileSuperPill;
			m_status = _text("current mode: super pill placement");
		}

		if(m_currentEditorMode == k_editorMode_GoalNodePlacement)
		{
			m_currentTile = k_tilePinkyGoalNode;
			m_status+= _text("[pinky goal]");
		}
	}

	if(key == IKeyboardController::k_keyCode_4 && !(flags & k_keyFlagRepeat))
	{
		if(m_currentEditorMode == k_editorMode_None)
		{
			m_currentEditorMode = k_editorMode_TilesPlacement;
			m_currentTile = k_tilePacman;
			m_status = _text("current mode: pacman placement");
		}

		if(m_currentEditorMode == k_editorMode_GoalNodePlacement)
		{
			m_currentTile = k_tileInkyGoalNode;
			m_status+= _text("[inky goal]");
		}
	}

	if(key == IKeyboardController::k_keyCode_5 && !(flags & k_keyFlagRepeat))
	{
		if(m_currentEditorMode == k_editorMode_None)
		{
			m_currentEditorMode = k_editorMode_TilesPlacement;
			m_currentTile = k_tileDoor;
			m_status = _text("current mode: room door placement");
		}

		if(m_currentEditorMode == k_editorMode_GoalNodePlacement)
		{
			m_currentTile = k_tileClydeGoalNode;
			m_status+= _text("[clyde goal]");
		}
	}

	if(key == IKeyboardController::k_keyCode_6)
	{
		m_currentEditorMode = k_editorMode_TilesPlacement;
		m_currentTile = k_tileTunnel;
		m_status = _text("current mode: tonnel placement");
	}

	if(key == IKeyboardController::k_keyCode_7)
	{
		m_currentEditorMode = k_editorMode_TilesPlacement;
		m_currentTile = k_tileBlinky;
		m_status = _text("current mode: blinky placement");
	}

	if(key == IKeyboardController::k_keyCode_8)
	{
		m_currentEditorMode = k_editorMode_TilesPlacement;
		m_currentTile = k_tilePinky;
		m_status = _text("current mode: pinky placement");
	}

	if(key == IKeyboardController::k_keyCode_9)
	{
		m_currentEditorMode = k_editorMode_TilesPlacement;
		m_currentTile = k_tileInky;
		m_status = _text("current mode: inky placement");
	}

	if(key == IKeyboardController::k_keyCode_0)
	{
		m_currentEditorMode = k_editorMode_TilesPlacement;
		m_currentTile = k_tileClyde;
		m_status = _text("current mode: clyde placement");
	}

	if(key == IKeyboardController::k_keyCode_B)
	{
		m_currentEditorMode = k_editorMode_TilesPlacement;
		m_currentTile = k_tileBonus;
		m_status = _text("current mode: bonus placement");
	}

	if(key == IKeyboardController::k_keyCode_W && !(flags & k_keyFlagRepeat))
	{
		m_debugDrawFlags |= TileGrid::k_debugDrawObstacles;
		m_currentEditorMode = k_editorMode_GoalNodePlacement;
		m_status = _text("current mode: goal node placement ");
	}
}

void EditorLayer::onKeyUp(KeyCode key, KeyFlags flags)
{
	GUILayer::onKeyUp(key, flags);

	if(key != IKeyboardController::k_keyCodeUP
		&& key != IKeyboardController::k_keyCodeDOWN
		&& key != IKeyboardController::k_keyCodeLEFT
		&& key != IKeyboardController::k_keyCodeRIGHT)
	{
		m_currentEditorMode = k_editorMode_None;
		m_currentTile = TileGrid::k_emptyCellTileId;
		m_status = _text("");
	}
}

void EditorLayer::onMouseButtonDown(MouseButton button, float x, float y, MouseFlags flags)
{
	GUILayer::onMouseButtonDown(button, x, y, flags);

	if(button == k_mouseButtonLeft && m_currentEditorMode == k_editorMode_TilesPlacement)
	{
		if(m_currentTile == k_tilePacman 
			|| m_currentTile == k_tileBlinky 
			|| m_currentTile == k_tilePinky
			|| m_currentTile == k_tileInky
			|| m_currentTile == k_tileClyde)
		{
			std::list<Vector3> tileCoords;
			m_tileGrid.getTiles(m_currentTile, tileCoords);
			for(std::list<Vector3>::iterator it = tileCoords.begin(); it != tileCoords.end(); ++it)
			{
				m_tileGrid.setTilePoint(it->_x, it->_y, TileGrid::k_emptyCellTileId); 
			}
		}

		TILEID prevID = m_tileGrid.getTilePoint(x, y);
		prevID = prevID != m_currentTile ? m_currentTile : TileGrid::k_emptyCellTileId;
		m_tileGrid.setTilePoint(x, y, prevID);

		int32 row, col;
		m_tileGrid.pointToCell(x, y, row, col);
		OSUtils::getInstance().debugOutput("tile clicked [row = %d, column = %d]", row, col);

		if(m_staticSprites.count(m_currentTile) > 0)
		{
			SpriteParameters& sprite = m_staticSprites[m_currentTile];
			m_tileGrid.cellCoords(row, col, sprite._left, sprite._top, true);
			sprite._left-= sprite._width * 0.5;
			sprite._top-= sprite._height * 0.5;
		}
	}

	if(button == k_mouseButtonLeft 
		&& m_currentEditorMode == k_editorMode_GoalNodePlacement
		&& m_currentTile != TileGrid::k_emptyCellTileId)
	{
		if(m_currentTile != k_tilePreyGoalNode)
		{
			std::list<Vector3> tileCoords;
			m_tileGrid.getTiles(m_currentTile, tileCoords);
			for(std::list<Vector3>::iterator it = tileCoords.begin(); it != tileCoords.end(); ++it)
			{
				m_tileGrid.setTilePoint(it->_x, it->_y, TileGrid::k_emptyCellTileId); 
			}
		}		

		TILEID prevID = m_tileGrid.getTilePoint(x, y);
		prevID = prevID != m_currentTile ? m_currentTile : TileGrid::k_emptyCellTileId;
		m_tileGrid.setTilePoint(x, y, prevID);

		int32 row, col;
		m_tileGrid.pointToCell(x, y, row, col);
		OSUtils::getInstance().debugOutput("tile clicked [row = %d, column = %d]", row, col);

		if(m_staticSprites.count(m_currentTile) > 0)
		{
			SpriteParameters& sprite = m_staticSprites[m_currentTile];
			m_tileGrid.cellCoords(row, col, sprite._left, sprite._top, true);
			sprite._left-= sprite._width * 0.5;
			sprite._top-= sprite._height * 0.5;
		}
	}
}

void EditorLayer::render(IPlatformContext* context)
{
	GUILayer::render(context);

	GrafManager::getInstance().drawSprite(m_maze);
	GrafManager::getInstance().drawLine(m_workZone_fromX, m_workZone_fromY, m_workZone_toX, m_workZone_toY, 0xff0000ff);

	m_tileGrid.draw();
	m_tileGrid.debugDraw(m_debugDrawFlags);

	GrafManager::getInstance().drawText(m_status, m_statusTextParams);

	if(m_showSprites)
	{
		for(std::map<TILEID, SpriteParameters>::iterator it = m_staticSprites.begin(); it != m_staticSprites.end(); ++it)
		{
			if(m_tileGrid.getNumTiles(it->first) > 0)
			{
				GrafManager::getInstance().drawSprite(it->second);
			}

			if(it->first == m_currentTile)
			{
				GrafManager::getInstance().drawRectangle(it->second._left, it->second._top, 
					it->second._width, it->second._height, 0xff00ff00, 0);
			}
		}//for(std::map<TILEID, SpriteParameters>::iterator it = m_staticSprites.begin();
	}//if(m_showSprites)
}


/*******************************************************************/
Editor::Editor():
	DefaultGameState(k_stateEditor)
{
	
}

void Editor::enter(IPlatformContext* context)
{
	DefaultGameState::enter(context);

	pushLayer(BaseScreenLayerPtr(new EditorLayer(context)));
	pushLayer(BaseScreenLayerPtr(new FaderLayer(k_layerFader)));

	EventPtr evt(new Event_GUI_StartFadeout(1));
	TheEventMgr.triggerEvent(evt);
}