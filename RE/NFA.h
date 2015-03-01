#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include "RPN.h"
#include "Helper.h"

using namespace std;

class NFAState;

typedef vector<NFAState*> NFAStates;

class NFAState
{
public:
	typedef pair<uchar_t, NFAState*> edge;
	size_t num;
	vector<edge>
		incoming,
		outgoing;
	NFAStates outgoingEpsilon;
	bool isFinal, isStarting;

	NFAState ( ) : num ( 0 ), isStarting ( false ), isFinal ( false ) { }

	NFAState ( int num, bool isStarting, bool isFinal );

	bool addIncoming ( uchar_t, NFAState* );

	bool addIncoming ( NFAState* );

	bool addIncoming ( uchar_t, NFAStates );

	bool addOutgoing ( uchar_t, NFAState* );

	bool addOutgoing ( NFAState* );

	bool addOutgoing ( uchar_t, NFAStates );

	NFAState *nextState ( uchar_t );

	string toString ( );
};

class NFA
{
public:
	string
		expression,
		postfix,
		symbols;
	NFAStates states;
	NFAState
		*startingState,
		*finalState;
	NFAState ***lookup_table;
	string state_table;

	NFA ( uchar_t, int );

	NFA ( string );

private:

	void adjustNum ( );

	void concat ( uchar_t, NFA* );

	void concat ( NFA* );

	void alter ( uchar_t, NFA* );

	void alter ( NFA* );

	void kleene ( );

	void positive ( );

	void atmost1 ( );

	void buildSymbols ( );

	void build_tables ( );

	void finalize ( );

	void print_state_table ( );

	void print_graph ( );
};