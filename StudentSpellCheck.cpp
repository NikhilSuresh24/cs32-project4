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
	m_root->value = 0; // dummy value provided that will never be checked
}

StudentSpellCheck::~StudentSpellCheck()
{
	destroyTrie(m_root);
}

bool StudentSpellCheck::load(std::string dictionaryFile)
{
	// O(N): looping trough line and adding to trie is constant time
	ifstream infile(dictionaryFile);

	// dict could not be processed
	if (!infile)
	{
		return false;
	}

	string line;
	while (getline(infile, line))
	{
		// strip nonalpha non apostrophe chars
		string processedLine;
		for (int i = 0; i < line.size(); ++i)
		{
			if (isalpha(line[i]) || line[i] == '\'')
			{
				processedLine += toupper(line[i]);
			}
		}

		// add to trie, if there's something to add
		if (!processedLine.empty())
		{
			insert(processedLine);
		}
	}

	// trie created, so return true
	return true;
}

bool StudentSpellCheck::spellCheck(std::string word, int max_suggestions, std::vector<std::string> &suggestions)
{
	// O(L^2 + oldS)
	// check if word already in dict
	if (findWord(word))
	{
		return true;
	}

	suggestions.clear();
	int numFound = 0;
	// check all possible 1-off word combinations
	// as long as we still want to find suggestions
	for (int ch = 0; ch < word.size() && numFound != max_suggestions; ++ch)
	{
		string wordCopy = word;
		for (int letter = 0; letter < ALPHABET.size(); ++letter)
		{
			// determine capitalization of char replacement
			if (isupper(wordCopy[ch]))
			{
				wordCopy[ch] = toupper(ALPHABET[letter]);
			}
			else
			{
				wordCopy[ch] = tolower(ALPHABET[letter]);
			}
			// if modified word in trie, add to suggestions
			if (findWord(wordCopy))
			{
				suggestions.push_back(wordCopy);
				++numFound;
			}
			// if we found enough suggestions, exit
			if (numFound == max_suggestions)
			{
				break;
			}
		}
	}

	// misspelled word, so ret false
	return false;
}

void StudentSpellCheck::spellCheckLine(const std::string &line, std::vector<SpellCheck::Position> &problems)
{
	problems.clear();

	// get vector of all word positions
	vector<SpellCheck::Position> wordPoses = splitLine(line);

	// check if each word is in trie
	for (auto it = wordPoses.begin(); it != wordPoses.end(); ++it)
	{
		int length = it->end - it->start + 1;
		std::string word = line.substr(it->start, length);
		// if word not in trie, add to problems
		if (!findWord(word))
		{
			problems.push_back(*it);
		}
	}
}

void StudentSpellCheck::insert(std::string word)
{
	// O(L), L = length of word
	Node *p = m_root;
	for (int i = 0; i < word.size(); ++i)
	{
		bool matchFound = false;
		char letter = toupper(word[i]);
		// check if current char in node's children
		for (auto it = p->children.begin(); it != p->children.end(); ++it)
		{
			// if matching child node found, proceed to that node
			if (letter == (*it)->value)
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
			newCharNode->value = letter;
			p->children.push_back(newCharNode);
			p = newCharNode;
		}
	}

	// check if word end has marker
	bool markerExists = false;
	for (auto it = p->children.begin(); it != p->children.end(); ++it)
	{
		if ((*it)->value == MARKER)
		{
			markerExists = true;
		}
	}

	// add marker to signify end of word if necessary
	if (!markerExists)
	{
		Node *marker = new Node;
		marker->value = MARKER;
		p->children.push_back(marker);
	}
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
	// O(L)
	Node *p = m_root;

	// loop through each char in word
	for (int i = 0; i < word.size(); ++i)
	{
		// loop through child nodes
		bool charFound = false;
		char letter = toupper(word[i]);
		for (auto it = p->children.begin(); it != p->children.end(); ++it)
		{
			// check if char in a child node
			if ((*it)->value == letter)
			{
				p = *it;
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

std::vector<SpellCheck::Position> StudentSpellCheck::splitLine(const std::string &line)
{
	vector<Position> words;
	int invalidPos = -1;
	int start = invalidPos;

	// loop over line
	for (int i = 0; i < line.size(); ++i)
	{
		bool inAlphabet = isalpha(line[i]) || line[i] == '\'';

		// if not in alpha or apostrophe, add to word vec
		if (!inAlphabet)
		{
			// only make word pos if valid alpha chars before
			if (start != invalidPos)
			{
				int end = i - 1;
				Position newPos;
				newPos.start = start;
				newPos.end = end;
				words.push_back(newPos);
				start = invalidPos;
			}
		}
		else
		{
			// if alpha/apostrophe char and no alpha chars since last word, set start
			if (start == invalidPos)
			{
				start = i;
			}
		}
	}

	// if last char is an alpha, then consider the last word
	if (start != invalidPos)
	{
		int end = line.size() - 1;
		Position newPos;
		newPos.start = start;
		newPos.end = end;
		words.push_back(newPos);
	}

	return words;
}
