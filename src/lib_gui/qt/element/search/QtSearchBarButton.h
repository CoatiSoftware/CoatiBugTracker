#ifndef QT_SEARCH_BAR_BUTTON_H
#define QT_SEARCH_BAR_BUTTON_H

#include "QtSelfRefreshIconButton.h"

class QtSearchBarButton: public QtSelfRefreshIconButton
{
public:
	QtSearchBarButton(const FilePath& iconPath, bool isSmall = false, QWidget* parent = nullptr);

protected:
	void refresh() override;

private:
	bool m_small;
};

#endif	  // QT_SEARCH_BAR_BUTTON_H
