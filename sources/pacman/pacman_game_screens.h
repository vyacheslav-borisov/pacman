#pragma once
#include "default_game_state.h"
#include "gui_layer.h"

namespace Pegas
{
	enum PacmanLayers
	{
		k_layerMainMenu = 1,
		k_layerOptions,
		k_layerPause,
		k_layerFader,
		k_layerEditor,
		k_layerGameWorld,
		k_layerDebug
	};

	enum PacmanGameStates
	{
		k_stateMainMenu = 1,
		k_stateGame,
		k_stateEditor		
	};

	const CURCOORD k_mazeWidth = 470.0f;
	const CURCOORD k_mazeHeight = 519.0f;
	const CURCOORD k_topBarHeight = 15;
	const CURCOORD k_bottomBarHeight = 10;
	const CURCOORD k_sideMargin = 5;

	/*******************************************************************************************************
		Main Menu
	********************************************************************************************************/
	class MainMenuLayer: public GUILayer, public IEventListener 
	{
	public:
		MainMenuLayer();

		virtual void create(IPlatformContext* context);
		virtual void destroy(IPlatformContext* context);
		virtual void onKeyDown(KeyCode key, KeyFlags flags);
		virtual void handleEvent(EventPtr evt);
		virtual void render(IPlatformContext* context);

		virtual ListenerType getListenerName() { return "MainMenuLayer"; }
	private:
		bool m_aboutToExit;
		IPlatformContext* m_platform;

		SpriteParameters m_logo;
		static const CURCOORD k_logoWidth;
		static const CURCOORD k_logoHeight;		
	};

	class MainMenu: public DefaultGameState
	{
	public:
		MainMenu();
		virtual void enter(IPlatformContext* context);
	};

	class Options: public GUILayer {};

	class GlobalPause: public GUILayer, public IEventListener 
	{
	public:
		GlobalPause();

		virtual void create(IPlatformContext* context);
		virtual void destroy(IPlatformContext* context);
		virtual void render(IPlatformContext* context);
		virtual void handleEvent(EventPtr evt);
		virtual ListenerType getListenerName() { return "GlobalPause"; }

		virtual void onKeyDown(KeyCode key, KeyFlags flags);
		virtual void onKeyUp(KeyCode key, KeyFlags flags);
		virtual void onActivate(bool isActive);

	private:
		IPlatformContext* m_platform;

		CURCOORD m_canvasWidth;
		CURCOORD m_canvasHeight;
		bool m_notClose;
	};	
}