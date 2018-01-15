#pragma once

#include "default_game_state.h"
#include "game_world.h"

namespace Pegas
{
	class Frag: public Process
	{
	public:
		Frag(int32 scores, const Vector3& position, float lifeTime);

		virtual void update(MILLISECONDS deltaTime);
		void draw();
	private:
		String m_fragText;
		TextParameters m_textParams;
		float m_initialLifeTime;
		float m_lifeTime;
	};

	class GameVerticalLayer: public BaseScreenLayer, public IEventListener
	{
	public:
		GameVerticalLayer();

		virtual void create(IPlatformContext* context);
		virtual void destroy(IPlatformContext* context);
		virtual void update(IPlatformContext* context, MILLISECONDS deltaTime, MILLISECONDS timeLimit);
		virtual void render(IPlatformContext* context);	
		virtual void onKeyDown(KeyCode key, KeyFlags flags);
		virtual void onKeyUp(KeyCode key, KeyFlags flags);
		virtual void handleEvent(EventPtr evt);
		virtual ListenerType getListenerName() { return "GameVerticalLayer"; }

	private:
		GameWorld m_gameWorld;
		IPlatformContext* m_context;

		SpriteParameters m_maze;
		SpriteParameters m_scoresTextSprite;
		SpriteParameters m_livesText;
		SpriteParameters m_liveIcon;
		
		TextParameters	 m_scoresTextParams;
		String			 m_scoresText;
		TextParameters	 m_levelTextParams;
		String			 m_levelText;

		int32 m_numLives;
		int32 m_numScore;
		int32 m_level;
		
		int32 m_controlledActor;
		int32 m_keyDownMutex;

		std::list<ProcessPtr> m_frags;
	};

	class DebugLayer: public BaseScreenLayer
	{
	public:
		DebugLayer();
		virtual void create(IPlatformContext* context);
		virtual void update(IPlatformContext* context, MILLISECONDS deltaTime, MILLISECONDS timeLimit);
		virtual void render(IPlatformContext* context);

	private:
		MILLISECONDS	m_ellapsedTime;
		int32			m_frames;
		String			m_fpsText;
		TextParameters	m_params;
	};

	class GameScreen: public DefaultGameState, public IEventListener
	{
	public:
		GameScreen();
		virtual void enter(IPlatformContext* context);
		virtual void leave(IPlatformContext* context);
		virtual void handleEvent(EventPtr evt);
		virtual ListenerType getListenerName() { return "GameScreen"; }
	};
}