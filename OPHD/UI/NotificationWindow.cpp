#include "NotificationWindow.h"

#include "../Cache.h"
#include "../Constants.h"

#include <NAS2D/Utility.h>
#include <NAS2D/Renderer/Renderer.h>


using namespace NAS2D;


NotificationWindow::NotificationWindow():
	mIcons{ imageCache.load("ui/icons.png") }
{
	size({ 300, 220 });

	add(btnOkay, { 245, 195 });
	btnOkay.size({ 50, 20 });
	btnOkay.click().connect(this, &NotificationWindow::btnOkayClicked);

	add(mMessageArea, { 5, 65 });
	mMessageArea.size({ size().x - 10, 125 });
	mMessageArea.font(constants::FONT_PRIMARY, constants::FONT_PRIMARY_NORMAL);
}


void NotificationWindow::notification(const NotificationArea::Notification& notification)
{
	mNotification = notification;
	title(StringFromNotificationType(mNotification.type));
	mMessageArea.text(mNotification.message);
}


void NotificationWindow::btnOkayClicked()
{
	hide();
}


void NotificationWindow::update()
{
	if (!visible()) { return; }

	Window::update();

	auto& renderer = Utility<Renderer>::get();

	Point<float> iconLocation = position() + Vector{ 10, 30 };

	renderer.drawSubImage(mIcons, iconLocation, { 128, 64, 32, 32 }, ColorFromNotification(mNotification.type));
	renderer.drawSubImage(mIcons, iconLocation, IconRectFromNotificationType(mNotification.type), Color::Normal);
}