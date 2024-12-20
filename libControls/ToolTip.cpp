#include "ToolTip.h"

#include <NAS2D/EventHandler.h>
#include <NAS2D/Utility.h>
#include <NAS2D/Renderer/Renderer.h>


namespace
{
	constexpr int MarginTight{2};
	constexpr auto PaddingSize = NAS2D::Vector{MarginTight, MarginTight};
}


ToolTip::ToolTip():
	mFont{getDefaultFont()}
{
	NAS2D::Utility<NAS2D::EventHandler>::get().mouseMotion().connect({this, &ToolTip::onMouseMove});
}


ToolTip::~ToolTip()
{
	NAS2D::Utility<NAS2D::EventHandler>::get().mouseMotion().disconnect({this, &ToolTip::onMouseMove});
}


void ToolTip::add(Control& c, const std::string& str)
{
	for (auto& item : mControls)
	{
		if (item.first == &c)
		{
			item.second = str;
			return;
		}
	}

	mControls.push_back(std::make_pair(&c, str));
}


void ToolTip::buildDrawParams(std::pair<Control*, std::string>& item, int mouseX)
{
	const auto toolTipLineHeight = mFont.height();
	const int numberOfLines = std::count(item.second.begin(), item.second.end(), '\n') + 1;
	const auto toolTipHeight = toolTipLineHeight * numberOfLines + PaddingSize.y * 2;

	std::istringstream stream(item.second);
	std::string line;
	int maxWidth = 0;
	while (std::getline(stream, line, '\n'))
	{
		maxWidth = std::max(maxWidth, mFont.size(line).x);
	}
	const auto toolTipWidth = maxWidth + PaddingSize.x * 2;

	const auto toolTipSize = NAS2D::Vector<int>{toolTipWidth, toolTipHeight};

	auto tooltipPosition = item.first->position();

	auto& renderer = NAS2D::Utility<NAS2D::Renderer>::get();
	const auto maxX = renderer.size().x - toolTipSize.x;
	tooltipPosition.x = (mouseX > maxX) ? maxX : mouseX;

	tooltipPosition.y += (tooltipPosition.y < toolTipSize.y) ? toolTipSize.y : -toolTipSize.y;

	position(tooltipPosition);
	size(toolTipSize);
}


void ToolTip::onMouseMove(NAS2D::Point<int> position, NAS2D::Vector<int> relative)
{
	if (relative != NAS2D::Vector{0, 0})
	{
		if (mFocusedControl)
		{
			if (mFocusedControl->first->rect().contains(position)) { return; }
			else { mFocusedControl = nullptr; }
		}

		mTimer.reset();
	}

	for (auto& item : mControls)
	{
		if (mFocusedControl) { break; }
		if (item.first->rect().contains(position))
		{
			mFocusedControl = &item;
			buildDrawParams(item, position.x);
			return;
		}
	}

	mFocusedControl = nullptr;
}


void ToolTip::update()
{
	if (mTimer.elapsedTicks() < 1000)
	{
		return;
	}
	draw();
}


void ToolTip::draw() const
{
	if (mFocusedControl)
	{
		auto& renderer = NAS2D::Utility<NAS2D::Renderer>::get();
		renderer.drawBoxFilled(rect(), NAS2D::Color::DarkGray);
		renderer.drawBox(rect(), NAS2D::Color::Black);

		std::istringstream stream(mFocusedControl->second);
		std::string line;
		auto linePosition = position() + PaddingSize;
		while (std::getline(stream, line, '\n'))
		{
			renderer.drawText(mFont, line, linePosition);
			linePosition.y += mFont.height();
		}
	}
}
