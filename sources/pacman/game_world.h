#pragma once

#include "game_objects.h"
#include "tile_grid.h"

namespace Pegas
{
	class GameWorld: public IEventListener
	{
	public:
		GameWorld();

		virtual void handleEvent(EventPtr evt);
		virtual ListenerType getListenerName() { return "GameWorld"; }

		void create(IPlatformContext* context);
		void destroy(IPlatformContext* context);
		void update(IPlatformContext* context, MILLISECONDS deltaTime, MILLISECONDS timeLimit);
		void render(IPlatformContext* context);
	private:
		typedef SmartPointer<Character> GameObjectPtr;
		typedef std::vector<GameObjectPtr> GameObjectList;
		typedef GameObjectList::iterator GameObjectListIt;
		typedef std::map<TILEID, SpriteParameters> SpriteList;

		struct Tunnel
		{
			int32 _row;
			int32 _column;
			CURCOORD _left;
			CURCOORD _top;
			CURCOORD _width;
			CURCOORD _height;
		};
		std::vector<Tunnel> m_tunnels;

		void loadMap();
		void createGameObjects();
		void initializeGame();
		void checkNewLife();
		void restartGame();

		IPlatformContext* m_context;

		TileGrid		  m_tileGrid;
		SpriteList		  m_sprites;
		GameObjectList	  m_gameObjects;

		int32 m_currentLevel;
		int32 m_currentScores;
		int32 m_remainPiles;
		int32 m_remainLives;

		int32 m_fragScores;
		int32 m_scoresToLive;
		int32 m_updateScoresToLive;

		bool  m_bonusActive;
		int32 m_bonusRow;
		int32 m_bonusColumn;

		ProcessHandle m_superForceTimer1;
		ProcessHandle m_superForceTimer2;

		static const int32 k_scoresForPile;
		static const int32 k_baseScoresForFrag;
		static const int32 k_baseScoresToLife;
		static const int32 k_baseScoresForBonus;
		static const int32 k_baseBonusScoresPerLevel;
		static const int32 k_updateScoresToLife;
		static const int32 k_numLives;
		static const float k_baseSuperForceTime;
	};
}