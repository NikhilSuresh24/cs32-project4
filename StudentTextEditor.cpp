#include "StudentTextEditor.h"
#include "Undo.h"
#include <string>
#include <list>
#include <iostream>
#include <fstream>

using namespace std;

TextEditor *createTextEditor(Undo *un)
{
	return new StudentTextEditor(un);
}

StudentTextEditor::StudentTextEditor(Undo *undo)
	: TextEditor(undo), m_lines({""}), m_editRowIter(m_lines.begin()), m_editRow(0), m_editCol(0)
{
}

StudentTextEditor::~StudentTextEditor()
{
	reset();
}

bool StudentTextEditor::load(std::string file)
{
	reset();
	ifstream infile(file);

	// invalid file
	if (!infile)
	{
		return false;
	}

	// loop through files and add line to node
	string line;
	while (getline(infile, line))
	{
		// remove \r from the line
		if (!line.empty() && line[line.size() - 1] == '\r')
		{
			line.pop_back();
		}

		// add to text list
		m_lines.push_back(line);
	}

	// reset editing position
	m_editCol = 0;
	m_editRow = 0;
	m_editRowIter = m_lines.begin();

	return true;
}

bool StudentTextEditor::save(std::string file)
{
	ofstream outfile(file);

	// check file creation success
	if (!outfile)
	{
		return false;
	}

	// save each line
	for (auto it = m_lines.begin(); it != m_lines.end(); ++it)
	{
		outfile << *it << endl;
	}

	return true;
}

void StudentTextEditor::reset()
{
	// clear lines and reset cursor
	m_lines.clear();
	m_editRowIter = m_lines.begin();
	m_editRow = 0;
	m_editCol = 0;

	// clear the undo state
	getUndo()->clear();
}

void StudentTextEditor::move(Dir dir)
{
	switch (dir)
	{
	case UP:
		// if not on first line, move 1 line down
		if (m_editRow != 0)
		{
			moveCursor(m_editRow - 1, m_editCol);
		}
		break;

	case DOWN:
		// increment editRow, unless we are already at the end
		if (m_editRow != m_lines.size() - 1)
		{
			moveCursor(m_editRow + 1, m_editCol);
		}
		break;

	case LEFT:
		// if first row, first col, don't do anything
		if (m_editRow == 0 && m_editCol == 0)
		{
			break;
		}
		// if first col, move to end of prev line
		else if (m_editCol == 0)
		{
			--m_editRowIter;
			--m_editRow;
			m_editCol = m_editRowIter->size(); // space after last char on that line
			break;
		}

		// otherwise, just move 1 char to the left
		else
		{
			--m_editCol;
			break;
		}

	case RIGHT:
		// if we at the last row, last col, then do nothing
		if (m_editCol == m_editRowIter->size() && m_editRow == m_lines.size() - 1)
		{
			break;
		}
		// if at end of a line, move to next line
		else if (m_editCol == m_editRowIter->size())
		{
			++m_editRowIter;
			++m_editRow;
			m_editCol = 0;
			break;
		}
		// otherwise, just increment col
		else
		{
			++m_editCol;
			break;
		}

	case HOME:
		// cursor at first row, first col
		moveCursor(0, 0);
		break;

	case END:
		// cursor at last row, last col
		m_editRowIter = m_lines.end();
		--m_editRowIter;
		m_editRow = m_lines.size() - 1;
		m_editCol = m_editRowIter->size(); // space after last char
		break;
	}
}

void StudentTextEditor::del()
{
	// always inform undo
	undoableDel(true);
}

void StudentTextEditor::backspace()
{
	// always inform undo
	undoableBackspace(true);
}

void StudentTextEditor::insert(char ch)
{
	// always inform undo
	undoableInsert(ch, true);
}

void StudentTextEditor::enter()
{
	// always inform undo
	undoableEnter(true);
}

void StudentTextEditor::getPos(int &row, int &col) const
{
	row = m_editRow;
	col = m_editCol;
}

int StudentTextEditor::getLines(int startRow, int numRows, std::vector<std::string> &lines) const
{
	// boundary conditions
	if (startRow < 0 || numRows < 0 || startRow > m_lines.size())
	{
		return -1;
	}
	lines.clear();

	// if startRow equal to size, nothing to be added to lines
	if (startRow == m_lines.size())
	{
		return 0;
	}

	// create copy of iterator to iterate through desired lines
	int endRow = (m_lines.size() < (startRow + numRows)) ? m_lines.size() : (startRow + numRows);
	auto rowCopy = m_editRowIter;
	std::advance(rowCopy, startRow - m_editRow); // get to startRow 

	// add to lines
	for (int i = startRow; i < endRow; ++i, ++rowCopy)
	{
		lines.push_back(*rowCopy);
	}

	// return num lines copied
	return endRow - startRow;
}

void StudentTextEditor::undo()
{
	// get undo info
	int row, col, count;
	string text;

	Undo::Action action = getUndo()->get(row, col, count, text);

	// apply undo action based on action type
	switch (action)
	{
	case Undo::Action::INSERT:
		moveCursor(row, col);
		for (int i = 0; i < text.size(); ++i)
		{
			// don't add this insert to undo stack
			undoableInsert(text[i], false);
		}
		moveCursor(row, col); // move cursor back after insertions
		break;
	case Undo::Action::DELETE:
		moveCursor(row, col);
		for (int i = 0; i < count; ++i)
		{
			// don't add this delete to undo stack
			undoableDel(false);
		}
		break;
	case Undo::Action::SPLIT:
		moveCursor(row, col);
		// don't add this enter to undo stack
		undoableEnter(false);
		moveCursor(row, col); // enter moves cursor down to next line
		break;
	case Undo::Action::JOIN:
		moveCursor(row, col);
		// don't add this del to undo stack
		undoableDel(false);

	case Undo::Action::ERROR:
		break;
	}
}

void StudentTextEditor::moveCursor(int row, int col)
{
	std::advance(m_editRowIter, row - m_editRow);
	m_editRow = row;
	int numCols = m_editRowIter->size();
	m_editCol = min(col, numCols);
}

void StudentTextEditor::undoableDel(bool isUndoable)
{
	// can't delete at EOF
	if (m_editRow == m_lines.size() - 1 && m_editCol == m_editRowIter->size())
	{
		return;
	}
	// if at end of line, merge with next line
	else if (m_editCol == m_editRowIter->size())
	{

		auto nextLine = ++m_editRowIter;
		--m_editRowIter;
		*m_editRowIter += *nextLine;
		m_lines.erase(nextLine);

		if (isUndoable)
		{
			getUndo()->submit(Undo::Action::JOIN, m_editRow, m_editCol);
		}
	}
	// otherwise, erase char and inform undo (if asked to)
	else
	{
		char ch = m_editRowIter->at(m_editCol);
		m_editRowIter->erase(m_editCol, 1);

		if (isUndoable)
		{
			getUndo()->submit(Undo::Action::DELETE, m_editRow, m_editCol, ch);
		}
	}
}
void StudentTextEditor::undoableBackspace(bool isUndoable)
{
	// if at top of doc, don't do anything
	if (m_editRow == 0 && m_editCol == 0)
	{
		return;
	}

	// if at first col, merge with line above
	else if (m_editCol == 0)
	{
		// update row and col trackers
		auto lineCopy = m_editRowIter;
		--m_editRowIter;
		--m_editRow;
		m_editCol = m_editRowIter->size();

		// merge lines and erase bottom line
		*m_editRowIter += *lineCopy;
		m_lines.erase(lineCopy);

		if (isUndoable)
		{
			getUndo()->submit(Undo::Action::JOIN, m_editRow, m_editCol);
		}
	}

	// else, delete char to left of editCol
	else
	{
		char ch = m_editRowIter->at(m_editCol - 1);
		m_editRowIter->erase(m_editCol - 1, 1);
		--m_editCol;

		if (isUndoable)
		{
			getUndo()->submit(Undo::Action::DELETE, m_editRow, m_editCol, ch);
		}
	}
}
void StudentTextEditor::undoableInsert(char ch, bool isUndoable)
{
	// add char depending on value (tab separate case)
	if (ch == '\t')
	{
		m_editRowIter->insert(m_editCol, "    "); // tab case
		m_editCol += 4;
	}
	else
	{
		m_editRowIter->insert(m_editCol, 1, ch); // insert 1 inst of ch at editcol
		++m_editCol;
	}

	// UNDO obj tracking
	if (isUndoable)
	{
		getUndo()->submit(Undo::Action::INSERT, m_editRow, m_editCol, ch);
	}
}
void StudentTextEditor::undoableEnter(bool isUndoable)
{
	// UNDO obj tracking
	if (isUndoable)
	{
		getUndo()->submit(Undo::Action::SPLIT, m_editRow, m_editCol);
	}

	// make new line if at the end of a page
	if (m_editRow == m_lines.size() - 1 && m_editCol == m_editRowIter->size())
	{
		m_lines.push_back("");
		++m_editRow;
		++m_editRowIter;
		m_editCol = 0;
	}
	// otherwise, make a new line for all chars from col - 1 to end, edit current row
	else
	{
		string nextLine = m_editRowIter->substr(m_editCol, string::npos);
		*m_editRowIter = m_editRowIter->substr(0, m_editCol);

		// add new line, update row and col counters
		m_editRowIter = m_lines.emplace(++m_editRowIter, nextLine);
		++m_editRow;
		m_editCol = 0;
	}
}