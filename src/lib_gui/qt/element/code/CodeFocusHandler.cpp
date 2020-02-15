#include "CodeFocusHandler.h"

#include "MessageFocusIn.h"
#include "MessageFocusOut.h"
#include "MessageFocusView.h"

void CodeFocusHandler::clear()
{
	setCurrentFocus({}, false);
	m_oldFocus = Focus();
	m_targetColumn = 0;
}

void CodeFocusHandler::focus()
{
	m_hasFocus = true;

	if (!m_oldFocus.isEmpty())
	{
		setCurrentFocus(m_oldFocus, false);
		m_oldFocus = Focus();
	}
}

void CodeFocusHandler::defocus()
{
	m_oldFocus = m_focus;
	setCurrentFocus({}, false);
	m_hasFocus = false;
}

const CodeFocusHandler::Focus& CodeFocusHandler::getCurrentFocus() const
{
	return m_focus;
}

void CodeFocusHandler::setCurrentFocus(const Focus& focus, bool fromMouse)
{
	if (m_hasFocus)
	{
		bool same = (m_focus.tokenIds == focus.tokenIds);
		if (!fromMouse && !same && m_focus.tokenIds.size())
		{
			MessageFocusOut(m_focus.tokenIds).dispatch();
		}

		m_focus = focus;
		updateFiles();

		if (!fromMouse && !same && m_focus.tokenIds.size())
		{
			MessageFocusIn(m_focus.tokenIds).dispatch();
		}
	}
	else
	{
		m_oldFocus = focus;
	}
}

bool CodeFocusHandler::hasCurrentFocus() const
{
	if (m_hasFocus)
	{
		return !m_focus.isEmpty();
	}
	else
	{
		return !m_oldFocus.isEmpty();
	}
}

void CodeFocusHandler::setFocusedLocationId(
	QtCodeArea* area, size_t lineNumber, size_t columnNumber, Id locationId, const std::vector<Id>& tokenIds, bool fromMouse)
{
	if (columnNumber)
	{
		m_targetColumn = columnNumber;
	}

	setCurrentFocus({nullptr, area, nullptr, lineNumber, locationId, tokenIds}, fromMouse);
}

void CodeFocusHandler::setFocusedScopeLine(QtCodeArea* area, QPushButton* scopeLine, size_t lineNumber)
{
	setCurrentFocus({nullptr, area, scopeLine, lineNumber, 0, {}}, false);
}

void CodeFocusHandler::setFocusedFile(QtCodeFile* file)
{
	setCurrentFocus({file, nullptr, nullptr, 0, 0, {}}, false);
}

size_t CodeFocusHandler::getTargetColumn() const
{
	return m_targetColumn;
}

void CodeFocusHandler::focusView()
{
	if (!m_hasFocus)
	{
		MessageFocusView(MessageFocusView::ViewType::CODE).dispatch();
	}
}
