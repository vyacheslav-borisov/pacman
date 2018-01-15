#pragma once
#include "default_game_state.h"
#include "gui_layer.h"
#include "tile_grid.h"

namespace Pegas
{
	

	/********************************************************************************************************
		Editor
	*********************************************************************************************************/
	class EditorLayer: public GUILayer, public IEventListener
	{
	public:
		EditorLayer(IPlatformContext* context);

		virtual void create(IPlatformContext* context);
		virtual void onKeyDown(KeyCode key, KeyFlags flags);
		virtual void onKeyUp(KeyCode key, KeyFlags flags);
		virtual void onMouseButtonDown(MouseButton button, float x, float y, MouseFlags flags);
		virtual void render(IPlatformContext* context);

		virtual void destroy(IPlatformContext* context);
		virtual void handleEvent(EventPtr evt);
		virtual ListenerType getListenerName() { return "EditorLayer"; }
	private:
		//режимы редактора
		enum EditorMode
		{
			k_editorMode_None, //никаких действий 
			k_editorMode_GridPan, //перемещение сетки по экрану (SPACE + UP/DOWN/LEFT/RIGHT)
			k_editorMode_GridCellSizing, //изменение размера €чейки сетки (C + UP/DOWN/LEFT/RIGHT)
			k_editorMode_GridCellsTweak, //изменение количества €чеек по горизонтали и вертикали (CTRL + UP/DOWN/LEFT/RIGHT)
			k_editorMode_TilesPlacement,//расстановка тайлов на сетке (<Num> + кликнуть мышью в €чейку сетки)
			k_editorMode_GoalNodePlacement
		};

		EditorMode m_currentEditorMode;
		int32	 m_debugDrawFlags;
		TILEID m_currentTile;

		CURCOORD m_gridPanStep;
		CURCOORD m_cellSizingStep;
		
		int32 m_minRows;
		int32 m_minColumns;
	private:
		IPlatformContext* m_context;	
		
		TileGrid m_tileGrid;
		SpriteParameters m_maze;

		TextParameters m_statusTextParams;
		String m_status;

		std::map<TILEID, SpriteParameters> m_staticSprites;
		bool m_showSprites;

		CURCOORD m_workZone_fromX;
		CURCOORD m_workZone_fromY;
		CURCOORD m_workZone_toX;
		CURCOORD m_workZone_toY;		
	};

	class Editor: public DefaultGameState
	{
	public:
		Editor();
		virtual void enter(IPlatformContext* context);
	};
}