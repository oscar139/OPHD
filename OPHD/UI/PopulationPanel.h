#pragma once

#include "Core/Control.h"

#include <NAS2D/Resource/Font.h>
#include <NAS2D/Renderer/RectangleSkin.h>

#include <vector>

class Population;


class PopulationPanel: public Control
{
public:
	PopulationPanel();

	void population(Population* pop) { mPopulation = pop; }

	void morale(int val) { mMorale = val; }
	void old_morale(int val) { mPreviousMorale = val; }

	void residentialCapacity(int val) { mResidentialCapacity = val; }

	void addMoraleReason(const std::string& str, int val)
	{
		if (val == 0) { return; }
		mMoraleChangeReasons.push_back(std::make_pair(str, val));
	}

	const auto& moraleReasonList() const { return mMoraleChangeReasons; }

	void clearMoraleReasons() { mMoraleChangeReasons.clear(); }

	void update() override;

private:
	const NAS2D::Font& mFont;
	const NAS2D::Font& mFontBold;
	const NAS2D::Image& mIcons;
	NAS2D::RectangleSkin mSkin;

	std::vector<std::pair<std::string,int>> mMoraleChangeReasons;

	Population* mPopulation = nullptr;

	int mMorale{ 0 };
	int mPreviousMorale{ 0 };
	int mResidentialCapacity{ 0 };
	int mPopulationPanelWidth{ 0 };
};
