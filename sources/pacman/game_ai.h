#pragma once

#include "engine.h"
#include "tile_grid.h"

namespace Pegas
{
	class BaseAIState: public Process, public IEventListener
	{
	public:
		BaseAIState(TileGrid* tileGrid, int32 controlledActor, int32 stateId);
		virtual ~BaseAIState();

		virtual void suspend();
		virtual void resume();
		virtual void update(MILLISECONDS deltaTime);
		virtual void handleEvent(EventPtr evt);
		virtual ListenerType getListenerName() { return "BaseAIState"; }

	protected:
		virtual void start(ProcessHandle myHandle, ProcessManagerPtr owner);

		virtual bool isObstacle(int32 row, int32 column);
		virtual int32 chooseDirection();
		virtual float getGoalHeuristic(int32 row, int32 column);

		TileGrid* m_tileGrid;
		
		int32 m_controlledActor;
		int32 m_stateId;

		int32 m_myRow;
		int32 m_myColumn;
		int32 m_myCurrentDirection;

		int32 m_blockMutex;
		bool  m_characterMoved;

		bool  m_inRoom;
		int32 m_outRoomRow;
		int32 m_outRoomColumn;

		bool m_bTerminate;
	};

	class BlinkyChaseState: public BaseAIState
	{
	public:
		BlinkyChaseState(TileGrid* tileGrid);
		virtual void handleEvent(EventPtr evt);

	protected:
		virtual float getGoalHeuristic(int32 row, int32 column);

	private:
		Vector3 m_pacmanPosition;		
	};

	class PinkyChaseState: public BaseAIState
	{
	public:
		PinkyChaseState(TileGrid* tileGrid);
		virtual void handleEvent(EventPtr evt);

	protected:
		virtual float getGoalHeuristic(int32 row, int32 column);

	private:
		void calculateGoalPosition();

		Vector3 m_goalPosition;
		int32 m_pacmanRow;
		int32 m_pacmanColumn;
		int32 m_pacmanDirection;
	};

	class InkyChaseState: public BaseAIState
	{
	public:
		InkyChaseState(TileGrid* tileGrid);
		virtual void handleEvent(EventPtr evt);

	protected:
		virtual float getGoalHeuristic(int32 row, int32 column);

	private:
		void calculateGoalPosition();

		Vector3 m_goalPosition;
		Vector3 m_blinkyPosition;
		int32 m_pacmanRow;
		int32 m_pacmanColumn;
		int32 m_pacmanDirection;
	};

	class ClydeChaseState: public BaseAIState
	{
	public:
		ClydeChaseState(TileGrid* tileGrid);
		virtual void handleEvent(EventPtr evt);

	protected:
		virtual float getGoalHeuristic(int32 row, int32 column);

	private:
		void calculateGoalPosition();

		Vector3 m_scatterPoint;
		Vector3 m_goalPosition;
		int32 m_pacmanRow;
		int32 m_pacmanColumn;		
	};

	class GoalDrivenState: public BaseAIState
	{
	public:
		GoalDrivenState(TileGrid* tileGrid, int32 controlledActor, int32 stateId);

		void setGoalPoint(const Vector3& point);
		virtual void update(MILLISECONDS deltaTime);

	protected:
		virtual float getGoalHeuristic(int32 row, int32 column);
		virtual void onGoalAchieved();	

		int32	m_goalRow;
		int32	m_goalColumn;		
	};

	class RunawayState: public BaseAIState
	{
	public:
		RunawayState(TileGrid* tileGrid, int32 controlledActor);
		virtual void handleEvent(EventPtr evt);

	protected:
		virtual int32 chooseDirection();
		Vector3 m_pacmanPosition;
	};

	class PrayState: public GoalDrivenState
	{
	public:
		PrayState(TileGrid* tileGrid, int32 controlledActor);

	protected:
		virtual void onGoalAchieved();
		virtual bool isObstacle(int32 row, int32 column);
	};
/*
	class RoomOutState: public GoalDrivenState
	{
	public:
		RoomOutState(TileGrid* tileGrid, int32 controlledActor);
		virtual void handleEvent(EventPtr evt);

		void addOuterState(ProcessPtr state);

	protected:
		virtual void start(ProcessHandle myHandle, ProcessManagerPtr owner);
		virtual void onGoalAchieved();
		virtual bool isObstacle(int32 row, int32 column);

		void activate();

		bool m_bActive;
		bool m_bWaitFor;
		std::vector<ProcessPtr> m_outerStates;
	};*/
}