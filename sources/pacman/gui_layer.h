#pragma once
#include "default_game_state.h"

namespace Pegas
{
	typedef int32 WidgetID;

	class Widget
	{
	public:
		Widget(WidgetID id): m_id(id) {}
		virtual ~Widget() {}

		WidgetID getID() const { return m_id; }

		virtual bool isPointIn(float x, float y) { return false; }
		virtual void setFocus() {}
		virtual void killFocus() {}

		virtual void onMouseButtonDown(MouseButton button, float x, float y, MouseFlags flags) {}
		virtual void onMouseButtonUp(MouseButton button, float x, float y, MouseFlags flags) {}
		virtual void onMouseMove(float x, float y, MouseFlags flags) {}
		virtual void onMouseWheel(NumNothes wheel, MouseFlags flags) {}

		virtual void onKeyDown(KeyCode key, KeyFlags flags) {}
		virtual void onKeyUp(KeyCode key, KeyFlags flags) {}
		virtual void onChar(tchar ch) {}

		virtual void render(IPlatformContext* context) {}

	protected:
		WidgetID m_id;
	};

	
	typedef SmartPointer<Widget> WidgetPtr;

	class GUILayer: public BaseScreenLayer
	{
	public:
		GUILayer(const LayerId& id, bool modal): 
		  BaseScreenLayer(_text("GUI"), id, modal)  
		{
			m_focusedWidget = -1;
		}

		void addWidget(WidgetPtr widget);
		void changeFocusNext();
		void changeFocusPrev();

		WidgetPtr getFocusedWidget();

		virtual void render(IPlatformContext* context);
		virtual void create(IPlatformContext* context);
		virtual void destroy(IPlatformContext* context);

		virtual void onMouseButtonDown(MouseButton button, float x, float y, MouseFlags flags);
		virtual void onMouseButtonUp(MouseButton button, float x, float y, MouseFlags flags);
		virtual void onMouseMove(float x, float y, MouseFlags flags);
		virtual void onMouseWheel(NumNothes wheel, MouseFlags flags);

		virtual void onKeyDown(KeyCode key, KeyFlags flags);
		virtual void onKeyUp(KeyCode key, KeyFlags flags);
		virtual void onChar(tchar ch);

	protected:
		typedef std::vector<WidgetPtr> WidgetList;
		typedef WidgetList::iterator WidgetListIt;

		WidgetList		m_widgets;
		int32			m_focusedWidget;
	};
}