#pragma once

#include "engine.h"

namespace Pegas
{
	typedef int32 TILEID;
	struct TileDesc
	{
		TileDesc() {};
			TileDesc(const RESOURCEID& texture, 
					 bool isObstacle, 
					 int32	collisionGroup = 0,
					 RGBCOLOR debugColor = 0xffff0000) 
			{
				_texture = texture;
				_isObstacle = isObstacle;
				_collisionGroup = collisionGroup;
				_debugColor = debugColor;
			};

			TILEID		_id;
			RESOURCEID	_texture;
			bool		_isObstacle;
			int32		_collisionGroup;
			RGBCOLOR	_debugColor;
	};	

	class TileGrid
	{
	public:
		enum
		{
			k_debugDrawGrid = 1,
			k_debugDrawObstacles = 2
		};

		enum
		{
			k_emptyCellTileId = -1			
		};		

		TileGrid();
		~TileGrid();

		void create(int32 rows, int32 cols);
		void destroy();
		void load(ISerializer& stream);
		void save(ISerializer& stream);
		void draw();
		void debugDraw(int32 flags = k_debugDrawGrid | k_debugDrawObstacles);
		
		void setArea(float left, float top, float width, float height);
		void getArea(float& left, float& top, float& width, float& height);
		void setCellSize(float width, float height);

		TILEID addTileDesc(const TileDesc& desc);
		TileDesc getTileDesc(TILEID tile);
		TILEID setTile(int32 row, int32 col, TILEID tile);
		TILEID setTilePoint(float x, float y, TILEID tile);
		TILEID getTile(int32 row, int32 col);
		TILEID getTilePoint(float x, float y);

		int32 getTiles(TILEID tile, std::list<Vector3>& outTiles, bool center = false);
		int32 getNumTiles(TILEID tile);

		bool isObstaclePoint(float x, float y, int32* collisionGroup = 0);
		bool isObstacle(int32 row, int32 col, int32* collisionGroup = 0);

		void pointToCell(float x, float y, int32& row, int32& col);
		void cellCoords(int32 row, int32 col, float& x, float& y, bool center = false);

		float getLeft() const { return m_left; }
		float getTop() const { return m_top; }
		float getWidth() const { return m_width; }
		float getHeight() const { return m_height; }

		float	getCellWidth() const { return m_cellWidth; }
		float	getCellHeight() const { return m_cellHeight; }
		
		int32 getNumRows() const { return m_numRows; }
		int32 getNumColumns() const { return m_numCols; }

	private:
		int32 packCoords(int32 row, int32 col);
		void  extractCoords(int32 packedVal, int32& row, int32& col);

		void updateCellSize();

		float	m_left;
		float	m_top;
		float	m_width;
		float	m_height;

		float	m_cellWidth;
		float	m_cellHeight;

		
		int32		m_numRows;
		int32		m_numCols;
		TILEID**	m_cells;

		static const RGBCOLOR k_debugDrawingGridColor;
		static const RGBCOLOR k_debugDrawingObstacleColor;

		typedef std::vector<TileDesc> TILEDESC_LIST;
		typedef TILEDESC_LIST::iterator TILEDESC_LIST_IT;

		typedef std::set<int32> TILES_RENDER_LIST;
		typedef TILES_RENDER_LIST::iterator TILES_RENDER_LIST_IT;
		typedef std::vector<TILES_RENDER_LIST> TILES_MAP;
		typedef TILES_MAP::iterator	TILES_MAP_IT;

		TILEDESC_LIST m_tilesDescs;
		TILES_MAP	  m_tileRenderMap;	
	};

	inline int32 TileGrid::packCoords(int32 row, int32 col)
	{
		return (row << 16 | col);
	}
	
	inline void  TileGrid::extractCoords(int32 packedVal, int32& row, int32& col)
	{
		col = packedVal & 0x0000ffff;
		row = (packedVal >> 16)/* & 0xffff0000*/;
	}

}