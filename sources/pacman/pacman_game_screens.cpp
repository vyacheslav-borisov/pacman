#include "stdafx.h"
#include "pacman_game_screens.h"

#include "game_resources.h"
#include "game_events.h"
#include "standart_widgets.h"
#include "fader_layer.h"


using namespace Pegas;

/*****************************************************************************************
	Main Menu 
******************************************************************************************/

const CURCOORD MainMenuLayer::k_logoWidth = 470;
const CURCOORD MainMenuLayer::k_logoHeight = 112;

enum MenuButtons
{
	k_menuButtonStart = 1,
	k_menuButtonOptions,
	k_menuButtonEditor,
	k_menuButtonExit	
};

MainMenuLayer::MainMenuLayer():
	GUILayer(k_layerMainMenu, false)
{
	m_aboutToExit = false;
}


void MainMenuLayer::create(IPlatformContext* context)
{
	GUILayer::create(context);

	m_platform = context;

	TheEventMgr.addEventListener(this, Event_GUI_FadeinComplete::k_type);
	TheEventMgr.addEventListener(this, Event_GUI_ButtonClick::k_type);

	CURCOORD canvasWidth = GrafManager::getInstance().getCanvasWidth();
	CURCOORD canvasHeight = GrafManager::getInstance().getCanvasHeight();

	CURCOORD logoScaledWidth = canvasWidth - 10;
	CURCOORD logoScaledHeight = (CURCOORD)((k_logoHeight * logoScaledWidth * 1.0) / k_logoWidth); 

	m_logo._texture = k_texturePacmanLogo;
	m_logo._left = 5;
	m_logo._top = 5;
	m_logo._width = logoScaledWidth;
	m_logo._height = logoScaledHeight;

	CURCOORD buttonWidth, buttonHeight;
	GrafManager::getInstance().getTextExtent(_text("options"), k_fontMenuButton, buttonWidth, buttonHeight);
	buttonWidth+= 8;
	buttonHeight+= 8;

	float left = (canvasWidth - buttonWidth) * 0.5f;
	float top = (canvasHeight - logoScaledHeight - (buttonHeight * 3.0f) - 50.0f) * 0.5f;
	RGBCOLOR borderColor = 0;
	RGBCOLOR textColor = 0xffffffff;
	RGBCOLOR activeTextColor = 0xffffff33;

	ButtonWidget* startButton = new ButtonWidget(k_menuButtonStart);
	startButton->setPosition(left, top);
	startButton->setSize(buttonWidth, buttonHeight);
	startButton->setButtonStyle(k_buttonStateNormal, k_fontMenuButton, textColor, borderColor);
	startButton->setButtonStyle(k_buttonStatePressed, k_fontMenuButton, textColor, borderColor);
	startButton->setButtonStyle(k_buttonStateActive, k_fontMenuButton, activeTextColor, borderColor);
	startButton->setCaption(_text("start"));

	addWidget(WidgetPtr(startButton));

	top+= (buttonHeight + 20);

	ButtonWidget* optionsButton = new ButtonWidget(k_menuButtonOptions);
	optionsButton->setPosition(left, top);
	optionsButton->setSize(buttonWidth, buttonHeight);
	optionsButton->setButtonStyle(k_buttonStateNormal, k_fontMenuButton, textColor, borderColor);
	optionsButton->setButtonStyle(k_buttonStatePressed, k_fontMenuButton, textColor, borderColor);
	optionsButton->setButtonStyle(k_buttonStateActive, k_fontMenuButton, activeTextColor, borderColor);
	optionsButton->setCaption(_text("options"));

	addWidget(WidgetPtr(optionsButton));

	top+= (buttonHeight + 20);

	ButtonWidget* editorButton = new ButtonWidget(k_menuButtonEditor);
	editorButton->setPosition(left, top);
	editorButton->setSize(buttonWidth, buttonHeight);
	editorButton->setButtonStyle(k_buttonStateNormal, k_fontMenuButton, textColor, borderColor);
	editorButton->setButtonStyle(k_buttonStatePressed, k_fontMenuButton, textColor, borderColor);
	editorButton->setButtonStyle(k_buttonStateActive, k_fontMenuButton, activeTextColor, borderColor);
	editorButton->setCaption(_text("editor"));

	addWidget(WidgetPtr(editorButton));

	top+= (buttonHeight + 20);

	ButtonWidget* exitButton = new ButtonWidget(k_menuButtonExit);
	exitButton->setPosition(left, top);
	exitButton->setSize(buttonWidth, buttonHeight);
	exitButton->setButtonStyle(k_buttonStateNormal, k_fontMenuButton, textColor, borderColor);
	exitButton->setButtonStyle(k_buttonStatePressed, k_fontMenuButton, textColor, borderColor);
	exitButton->setButtonStyle(k_buttonStateActive, k_fontMenuButton, activeTextColor, borderColor);
	exitButton->setCaption(_text("exit"));

	addWidget(WidgetPtr(exitButton));	
}

void MainMenuLayer::destroy(IPlatformContext* context)
{
	GUILayer::destroy(context);

	TheEventMgr.removeEventListener(this);
}

void MainMenuLayer::onKeyDown(KeyCode key, KeyFlags flags)
{
	GUILayer::onKeyDown(key, flags);

	if(key == IKeyboardController::k_keyCodeTAB || key == IKeyboardController::k_keyCodeDOWN)
	{
		changeFocusNext();

	}else if(key == IKeyboardController::k_keyCodeUP)
	{
		changeFocusPrev();
	}
}

void MainMenuLayer::handleEvent(EventPtr evt)
{
	if(evt->getType() == Event_GUI_ButtonClick::k_type)
	{
		Event_GUI_ButtonClick* pEvent = evt->cast<Event_GUI_ButtonClick>();
		if(pEvent->m_button->getID() == k_menuButtonStart || pEvent->m_button->getID() == k_menuButtonExit)
		{
			ProcessPtr fadein = new Fadein();
			m_platform->attachProcess(fadein);
			
			if(pEvent->m_button->getID() == k_menuButtonStart)
			{
				ProcessPtr toGame = new ChangeStateTask(m_platform, k_stateGame);
				fadein->attachNext(toGame);
			}else
			{
				m_aboutToExit = true;
			}
		}

		if(pEvent->m_button->getID() == k_menuButtonEditor)
		{
			ProcessPtr fadein = new Fadein();
			m_platform->attachProcess(fadein);
			ProcessPtr toEditor = new ChangeStateTask(m_platform, k_stateEditor);
			fadein->attachNext(toEditor);
		}
	}

	if(evt->getType() == Event_GUI_FadeinComplete::k_type)
	{
		if(m_aboutToExit)
		{
			m_platform->shutdownGame();
		}
	}
}

void MainMenuLayer::render(IPlatformContext* context)
{
	GUILayer::render(context);

	GrafManager::getInstance().drawSprite(m_logo);
}

/*******************************************************************/
MainMenu::MainMenu():
	DefaultGameState(k_stateMainMenu)
{
	
}

void MainMenu::enter(IPlatformContext* context)
{
	DefaultGameState::enter(context);

	pushLayer(BaseScreenLayerPtr(new MainMenuLayer()));
	pushLayer(BaseScreenLayerPtr(new FaderLayer(k_layerFader)));

	EventPtr evt(new Event_GUI_StartFadeout());
	TheEventMgr.triggerEvent(evt);
}

/**********************************************************************************************************
	GlobalPause
***********************************************************************************************************/
enum GlobalPauseButtons
{
	k_pauseButtonsResume = 1,
	k_pauseButtonsOptions,
	k_pauseButtonsMenu
};

GlobalPause::GlobalPause()
	:GUILayer(k_layerPause, true), m_notClose(false)
{
	
}

void GlobalPause::create(IPlatformContext* context)
{
	GUILayer::create(context);

	m_platform = context;

	TheEventMgr.addEventListener(this, Event_GUI_ButtonClick::k_type);

	m_canvasWidth = GrafManager::getInstance().getCanvasWidth();
	m_canvasHeight = GrafManager::getInstance().getCanvasHeight();

	CURCOORD buttonWidth, buttonHeight;
	GrafManager::getInstance().getTextExtent(_text("options"), k_fontMenuButton, buttonWidth, buttonHeight);

	float left = (m_canvasWidth - buttonWidth) * 0.5f;
	float top = (m_canvasHeight - (buttonHeight * 3.0f) - 40.0f) * 0.5f;
	RGBCOLOR borderColor = 0;
	RGBCOLOR textColor = 0xffffffff;
	RGBCOLOR activeTextColor = 0xffffff33;

	/* resume */
	ButtonWidget* button = new ButtonWidget(k_pauseButtonsResume);
	button->setPosition(left, top);
	button->setSize(buttonWidth, buttonHeight);
	button->setButtonStyle(k_buttonStateNormal, k_fontMenuButton, textColor, borderColor);
	button->setButtonStyle(k_buttonStatePressed, k_fontMenuButton, textColor, borderColor);
	button->setButtonStyle(k_buttonStateActive, k_fontMenuButton, activeTextColor, borderColor);
	button->setCaption(_text("resume"));

	addWidget(WidgetPtr(button));

	top+= (buttonHeight + 20);

	/* options */
	button = new ButtonWidget(k_pauseButtonsOptions);
	button->setPosition(left, top);
	button->setSize(buttonWidth, buttonHeight);
	button->setButtonStyle(k_buttonStateNormal, k_fontMenuButton, textColor, borderColor);
	button->setButtonStyle(k_buttonStatePressed, k_fontMenuButton, textColor, borderColor);
	button->setButtonStyle(k_buttonStateActive, k_fontMenuButton, activeTextColor, borderColor);
	button->setCaption(_text("options"));

	addWidget(WidgetPtr(button));

	top+= (buttonHeight + 20);

	/* menu */
	button = new ButtonWidget(k_pauseButtonsMenu);
	button->setPosition(left, top);
	button->setSize(buttonWidth, buttonHeight);
	button->setButtonStyle(k_buttonStateNormal, k_fontMenuButton, textColor, borderColor);
	button->setButtonStyle(k_buttonStatePressed, k_fontMenuButton, textColor, borderColor);
	button->setButtonStyle(k_buttonStateActive, k_fontMenuButton, activeTextColor, borderColor);
	button->setCaption(_text("quit"));

	addWidget(WidgetPtr(button));	
}

void GlobalPause::destroy(IPlatformContext* context)
{
	GUILayer::destroy(context);

	TheEventMgr.removeEventListener(this);
}

void GlobalPause::render(IPlatformContext* context)
{
	GrafManager::getInstance().drawRectangle(0, 0, m_canvasWidth, m_canvasHeight, 0x99000000, 0x99000000);

	GUILayer::render(context);
}

void GlobalPause::handleEvent(EventPtr evt)
{
	if(evt->getType() == Event_GUI_ButtonClick::k_type)
	{
		Event_GUI_ButtonClick* pEvent = evt->cast<Event_GUI_ButtonClick>();
		if(pEvent->m_button->getID() == k_pauseButtonsResume)
		{
			TheEventMgr.pushEventToQueye(EventPtr(new Event_Game_Resume()));	
		}

		if(pEvent->m_button->getID() == k_pauseButtonsMenu)
		{
			setActivity(false);

			ProcessPtr fadein = new Fadein();
			m_platform->attachProcess(fadein);
			ProcessPtr toMenu = new ChangeStateTask(m_platform, k_stateMainMenu);
			fadein->attachNext(toMenu);
		}
	}
}

void GlobalPause::onKeyDown(KeyCode key, KeyFlags flags)
{
	GUILayer::onKeyDown(key, flags);

	if(key == IKeyboardController::k_keyCodeTAB || key == IKeyboardController::k_keyCodeDOWN)
	{
		changeFocusNext();

	}else if(key == IKeyboardController::k_keyCodeUP)
	{
		changeFocusPrev();
	}

	if(key == IKeyboardController::k_keyCodeESCAPE && !m_notClose)
	{
		TheEventMgr.pushEventToQueye(EventPtr(new Event_Game_Resume()));
	}
}

void GlobalPause::onKeyUp(KeyCode key, KeyFlags flags)
{
	if(key == IKeyboardController::k_keyCodeESCAPE)
	{
		 m_notClose = false;
	}
}

void GlobalPause::onActivate(bool isActive)
{
	m_notClose = true;
}

