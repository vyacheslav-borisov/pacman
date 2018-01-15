#include "stdafx.h"
#include "game_objects.h"
#include "game_events.h"
#include "waiting.h"
//Sprite animation class
#include "platform_windows.h"

using namespace Pegas;
/*************************************************************************************************************
	Character
**************************************************************************************************************/
Character::Character(int actorId)
{
	m_actorId = actorId;
	m_tileGrid = 0;
	m_radius = 1.0f;

	m_directions[k_moveLeft] = Vector3(-1.0f, 0.0f, 0.0f);
	m_directions[k_moveTop] = Vector3(0.0f, -1.0f, 0.0f);
	m_directions[k_moveRight] = Vector3(1.0f, 0.0f, 0.0f);
	m_directions[k_moveBottom] = Vector3(0.0f, 1.0f, 0.0f);

	m_currentDirection = k_moveLeft;
	m_velocity = 100.0f;
	m_turnCommand = -1;

	m_blockMutex = 0;
	m_isMoving = false;
	m_isVisible = true;

	m_prevRow = 0;
	m_prevColumn = 0;
}

void Character::create(TileGrid* tileGrid, const Vector3& position)
{
	assert(tileGrid && "null pointer: tileGrid");

	m_tileGrid = tileGrid;
	m_radius = tileGrid->getCellWidth() * 0.5f;
	m_initialPosition = m_position = position;

	TheEventMgr.addEventListener(this, Event_ChangeDirection::k_type);
	TheEventMgr.addEventListener(this, Event_CancelChangingDirection::k_type);
	TheEventMgr.addEventListener(this, Event_EnableCharacterControl::k_type);
	TheEventMgr.addEventListener(this, Event_DisableCharacterControl::k_type);
	TheEventMgr.addEventListener(this, Event_ResetActors::k_type);
	TheEventMgr.addEventListener(this, Event_HideCharacter::k_type);
	TheEventMgr.addEventListener(this, Event_ShowCharacter::k_type);
	TheEventMgr.addEventListener(this, Event_CharacterTonnelOut::k_type);
}

void Character::destroy()
{
	TheEventMgr.removeEventListener(this);
}

void Character::handleEvent(EventPtr evt)
{
	if(evt->getType() == Event_ChangeDirection::k_type)
	{
		Event_ChangeDirection* eventObject = evt->cast<Event_ChangeDirection>();
		if(eventObject->_actorId != m_actorId || isBlocked())
		{
			//событие предназначено не нам или управление персонажем заблокировано
			return;
		}

		if(eventObject->_newDirection == m_currentDirection)
		{
			//персонаж уже движеться в этом направлении
			return;
		}

		float dotProduct = m_directions[m_currentDirection].dotProduct(m_directions[eventObject->_newDirection]);
		if(dotProduct < 0)
		{
			//разворот в противоположном направлении можно сделать сразу
			m_currentDirection = eventObject->_newDirection;

			EventPtr newEvent(new Event_DirectionChanged(m_actorId, m_currentDirection));
			TheEventMgr.pushEventToQueye(newEvent);

			if(!m_isMoving)
			{
				m_isMoving = true;

				EventPtr newEvent2(new Event_CharacterMoveOn(m_actorId));
				TheEventMgr.pushEventToQueye(newEvent2);
			}
		}else
		{
			m_turnCommand = eventObject->_newDirection;			
		}

		return;
	}

	if(evt->getType() == Event_CancelChangingDirection::k_type)
	{
		Event_CancelChangingDirection* eventObject = evt->cast<Event_CancelChangingDirection>();
		if(eventObject->_actorId != m_actorId)
		{
			//событие предназначено не нам
			return;
		}
		m_turnCommand = -1;
		return;
	}

	if(evt->getType() == Event_EnableCharacterControl::k_type)
	{
		Event_EnableCharacterControl* eventObject = evt->cast<Event_EnableCharacterControl>();
		if(eventObject->_actorId != m_actorId && eventObject->_actorId != k_actorAll)
		{
			//событие предназначено не нам
			return;
		}

		decrementBlock();
		
		if(!isBlocked())
		{
			m_isMoving = true;
			
			EventPtr newEvent(new Event_CharacterMoveOn(m_actorId));
			TheEventMgr.pushEventToQueye(newEvent);
		}
		return;
	}

	if(evt->getType() == Event_CharacterTonnelOut::k_type)
	{
		Event_CharacterTonnelOut* pEvent = evt->cast<Event_CharacterTonnelOut>();
		if(pEvent->_actorId == m_actorId)
		{
			m_prevRow = pEvent->_row;
			m_prevColumn = pEvent->_column;
			m_tileGrid->cellCoords(pEvent->_row, pEvent->_column, m_position._x, m_position._y, true);
			
			EventPtr evt2(new Event_CharacterMoved(this->m_actorId, this->m_position, pEvent->_row, pEvent->_column));
			TheEventMgr.pushEventToQueye(evt2);
		}
		return;
	}

	if(evt->getType() == Event_DisableCharacterControl::k_type)
	{
		Event_DisableCharacterControl* eventObject = evt->cast<Event_DisableCharacterControl>();
		if(eventObject->_actorId != m_actorId && eventObject->_actorId != k_actorAll)
		{
			//событие предназначено не нам
			return;
		}

		incrementBlock();

		if(isBlocked())
		{
			m_isMoving = false;

			EventPtr newEvent(new Event_CharacterStopped(m_actorId));
			TheEventMgr.pushEventToQueye(newEvent);
		}
		return;
	}

	if(evt->getType() == Event_ResetActors::k_type)
	{
		m_position = m_initialPosition;
		m_currentDirection = k_moveLeft;
		m_isVisible = true;
		return;
	}

	if(evt->getType() == Event_HideCharacter::k_type)
	{
		Event_HideCharacter* pEvent = evt->cast<Event_HideCharacter>();
		if(pEvent->_actorId == m_actorId || pEvent->_actorId == k_actorAll)
		{
			m_isVisible = false;
		}
		return;
	}

	if(evt->getType() == Event_ShowCharacter::k_type)
	{
		Event_ShowCharacter* pEvent = evt->cast<Event_ShowCharacter>();
		if(pEvent->_actorId == m_actorId || pEvent->_actorId == k_actorAll)
		{
			m_isVisible = true;
		}
		return;
	}
}

void Character::update(float deltaTime)
{
	int32 currentRow, currentColumn;
	
	if(m_isMoving)
	{
		m_tileGrid->pointToCell(m_position._x, m_position._y, currentRow, currentColumn);

		//перемещение по лабиринту
		Vector3 newPos = m_position + (m_directions[m_currentDirection] * m_velocity * deltaTime);
		Vector3 facingPoint = newPos + m_directions[m_currentDirection] * m_radius;
		if(isObstacle(facingPoint))
		{
			//натолкнулись на препятсвие
			//корркетируем позицию персонажа чтобы он был в середине текущей клетки
			m_tileGrid->cellCoords(currentRow, currentColumn, m_position._x, m_position._y, true);
			m_isMoving = false;

			EventPtr newEvent(new Event_CharacterStopped(m_actorId));
			TheEventMgr.pushEventToQueye(newEvent);
		}else
		{
			m_position = newPos;
		}

		if(currentRow != m_prevRow || currentColumn != m_prevColumn)
		{
			m_prevRow = currentRow;
			m_prevColumn = currentColumn;

			EventPtr evt(new Event_CharacterMoved(m_actorId, m_position, currentRow, currentColumn));
			TheEventMgr.pushEventToQueye(evt);

			TILEID tile = m_tileGrid->getTile(currentRow, currentColumn);
			if(tile == k_tileTunnel)
			{
				EventPtr evt2(new Event_CharacterTonnelIn(m_actorId, currentRow, currentColumn));
				TheEventMgr.pushEventToQueye(evt2);
			}
		}
	}

	//поворот
	if(m_turnCommand != -1 && !isBlocked())
	{
		int32 nextRow, nextColumn;
		
		m_tileGrid->pointToCell(m_position._x, m_position._y, currentRow, currentColumn);

		switch(m_turnCommand)
		{
		case k_moveLeft:
			nextRow = currentRow;
			nextColumn = currentColumn - 1;
			break;
		case k_moveTop:
			nextRow = currentRow - 1;
			nextColumn = currentColumn;
			break;
		case k_moveRight:
			nextRow = currentRow;
			nextColumn = currentColumn + 1;
			break;
		case k_moveBottom:
			nextRow = currentRow + 1;
			nextColumn = currentColumn;
			break;
		};

		if(!isObstacle(nextRow, nextColumn))
		{
			Vector3 cellPosition;
			m_tileGrid->cellCoords(currentRow, currentColumn, cellPosition._x, cellPosition._y, true);
			
			Vector3 distanceVec = cellPosition - m_position;
			float distance = distanceVec.length();
			float epsilon = distance / m_radius; 

			if(epsilon < 0.75f)
			{
				m_position = cellPosition;
				m_currentDirection = m_turnCommand;
				m_turnCommand = -1;

				EventPtr newEvent(new Event_DirectionChanged(m_actorId, m_currentDirection));
				TheEventMgr.pushEventToQueye(newEvent);

				if(!m_isMoving)
				{
					m_isMoving = true;

					EventPtr newEvent2(new Event_CharacterMoveOn(m_actorId));
					TheEventMgr.pushEventToQueye(newEvent2);
				}
			}
		}
	}//if(m_turnCommand != -1)
}

bool Character::isObstacle(const Vector3 position)
{
	int32 row, column;
	m_tileGrid->pointToCell(position._x, position._y, row, column);

	return isObstacle(row, column);
}

bool Character::isObstacle(int32 row, int32 column)
{
	return m_tileGrid->isObstacle(row, column);
}

/******************************************************************************************************************************
	Pacman
*******************************************************************************************************************************/
Pacman::Pacman(int actorId, IPlatformContext* platform):
	Character(actorId)
{
	m_platform = platform;
	m_currentAnimation = 0;
}

void Pacman::create(TileGrid* tileGrid, const Vector3& position)
{
	Character::create(tileGrid, position);

	TheEventMgr.addEventListener(this, Event_CharacterStopped::k_type);
	TheEventMgr.addEventListener(this, Event_CharacterMoveOn::k_type);
	TheEventMgr.addEventListener(this, Event_PacmanDeath::k_type);
}

void Pacman::setAnimation(int state, ProcessPtr animation)
{
	assert(state >= 0 && state <k_animationTotal && "invalid animation key");

	m_animations[state] = animation;
	
	if(state == k_animationRunning)
	{
		m_platform->attachProcess(animation);
		animation->suspend();
	}
}

void Pacman::handleEvent(EventPtr evt)
{
	if(evt->getType() == Event_CharacterStopped::k_type)
	{
		Event_CharacterStopped* pEvent = evt->cast<Event_CharacterStopped>();
		if(pEvent->_actorId == m_actorId)
		{
			m_animations[k_animationRunning]->suspend();
		}
		return;
	}

	if(evt->getType() == Event_CharacterMoveOn::k_type)
	{
		Event_CharacterMoveOn* pEvent = evt->cast<Event_CharacterMoveOn>();
		if(pEvent->_actorId == m_actorId)
		{
			m_currentAnimation = k_animationRunning;
			m_animations[k_animationRunning]->resume();
		}
		return;
	}

	if(evt->getType() == Event_PacmanDeath::k_type)
	{
		m_currentDirection = k_moveLeft;  
		m_currentAnimation = k_animationDeath;
		m_platform->attachProcess(m_animations[m_currentAnimation]);

		Waiting* waiting = new Waiting(1.0f);
		waiting->addFinalEvent(EventPtr(new Event_PacmanDeathComplete()));
		m_animations[m_currentAnimation]->attachNext(ProcessPtr(waiting));
		return;
	}

	if(evt->getType() == Event_ResetActors::k_type)
	{
		m_currentAnimation = k_animationRunning;		
	}

	Character::handleEvent(evt);
}

void Pacman::update(float deltaTime)
{
	Character::update(deltaTime);

	if(m_isMoving)
	{
		int32 row, column;
		m_tileGrid->pointToCell(m_position._x, m_position._y, row, column);
		if(row != m_prevRow || column != m_prevColumn)
		{
			TILEID tile = m_tileGrid->getTile(row, column);

			if(tile == k_tilePill || tile == k_tileSuperPill || tile == k_tileBonus)
			{
				EventPtr newEvent(new Event_PacmanSwallowedPill(tile, row, column));
				TheEventMgr.pushEventToQueye(newEvent);
			}
		}//if(row != m_prevRow && column != m_prevColumn)
	}//if(m_isMoving)
}

void Pacman::draw()
{
	if(!m_isVisible)
	{
		return;
	}

	if(m_currentAnimation < 0 || m_currentAnimation >= k_animationTotal)
		return;

	if(m_animations[m_currentAnimation]->getStatus() == k_processStatusKilled)
		return;

	SpriteAnimation* animation = (SpriteAnimation*)m_animations[m_currentAnimation].get();
	SpriteParameters* sprite =  animation->getSprite();

	sprite->_left = (CURCOORD)(m_position._x - (sprite->_width * 0.5f));
	sprite->_top =	(CURCOORD)(m_position._y - (sprite->_height * 0.5f));
		
	switch(m_currentDirection)
	{
	case k_moveLeft:
		sprite->_angle = 180.0f;
		break;
	case k_moveTop:
		sprite->_angle = -90.0f;
		break;
	case k_moveRight:
		sprite->_angle = 0.0f;
		break;
	case k_moveBottom:
		sprite->_angle = 90.0f;
		break;
	};

	GrafManager::getInstance().drawSprite(*sprite);
}

/****************************************************************************************************************
	Ghost class implementation
****************************************************************************************************************/
Ghost::Ghost(int actorId, IPlatformContext* platform)
	:Character(actorId), m_platform(platform)
{
	m_prevRow = -1;
	m_prevColumn = -1;
	m_pacmanRow = -1;
	m_pacmanColumn = -1;
}

void Ghost::create(TileGrid* tileGrid, const Vector3& position)
{
	Character::create(tileGrid, position);

	TheEventMgr.addEventListener(this, Event_DirectionChanged::k_type);
	TheEventMgr.addEventListener(this, Event_SuperForceOn::k_type);
	TheEventMgr.addEventListener(this, Event_SuperForceOff::k_type);
	TheEventMgr.addEventListener(this, Event_SuperForcePreOff::k_type);
	TheEventMgr.addEventListener(this, Event_CharacterMoved::k_type);
	TheEventMgr.addEventListener(this, Event_CharacterStateChanged::k_type);
	TheEventMgr.addEventListener(this, Event_CharacterChangeState::k_type);
	TheEventMgr.addEventListener(this, Event_PacmanDeath::k_type);

	m_stateVelocity[k_stateChasing] = m_velocity * 0.8;
	m_stateVelocity[k_stateRunaway] = m_velocity * 0.5;
	m_stateVelocity[k_statePray] = m_velocity * 1.1;

	m_currentState = k_stateChasing;
	m_velocity = m_stateVelocity[k_stateChasing];

	EventPtr evt(new Event_CharacterStateChanged(m_actorId, m_currentState));
	TheEventMgr.pushEventToQueye(evt);
}

void Ghost::handleEvent(EventPtr evt)
{
	if(evt->getType() == Event_CharacterMoved::k_type)
	{
		Event_CharacterMoved* pEvent = evt->cast<Event_CharacterMoved>();
		if(pEvent->_actorId == k_actorPacman)
		{
			m_pacmanRow = pEvent->_row;
			m_pacmanColumn = pEvent->_column;
		}
		return;
	}

	if(evt->getType() == Event_DirectionChanged::k_type)
	{
		Event_DirectionChanged* pEvent = evt->cast<Event_DirectionChanged>();
		if(pEvent->_actorId == m_actorId)
		{
			changeAnimation(m_currentState);
		}
		return;
	}

	if(evt->getType() == Event_SuperForceOn::k_type)
	{
		if(m_currentState != k_statePray)
		{
			m_currentState = k_stateRunaway;
			
			EventPtr evt2(new Event_CharacterStateChanged(m_actorId, m_currentState));
			TheEventMgr.pushEventToQueye(evt2);
		}
		return;
	}

	if(evt->getType() == Event_SuperForceOff::k_type)
	{
		if(m_currentState != k_statePray)
		{
			m_currentState = k_stateChasing;
			
			EventPtr evt2(new Event_CharacterStateChanged(m_actorId, m_currentState));
			TheEventMgr.pushEventToQueye(evt2);
		}
		return;
	}

	if(evt->getType() == Event_SuperForcePreOff::k_type)
	{
		if(m_currentState == k_stateRunaway)
		{
			m_animations[m_currentAnimation]->suspend();
			m_currentAnimation = k_animationBlink;
			m_animations[m_currentAnimation]->resume();
		}
		return;
	}	

	if(evt->getType() == Event_CharacterStateChanged::k_type)
	{
		Event_CharacterStateChanged* pEvent = evt->cast<Event_CharacterStateChanged>();
		if(pEvent->_actorId == m_actorId || pEvent->_actorId == k_actorAll)
		{
			changeAnimation(pEvent->_newState);
			m_velocity = m_stateVelocity[pEvent->_newState];
		}
		return;
	}

	if(evt->getType() == Event_CharacterChangeState::k_type)
	{
		Event_CharacterChangeState* pEvent = evt->cast<Event_CharacterChangeState>();
		if(pEvent->_actorId == m_actorId || pEvent->_actorId == k_actorAll)
		{
			m_currentState = pEvent->_newState;

			EventPtr evt2(new Event_CharacterStateChanged(m_actorId, m_currentState));
			TheEventMgr.pushEventToQueye(evt2);
		}
		return;
	}

	if(evt->getType() == Event_ResetActors::k_type)
	{
		m_currentState = k_stateChasing;
		
		EventPtr evt2(new Event_CharacterStateChanged(m_actorId, m_currentState));
		TheEventMgr.pushEventToQueye(evt2);		
	}

	if(evt->getType() == Event_PacmanDeath::k_type)
	{
		EventPtr evt2(new Event_HideCharacter(m_actorId));
		TheEventMgr.pushEventToQueye(evt2);
		return;
	}

	Character::handleEvent(evt);
}

void Ghost::update(float deltaTime)
{
	Character::update(deltaTime);

	if(m_currentState == k_stateChasing || m_currentState == k_stateRunaway)
	{
		int32 currentRow, currentColumn;
		m_tileGrid->pointToCell(m_position._x, m_position._y, currentRow, currentColumn);

		if(currentRow == m_prevRow && currentColumn == m_prevColumn)
		{
			return;
		}
		
		if(currentRow != m_pacmanRow || currentColumn != m_pacmanColumn)
		{
			return;
		}

		m_prevRow = currentRow;
		m_prevColumn = currentColumn;

		int32 actorID = m_currentState == k_stateChasing ? k_actorPacman : m_actorId;
		EventPtr evt(new Event_CharacterKilled(actorID, m_position));
		TheEventMgr.pushEventToQueye(evt);

		if(m_currentState == k_stateRunaway)
		{
			m_currentState = k_statePray;
			
			EventPtr evt(new Event_CharacterStateChanged(m_actorId, m_currentState));
			TheEventMgr.pushEventToQueye(evt);
		}	
	}//if(m_currentState == k_stateChasing || m_currentState == k_stateRunaway)
}

void Ghost::draw()
{
	if(!m_isVisible)
	{
		return;
	}

	if(m_currentAnimation < 0 || m_currentAnimation >= k_animationTotal)
		return;

	if(m_animations[m_currentAnimation]->getStatus() == k_processStatusKilled)
		return;

	SpriteAnimation* animation = (SpriteAnimation*)m_animations[m_currentAnimation].get();
	SpriteParameters* sprite =  animation->getSprite();

	sprite->_left = (CURCOORD)(m_position._x - (sprite->_width * 0.5f));
	sprite->_top =	(CURCOORD)(m_position._y - (sprite->_height * 0.5f));
		
	GrafManager::getInstance().drawSprite(*sprite);
}

void Ghost::setAnimation(int state, ProcessPtr animation)
{
	assert(state >= 0 && state < k_animationTotal && "invalid animation key");

	m_currentAnimation = state;
	m_platform->attachProcess(animation);
	m_animations[state] = animation;
	m_animations[state]->suspend();	
}

void Ghost::changeAnimation(int32 newState)
{
	m_animations[m_currentAnimation]->suspend();
	
	switch(newState)
	{
	case k_statePray:
		m_currentAnimation = m_currentDirection + k_moveTotalDirections;
		break;
	case k_stateRunaway:
		m_currentAnimation = k_animationRunaway;
		break;
	default:
		m_currentAnimation = m_currentDirection;
		break;
	};
	assert(m_currentAnimation >= 0 && m_currentAnimation < k_animationTotal && "invalid animation index"); 
	m_animations[m_currentAnimation]->resume();
}

bool Ghost::isObstacle(int32 row, int32 column)
{
	int32 collisionGroup;
	if(m_tileGrid->isObstacle(row, column, &collisionGroup))
	{
		return collisionGroup == k_collisionGroupDefault;
	}

	return false;
}