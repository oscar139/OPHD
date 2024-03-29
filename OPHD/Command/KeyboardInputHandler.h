#pragma once
#include <NAS2D/EventHandler.h>
#include <NAS2D/Renderer/Renderer.h>
#include <NAS2D/Utility.h>

#include "../Constants/Strings.h"
#include "../Map/MapView.h"

class KeyboardInputHandler
{
public:
	KeyboardInputHandler(std::unique_ptr<MapView> mapView);
	void handleInput(NAS2D::EventHandler::KeyCode key, NAS2D::EventHandler::KeyModifier mod);

private:
	std::unique_ptr<MapView> mMapView;
};