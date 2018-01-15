#include "stdafx.h"
#include "gui_layer.h"

using namespace Pegas;

WidgetPtr GUILayer::getFocusedWidget()
{
	if(m_widgets.size() > 0 && m_focusedWidget != -1)
	{
		return m_widgets[m_focusedWidget];
	}

	return WidgetPtr(); 
}

void GUILayer::addWidget(WidgetPtr widget)
{
	m_widgets.push_back(widget);	
}

void GUILayer::changeFocusNext()
{
	if(m_widgets.size() == 0) return;

	if(m_focusedWidget != -1)
	{
		m_widgets[m_focusedWidget]->killFocus();
	}

	m_focusedWidget++;
	if(m_focusedWidget >= m_widgets.size())
	{
		m_focusedWidget = 0;
	}

	m_widgets[m_focusedWidget]->setFocus();	
}

void GUILayer::changeFocusPrev()
{
	if(m_widgets.size() == 0) return;

	if(m_focusedWidget != -1)
	{
		m_widgets[m_focusedWidget]->killFocus();
	}

	m_focusedWidget--;
	if(m_focusedWidget < 0)
	{
		m_focusedWidget = m_widgets.size() - 1;
	}

	m_widgets[m_focusedWidget]->setFocus();
}

void GUILayer::render(IPlatformContext* context)
{
	for(WidgetListIt it = m_widgets.begin(); it != m_widgets.end(); ++it)
	{
		(*it)->render(context);
	}
}

void GUILayer::create(IPlatformContext* context)
{
	m_focusedWidget = -1;
}

void GUILayer::destroy(IPlatformContext* context)
{
	m_widgets.clear();
}

void GUILayer::onMouseButtonDown(MouseButton button, float x, float y, MouseFlags flags)
{
	if(isActive())
	{
		WidgetPtr focused = getFocusedWidget(); 
		for(int32 i = 0; i < m_widgets.size(); i++)
		{
			WidgetPtr current = m_widgets[i];

			if(current->isPointIn(x, y))
			{
				current->setFocus();
				current->onMouseButtonDown(button, x, y, flags);

				if(focused.IsValid())
				{
					focused->killFocus();
				}

				m_focusedWidget = i;
				break;

			}//if((*it)->isPointIn(x, y))
		}//for(int32 i = 0; i < m_widgets.size(); i++)		
	}//if(isActive())

	

	BaseScreenLayer::onMouseButtonDown(button, x, y, flags);
}

void GUILayer::onMouseButtonUp(MouseButton button, float x, float y, MouseFlags flags)
{
	WidgetPtr focused = getFocusedWidget();

	if(isActive())
	{
		
		for(int32 i = 0; i < m_widgets.size(); i++)
		{
			WidgetPtr current = m_widgets[i];
		
			if(focused.IsValid() && current == focused) continue;

			if(current->isPointIn(x, y))
			{
				current->onMouseButtonUp(button, x, y, flags);
				break;
			}//if((*it)->isPointIn(x, y))
		}//for(WidgetListIt it = m_widgets.begin(); it != m_widgets.end(); ++it)
	}//if(isActive())

	if(focused.IsValid())
	{
		focused->onMouseButtonUp(button, x, y, flags);
	}

	BaseScreenLayer::onMouseButtonUp(button, x, y, flags);
}

void GUILayer::onMouseMove(float x, float y, MouseFlags flags)
{
	if(isActive())
	{
		for(WidgetListIt it = m_widgets.begin(); it != m_widgets.end(); ++it)
		{
			(*it)->onMouseMove(x, y, flags);
		}//for(WidgetListIt it = m_widgets.begin(); it != m_widgets.end(); ++it)
	}//if(isActive())

	BaseScreenLayer::onMouseMove(x, y, flags);
}

void GUILayer::onMouseWheel(NumNothes wheel, MouseFlags flags)
{
	WidgetPtr focused = getFocusedWidget();
	if(focused.IsValid())
	{
		focused->onMouseWheel(wheel, flags);
	}

	BaseScreenLayer::onMouseWheel(wheel, flags);
}

void GUILayer::onKeyDown(KeyCode key, KeyFlags flags)
{
	WidgetPtr focused = getFocusedWidget();
	if(focused.IsValid())
	{
		focused->onKeyDown(key, flags);
	}	

	BaseScreenLayer::onKeyDown(key, flags);
}

void GUILayer::onKeyUp(KeyCode key, KeyFlags flags)
{
	WidgetPtr focused = getFocusedWidget();
	if(focused.IsValid())
	{
		focused->onKeyUp(key, flags);
	}	

	BaseScreenLayer::onKeyUp(key, flags);
}

void GUILayer::onChar(tchar ch)
{
	WidgetPtr focused = getFocusedWidget();
	if(focused.IsValid())
	{
		focused->onChar(ch);
	}	

	BaseScreenLayer::onChar(ch);
}