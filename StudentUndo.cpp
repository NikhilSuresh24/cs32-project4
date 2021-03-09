#include "StudentUndo.h"
#include <string>

using namespace std;

Undo *createUndo()
{
	return new StudentUndo;
}

StudentUndo::~StudentUndo()
{
	clear();
}

void StudentUndo::submit(const Action action, int row, int col, char ch)
{
	string text;
	if (!m_actions.empty())
	{
		Undoable *top = m_actions.top();
		bool batchableAction = action == INSERT || action == DELETE;
		// if batching, combine two actions
		if (batchableAction && top->m_action == action && top->m_row == row && top->m_col == col)
		{
			text += top->m_text + ch;
			delete top;
			m_actions.pop();
		}
	}

	// if batching not done, set text to ch
	if (text.size() == 0)
	{
		text += ch;
	}

	// add undoable
	Undoable *newUndoable = new Undoable(action, row, col, text);
	m_actions.push(newUndoable);
}

StudentUndo::Action StudentUndo::get(int &row, int &col, int &count, std::string &text)
{
	// no undoable actions performed, so return err
	if (m_actions.empty())
	{
		return ERROR;
	}

	// determine inverse action
	Undoable *top = m_actions.top();
	Action inverseAction;
	switch (top->m_action)
	{
	case INSERT:
		inverseAction = DELETE;
		break;
	case DELETE:
		inverseAction = INSERT;
		text = top->m_text;
		break;
	case JOIN:
		inverseAction = SPLIT;
		break;
	case SPLIT:
		inverseAction = JOIN;
		break;
	case ERROR:
		inverseAction = ERROR;
		break;
	}

	// set text param
	if (inverseAction == INSERT)
	{
		text = top->m_text;
	}
	else
	{
		text = "";
	}

	// set row and col
	row = top->m_row; // TODO: is this right, do i need to check by different actions
	col = top->m_col;

	// pop top
	delete top;
	m_actions.pop();

	return inverseAction;
}

void StudentUndo::clear()
{
	// clear undo stack
	while (!m_actions.empty())
	{
		Undoable *top = m_actions.top();
		delete top;
		m_actions.pop();
	}
}
