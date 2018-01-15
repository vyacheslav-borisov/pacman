#include "stdafx.h"
#include "tile_grid.h"

using namespace Pegas;

TileGrid::TileGrid():
	m_left(0), m_top(0), m_width(0), m_height(0),
		m_numRows(0), m_numCols(0), m_cells(0)
{
	
}

TileGrid::~TileGrid()
{
	destroy();
}

void TileGrid::create(int32 rows, int32 cols)
{
	destroy();

	assert(rows > 0);
	assert(cols > 0);

	m_numRows = rows;
	m_numCols = cols;
	
	m_cells = new TILEID*[m_numCols];
	for(int32 col = 0;  col < m_numCols; col++)
	{
		m_cells[col] = new TILEID[m_numRows];
		for(int32 row = 0; row < m_numRows; row++)
		{
			m_cells[col][row] = k_emptyCellTileId;
		}
	}

	updateCellSize();
}

void TileGrid::destroy()
{
	if(!m_cells) return;

	for(int32 col = 0;  col < m_numCols; col++)
	{
		delete[] m_cells[col];
	}
	delete[] m_cells;

	m_cells = 0;
	for(TILEID id = 0; id < m_tileRenderMap.size(); id++)
	{
		m_tileRenderMap[id].clear();
	}
}

void TileGrid::load(ISerializer& stream)
{
	destroy();

	TILEID id;
	
	
	stream >> m_numRows >> m_numCols;
	stream >> m_left >> m_top;
	stream >> m_cellWidth >> m_cellHeight;

	assert(m_numRows > 0);
	assert(m_numCols > 0);
	assert(m_cellWidth > 0);
	assert(m_cellHeight > 0);

	m_width = m_cellWidth * m_numCols;
	m_height = m_cellHeight * m_numRows;

	int32 numTileDescs;
	stream >> numTileDescs;
	if(numTileDescs > 0)
	{
		TileDesc desc;
		for(int32 i = 0; i < numTileDescs; i++)
		{
			stream >> desc._texture;
			stream >> desc._isObstacle;
			stream >> desc._collisionGroup;
			stream >> desc._debugColor;

			addTileDesc(desc);
		}
	}	

	m_cells = new int32*[m_numCols];
	for(int32 col = 0;  col < m_numCols; col++)
	{
		m_cells[col] = new int32[m_numRows];
		for(int32 row = 0; row < m_numRows; row++)
		{
			stream >> id;
			m_cells[col][row] = id;
			
			assert(id < numTileDescs);
			if(id != k_emptyCellTileId && id < numTileDescs)
			{
				int32 value = packCoords(row, col);
				m_tileRenderMap[id].insert(value);
			}
		}
	}	
}

void TileGrid::save(ISerializer& stream)
{
	if(!m_cells) return;

	stream << m_numRows << m_numCols;
	stream << m_left << m_top;
	stream << m_cellWidth << m_cellHeight;

	stream << (int32)m_tilesDescs.size();
	for(int32 i = 0; i < m_tilesDescs.size(); i++)
	{
		stream << m_tilesDescs[i]._texture;
		stream << m_tilesDescs[i]._isObstacle;
		stream << m_tilesDescs[i]._collisionGroup;
		stream << m_tilesDescs[i]._debugColor;
	}

	for(int32 col = 0;  col < m_numCols; col++)
	{
		for(int32 row = 0; row < m_numRows; row++)
		{
			stream << m_cells[col][row];
		}
	}
}

void TileGrid::draw()
{
	int32 row, col;
	
	SpriteParameters params;
	params._width = (CURCOORD)m_cellWidth;
	params._height = (CURCOORD)m_cellHeight;

	GrafManager& graf = GrafManager::getInstance();

	for(TILEID id = 0; id < m_tileRenderMap.size(); id++)
	{
		if(m_tilesDescs[id]._texture <= 0) continue;

		TILES_RENDER_LIST& renderList = m_tileRenderMap[id];
		params._texture = m_tilesDescs[id]._texture;

		for(TILES_RENDER_LIST_IT iit = renderList.begin(); iit != renderList.end(); ++iit)
		{
			extractCoords((*iit), row, col);
			cellCoords(row, col, params._left, params._top);
		
			graf.drawSprite(params);
		}
	}
}

const RGBCOLOR TileGrid::k_debugDrawingGridColor = 0xff00ff00;
const RGBCOLOR TileGrid::k_debugDrawingObstacleColor = 0xffff0000;

void TileGrid::debugDraw(int32 flags)
{
	GrafManager& graf = GrafManager::getInstance();

	if(flags & k_debugDrawGrid)
	{
		CURCOORD fromX, fromY, toX, toY;
		
		fromX = m_left;
		toX = m_left + m_width;
		fromY = toY = m_top;

		for(int32 row = 0; row <= m_numRows; row++)
		{
			graf.drawLine(fromX, fromY, toX, toY, k_debugDrawingGridColor);
			fromY+= m_cellHeight;
			toY+= m_cellHeight;
		}

		fromX = toX = m_left;
		fromY = m_top;
		toY = m_top + m_height;

		for(int32 column = 0; column <= m_numCols; column++)
		{
			graf.drawLine(fromX, fromY, toX, toY, k_debugDrawingGridColor);
			fromX+= m_cellWidth;
			toX+= m_cellWidth;
		}
	}

	if(flags & k_debugDrawObstacles)
	{
		for(TILEDESC_LIST_IT it = m_tilesDescs.begin(); it != m_tilesDescs.end(); ++it)
		{
			if(!(*it)._isObstacle) continue;

			TILES_RENDER_LIST& renderList = m_tileRenderMap[(*it)._id];
			CURCOORD x, y;
			int32 row, col;

			for(TILES_RENDER_LIST_IT iit = renderList.begin(); iit != renderList.end(); ++iit)
			{
				extractCoords((*iit), row, col);
				cellCoords(row, col, x, y);
			
				graf.drawRectangle(x, y, m_cellWidth, m_cellHeight, (*it)._debugColor, (*it)._debugColor); 
			}
		}
	}//if(flags & k_debugDrawObstacles)
}

void TileGrid::updateCellSize()
{
	assert(m_numCols > 0);
	assert(m_numRows > 0);

	if(m_width > 0 && m_height > 0)
	{
		m_cellWidth = (CURCOORD)((m_width * 1.0) / m_numCols);
		m_cellHeight = (CURCOORD)((m_height * 1.0) / m_numRows);
	}

	m_width = m_cellWidth * m_numCols;
	m_height = m_cellHeight * m_numRows;	
}

void TileGrid::setCellSize(CURCOORD width, CURCOORD height)
{
	assert(m_numCols > 0);
	assert(m_numRows > 0);
	assert(width > 0);
	assert(height > 0);

	m_cellWidth = width;
	m_cellHeight = height;

	m_width = m_cellWidth * m_numCols;
	m_height = m_cellHeight * m_numRows;
}

void TileGrid::setArea(CURCOORD left, CURCOORD top, CURCOORD width, CURCOORD height)
{
	m_left = left;
	m_top =  top;
	m_width = width;
	m_height = height;

	updateCellSize();
}

void TileGrid::getArea(CURCOORD& left, CURCOORD& top, CURCOORD& width, CURCOORD& height)
{
	left = m_left;
	top =  m_top;
	width = m_width;
	height = m_height;
}

TILEID TileGrid::addTileDesc(const TileDesc& desc)
{
	TILEID id = (TILEID)m_tilesDescs.size();
	m_tilesDescs.push_back(desc);
	m_tilesDescs[id]._id = id;
	m_tileRenderMap.push_back(TILES_RENDER_LIST());

	return id;
}

TileDesc TileGrid::getTileDesc(TILEID tile)
{
	assert(tile >= 0 && tile < m_tilesDescs.size());

	return m_tilesDescs[tile];
}

TILEID TileGrid::setTile(int32 row, int32 col, TILEID tile)
{
	assert(m_cells != 0);
	assert(col >= 0 && col < m_numCols);
	assert(row >= 0 && row < m_numRows);

	TILEID prevId = m_cells[col][row];
	m_cells[col][row] = tile;

	int32 value = packCoords(row, col);
	
	if(prevId != k_emptyCellTileId)
	{
		m_tileRenderMap[prevId].erase(value);
	}
	if(tile != k_emptyCellTileId)
	{
		m_tileRenderMap[tile].insert(value);
	}

	return prevId;
}

TILEID TileGrid::setTilePoint(CURCOORD x, CURCOORD y, TILEID tile)
{
	int32 row, col;
	pointToCell(x, y, row, col);

	return setTile(row, col, tile); 
}

TILEID TileGrid::getTile(int32 row, int32 col)
{
	assert(m_cells != 0);
	/*assert(col >= 0 && col < m_numCols);
	assert(row >= 0 && row < m_numRows);*/

	if(col < 0 || col >= m_numCols || row < 0 || row >= m_numRows)
	{
		return k_emptyCellTileId;
	}

	return	m_cells[col][row]; 
}

TILEID TileGrid::getTilePoint(CURCOORD x, CURCOORD y)
{
	int32 row, col;
	pointToCell(x, y, row, col);

	return getTile(row, col);
}

bool TileGrid::isObstaclePoint(CURCOORD x, CURCOORD y, int32* collisionGroup)
{
	int32 row, col;
	pointToCell(x, y, row, col);

	return isObstacle(row, col, collisionGroup);
}

bool TileGrid::isObstacle(int32 row, int32 col, int32* collisionGroup)
{
	TILEID id = getTile(row, col);

	if(id == k_emptyCellTileId)
	{
		return false;
	}

	if(collisionGroup != 0)
	{
		(*collisionGroup) = m_tilesDescs[id]._collisionGroup;
	}

	return m_tilesDescs[id]._isObstacle;
}

void TileGrid::pointToCell(CURCOORD x, CURCOORD y, int32& row, int32& col)
{
	assert(m_cellWidth > 0);
	assert(m_cellHeight > 0);

	col = (int32)floor((float)(x - m_left) * 1.0f / m_cellWidth);
	row = (int32)floor((float)(y - m_top) * 1.0f / m_cellHeight);
}

void TileGrid::cellCoords(int32 row, int32 col, CURCOORD& x, CURCOORD& y, bool center)
{
	x = m_left + (col * m_cellWidth);
	y = m_top + (row * m_cellHeight);

	if(center)
	{
		x+= m_cellWidth * 0.5;
		y+= m_cellHeight * 0.5;
	}
}

int32 TileGrid::getTiles(TILEID tile, std::list<Vector3>& outTiles, bool center)
{
	if(tile == k_emptyCellTileId)
	{
		Vector3 coords;
		for(int32 col = 0;  col < m_numCols; col++)
		{
			for(int32 row = 0; row < m_numRows; row++)
			{
				if(m_cells[col][row] == k_emptyCellTileId)
				{
					cellCoords(row, col, coords._x, coords._y, center);
					outTiles.push_back(coords);
				}
			}//for(int32 row = 0; row < m_numRows; row++)
		}//for(int32 col = 0;  col < m_numCols; col++)		
	}else
	{
		int32 row, column;
		Vector3 coords;
		for(TILES_RENDER_LIST_IT it = m_tileRenderMap[tile].begin(); it != m_tileRenderMap[tile].end(); ++it)
		{
			extractCoords((*it), row, column);
			cellCoords(row, column, coords._x, coords._y, center);

			outTiles.push_back(coords);
		}		
	}

	return outTiles.size();
}

int32 TileGrid::getNumTiles(TILEID tile)
{
	if(tile == k_emptyCellTileId)
	{
		int32 total = m_numRows * m_numCols;
		for(TILES_MAP_IT it = m_tileRenderMap.begin(); it != m_tileRenderMap.end(); ++it)
		{
			total-= it->size();
		}

		return total;
	}

	return m_tileRenderMap[tile].size();
}