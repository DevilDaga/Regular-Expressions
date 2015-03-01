#include "DFA.h"
#include <windows.h>
#include <ctime>
#include <cstdlib>

typedef pair<string, size_t> match;

class Parser : public DFA
{
public:
	Colour colMatch, colOther;

	Parser ( string );

	Parser ( string, Colour );

	Parser ( string, Colour, Colour );

private:
	vector<match> parse ( string );

public:
	bool verify ( string );

	void print_matches ( string );

	void highlight_matches ( string );

	string getRandom ( );

	string getRandom ( size_t );
};