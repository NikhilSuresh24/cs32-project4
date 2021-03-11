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
	// consider batching only if there are old actions
	if (!m_actions.empty())
	{
		Undoable *top = m_actions.top();
		bool deleteCond = action == DELETE && top->m_col == col;
		bool backspaceCond = action == DELETE && top->m_col - 1 == col;
		bool insertCond = action == INSERT && top->m_col + 1 == col;
		// if batching conditions met, combine text and get rid of top
		if (top->m_action == action && top->m_row == row && (deleteCond || backspaceCond || insertCond))
		{
			if (backspaceCond)
			{
				text += ch + top->m_text;
			}
			else
			{
				text += top->m_text + ch;
			}
			// text += top->m_text + ch;
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
		break;
	case JOIN:
		inverseAction = SPLIT;
		break;
	case SPLIT:
		inverseAction = JOIN;
		break;
	// error case should never occur, just listed for switch statement completeness
	case ERROR:
		inverseAction = ERROR;
		break;
	}

	// set count and col param
	if (inverseAction == DELETE)
	{
		// starting pos to delete different from other inverses
		count = top->m_text.size();
		col = top->m_col - count;
	}
	else
	{
		count = 1;
		col = top->m_col;
	}

	// set text param, only insert has text
	if (inverseAction == INSERT)
	{
		text = top->m_text;
	}
	else
	{
		text = "";
	}

	// set row
	row = top->m_row; // universal for all actions


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