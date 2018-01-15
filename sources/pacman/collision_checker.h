#pragma once

#include "tile_grid.h"
#include "cell_grid.h"

namespace Pegas
{
	class IColissionHull
	{
	public:
		virtual int32	getCollisionGroup() = 0;
		virtual Vector3 getPosition() = 0;
		virtual float	getRadius() = 0;
		virtual void	onCollisionEnter(IColissionHull* other) {};
		virtual void	onCollisionLeave(IColissionHull* other) {};
	};

	class CollisionChecker
	{
	public:
		CollisionChecker() {}

		void init(TileGrid* tileGrid);
		void destroy();
		void update(MILLISECONDS deltaTime);

		void addCollisionHull(IColissionHull* hull);		

	private:
		bool isIntersects(IColissionHull* a, IColissionHull* b);
		
		typedef std::list<IColissionHull*> ProcessingList;
		typedef ProcessingList::iterator ProcessingListIt;
		
		typedef std::pair<IColissionHull*, IColissionHull*> CollisionPair;
		typedef std::set<CollisionPair> CollisionPairs;
		typedef CollisionPairs::iterator	CollisionPairsIt;
				
		CellGrid<IColissionHull*> m_collisionGrid;
		ProcessingList			  m_processingLists;
		CollisionPairs			  m_prevCollisionPairs;	
	};
}