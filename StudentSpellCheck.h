#ifndef STUDENTSPELLCHECK_H_
#define STUDENTSPELLCHECK_H_

#include "SpellCheck.h"

#include <string>
#include <vector>

class StudentSpellCheck : public SpellCheck
{
public:
	StudentSpellCheck();
	virtual ~StudentSpellCheck();
	bool load(std::string dict_file);
	bool spellCheck(std::string word, int maxSuggestions, std::vector<std::string> &suggestions);
	void spellCheckLine(const std::string &line, std::vector<Position> &problems);

private:
	struct Node
	{
		std::vector<Node *> children;
		char value;
	};

	Node *m_root;

	static const char MARKER = '.';
	inline static const std::string ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ'"; //TODO: can i do this?

	void insert(std::string word);
	void destroyTrie(Node *root);
	bool findWord(std::string word);
	std::vector<Position> splitLine(const std::string &line);
};

#endif // STUDENTSPELLCHECK_H_
