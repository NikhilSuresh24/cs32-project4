#ifndef STUDENTTEXTEDITOR_H_
#define STUDENTTEXTEDITOR_H_

#include "TextEditor.h"
#include <list>
#include <string>

class Undo;

class StudentTextEditor : public TextEditor {
public:

	StudentTextEditor(Undo* undo);
	~StudentTextEditor();
	bool load(std::string file);
	bool save(std::string file);
	void reset();
	void move(Dir dir);
	void del();
	void backspace();
	void insert(char ch);
	void enter();
	void getPos(int& row, int& col) const;
	int getLines(int startRow, int numRows, std::vector<std::string>& lines) const;
	void undo();

private:
	std::list<std::string> m_lines;
	std::list<std::string>::iterator m_editRowIter; 
	int m_editRow;
	int m_editCol;

	void moveCursor(int row, int col);
	void undoableDel(bool isUndoable);
	void undoableBackspace(bool isUndoable);
	void undoableInsert(char ch, bool isUndoable);
	void undoableEnter(bool isUndoable);



};

#endif // STUDENTTEXTEDITOR_H_
