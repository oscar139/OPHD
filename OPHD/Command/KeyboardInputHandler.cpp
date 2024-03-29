#include "KeyboardInputHandler.h"

#include <NAS2D/Math/Vector.h>
#include <OPHD/Map/MapOffset.h>
#include <OPHD/DirectionOffset.h>
#include <OPHD/States/MapViewState.h>


KeyboardInputHandler::KeyboardInputHandler(std::unique_ptr<MapView> mapView) :
	mMapView(std::move(mapView))
{};


void KeyboardInputHandler::handleInput(NAS2D::EventHandler::KeyCode key, NAS2D::EventHandler::KeyModifier mod){
	const int shiftMoveScalar = NAS2D::EventHandler::shift(mod) ? 5 : 1;

	switch (key)
	{

	case NAS2D::EventHandler::KeyCode::KEY_w:
	case NAS2D::EventHandler::KeyCode::KEY_UP:
		mMapView->moveView(MapOffset{DirectionNorthWest * shiftMoveScalar, 0});
		break;

	case NAS2D::EventHandler::KeyCode::KEY_s:
	case NAS2D::EventHandler::KeyCode::KEY_DOWN:
		mMapView->moveView(MapOffset{DirectionSouthEast * shiftMoveScalar, 0});
		break;

	case NAS2D::EventHandler::KeyCode::KEY_a:
	case NAS2D::EventHandler::KeyCode::KEY_LEFT:
		mMapView->moveView(MapOffset{DirectionSouthWest * shiftMoveScalar, 0});
		break;

	case NAS2D::EventHandler::KeyCode::KEY_d:
	case NAS2D::EventHandler::KeyCode::KEY_RIGHT:
		mMapView->moveView(MapOffset{DirectionNorthEast * shiftMoveScalar, 0});
		break;

	case NAS2D::EventHandler::KeyCode::KEY_0:
		changeViewDepth(0);
		break;

	case NAS2D::EventHandler::KeyCode::KEY_1:
		changeViewDepth(1);
		break;

	case NAS2D::EventHandler::KeyCode::KEY_2:
		changeViewDepth(2);
		break;

	case NAS2D::EventHandler::KeyCode::KEY_3:
		changeViewDepth(3);
		break;

	case NAS2D::EventHandler::KeyCode::KEY_4:
		changeViewDepth(4);
		break;

	case NAS2D::EventHandler::KeyCode::KEY_PAGEUP:
		mMapView->moveView(MapOffsetUp);
		break;

	case NAS2D::EventHandler::KeyCode::KEY_PAGEDOWN:
		mMapView->moveView(MapOffsetDown);
		break;


	case NAS2D::EventHandler::KeyCode::KEY_HOME:
		changeViewDepth(0);
		break;

	case NAS2D::EventHandler::KeyCode::KEY_END:
		changeViewDepth(mTileMap->maxDepth());
		break;

	case NAS2D::EventHandler::KeyCode::KEY_F10:
		if (NAS2D::Utility<NAS2D::EventHandler>::get().control(mod) && NAS2D::Utility<NAS2D::EventHandler>::get().shift(mod))
		{
			mCheatMenu.show();
			mWindowStack.bringToFront(&mCheatMenu);
		}
		break;

	case NAS2D::EventHandler::KeyCode::KEY_F2:
		mFileIoDialog.scanDirectory(constants::SaveGamePath);
		mFileIoDialog.setMode(FileIo::FileOperation::Save);
		mFileIoDialog.show();
		break;

	case NAS2D::EventHandler::KeyCode::KEY_F3:
		mFileIoDialog.scanDirectory(constants::SaveGamePath);
		mFileIoDialog.setMode(FileIo::FileOperation::Load);
		mFileIoDialog.show();
		break;

	case NAS2D::EventHandler::KeyCode::KEY_ESCAPE:
		clearMode();
		resetUi();
		break;

	case NAS2D::EventHandler::KeyCode::KEY_ENTER:
		if (mBtnTurns.enabled()) { nextTurn(); }
		break;

	default:
		break;
	}
};