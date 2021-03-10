#include "StudentSpellCheck.h"
#include <string>
#include <vector>
#include <cctype>
#include <iostream>
#include <fstream>

using namespace std;

SpellCheck *createSpellCheck()
{
	return new StudentSpellCheck;
}

StudentSpellCheck::StudentSpellCheck()
{
	m_root = new Node;
	m_root->value = 0;
}

StudentSpellCheck::~StudentSpellCheck()
{
	destroyTrie(m_root);
}

bool StudentSpellCheck::load(std::string dictionaryFile)
{
	//TODO: Upper case only 
	ifstream infile(dictionaryFile);
	// dict could not be processed
	if (!infile)
	{
		cout << "RIP FILE" << endl;
		return false;
	}
	string line;
	while (getline(infile, line))
	{
		// strip nonalpha non apostrophe chars
		string processedLine;
		for (int i = 0; i < line.size(); ++i)
		{
			if (!isalpha(line[i]) || line[i] != '\'')
			{
				processedLine += line[i];
			}
		}

		// add to trie
		insert(processedLine);
	}

	return true;
}

bool StudentSpellCheck::spellCheck(std::string word, int max_suggestions, std::vector<std::string> &suggestions)
{
	if (findWord(word))
	{
		return true;
	}

	suggestions.clear();
	return false;
}

void StudentSpellCheck::spellCheckLine(const std::string &line, std::vector<SpellCheck::Position> &problems)
{
	// TODO
}

void StudentSpellCheck::insert(std::string word)
{
	// loop through chars in word
	Node *p = m_root;
	for (int i = 0; i < word.size(); ++i)
	{
		bool matchFound = false;
		// check if current char in node's children
		for (auto it = p->children.begin(); it != p->children.end(); ++it)
		{
			// if matching child node found, proceed to that node
			if (word[i] == (*it)->value)
			{
				p = *it;
				matchFound = true;
				break;
			}
		}

		// if matching child node not found, add it
		if (!matchFound)
		{
			Node *newCharNode = new Node;
			newCharNode->value = word[i];
			p->children.push_back(newCharNode);
			p = newCharNode;
		}
	}

	// add marker to signify end of word
	Node *marker = new Node;
	marker->value = MARKER;
	p->children.push_back(marker);
}

void StudentSpellCheck::destroyTrie(Node *root)
{
	// should never happen but just a check
	if (root == nullptr)
	{
		return;
	}

	// destroy all children, then destroy root
	for (int i = 0; i < root->children.size(); ++i)
	{
		destroyTrie(root->children[i]);
	}

	delete root;
}

bool StudentSpellCheck::findWord(std::string word)
{
	Node *p = m_root;

	// loop through each char in word
	for (int i = 0; i < word.size(); ++i)
	{

		// loop through child nodes
		bool charFound = false;
		for (auto it = p->children.begin(); it != p->children.end(); ++it)
		{
			// check if char in a child node
			if ((*it)->value == word[i])
			{
				charFound = true;
				break;
			}
		}

		// if char not in a child node, then word not in trie
		if (!charFound)
		{
			return false;
		}
	}

	// ensure last char has a marker
	for (auto it = p->children.begin(); it != p->children.end(); ++it)
	{
		if ((*it)->value == MARKER)
		{
			return true;
		}
	}

	// if no marker, that means the word is not in trie
	return false;
}
