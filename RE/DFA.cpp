#include "DFA.h"

//#define PRINTFULLKEY
//#define _TEST_CASE

DFAState::DFAState ( NFAState *nfaState, size_t max )
{
	myNFAStates.push_back ( nfaState );
	myInitialNFAStates.push_back ( nfaState );
	isFinal = nfaState->isFinal;
	isStarting = nfaState->isStarting;
	EpsillonClosure ( max );
	string key = "";
	for each ( auto state in myNFAStates )
		key.append ( getKey ( state->num ) + "," );
	key.pop_back ( );
	this->key = key;
}

DFAState::DFAState ( NFAStates nfaStates, size_t max )
{
	myNFAStates = myInitialNFAStates = nfaStates;
	isStarting = false;
	isFinal = false;
	EpsillonClosure ( max );
#ifdef PRINTFULLKEY
	string key = "";
	for each ( auto state in myNFAStates )
		key.append ( getKey ( state->num ) + "," );
	if ( key.length ( ) )
		key.pop_back ( );
	this->key = key;
#endif // PRINTFULLKEY
}

bool DFAState::addOutgoing ( uchar_t symbol, DFAState *state )
{
	auto next = make_pair ( symbol, state );
	for each ( edge e in outgoing )
		if ( e.first == symbol )
			return false;
	this->outgoing.push_back ( next );
	state->incoming.push_back ( make_pair ( symbol, this ) );
	return true;
}

bool DFAState::removeIncoming ( uchar_t symbol, DFAState *state )
{
	auto compareWith = make_pair ( symbol, state );
	for ( size_t i = 0; i != incoming.size ( ); ++i )
	{
		if ( incoming[ i ] == compareWith )
		{
			incoming.erase ( incoming.begin ( ) + i );
			return true;
		}
	}
	return false;
}

bool DFAState::removeOutgoing ( uchar_t symbol, DFAState* state )
{
	auto compareWith = make_pair ( symbol, state );
	for ( size_t i = 0; i != outgoing.size ( ); ++i )
	{
		if ( outgoing[ i ] == compareWith )
		{
			outgoing.erase ( outgoing.begin ( ) + i );
			return true;
		}
	}
	return false;
}

void DFAState::EpsillonClosure ( size_t max )
{
	bool *visited = new bool[ max ];
	memset ( visited, 0, max * sizeof ( bool ) );
	for ( size_t i = 0; i != myNFAStates.size ( ); ++i )
	{
		auto state = myNFAStates[ i ];
		isFinal |= state->isFinal;
		for each ( auto it in state->outgoingEpsilon )
		{
			if ( !visited[ it->num ] )
			{
				myNFAStates.push_back ( it );
				visited[ it->num ] = true;
			}
		}
	}
	delete[ ] visited;
}

DFAState* DFAState::nextState ( uchar_t symbol )
{
	for each ( auto e in outgoing )
		if ( e.first == symbol )
			return e.second;
	return nullptr;
}

DFAState* DFAState::prevState ( uchar_t symbol )
{
	for each ( auto e in incoming )
		if ( e.first == symbol )
			return e.second;
	return nullptr;
}

DFA::DFA ( string infix ) : NFA ( infix )
{
	buildDFA ( );
	print_graph ( );
}

void DFA::buildDFA ( )						// Already minimized! >:]	EDIT: No..  It's not..
{
	auto start = chrono::high_resolution_clock::now ( );
	startingState = new DFAState ( __super::startingState,
								   __super::states.size ( ) );
	states.push_back ( startingState );
	build_using_memory ( );
	auto end = chrono::high_resolution_clock::now ( );
	std::cout << "DFA built in:\t"
		<< chrono::duration_cast<chrono::milliseconds>( end - start ).count ( )
		<< "ms\t\t" << "States:\t" << states.size ( ) << std::endl;
	Minimize ( );
	finalStates.clear ( );
	for each ( auto state in states )
		if ( state->isFinal )
			finalStates.push_back ( state );
	shorten_keys ( );
	build_tables ( );
}

void DFA::build_using_memory ( )
{
	vector<size_t*> initial_nfa_states_table;
	auto nfa_states_count = __super::states.size ( );
	auto starting_entry = new size_t[ nfa_states_count ];
	memset ( starting_entry, 0, nfa_states_count * sizeof ( bool ) );
	size_t max_infas = 0;
	for each ( auto nfaState in startingState->myInitialNFAStates )
		starting_entry[ max_infas++ ] = nfaState->num;
	max_infas = GetMaxINFAS ( ) + 1;
	initial_nfa_states_table.push_back ( starting_entry );
	auto visited = new bool[ nfa_states_count ];
	for ( size_t i = 0; i != states.size ( ); ++i )		// Subset construction.
	{
		auto state = states[ i ];
		for each ( uchar_t symbol in symbols )
		{
			NFAStates initialNFAStates;
			auto infas = new size_t[ max_infas ];
			memset ( infas, 0, max_infas * sizeof ( size_t ) );
			memset ( visited, 0, nfa_states_count * sizeof ( bool ) );
			size_t count = 0;
#ifndef _DEBUG
			for each ( auto nfaState in state->myNFAStates )				// Faster
#else
			auto size_myNFAStates = state->myNFAStates.size ( );
			for ( size_t j = 0; j != size_myNFAStates; ++j )				// Even faster?		Or not?		Faster only in _DEBUG?
#endif
			{
#ifdef _DEBUG
				auto temp = __super::lookup_table[ state->myNFAStates[ j ]->num ][ symbol ];
#else
				auto temp = __super::lookup_table[ nfaState->num ][ symbol ];
#endif
				if ( temp && !visited[ temp->num ] )
				{
					initialNFAStates.push_back ( temp );
					infas[ count++ ] = temp->num;
					visited[ temp->num ] = true;
				}
			}
			if ( !count )
				continue;
			DFAState *tempDFAState = nullptr;
			bool flag = false;
			auto size = states.size ( );
			for ( size_t j = 0; j != size; ++j )
			{
				if ( !memcmp ( initial_nfa_states_table[ j ], infas, max_infas * sizeof size_t ) )
				{
					tempDFAState = states[ j ];									// Re-use.
					flag = true;
					break;
				}
			}
			if ( !flag )
			{
				tempDFAState = new DFAState ( initialNFAStates, __super::states.size ( ) );
				states.push_back ( tempDFAState );
				initial_nfa_states_table.push_back ( infas );
			}
			state->addOutgoing ( symbol, tempDFAState );
		}
	}
	for each ( auto it in initial_nfa_states_table )			// Goes up to 100s of MBs sometimes..
		delete[ ] it;
}

void DFA::Minimize ( )							// 21.1% time			:)))))		EDIT: 4.8%  >:D
{
	auto start = chrono::high_resolution_clock::now ( );
	auto height = states.size ( );
	lookup_table = new DFAState**[ states.size ( ) ];
	for ( size_t i = 0; i != height; ++i )
	{
		auto state = states[ i ];
		lookup_table[ i ] = new DFAState*[ 256 ];
		memset ( lookup_table[ i ], 0, 256 * sizeof lookup_table[ 0 ] );
		for each ( auto e in state->outgoing )
			lookup_table[ i ][ e.first ] = e.second;
	}
	size_t
		lt_start = symbols.front ( ),
		lt_range = symbols.back ( ) - lt_start + 1;
	shorten_keys ( );
	size_t deleted = 0;
	for ( size_t i = 0; i != states.size ( ); ++i )		// O(n^2)   could be O(nlog n) ? Could be O(N) with hashing ? Nah. All good.
	{
		auto state = states[ i ];
		for ( size_t j = 0; j != i; j++ )
		{
			size_t last_changed = ~0;
			auto compareWith = states[ j ];
			if ( compareWith->isFinal == state->isFinal &&
				 !memcmp (
				 LOOKUP ( compareWith ) + lt_start,
				 LOOKUP ( state ) + lt_start,
				 lt_range * sizeof ( void* )
				 )
				 )			// 71.7% time.. NO MORE!		1.9% time..  Better to read from raw memory!
			{
				//while ( false );					// For debugging
				for each ( auto e in state->incoming )
				{
					e.second->removeOutgoing ( e.first, state );			// Might be wrong..... or not..
					e.second->addOutgoing ( e.first, compareWith );
					last_changed = min ( last_changed, e.second->num - deleted );	// First 'probably unsafe' node..
					LOOKUP ( e.second )[ e.first ] = compareWith;
				}
				for each ( auto e in state->outgoing )
				{
					e.second->removeIncoming ( e.first, state );			// This is being a bitch.
					compareWith->addOutgoing ( e.first, e.second );
					last_changed = min ( last_changed, e.second->num - deleted );
					LOOKUP ( compareWith )[ e.first ] = e.second;
				}
				states.erase ( states.begin ( ) + i );
				i = min ( last_changed, i - 1 );
				++deleted;
				break;
			}
		}
	}
	auto end = chrono::high_resolution_clock::now ( );
	std::cout << "Minimized in:\t"
		<< chrono::duration_cast<chrono::milliseconds> ( end - start ).count ( )
		<< "ms\t\t" << "States:\t" << states.size ( ) << std::endl;
}

size_t DFA::GetMaxINFAS ( )
{
	size_t max = 0;
	for each ( uchar_t symbol in symbols )
	{
		size_t counter = 0;
		for each ( auto state in __super::states )
			if ( __super::lookup_table[ state->num ][ symbol ] )
				++counter;
		max = max ( counter, max );
	}
	return max;
}

void DFA::shorten_keys ( )
{
	int nextKey = 0;
	for each ( auto state in states )
	{
		state->num = nextKey;
		state->key = ( getKey ( nextKey++ ) );
	}
}

void DFA::build_tables ( )
{
	// Lookup table starts.
	auto height = states.size ( );
	lookup_table = new DFAState**[ states.size ( ) ];
	for ( size_t i = 0; i != height; ++i )
	{
		auto state = states[ i ];
		lookup_table[ i ] = new DFAState*[ 256 ];
		memset ( lookup_table[ i ], 0, 256 * sizeof lookup_table[ 0 ] );
		for each ( auto e in state->outgoing )
			lookup_table[ i ][ e.first ] = e.second;
	}

	// State table starts.
	size_t WIDTH = 0;
	for each ( auto state in states )
		if ( state->key.length ( ) > WIDTH )
			WIDTH = state->key.length ( );
	stringstream ss_state_table;
	WIDTH += 6;
	ss_state_table << "State" + string ( WIDTH - 3, ' ' );
	for each ( auto symbol in symbols )
		ss_state_table << symbol << string ( WIDTH - 1, ' ' );
	ss_state_table << "\n";
	for each ( auto state in states )
	{
		string padd ( WIDTH - 2 - state->key.length ( ), ' ' );
		ss_state_table << ( ( state->isStarting ) ? '>' : ' ' )
			<< ( ( state->isFinal ) ? '*' : ' ' )
			<< state->key
			<< padd;
		for each ( auto symbol in symbols )
		{
			string out = "";
			for each ( auto edge in state->outgoing )
				if ( edge.first == symbol )
					out += edge.second->key + ",";
			if ( out.length ( ) )
			{
				out.pop_back ( );
				out = "{" + out + "}";
			}
			if ( out.length ( ) <= WIDTH )
				out.append ( WIDTH - out.length ( ), ' ' );
			ss_state_table << out;
		}
		ss_state_table << "\n";
	}
	ss_state_table << "\n\n\0";
	state_table = ss_state_table.str ( );
	build_min_costs ( );
}

void DFA::build_min_costs ( )
{
	auto visited = new size_t[ states.size ( ) ];					// For a greedy search
	min_dist = new size_t[ states.size ( ) ];
	memset ( visited, 0, states.size ( ) * sizeof ( int ) );
	DFAStates temp, temp2;
	for each ( auto fstate in finalStates )
	{
		visited[ fstate->num ] = -1;
		temp.push_back ( fstate );
	}
	int inc = 0;
	do
	{
		temp2.clear ( );
		++inc;
		for each ( auto state in temp )
		{
			for each ( auto e in state->incoming )
			{
				if ( !visited[ e.second->num ] )
				{
					visited[ e.second->num ] = inc;
					temp2.push_back ( e.second );
				}
			}
		}
		temp = temp2;
	} while ( temp2.size ( ) );
	for each ( auto fstate in finalStates )
		visited[ fstate->num ] = 0;
	memcpy ( min_dist, visited, states.size ( ) * sizeof ( int ) );
	minLength = MINDIST ( startingState );
	maxLength = inc - 1;
	build_max_costs ( );
	delete[ ] visited;
}

void DFA::build_max_costs ( )						// Kinda proud of this one..
{
	size_t size = states.size ( );
	max_dist = new size_t[ size ];
	memset ( max_dist, 0, size * sizeof ( int ) );
	//memcpy ( max_dist, min_dist, states.size ( ) * sizeof ( int ) );
	auto visited = new bool[ size ];
	memset ( visited, 0, size * sizeof ( bool ) );
	DFAStates safe;
	vector<pair<DFAState::edge, DFAState*>> backup;
	for each ( auto state in states )				// Can probably make it finalStates..
		if ( !state->outgoing.size ( ) )
			safe.push_back ( state );
	for ( size_t i = 0; i != safe.size ( ); ++i )	// Like Topo sort
	{
		auto cur = safe[ i ];
		visited[ cur->num ] = true;					// 'cur' doesn't have access to a cycle
		for each ( auto e in cur->incoming )
		{
			e.second->removeOutgoing ( e.first, cur );
			backup.push_back ( make_pair ( e, cur ) );
			if ( !e.second->outgoing.size ( ) )		// 0 out-degree
				safe.push_back ( e.second );
		}
	}
	for each ( auto e in backup )					// Restore deleted edges.
		e.first.second->outgoing.push_back ( make_pair ( e.first.first, e.second ) );
	for ( size_t i = 0; i != size; ++i )
		if ( !visited[ i ] )						// Remainder graph was only of cyclic members
			max_dist[ i ] = -1;						// They'll have MAXDIST = #INF

	// Need a DFS here for max_costs..   Start from final states? EDIT: Yes

	memset ( visited, 0, size * sizeof ( bool ) );
	for each ( auto fstate in finalStates )			// DFS from ALL final states
	{
		vector<pair<DFAState*, size_t>> stack;
		stack.push_back ( make_pair ( fstate, 0 ) );
		while ( stack.size ( ) )
		{
			auto cur = stack.back ( );
			visited[ cur.first->num ] = true;
			stack.pop_back ( );
			for each ( auto e in cur.first->incoming )
			{
				if ( MAXDIST ( e.second ) <= cur.second )	// Won't update ones with MAXDIST = #INF
				{
					MAXDIST ( e.second ) = cur.second + 1;
					stack.push_back ( make_pair ( e.second, cur.second + 1 ) );
				}
			}
		}
	}
	maxLength = max_dist[ 0 ];
	delete[ ] visited;
}

void DFA::print_graph ( )
{
	ofstream oFile;
	oFile.open ( "DFA.gv", ofstream::out | ofstream::trunc );
	oFile << "/*\nExpression:\t" << expression << "\n\n"
		<< state_table
		<< "*/\n\ndigraph DFA\n{\n"
		<< "\tnode [shape=\"circle\"];\n"
		<< "\tedge [fontsize=20];\n";
	for each ( auto state in states )
	{
		oFile << "\ts" << state->num;
		if ( state->isFinal )
		{
			oFile << "[shape=\"doublecircle\", color=Red";
			if ( state->isStarting )
				oFile << ", style=filled, fillcolor=yellow";
			oFile << "]";
		}
		if ( state->isStarting )
			oFile << "[style=filled, fillcolor=yellow]";
		oFile << ";\n";
	}
	auto size = states.size ( );
	bool *visited = new bool[ size ];
	for each ( auto state in states )
	{
		string allMyOutgoing = "{";
		string *edge_labes = new string[ size ];
		memset ( visited, 0, size * sizeof ( bool ) );
		for each ( auto e in state->outgoing )
		{
			edge_labes[ e.second->num ].push_back ( e.first );
			edge_labes[ e.second->num ].push_back ( ',' );
		}
		for each ( auto e in state->outgoing )
		{
			if ( visited[ e.second->num ] )
				continue;
			edge_labes[ e.second->num ].pop_back ( );
			shrink ( edge_labes[ e.second->num ] );
			oFile << "\ts" << state->num << "->"
				<< "s" << e.second->num << "[label=\""
				<< edge_labes[ e.second->num ] << "\", penwidth="
				<< ( ( e.second->isFinal ) ? 4.0 : 1.0 )
				<< "];\n";
			visited[ e.second->num ] = true;
		}
	}
	oFile << "}";
	oFile.close ( );
	delete[ ] visited;
}