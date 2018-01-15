#include "stdafx.h"
#include "game_world.h"
#include "game_ai.h"

#include "game_resources.h"
#include "pacman_game_screens.h"
#include "fader_layer.h"
#include "waiting.h"

#include "platform_windows.h"

using namespace Pegas;

/**************************************************************************************************************
	GameWorld class
***************************************************************************************************************/

const int32 GameWorld::k_scoresForPile = 10;
const int32 GameWorld::k_numLives = 4;
const int32 GameWorld::k_baseScoresForFrag = 200;
const int32 GameWorld::k_baseScoresToLife = 10000;
const int32 GameWorld::k_updateScoresToLife = 5000;
const float GameWorld::k_baseSuperForceTime = 10.0f;
const int32 GameWorld::k_baseScoresForBonus = 100;
const int32 GameWorld::k_baseBonusScoresPerLevel = 200;

GameWorld::GameWorld()
{
	m_superForceTimer1 = 0;
	m_superForceTimer2 = 0;
	m_bonusActive = false;
	m_bonusRow = 0;
	m_bonusColumn = 0;
}

void GameWorld::create(IPlatformContext* context)
{
	m_context = context;

	TheEventMgr.addEventListener(this, Event_CharacterKilled::k_type);
	TheEventMgr.addEventListener(this, Event_PacmanSwallowedPill::k_type);
	TheEventMgr.addEventListener(this, Event_PacmanDeathComplete::k_type);
	TheEventMgr.addEventListener(this, Event_RestartGame::k_type);
	TheEventMgr.addEventListener(this, Event_BonusOn::k_type);
	TheEventMgr.addEventListener(this, Event_BonusOff::k_type);
	TheEventMgr.addEventListener(this, Event_Game_Pause::k_type);
	TheEventMgr.addEventListener(this, Event_Game_Resume::k_type);
	TheEventMgr.addEventListener(this, Event_CharacterTonnelIn::k_type);
	
	loadMap();
	createGameObjects();
	initializeGame();	
}

void GameWorld::destroy(IPlatformContext* context)
{
	for(GameObjectListIt it = m_gameObjects.begin(); it != m_gameObjects.end(); ++it)
	{
		(*it)->destroy();
	}

	m_tileGrid.destroy();

	TheEventMgr.removeEventListener(this);
}

void GameWorld::loadMap()
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

			
			m_sprites[id]._left = left;
			m_sprites[id]._top = top;
			m_sprites[id]._width = width;
			m_sprites[id]._height = height;			
		}//for(int32 i = 0; i < numSprites; i++)				

		m_context->closeFile(fileStream);

		OSUtils::getInstance().debugOutput("loading complete");
	}else
	{
		OSUtils::getInstance().debugOutput("can not open file");
	}

	m_remainPiles = m_tileGrid.getNumTiles(k_tilePill);

	std::list<Vector3> tunnels;
	m_tileGrid.getTiles(k_tileTunnel, tunnels, true);
	assert(tunnels.size() >= 2 && "tunnels not found");

	CURCOORD spriteWidth = m_tileGrid.getCellWidth() * 3;
	CURCOORD spriteHeight = m_tileGrid.getCellWidth() * 4;
	for(std::list<Vector3>::iterator it = tunnels.begin(); it != tunnels.end(); ++it)
	{
		Tunnel tunnel;
		m_tileGrid.pointToCell((*it)._x, (*it)._y, tunnel._row, tunnel._column);
		tunnel._width = spriteWidth;
		tunnel._height = spriteHeight;
		tunnel._left = (*it)._x - (tunnel._width * 0.5);
		tunnel._top = (*it)._y - (tunnel._height * 0.5);

		m_tunnels.push_back(tunnel);
	}
}

void GameWorld::createGameObjects()
{
	std::list<Vector3> tiles;
	m_tileGrid.getTiles(k_tilePacman, tiles, true);
	assert(tiles.size() > 0 && "pacman position not found");
	assert(m_sprites.count(k_tilePacman) > 0 && "pacman position not found");

	/****************************************************************
		Pacman
	*****************************************************************/
	if(tiles.size() > 0 && m_sprites.count(k_tilePacman) > 0)
	{
		Pacman* pacman = new Pacman(k_actorPacman, m_context);
		pacman->create(&m_tileGrid, tiles.front());

		m_gameObjects.push_back(GameObjectPtr(pacman));

		SpriteAnimation* running = new SpriteAnimation(k_textureCharactersSpriteSheet, 8, 8);
		running->setNumFrames(0, 4);
		running->setFPS(16);
		running->getSprite()->_width = m_sprites[k_tilePacman]._width;
		running->getSprite()->_height = m_sprites[k_tilePacman]._height;

		pacman->setAnimation(Pacman::k_animationRunning, ProcessPtr(running));

		SpriteAnimation* death = new SpriteAnimation(k_textureCharactersSpriteSheet, 8, 8, 0);
		death->setNumFrames(8, 8);
		death->setFPS(8);
		death->getSprite()->_width = m_sprites[k_tilePacman]._width;
		death->getSprite()->_height = m_sprites[k_tilePacman]._height;

		pacman->setAnimation(Pacman::k_animationDeath, ProcessPtr(death));
	}

	

	int32 blinkyStartFrame[] = {28, 24, 30, 26, 62, 60, 62, 61, 56, 56 };
	
	int32 pinkyStartFrame[] = {36, 32, 38, 34, 62, 60, 62, 61, 56, 56 };
	
	int32 inkyStartFrame[] = {44, 40, 46, 42, 62, 60, 62, 61, 56, 56 };
	
	int32 clydeStartFrame[] = {52, 48, 54, 50, 62, 60, 62, 61, 56, 56 };

	int32 numFrames[] = {2, 2, 2, 2, 1, 1, 1, 1, 2, 4 };

	/******************************************************************
		Blinky
	*******************************************************************/
	
	tiles.clear();
	m_tileGrid.getTiles(k_tileBlinky, tiles, true);
	
	assert(tiles.size() > 0 && "Blinky position not found");
	assert(m_sprites.count(k_tileBlinky) > 0 && "Blinky position not found");

	if(tiles.size() > 0 && m_sprites.count(k_tileBlinky) > 0)
	{
		Ghost* blinky = new Ghost(k_actorBlinky, m_context); 
		blinky->create(&m_tileGrid, tiles.front());

		m_gameObjects.push_back(GameObjectPtr(blinky));
		
		for(int32 i = 0; i < Ghost::k_animationTotal; i++)
		{
			SpriteAnimation* animation = new SpriteAnimation(k_textureCharactersSpriteSheet, 8, 8);
			animation->setNumFrames(blinkyStartFrame[i], numFrames[i]);
			animation->setFPS(8);
			animation->getSprite()->_width = m_sprites[k_tileBlinky]._width;
			animation->getSprite()->_height = m_sprites[k_tileBlinky]._height;

			blinky->setAnimation(i, ProcessPtr(animation));
		}

		ProcessPtr chaseState = new BlinkyChaseState(&m_tileGrid);
		m_context->attachProcess(chaseState);

		ProcessPtr runAwayState = new RunawayState(&m_tileGrid, k_actorBlinky);
		m_context->attachProcess(runAwayState);

		ProcessPtr prayState = new PrayState(&m_tileGrid, k_actorBlinky);
		m_context->attachProcess(prayState);
	}
	
	/******************************************************************
		Pinky
	*******************************************************************/
	
	tiles.clear();
	m_tileGrid.getTiles(k_tilePinky, tiles, true);
	
	assert(tiles.size() > 0 && "Pinky position not found");
	assert(m_sprites.count(k_tilePinky) > 0 && "Pinky position not found");

	if(tiles.size() > 0 && m_sprites.count(k_tilePinky) > 0)
	{
		Ghost* pinky = new Ghost(k_actorPinky, m_context); 
		pinky->create(&m_tileGrid, tiles.front());

		m_gameObjects.push_back(GameObjectPtr(pinky));
		
		for(int32 i = 0; i < Ghost::k_animationTotal; i++)
		{
			SpriteAnimation* animation = new SpriteAnimation(k_textureCharactersSpriteSheet, 8, 8);
			animation->setNumFrames(pinkyStartFrame[i], numFrames[i]);
			animation->setFPS(8);
			animation->getSprite()->_width = m_sprites[k_tilePinky]._width;
			animation->getSprite()->_height = m_sprites[k_tilePinky]._height;

			pinky->setAnimation(i, ProcessPtr(animation));
		}

		ProcessPtr chaseState = new PinkyChaseState(&m_tileGrid);
		m_context->attachProcess(chaseState);

		ProcessPtr runAwayState = new RunawayState(&m_tileGrid, k_actorPinky);
		m_context->attachProcess(runAwayState);

		ProcessPtr prayState = new PrayState(&m_tileGrid, k_actorPinky);
		m_context->attachProcess(prayState);
	}
	
	/******************************************************************
		Inky
	*******************************************************************/
	
	tiles.clear();
	m_tileGrid.getTiles(k_tileInky, tiles, true);
	
	assert(tiles.size() > 0 && "Inky position not found");
	assert(m_sprites.count(k_tileInky) > 0 && "Inky position not found");

	if(tiles.size() > 0 && m_sprites.count(k_tileInky) > 0)
	{
		Ghost* Inky = new Ghost(k_actorInky, m_context); 
		Inky->create(&m_tileGrid, tiles.front());

		m_gameObjects.push_back(GameObjectPtr(Inky));
		
		for(int32 i = 0; i < Ghost::k_animationTotal; i++)
		{
			SpriteAnimation* animation = new SpriteAnimation(k_textureCharactersSpriteSheet, 8, 8);
			animation->setNumFrames(inkyStartFrame[i], numFrames[i]);
			animation->setFPS(8);
			animation->getSprite()->_width = m_sprites[k_tileInky]._width;
			animation->getSprite()->_height = m_sprites[k_tileInky]._height;

			Inky->setAnimation(i, ProcessPtr(animation));
		}

		ProcessPtr chaseState = new InkyChaseState(&m_tileGrid);
		m_context->attachProcess(chaseState);

		ProcessPtr runAwayState = new RunawayState(&m_tileGrid, k_actorInky);
		m_context->attachProcess(runAwayState);

		ProcessPtr prayState = new PrayState(&m_tileGrid, k_actorInky);
		m_context->attachProcess(prayState);
	}

	/******************************************************************
		Clyde
	*******************************************************************/
	tiles.clear();
	m_tileGrid.getTiles(k_tileClyde, tiles, true);
	
	assert(tiles.size() > 0 && "Inky position not found");
	assert(m_sprites.count(k_tileClyde) > 0 && "Inky position not found");

	if(tiles.size() > 0 && m_sprites.count(k_tileClyde) > 0)
	{
		Ghost* Clyde = new Ghost(k_actorClyde, m_context); 
		Clyde->create(&m_tileGrid, tiles.front());

		m_gameObjects.push_back(GameObjectPtr(Clyde));
		
		for(int32 i = 0; i < Ghost::k_animationTotal; i++)
		{
			SpriteAnimation* animation = new SpriteAnimation(k_textureCharactersSpriteSheet, 8, 8);
			animation->setNumFrames(clydeStartFrame[i], numFrames[i]);
			animation->setFPS(8);
			animation->getSprite()->_width = m_sprites[k_tileClyde]._width;
			animation->getSprite()->_height = m_sprites[k_tileClyde]._height;

			Clyde->setAnimation(i, ProcessPtr(animation));
		}

		ProcessPtr chaseState = new ClydeChaseState(&m_tileGrid);
		m_context->attachProcess(chaseState);

		ProcessPtr runAwayState = new RunawayState(&m_tileGrid, k_actorClyde);
		m_context->attachProcess(runAwayState);

		ProcessPtr prayState = new PrayState(&m_tileGrid, k_actorClyde);
		m_context->attachProcess(prayState);
	}
}

void GameWorld::initializeGame()
{
	m_currentLevel = 1;
	m_currentScores = 0;
	m_remainLives = k_numLives;
	m_scoresToLive = k_baseScoresToLife;
	m_updateScoresToLive = k_baseScoresToLife + k_updateScoresToLife;

	EventPtr evtLevel(new Event_HUD_LevelChanged(m_currentLevel));
	TheEventMgr.pushEventToQueye(evtLevel);
	
	EventPtr evtScores(new Event_HUD_ScoresChanged(m_currentScores));
	TheEventMgr.pushEventToQueye(evtScores);

	EventPtr evtLives(new Event_HUD_LivesChanged(m_remainLives));
	TheEventMgr.pushEventToQueye(evtLives);

	EventPtr evtDisableControl(new Event_DisableCharacterControl(k_actorAll));
	TheEventMgr.pushEventToQueye(evtDisableControl);

	EventPtr evtGetReady(new Event_HUD_GetReady());
	TheEventMgr.pushEventToQueye(evtGetReady);
		
	Waiting* waiting = new Waiting(3.0f);
	waiting->addFinalEvent(EventPtr(new Event_RestartGame()));
	m_context->attachProcess(ProcessPtr(waiting));
}

void GameWorld::checkNewLife()
{
	if(m_currentScores >= m_scoresToLive)
	{
		m_remainLives++;
		m_scoresToLive+= m_updateScoresToLive;
		m_updateScoresToLive = k_updateScoresToLife;

		EventPtr evt(new Event_HUD_LivesChanged(m_remainLives));
		TheEventMgr.pushEventToQueye(evt);
	}
}

void GameWorld::restartGame()
{
	EventPtr evt(new Event_HUD_GetReady(false));
	TheEventMgr.pushEventToQueye(evt);

	EventPtr evt2(new Event_EnableCharacterControl(k_actorPacman));
	EventPtr evt3(new Event_EnableCharacterControl(k_actorBlinky));

	TheEventMgr.pushEventToQueye(evt2);
	TheEventMgr.pushEventToQueye(evt3);
	
	Waiting* waiting = new Waiting(2.0f);
	waiting->addFinalEvent(EventPtr(new Event_EnableCharacterControl(k_actorPinky)));
	m_context->attachProcess(ProcessPtr(waiting));
		
	Waiting* waiting2 = new Waiting(2.0f);
	waiting2->addFinalEvent(EventPtr(new Event_EnableCharacterControl(k_actorInky)));
	waiting->attachNext(ProcessPtr(waiting2));
		
	Waiting* waiting3 = new Waiting(2.0f);
	waiting3->addFinalEvent(EventPtr(new Event_EnableCharacterControl(k_actorClyde)));
	waiting2->attachNext(ProcessPtr(waiting3));
}

void GameWorld::update(IPlatformContext* context, MILLISECONDS deltaTime, MILLISECONDS timeLimit)
{
	float fDeltaTime = deltaTime / 1000.0f;
	for(GameObjectListIt it = m_gameObjects.begin(); it != m_gameObjects.end(); ++it)
	{
		(*it)->update(fDeltaTime);
	}
}

void GameWorld::render(IPlatformContext* context)
{
	m_tileGrid.draw();

	for(GameObjectListIt it = m_gameObjects.begin(); it != m_gameObjects.end(); ++it)
	{
		(*it)->draw();
	}

	GrafManager& graf = GrafManager::getInstance();
	for(std::vector<Tunnel>::iterator it = m_tunnels.begin(); it != m_tunnels.end(); ++it)
	{
		graf.drawRectangle((*it)._left, (*it)._top, (*it)._width, (*it)._height, 0xff000000, 0xff000000);
	}
}

void GameWorld::handleEvent(EventPtr evt)
{
	if(evt->getType() == Event_PacmanSwallowedPill::k_type)
	{
		Event_PacmanSwallowedPill* pEvent = evt->cast<Event_PacmanSwallowedPill>();
		if(pEvent->_pill == k_tilePill)
		{
			m_tileGrid.setTile(pEvent->_row, pEvent->_column, TileGrid::k_emptyCellTileId);

			m_currentScores+= k_scoresForPile;
			EventPtr evt(new Event_HUD_ScoresChanged(m_currentScores));
			TheEventMgr.pushEventToQueye(evt);
			checkNewLife();
			
			m_remainPiles--;
			if(m_remainPiles <= 0)
			{
				EventPtr blockControl(new Event_DisableCharacterControl(k_actorAll));
				TheEventMgr.pushEventToQueye(blockControl);

				EventPtr hideCharacters(new Event_HideCharacter(k_actorAll));
				TheEventMgr.pushEventToQueye(hideCharacters);

				EventPtr fadeOn(new Event_GUI_FadeOn(50));
				TheEventMgr.pushEventToQueye(fadeOn);

				loadMap();

				m_currentLevel++;
				EventPtr levelChanged(new Event_HUD_LevelChanged(m_currentLevel));
				TheEventMgr.pushEventToQueye(levelChanged);
				EventPtr newLevel(new Event_HUD_NewLevel(m_currentLevel, 2.0f));
				TheEventMgr.pushEventToQueye(newLevel);
				EventPtr resetActors(new Event_ResetActors());
				TheEventMgr.pushEventToQueye(resetActors);

				Waiting* waiting1 = new Waiting(2.0f);
				waiting1->addFinalEvent(EventPtr(new Event_GUI_FadeOff()));
				waiting1->addFinalEvent(EventPtr(new Event_HUD_GetReady()));
				m_context->attachProcess(ProcessPtr(waiting1));

				Waiting* waiting2 = new Waiting(1.0f);
				waiting2->addFinalEvent(EventPtr(new Event_RestartGame()));
				waiting1->attachNext(ProcessPtr(waiting2));
			}
		}
		if(pEvent->_pill == k_tileSuperPill)
		{
			m_tileGrid.setTile(pEvent->_row, pEvent->_column, TileGrid::k_emptyCellTileId);

			EventPtr evt(new Pegas::Event_SuperForceOn());
			TheEventMgr.pushEventToQueye(evt);

			float superForceTime = k_baseSuperForceTime - (1.0f * (m_currentLevel - 1));
			if(superForceTime <= 0.0f)
			{
				superForceTime = 1.0f;
			}
			
			float superForcePreTime = superForceTime - 1.0f;
			if(superForcePreTime <= 0.0f)
			{
				superForcePreTime = 0.1f;
			}

			m_context->terminateProcess(m_superForceTimer1);
			m_context->terminateProcess(m_superForceTimer2);
			
			Waiting* waiting1 = new Waiting(superForceTime);
			waiting1->addFinalEvent(EventPtr(new Event_SuperForceOff()));
			m_superForceTimer1 = m_context->attachProcess(ProcessPtr(waiting1));

			Waiting* waiting2 = new Waiting(superForcePreTime);
			waiting2->addFinalEvent(EventPtr(new Event_SuperForcePreOff()));
			m_superForceTimer2 = m_context->attachProcess(ProcessPtr(waiting2));
			
			m_fragScores = k_baseScoresForFrag;
		}
		if(pEvent->_pill == k_tileBonus && m_bonusActive)
		{
			if(pEvent->_row == m_bonusRow && pEvent->_column == m_bonusColumn)
			{
				int32 bonusScores = k_baseScoresForBonus + (k_baseBonusScoresPerLevel * (m_currentLevel - 1));
				m_currentScores+= bonusScores;

				EventPtr evt(new Event_HUD_ScoresChanged(m_currentScores));
				TheEventMgr.pushEventToQueye(evt);

				checkNewLife();

				Vector3 fragPosition;
				m_tileGrid.cellCoords(pEvent->_row, pEvent->_column, fragPosition._x, fragPosition._y, true);

				EventPtr evt2(new Event_HUD_Frag(bonusScores, fragPosition));
				TheEventMgr.pushEventToQueye(evt2);
			}
		}

		return;
	}

	if(evt->getType() == Event_CharacterTonnelIn::k_type)
	{
		Event_CharacterTonnelIn* pEvent = evt->cast<Event_CharacterTonnelIn>();
		
		for(int32 i = 0; i < m_tunnels.size(); i++)
		{
			if(m_tunnels[i]._row == pEvent->_row && m_tunnels[i]._column == pEvent->_column)
			{
				int32 outTunnel = (i + 1) % m_tunnels.size();
				
				EventPtr evt2(new Event_CharacterTonnelOut(pEvent->_actorId, 
					m_tunnels[outTunnel]._row, m_tunnels[outTunnel]._column));
				TheEventMgr.pushEventToQueye(evt2);

				break;
			}
		}
		return; 
	}

	if(evt->getType() == Event_CharacterKilled::k_type)
	{
		Event_CharacterKilled* pEvent = evt->cast<Event_CharacterKilled>();
		if(pEvent->_actorId == k_actorPacman)
		{
			EventPtr evt(new Event_DisableCharacterControl(k_actorAll));
			TheEventMgr.pushEventToQueye(evt);
			
			Waiting*  waiting = new Waiting(1.0f);
			waiting->addFinalEvent(EventPtr(new Event_PacmanDeath()));
			m_context->attachProcess(ProcessPtr(waiting));
		}else
		{
			m_currentScores+= m_fragScores;
			
			EventPtr evt(new Event_HUD_ScoresChanged(m_currentScores));
			TheEventMgr.pushEventToQueye(evt);
			EventPtr evt2(new Event_HUD_Frag(m_fragScores, pEvent->_position));
			TheEventMgr.pushEventToQueye(evt2);

			m_fragScores*= 2;

			checkNewLife();
		}
		return;
	}

	if(evt->getType() == Event_PacmanDeathComplete::k_type)
	{
		m_remainLives--;
		if(m_remainLives < 0)
		{
			EventPtr evt(new Event_HUD_GameOver());
			TheEventMgr.pushEventToQueye(evt);
			
			ProcessPtr waiting(new Waiting(2.0f));

			ProcessPtr fadein(new Fadein());
			waiting->attachNext(fadein);

			ProcessPtr toMainMenu(new ChangeStateTask(m_context, k_stateMainMenu));
			fadein->attachNext(toMainMenu);

			m_context->attachProcess(waiting);
		}else
		{
			EventPtr evt(new Event_HUD_GetReady());
			TheEventMgr.pushEventToQueye(evt);

			EventPtr evt2(new Event_HUD_LivesChanged(m_remainLives));
			TheEventMgr.pushEventToQueye(evt2);

			EventPtr evt3(new Event_ResetActors());
			TheEventMgr.pushEventToQueye(evt3);

			Waiting* waiting = new Waiting(2.0f);
			waiting->addFinalEvent(EventPtr(new Event_RestartGame()));
			m_context->attachProcess(ProcessPtr(waiting));
		}
		return;
	}

	if(evt->getType() == Event_RestartGame::k_type)
	{
		restartGame();
		return;
	}

	if(evt->getType() == Event_Game_Pause::k_type)
	{
		EventPtr evt(new Event_DisableCharacterControl(k_actorAll));
		TheEventMgr.pushEventToQueye(evt);
		return;
	}

	if(evt->getType() == Event_Game_Resume::k_type)
	{
		EventPtr evt(new Event_EnableCharacterControl(k_actorAll));
		TheEventMgr.pushEventToQueye(evt);
		return;
	}

	if(evt->getType() == Event_BonusOn::k_type)
	{
		Event_BonusOn* pEvent = evt->cast<Event_BonusOn>();
		m_bonusActive = true;
		m_bonusRow = pEvent->_row;
		m_bonusColumn = pEvent->_column;
		return;
	}

	if(evt->getType() == Event_BonusOff::k_type)
	{
		m_bonusActive = false;
		return;
	}

		
}