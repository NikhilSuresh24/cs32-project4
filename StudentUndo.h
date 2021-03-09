#ifndef STUDENTUNDO_H_
#define STUDENTUNDO_H_

#include <stack>
#include <string>
#include "Undo.h"

class StudentUndo : public Undo
{
public:
	void submit(Action action, int row, int col, char ch = 0);
	Action get(int &row, int &col, int &count, std::string &text);
	void clear();
	~StudentUndo();

private:
	struct Undoable
	{
		Action m_action;
		int m_row;
		int m_col;
		std::string m_text;

		Undoable(Action action, int row, int col, std::string text) : m_action(action), m_row(row), m_col(col), m_text(text) {}
	};
	std::stack<Undoable *> m_actions;
};

#endif // STUDENTUNDO_H_
