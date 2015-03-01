#include "NFA.h"

#define		MINDIST(state)		( min_dist [ (state)->num ] )
#define		MAXDIST(state)		( max_dist [ (state)->num ] )
#define		LOOKUP(state)		lookup_table [ state->num ]

class DFAState;

typedef vector<DFAState*> DFAStates;

class DFAState
{
public:
	typedef pair<uchar_t, DFAState*> edge;
	string key;
	size_t num;
	NFAStates
		myNFAStates,
		myInitialNFAStates;
	vector<edge>
		incoming,
		outgoing;
	bool isStarting,
		isFinal;

	DFAState ( NFAState*, size_t );

	DFAState ( NFAStates, size_t );

	bool addOutgoing ( uchar_t, DFAState* );

	bool removeIncoming ( uchar_t, DFAState* );

	bool removeOutgoing ( uchar_t, DFAState* );

	void EpsillonClosure ( size_t );

	DFAState* nextState ( uchar_t );

	DFAState* prevState ( uchar_t );
};

class DFA : public NFA
{
protected:
	DFAStates states,
		finalStates;
	DFAState *startingState,
		***lookup_table;
	string state_table;
	size_t *min_dist,
		*max_dist;

public:
	size_t minLength, maxLength;

	DFA ( string );

private:

	void buildDFA ( );

	size_t GetMaxINFAS ( );

	void build_using_memory ( );

	void Minimize ( );

	void shorten_keys ( );

	void build_tables ( );

	void build_min_costs ( );

	void build_max_costs ( );

	void print_state_table ( );

	void print_graph ( );
};