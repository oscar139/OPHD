// ==================================================================================
// = NAS2D+GUI
// = Copyright © 2008 - 2020, Leeor Dicker
// ==================================================================================
// = This file is part of the NAS2D+GUI library.
// ==================================================================================

#pragma once

#include "TextControl.h"
#include <NAS2D/Renderer/Color.h>
#include <string>


namespace NAS2D
{
	class Font;
}

/**
 * \class Label
 * \brief A control that contains readonly text.
 * 
 */
class Label : public TextControl
{
public:
	Label(std::string newText = "");

	void font(const NAS2D::Font* font);
	bool empty() const { return text().empty(); }
	void clear() { mText.clear(); }
	void update() override;

	void color(const NAS2D::Color& color);

protected:
	void resize();

private:
	NAS2D::Color textColor{NAS2D::Color::White};
	const NAS2D::Font* mFont;
	const int mPadding = 2;
};
