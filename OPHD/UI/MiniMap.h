#pragma once

#include "Core/Control.h"

#include <NAS2D/Math/Rectangle.h>
#include <NAS2D/Resource/Image.h>
#include <NAS2D/EventHandler.h>

#include <map>
#include <string>


class Tile;
class TileMap;
class Robot;
class MapViewState;


class MiniMap : public Control
{
public:
	MiniMap(TileMap* tileMap, const std::map<Robot*, Tile*>& robotList, const std::string& mapName);

	bool heightMapVisible() const;
	void heightMapVisible(bool isVisible);

	void draw() const override;

protected:
	friend MapViewState;
	void onActivate();
	void onMouseUp(NAS2D::EventHandler::MouseButton button, int x, int y);
	void onMouseDown(NAS2D::EventHandler::MouseButton button, int x, int y);
	void onMouseMove(int x, int y, int rX, int rY);
	void onSetView(NAS2D::Point<int> mousePixel);

private:
	TileMap* mTileMap;
	const std::map<Robot*, Tile*>& mRobotList;
	bool mIsHeightMapVisible;
	NAS2D::Image mBackgroundSatellite;
	NAS2D::Image mBackgroundHeightMap;
	const NAS2D::Image& mUiIcons;
	bool mLeftButtonDown{false};
};