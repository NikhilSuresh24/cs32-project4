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
	: TextEditor(undo), m_lines({""}), m_editRowIter(m_lines.begin()), m_editRow(0), m_editCol(0) //TODO: if this doesn't work, just declare m_lines and iter in body
{
}

StudentTextEditor::~StudentTextEditor()
{
	m_lines.clear();
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
		if (line[line.size() - 1] == '\r')
		{
			line.pop_back();
		}
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

	for (auto it = m_lines.begin(); it != m_lines.end(); ++it)
	{
		outfile << *it << endl; // TODO: make sure this adds newline to end of file
	}

	return true;
}

void StudentTextEditor::reset()
{
	m_lines.clear();
	m_editRowIter = m_lines.begin();
	m_editRow = 0;
	m_editCol = 0;
	//TODO: clear the undo state
}

void StudentTextEditor::move(Dir dir)
{
	switch (dir)
	{
	case UP:
		// if not on first line, move 1 line down
		if (m_editRow != 0)
		{
			--m_editRowIter;
			--m_editRow;
		}
		break;

	case DOWN:
		// increment editRow, unless we are already at the end
		if (m_editRow != m_lines.size() - 1)
		{
			++m_editRowIter;
			++m_editRow;
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
		m_editRowIter = m_lines.begin();
		m_editCol = 0;
		m_editRow = 0;
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
		m_lines.erase(nextLine); //TODO: im pretty sure rowIter should not be invalidated
	}
	else
	{
		m_editRowIter->erase(m_editCol, 1); //TODO: make sure this deletes the right char
	}

	//TODO: UNDO obj tracking
}

void StudentTextEditor::backspace()
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
	}

	// else, delete char to left of editCol
	else
	{
		m_editRowIter->erase(m_editCol - 1, 1);
		--m_editCol;
	}

	//TODO: UNDO obj tracking
}

void StudentTextEditor::insert(char ch)
{
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
	//TODO: UNDO obj tracking
}

void StudentTextEditor::enter()
{
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

	//TODO: UNDO obj tracking
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
	std::advance(rowCopy, startRow - m_editRow); // get to startRow TODO:is this allowed?

	// add to lines
	for (int i = startRow; i < endRow; ++i, ++rowCopy)
	{
		lines.push_back(*rowCopy);
	}

	return endRow - startRow;
}

void StudentTextEditor::undo()
{
	// TODO
}
