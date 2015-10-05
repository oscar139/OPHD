#include "GameState.h"

#include "GraphWalker.h"

#include "Tile.h"

#include "Things/Robots/Robots.h"
#include "Things/Structures/Structures.h"

#include "UiConstants.h"

#include <sstream>
#include <vector>

// Disable some warnings that can be safely ignored.
#pragma warning(disable: 4244) // possible loss of data (floats to int and vice versa)


using namespace std;
using namespace NAS2D;


const std::string	MAP_TERRAIN_EXTENSION		= "_a.png";
const std::string	MAP_DISPLAY_EXTENSION		= "_b.png";

const int MAX_TILESET_INDEX	= 4;


// Throw away string stream for font rendering.
stringstream str;


/**
 * Utility function to cleanly draw a semi-formatted string with an integer value.
 */
void drawString(Renderer& r, Font& f, std::string s, int i, int x, int y, int red = 0, int green = 0, int blue = 0)
{
	str.str("");
	str << s << i;
	r.drawText(f, str.str(), x, y, red, green, blue);
}


/**
 * Utility function to cleanly draw an integer value;
 */
void drawNumber(Renderer& r, Font& f, int i, int x, int y, int red = 0, int green = 0, int blue = 0)
{
	str.str("");
	str << i;
	r.drawText(f, str.str(), x, y, red, green, blue);
}


/**
 * C'Tor
 */
GameState::GameState(const std::string& map_path):	mFont("fonts/Fresca-Regular.ttf", 14),
													mTinyFont("fonts/Fresca-Regular.ttf", 10),
													mTileMap(map_path, 4),
													mBackground("ui/background.png"),
													mMapDisplay(map_path + MAP_DISPLAY_EXTENSION),
													mHeightMap(map_path + MAP_TERRAIN_EXTENSION),
													mCurrentPointer(POINTER_NORMAL),
													mCurrentStructure(STRUCTURE_NONE),
													mDiggerDirection(mTinyFont),
													mTubesPalette(mTinyFont),
													mTileInspector(mTinyFont),
													mInsertMode(INSERT_NONE),
													mTurnCount(0),
													mReturnState(NULL),
													mLeftButtonDown(false),
													mDebug(false)
{}


/**
 * D'Tor
 */
GameState::~GameState()
{
	/**
	 * Robots are managed by the Robot Pool. Remove them from
	 * Tile's before Tile decides to free memory it shouldn't
	 * be freeing.
	 * 
	 * \note	Structures are not specially handled by a
	 *			manager object so Tile can safely free those.
	 */
	RobotMap::iterator it = mRobotList.begin();
	while(it != mRobotList.end())
	{
		it->second.tile->removeThing();
		++it;
	}
}


/**
 * Initialize values, the UI and set up event handling.
 */
void GameState::initialize()
{
	mReturnState = this;

	EventHandler& e = Utility<EventHandler>::get();
	e.keyDown().Connect(this, &GameState::onKeyDown);

	e.mouseButtonDown().Connect(this, &GameState::onMouseDown);
	e.mouseButtonUp().Connect(this, &GameState::onMouseUp);
	e.mouseMotion().Connect(this, &GameState::onMouseMove);
	e.mouseWheel().Connect(this, &GameState::onMouseWheel);

	// UI
	initUi();

	mPointers.push_back(Pointer("ui/pointers/normal.png", 0, 0));
	mPointers.push_back(Pointer("ui/pointers/place_tile.png", 16, 16));
	mPointers.push_back(Pointer("ui/pointers/inspect.png", 8, 8));
}


/**
 * Updates the entire state of the game.
 */
State* GameState::update()
{
	Renderer& r = Utility<Renderer>::get();

	r.drawImageStretched(mBackground, 0, 0, r.width(), r.height());

	mTileMap.injectMouse(mMousePosition.x(), mMousePosition.y());
	mTileMap.draw();
	drawUI();

	if(mDebug) drawDebug();

	return mReturnState;
}


void GameState::drawMiniMap()
{
	Renderer& r = Utility<Renderer>::get();

	if(mBtnToggleHeightmap.toggled())
		r.drawImage(mHeightMap, mMiniMapBoundingBox.x(), mMiniMapBoundingBox.y());
	else
		r.drawImage(mMapDisplay, mMiniMapBoundingBox.x(), mMiniMapBoundingBox.y());

	//r.drawBox(mMiniMapBoundingBox, 0, 200, 0);
	r.drawBox(mMiniMapBoundingBox.x() + mTileMap.mapViewLocation().x(), mMiniMapBoundingBox.y() + mTileMap.mapViewLocation().y(), mTileMap.edgeLength(), mTileMap.edgeLength(), 255, 255, 255);

	if(mCCLocation.x() != 0 && mCCLocation.y() != 0)
		r.drawBoxFilled(mCCLocation.x() + mMiniMapBoundingBox.x() - 1, mCCLocation.y() + mMiniMapBoundingBox.y() - 1, 3, 3, 255, 255, 255);


	for(size_t i = 0; i < mTileMap.mineLocations().size(); i++)
	{
		if(mTileMap.getTile(mTileMap.mineLocations()[i].x(), mTileMap.mineLocations()[i].y(), 0)->mine()->active())
			r.drawBoxFilled(mTileMap.mineLocations()[i].x() + mMiniMapBoundingBox.x() - 1, mTileMap.mineLocations()[i].y() + mMiniMapBoundingBox.y() - 1, 3, 3, constants::ACTIVE_MINE_COLOR.red(), constants::ACTIVE_MINE_COLOR.green(), constants::ACTIVE_MINE_COLOR.blue());
		else
			r.drawBoxFilled(mTileMap.mineLocations()[i].x() + mMiniMapBoundingBox.x() - 1, mTileMap.mineLocations()[i].y() + mMiniMapBoundingBox.y() - 1, 3, 3, constants::MINE_COLOR.red(), constants::MINE_COLOR.green(), constants::MINE_COLOR.blue());
	}
}


void GameState::drawResourceInfo()
{
	Renderer& r = Utility<Renderer>::get();

	r.drawBoxFilled(mResourceInfoBox, 0, 0, 0);
	r.drawBox(mResourceInfoBox, 0, 200, 0);

	// Resources
	int x = mResourceInfoBox.x() + 2;
	int y = mResourceInfoBox.y() + 2;
	int line = 0, numberX = 100;
	r.drawText(mTinyFont, "Common Metals Ore:", x, y + line * 10, 255, 255, 255);
	drawNumber(r, mTinyFont, mPlayerResources.commonMetalsOre, x + numberX, y + line++ * 10, 255, 255, 255);

	r.drawText(mTinyFont, "Rare Metals Ore:", x, y + line * 10, 255, 255, 255);
	drawNumber(r, mTinyFont, mPlayerResources.rareMetalsOre, x + numberX, y + line++ * 10, 255, 255, 255);

	r.drawText(mTinyFont, "Common Minerals Ore:", x, y + line * 10, 255, 255, 255);
	drawNumber(r, mTinyFont, mPlayerResources.commonMineralsOre, x + numberX, y + line++ * 10, 255, 255, 255);

	r.drawText(mTinyFont, "Rare Minerals Ore:", x, y + line * 10, 255, 255, 255);
	drawNumber(r, mTinyFont, mPlayerResources.rareMineralsOre, x + numberX, y + line++ * 10, 255, 255, 255);

	line++;

	r.drawText(mTinyFont, "Common Metals:", x, y + line * 10, 255, 255, 255);
	drawNumber(r, mTinyFont, mPlayerResources.commonMetals, x + numberX, y + line++ * 10, 255, 255, 255);

	r.drawText(mTinyFont, "Rare Metals:", x, y + line * 10, 255, 255, 255);
	drawNumber(r, mTinyFont, mPlayerResources.rareMetals, x + numberX, y + line++ * 10, 255, 255, 255);

	r.drawText(mTinyFont, "Common Minerals:", x, y + line * 10, 255, 255, 255);
	drawNumber(r, mTinyFont, mPlayerResources.commonMinerals, x + numberX, y + line++ * 10, 255, 255, 255);

	r.drawText(mTinyFont, "Rare Minerals:", x, y + line * 10, 255, 255, 255);
	drawNumber(r, mTinyFont, mPlayerResources.rareMinerals, x + numberX, y + line++ * 10, 255, 255, 255);

	line++;

	r.drawText(mTinyFont, "Energy:", x, y + line * 10, 255, 255, 255);
	drawNumber(r, mTinyFont, mPlayerResources.energy, x + numberX, y + line++ * 10, 255, 255, 255);
	r.drawText(mTinyFont, "Food:", x, y + line * 10, 255, 255, 255);
	drawNumber(r, mTinyFont, mPlayerResources.food, x + numberX, y + line++ * 10, 255, 255, 255);
}


void GameState::drawDebug()
{
	Renderer& r = Utility<Renderer>::get();

	str.str("");
	str << "FPS: " << mFps.fps();
	r.drawText(mTinyFont, str.str(), 10, 10, 255, 255, 255);
}


/**
 * Key down event handler.
 */
void GameState::onKeyDown(KeyCode key, KeyModifier mod, bool repeat)
{
	Point_2d pt = mTileMap.mapViewLocation();

	switch(key)
	{
		case KEY_w:
		case KEY_UP:
			pt.y(clamp(--pt.y(), 0, mTileMap.height() - mTileMap.edgeLength()));
			break;

		case KEY_s:
		case KEY_DOWN:
			pt.y(clamp(++pt.y(), 0, mTileMap.height() - mTileMap.edgeLength()));
			break;

		case KEY_a:
		case KEY_LEFT:
			pt.x(clamp(--pt.x(), 0, mTileMap.width() - mTileMap.edgeLength()));
			break;

		case KEY_d:
		case KEY_RIGHT:
			pt.x(clamp(++pt.x(), 0, mTileMap.width() - mTileMap.edgeLength()));
			break;

		case KEY_0:
			mTileMap.currentDepth(0);
			break;

		case KEY_1:
			mTileMap.currentDepth(1);
			break;

		case KEY_2:
			mTileMap.currentDepth(2);
			break;

		case KEY_3:
			mTileMap.currentDepth(3);
			break;

		case KEY_4:
			mTileMap.currentDepth(4);
			break;

		case KEY_ESCAPE:
			clearMode();
			break;

		default:
			break;
	}

	mTileMap.mapViewLocation(pt.x(), pt.y());
}


void GameState::clearMode()
{
	mInsertMode = INSERT_NONE;
	mCurrentPointer = POINTER_NORMAL;
}


/**
 * Mouse Down event handler.
 */
void GameState::onMouseDown(MouseButton button, int x, int y)
{
	if(button == BUTTON_RIGHT)
	{
		if(mInsertMode != INSERT_NONE)
		{
			clearMode();
		}
		else
		{
			int x = mTileMap.tileHighlight().x() + mTileMap.mapViewLocation().x();
			int y = mTileMap.tileHighlight().y() + mTileMap.mapViewLocation().y();

			mTileInspector.tile(mTileMap.getTile(x, y));
			mTileInspector.visible(true);
			// bring up tile inspector window
		}
	}

	if(button == BUTTON_LEFT)
	{
		mLeftButtonDown = true;
		
		// MiniMap Check
		if(isPointInRect(mMousePosition, mMiniMapBoundingBox))
		{
			updateMapView();
		}
		// Click was within the bounds of the TileMap.
		else if(isPointInRect(mMousePosition, mTileMap.boundingBox()))
		{
			if(mInsertMode == INSERT_STRUCTURE)
			{
				if(mCurrentStructure == STRUCTURE_NONE)
					return;

				placeStructure();
			}
			else if(mInsertMode == INSERT_ROBOT)
			{
				placeRobot();
			}
			else if(mInsertMode == INSERT_TUBE)
			{
				placeTubes();
			}
		}
	}
}


void GameState::placeTubes()
{
	int x = mTileMap.tileHighlight().x() + mTileMap.mapViewLocation().x();
	int y = mTileMap.tileHighlight().y() + mTileMap.mapViewLocation().y();

	Tile* tile = mTileMap.getTile(x, y, mTileMap.currentDepth());
	if(!tile)
		return;

	// Check the basics.
	if (tile->thing() || tile->mine() || !tile->bulldozed() || !tile->excavated())
		return;

	// FIXME: FUGLY!
	if (validTubeConnection(mTileMap.getTile(x + 1, y, mTileMap.currentDepth()), DIR_EAST) ||
		validTubeConnection(mTileMap.getTile(x - 1, y, mTileMap.currentDepth()), DIR_WEST) ||
		validTubeConnection(mTileMap.getTile(x, y + 1, mTileMap.currentDepth()), DIR_SOUTH) ||
		validTubeConnection(mTileMap.getTile(x, y - 1, mTileMap.currentDepth()), DIR_NORTH)
		)
	{
		// Warning expected. Any non-0 value for depth is considered underground.
		if(mCurrentStructure == STRUCTURE_TUBE_INTERSECTION)
			mStructureManager.addStructure(new Tube(CONNECTOR_INTERSECTION, mTileMap.currentDepth()), mTileMap.getTile(x, y), x, y, mTileMap.currentDepth(), true);
		else if (mCurrentStructure == STRUCTURE_TUBE_RIGHT)
			mStructureManager.addStructure(new Tube(CONNECTOR_RIGHT, mTileMap.currentDepth()), mTileMap.getTile(x, y), x, y, mTileMap.currentDepth(), true);
		else if (mCurrentStructure == STRUCTURE_TUBE_LEFT)
			mStructureManager.addStructure(new Tube(CONNECTOR_LEFT, mTileMap.currentDepth()), mTileMap.getTile(x, y), x, y, mTileMap.currentDepth(), true);
		else
			throw Exception(0, "Structure Not a Tube", "GameState::placeTube() called but Current Structure is not a tube!");
	}
}


/**
 * Checks to see if a tile is a valid tile to place a tube against.
 * 
 * \todo	This seems way over complicated... Find a better way to do this check.
 */
bool GameState::validTubeConnection(Tile *tile, Direction dir)
{

	if(tile->mine() || !tile->bulldozed() || !tile->excavated() || !tile->thingIsStructure())
		return false;

	// FIXME: FUGLY FUGLY FUGLY!!!!!
	Structure* _structure = reinterpret_cast<Structure*>(tile->thing());

	if(mCurrentStructure == STRUCTURE_TUBE_INTERSECTION)
	{
		if (dir == DIR_EAST || dir == DIR_WEST)
		{
			if (_structure->connectorDirection() == CONNECTOR_INTERSECTION || _structure->connectorDirection() == CONNECTOR_RIGHT)
				return true;
		}
		else // NORTH/SOUTH
		{
			if (_structure->connectorDirection() == CONNECTOR_INTERSECTION || _structure->connectorDirection() == CONNECTOR_LEFT)
				return true;
		}
	}
	else if(mCurrentStructure == STRUCTURE_TUBE_RIGHT)
	{
		if (dir == DIR_EAST || dir == DIR_WEST)
		{
			if (_structure->connectorDirection() == CONNECTOR_INTERSECTION || _structure->connectorDirection() == CONNECTOR_RIGHT)
				return true;
		}
	}
	else if(mCurrentStructure == STRUCTURE_TUBE_LEFT)
	{
		if (dir == DIR_NORTH || dir == DIR_SOUTH)
		{
			if (_structure->connectorDirection() == CONNECTOR_INTERSECTION || _structure->connectorDirection() == CONNECTOR_LEFT)
				return true;
		}
	}

	return false;
}


/**
* Checks a tile to see if a valid tube connection is available for structure placement.
*
* \todo	It would seem that we can combine this function and the validTubeConnection function
*		in some way.
*/
bool GameState::validStructurePlacement(Tile *tile, Direction dir)
{

	if (tile->mine() || !tile->bulldozed() || !tile->excavated() || !tile->thingIsStructure() || !tile->connected())
		return false;

	// FIXME: FUGLY FUGLY FUGLY!!!!!
	Structure* _structure = reinterpret_cast<Structure*>(tile->thing());
	if (!_structure->isConnector())
		return false;

	if (dir == DIR_EAST || dir == DIR_WEST)
	{
		if (_structure->connectorDirection() == CONNECTOR_INTERSECTION || _structure->connectorDirection() == CONNECTOR_RIGHT)
			return true;
	}
	else // NORTH/SOUTH
	{
		if (_structure->connectorDirection() == CONNECTOR_INTERSECTION || _structure->connectorDirection() == CONNECTOR_LEFT)
			return true;
	}

	return false;
}


void GameState::placeRobot()
{
	int x = mTileMap.tileHighlight().x() + mTileMap.mapViewLocation().x();
	int y = mTileMap.tileHighlight().y() + mTileMap.mapViewLocation().y();

	Tile* tile = mTileMap.getTile(x, y, mTileMap.currentDepth());
	if(!tile)
		return;

	// Robodozer has been selected.
	if(mRobotsMenu.selectionText() == constants::ROBODOZER)
	{
		/**
		 * \todo	If there's a structure here we want to confirm
		 *			with the player that they want to destroy it.
		 */
		if(tile->thing() || tile->mine() || tile->index() == 0 || !tile->excavated())
			return;

		Robot* r = mRobotPool.getRobot(RobotPool::ROBO_DOZER);
		r->startTask(tile->index());
		insertRobot(r, tile, x, y, mTileMap.currentDepth());
		tile->index(0);

		//clearMode();

		if(!mRobotPool.robotAvailable(RobotPool::ROBO_DOZER))
		{
			mRobotsMenu.removeItem(constants::ROBODOZER);
			clearMode();
		}
	}
	// Robodigger has been selected.
	else if(mRobotsMenu.selectionText() == constants::ROBODIGGER)
	{
		// Die if tile is occupied or not excavated.
		if(tile->thing() || tile->mine() || !tile->excavated())
			return;

		// Keep digger within a safe margin of the map boundaries.
		if(x < 3 || x > mTileMap.width() - 4 || y < 3 || y > mTileMap.height() - 4)
		{
			cout << "GameState::placeRobot(): Can't place digger within 3 tiles of the edge of a map." << endl;
			return;
		}

		hideUi();
		mDiggerTile(tile, mTileMap.currentDepth(), x, y);

		// If we're placing on the top level we can only ever go down.
		if(mTileMap.currentDepth() == 0)
			diggerSelectionDialog(DiggerDirection::SEL_DOWN);
		else
			mDiggerDirection.visible(true);

		clearMode();
	}
	// Robominer has been selected.
	else if(mRobotsMenu.selectionText() == constants::ROBOMINER)
	{
		if(tile->thing() || !tile->mine() || !tile->excavated())
			return;

		Robot* r = mRobotPool.getRobot(RobotPool::ROBO_MINER);
		r->startTask(6);
		insertRobot(r, tile, x, y, mTileMap.currentDepth());
		tile->index(0);

		clearMode();

		if(mRobotPool.getRobot(RobotPool::ROBO_MINER) == NULL)
			mRobotsMenu.removeItem(constants::ROBOMINER);
	}

	if(mRobotPool.allRobotsBusy())
		mBtnRobotPicker.enabled(false);
}


/**
 * Called whenever a RoboDozer completes its task.
 */
void GameState::dozerTaskFinished(Robot* _r)
{
	if(!mRobotsMenu.itemExists(constants::ROBODOZER))
	{
		mRobotsMenu.addItem(constants::ROBODOZER);

		if(!mBtnRobotPicker.enabled())
			mBtnRobotPicker.enabled(true);
	}
}


/**
 * Called whenever a RoboDigger completes its task.
 */
void GameState::diggerTaskFinished(Robot* _r)
{
	if(mRobotList.find(_r) == mRobotList.end())
		throw Exception(0, "Renegade Robot", "GameState::diggerTaskFinished() called with a Robot not in the Robot List!");

	TilePositionInfo tpi = mRobotList[_r];

	// FIXME: Fugly cast. Why do we have a robot list and a robot pool?
	Direction dir = reinterpret_cast<Robodigger*>(_r)->direction();

	// 
	int originX = 0, originY = 0, depthAdjust = 0;

	if(dir == DIR_DOWN)
	{
		mStructureManager.addStructure(new AirShaft(), tpi.tile, tpi.x, tpi.y, tpi.depth, false);
		AirShaft *as = new AirShaft();
		as->ug();
		mStructureManager.addStructure(as, mTileMap.getTile(tpi.x, tpi.y, tpi.depth + 1), tpi.x, tpi.y, tpi.depth + 1, false);

		originX = tpi.x;
		originY = tpi.y;
		depthAdjust = 1;

		mTileMap.getTile(originX, originY, tpi.depth)->index(0);
		mTileMap.getTile(originX, originY, tpi.depth + depthAdjust)->index(0);
	}
	else if(dir == DIR_NORTH)
	{
		originX = tpi.x;
		originY = tpi.y - 1;
	}
	else if(dir == DIR_SOUTH)
	{
		originX = tpi.x;
		originY = tpi.y + 1;
	}
	else if(dir == DIR_WEST)
	{
		originX = tpi.x - 1;
		originY = tpi.y;
	}
	else if(dir == DIR_EAST)
	{
		originX = tpi.x + 1;
		originY = tpi.y;
	}

	/**
	 * \todo	Add checks for obstructions and things that explode if
	 *			a digger gets in the way (or should diggers be smarter than
	 *			puncturing a fusion reactor containment vessel?)
	 */
	for(int y = originY - 1; y <= originY + 1; ++y)
	{
		for(int x = originX - 1; x <= originX + 1; ++x)
		{
			mTileMap.getTile(x, y, tpi.depth + depthAdjust)->excavated(true);
		}
	}


	if(!mRobotsMenu.itemExists(constants::ROBODIGGER))
	{
		mRobotsMenu.addItem(constants::ROBODIGGER);

		if(!mBtnRobotPicker.enabled())
			mBtnRobotPicker.enabled(true);
	}
}


/**
 * Called whenever a RoboMiner completes its task.
 */
void GameState::minerTaskFinished(Robot* _r)
{
	if(mRobotList.find(_r) == mRobotList.end())
		throw Exception(0, "Renegade Robot", "GameState::minerTaskFinished() called with a Robot not in the Robot List!");

	TilePositionInfo tpi = mRobotList[_r];

	MineFacility* m = new MineFacility(tpi.tile->mine());
	m->idle(false);
	mStructureManager.addStructure(m, tpi.tile, tpi.x, tpi.y, tpi.depth, false);

	if(!mRobotsMenu.itemExists(constants::ROBOMINER))
	{
		mRobotsMenu.addItem(constants::ROBOMINER);

		if(!mBtnRobotPicker.enabled())
			mBtnRobotPicker.enabled(true);
	}
}


/**
 * Places a structure into the map.
 */
void GameState::placeStructure()
{
	int x = mTileMap.tileHighlight().x() + mTileMap.mapViewLocation().x();
	int y = mTileMap.tileHighlight().y() + mTileMap.mapViewLocation().y();

	Tile* tile = mTileMap.getTile(x, y, mTileMap.currentDepth());
	if(!tile)
		return;

	if(tile->mine() || tile->thing() || !tile->bulldozed() && mCurrentStructure != STRUCTURE_SEED_LANDER)
	{
		cout << "GameState::placeStructure(): Tile is unsuitable to place a structure." << endl;
		return;
	}

	// Seed lander is a special case and only one can ever be placed by the player ever.
	if(mCurrentStructure == STRUCTURE_SEED_LANDER)
	{
		// Has to be built away from the edges of the map
		if(x > 3 && x < mTileMap.width() - 3 && y > 3 && y < mTileMap.height() - 3)
		{
			// check for obstructions
			if(!landingSiteSuitable(x, y))
			{
				cout << "Unable to place SEED Lander. Tiles obstructed." << endl;
				return;
			}

			SeedLander* s = new SeedLander(x, y);
			s->deployCallback().Connect(this, &GameState::deploySeedLander);
			mStructureManager.addStructure(s, tile, x, y, 0, true); // Can only ever be placed on depth level 0
			clearMode();
			mStructureMenu.dropAllItems();
			mBtnStructurePicker.enabled(false);
		}
	}
	else
	{
		if (validStructurePlacement(mTileMap.getTile(x, y - 1), DIR_NORTH) == false &&
			validStructurePlacement(mTileMap.getTile(x + 1, y), DIR_EAST) == false && 
			validStructurePlacement(mTileMap.getTile(x, y + 1), DIR_SOUTH) == false && 
			validStructurePlacement(mTileMap.getTile(x - 1, y), DIR_WEST) == false)
		{
			cout << "GameState::placeStructure(): Invalid structure placement." << endl;
			return;
		}

		if (mCurrentStructure == STRUCTURE_AGRIDOME)
		{
			mStructureManager.addStructure(new Agridome(), tile, x, y, 0, false);
		}
		if (mCurrentStructure == STRUCTURE_CHAP)
		{
			mStructureManager.addStructure(new CHAP(), tile, x, y, 0, false);
		}
	}
}


/**
 * Mouse Up event handler.
 */
void GameState::onMouseUp(MouseButton button, int x, int y)
{
	if(button == BUTTON_LEFT)
	{
		mLeftButtonDown = false;
	}
}


/**
 * Mouse motion event handler.
 */
void GameState::onMouseMove(int x, int y, int rX, int rY)
{
	mMousePosition(x, y);

	if(mLeftButtonDown)
	{
		if(isPointInRect(mMousePosition, mMiniMapBoundingBox))
		{
			updateMapView();
		}
	}
}

void GameState::updateMapView()
{
	int x = clamp(mMousePosition.x() - mMiniMapBoundingBox.x() - mTileMap.edgeLength() / 2, 0, mTileMap.width() - mTileMap.edgeLength());
	int y = clamp(mMousePosition.y() - mMiniMapBoundingBox.y() - mTileMap.edgeLength() / 2, 0, mTileMap.height() - mTileMap.edgeLength());

	mTileMap.mapViewLocation(x, y);
}


/**
 * Mouse wheel event handler.
 */
void GameState::onMouseWheel(int x, int y)
{}


/**
 * Inserts a Thing into a Tile and also adds an entry into the ThingMapInfo list.
 * 
 * \return	Returns false if tile is already occupied by a Thing or Mine.
 *
 * \todo	Currently fails if Tile is occupied. Should add parameters to overwrite
 *			what's in the Tile given several parameters.
 */
bool GameState::insertThing(Thing* thing, Tile* tile, int x, int y, int depth)
{
	if(!tile)
		return false;

	ThingMap::iterator it = mThingList.find(thing);
	if(it != mThingList.end())
		throw Exception(0, "Duplicate Thing", "GameState::insertThing(): Attempting to add a duplicate Thing* pointer.");

	mThingList[thing] = TilePositionInfo(tile, x, y, depth);
	tile->pushThing(thing);

	return true;
}


bool GameState::insertRobot(Robot* robot, Tile* tile, int x, int y, int depth)
{
	if(!tile)
		return false;

	RobotMap::iterator it = mRobotList.find(robot);
	if(it != mRobotList.end())
		throw Exception(0, "Duplicate Robot", "GameState::insertRobot(): Attempting to add a duplicate Robot* pointer.");

	mRobotList[robot] = TilePositionInfo(tile, x, y, depth);
	tile->pushThing(robot);

	return true;
}


/**
 * Check landing site for obstructions such as mining beacons, things
 * and impassable terrain.
 */
bool GameState::landingSiteSuitable(int x, int y)
{
	for(int offY = y - 1; offY <= y + 1; ++offY)
		for(int offX = x - 1; offX <= x + 1; ++offX)
			if(mTileMap.getTile(offX, offY)->index() > 3 || mTileMap.getTile(offX, offY)->mine() || mTileMap.getTile(offX, offY)->thing())
				return false;

	return true;
}


/**
 * Sets up the initial colony deployment.
 */
void GameState::deploySeedLander(int x, int y)
{
	mTileMap.getTile(x, y)->index(0);
	
	// TOP ROW
	mStructureManager.addStructure(new SeedPower(), mTileMap.getTile(x - 1, y - 1), x - 1, y - 1, 0, true);
	mTileMap.getTile(x - 1, y - 1)->index(0);

	mStructureManager.addStructure(new Tube(CONNECTOR_INTERSECTION, false), mTileMap.getTile(x, y - 1), x, y - 1, 0, true);
	mTileMap.getTile(x, y - 1)->index(0);

	CommandCenter* cc = new CommandCenter();
	cc->sprite().skip(3);
	mStructureManager.addStructure(cc, mTileMap.getTile(x + 1, y - 1), x + 1, y - 1, 0, true);
	mTileMap.getTile(x + 1, y - 1)->index(0);
	mCCLocation(x + 1, y - 1);


	// MIDDLE ROW
	mTileMap.getTile(x - 1, y)->index(0);
	mStructureManager.addStructure(new Tube(CONNECTOR_INTERSECTION, false), mTileMap.getTile(x - 1, y), x - 1, y, 0, true);

	mTileMap.getTile(x + 1, y)->index(0);
	mStructureManager.addStructure(new Tube(CONNECTOR_INTERSECTION, false), mTileMap.getTile(x + 1, y), x + 1, y, 0, true);


	// BOTTOM ROW
	SeedFactory* sf = new SeedFactory();
	sf->sprite().skip(7);
	mStructureManager.addStructure(sf, mTileMap.getTile(x - 1, y + 1), x - 1, y + 1, 0, true);
	mTileMap.getTile(x - 1, y + 1)->index(0);

	mTileMap.getTile(x, y + 1)->index(0);
	mStructureManager.addStructure(new Tube(CONNECTOR_INTERSECTION, false), mTileMap.getTile(x, y + 1), x, y + 1, 0, true);

	SeedSmelter* ss = new SeedSmelter();
	ss->sprite().skip(10);
	mStructureManager.addStructure(ss, mTileMap.getTile(x + 1, y + 1), x + 1, y + 1, 0, true);
	mTileMap.getTile(x + 1, y + 1)->index(0);

	// Enable UI Contruction Buttons
	mBtnTubePicker.enabled(true);	
	mBtnRobotPicker.enabled(true);

	// Robots only become available after the SEED Factor is deployed.
	mRobotsMenu.sorted(true);
	mRobotsMenu.addItem(constants::ROBODOZER);
	mRobotsMenu.addItem(constants::ROBODIGGER);
	mRobotsMenu.addItem(constants::ROBOMINER);

	mRobotPool.addRobot(RobotPool::ROBO_DOZER)->taskComplete().Connect(this, &GameState::dozerTaskFinished);
	mRobotPool.addRobot(RobotPool::ROBO_DIGGER)->taskComplete().Connect(this, &GameState::diggerTaskFinished);
	mRobotPool.addRobot(RobotPool::ROBO_MINER)->taskComplete().Connect(this, &GameState::minerTaskFinished);

	// FIXME: After debugging remove these extra robots from the beginning
	mRobotPool.addRobot(RobotPool::ROBO_DOZER)->taskComplete().Connect(this, &GameState::dozerTaskFinished);
	mRobotPool.addRobot(RobotPool::ROBO_DOZER)->taskComplete().Connect(this, &GameState::dozerTaskFinished);
	mRobotPool.addRobot(RobotPool::ROBO_DOZER)->taskComplete().Connect(this, &GameState::dozerTaskFinished);
	mRobotPool.addRobot(RobotPool::ROBO_DOZER)->taskComplete().Connect(this, &GameState::dozerTaskFinished);
	mRobotPool.addRobot(RobotPool::ROBO_DOZER)->taskComplete().Connect(this, &GameState::dozerTaskFinished);
	mRobotPool.addRobot(RobotPool::ROBO_DOZER)->taskComplete().Connect(this, &GameState::dozerTaskFinished);


	// FIXME: Magic numbers
	mPlayerResources.commonMetals = 100;
	mPlayerResources.commonMinerals = 20;

	mPopulationPool.addWorkers(30);
	mPopulationPool.addScientists(20);
}



/**
 * Updates all robots.
 */
void GameState::updateRobots()
{
	RobotMap::iterator robot_it = mRobotList.begin();
	while(robot_it != mRobotList.end())
	{
		robot_it->first->update();

		/**
		 * Clear Idle robots from tiles.
		 */
		if(robot_it->first->idle())
		{
			// Make sure that we're the robot from a Tile and not something else
			if(robot_it->second.tile->thing() == robot_it->first)
				robot_it->second.tile->removeThing();
	
			robot_it = mRobotList.erase(robot_it);
		}
		else
			++robot_it;
	}
}





/**
 * Checks the connectedness of all tiles surrounding
 * the Command Center.
 */
void GameState::checkConnectedness()
{
	cout << "Checking connectedness... ";

	// Assumes that a 0,0 location means the CC hasn't yet been placed
	// as CC's can't be placed on the edges of the map.
	if (mCCLocation.x() == 0 || mCCLocation.y() == 0)
	{
		cout << "CC not yet placed." << endl;
		return;
	}

	// Assumes that the 'thing' at mCCLocation is in fact a structure.
	Tile *t = mTileMap.getTile(mCCLocation.x(), mCCLocation.y(), 0);
	Structure *cc = reinterpret_cast<Structure*>(t->thing());

	// No point in graph walking if the CC isn't operating normally.
	if (cc->state() != Structure::OPERATIONAL)
	{
		cout << "CC not operational." << endl;
		return;
	}

	// Create the initial node so we can instruct it to walk through all
	// surrounding tiles and start determining connectedness.
	GraphWalker CommandCenter;
	CommandCenter.gridPosition(mCCLocation);
	CommandCenter.depth(0);
	CommandCenter.tileMap(&mTileMap);
	CommandCenter.walkGraph();

	cout << "done." << endl;
}