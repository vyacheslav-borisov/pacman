#pragma once
#include "platform_windows.h"

namespace Pegas
{

	class GameApplication: public Singleton<GameApplication>, public IPlatformContext, public IEventListener
	{
	public:
		GameApplication(HINSTANCE hInstance): 
		  Singleton(*this), m_hInstance(hInstance), m_isActive(false), m_exitApplication(false) {};
		virtual ~GameApplication() {};
		
		void init(HWND hWnd);
		bool run();
		void cleanup();
		void resize(int width, int height);
		void activate(bool bActive) { m_isActive = bActive; };

		void processInput(UINT message, WPARAM wParam, LPARAM lParam);

		virtual void handleEvent(EventPtr evt);
		virtual ListenerType getListenerName() { return "GameApplication"; }

	public:

		virtual void addKeyboardController(IKeyboardController* controller);
		virtual void removeKeyboardController(IKeyboardController* controller);
		virtual void addMouseController(IMouseController* controller);
		virtual void removeMouseController(IMouseController* controller);
		
		virtual ProcessHandle attachProcess(ProcessPtr process);
		virtual void terminateProcess(const ProcessHandle& handle);

		virtual ISerializer* createFile(const String& fileName);
		virtual ISerializer* openFile(const String& fileName, uint32 mode);
		virtual void closeFile(ISerializer* file);

		virtual void addGameState(GameStatePtr state);
		virtual void removeState(GameStateID id);
		virtual void changeState(GameStateID newStateId);
		virtual void forwardToState(GameStateID newStateId);
		virtual void backwardToPreviousState();
		virtual void shutdownGame();

	private:
		std::set<IKeyboardController*> m_keyboardControllers;
		std::set<IMouseController*>	m_mouseControllers;

		std::map<GameStateID, GameStatePtr> m_statesMap;
		std::stack<GameStatePtr> m_statesStack;

		typedef SmartPointer<File> FilePtr;
		std::list<FilePtr> m_openedFiles;

	private:
		GameApplication(const GameApplication& src);
		GameApplication& operator=(const GameApplication& src);

		void registerResources();

		void _changeState(GameStateID newStateId);
		void _forwardToState(GameStateID newStateId);
		void _backwardToPreviousState();
		
		HINSTANCE				m_hInstance;
		HWND					m_hWnd;
		bool                    m_isActive;
		bool					m_exitApplication;

		//graphics, sound and tools
		OGLGrafManager			m_grafManager;
		WinMMSoundManager       m_soundPlayer;
		WindowsOSUtils			m_utils;

		//resources
		StringResourceManager	m_stringManager;
		TextureResourceManager	m_textureManager;
		FontResourceManager		m_fontManager;
		SoundResourceManager    m_soundManager;

		//game loop
		EventManager			m_eventManager;
		ProcessManager			m_processManager;
		MILLISECONDS			m_lastTime;
	};

}

