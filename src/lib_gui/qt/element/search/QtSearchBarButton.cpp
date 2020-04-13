#include "QtSearchBarButton.h"

#include "ApplicationSettings.h"

QtSearchBarButton::QtSearchBarButton(const FilePath& iconPath, bool isSmall, QWidget* parent)
	: QtSelfRefreshIconButton(QLatin1String(""), iconPath, "search/button", parent), m_small(isSmall)
{
	refresh();
}

void QtSearchBarButton::refresh()
{
	QtSelfRefreshIconButton::refresh();

	const int size = m_small ? 10 : 16;

	const float height = std::max(
		static_cast<float>(ApplicationSettings::getInstance()->getFontSize() + size),
		static_cast<float>(size + 14));
	setFixedHeight(static_cast<int>(height));

	if (!m_small)
	{
		const int iconSize = int(height / 4) * 2 + 2;
		setIconSize(QSize(iconSize, iconSize));
	}
}
