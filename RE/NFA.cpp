#include "NFA.h"

//#define DEBUGPRINT

NFAState::NFAState ( int num, bool isStarting, bool isFinal )
{
	this->num = num;
	this->isStarting = isStarting;
	this->isFinal = isFinal;
}

bool NFAState::addIncoming ( uchar_t symbol, NFAState* state )
{
	edge next = make_pair ( symbol, state );
	for each ( edge e in incoming )
		if ( e == next )
			return false;
	this->incoming.push_back ( next );
	state->outgoing.push_back ( make_pair ( symbol, this ) );
	return true;
}

bool NFAState::addIncoming ( NFAState* state )
{
	return addIncoming ( EPSILON, state );
}

bool NFAState::addIncoming ( uchar_t symbol, NFAStates states )
{
	bool ret = true;
	for each ( auto state in states )
		ret &= addIncoming ( symbol, state );
	return ret;
}

bool NFAState::addOutgoing ( uchar_t symbol, NFAState* state )
{
	edge next = make_pair ( symbol, state );
	for each ( edge e in outgoing )
		if ( e == next )
			return false;
	this->outgoing.push_back ( next );
	state->incoming.push_back ( make_pair ( symbol, this ) );
	return true;
}

bool NFAState::addOutgoing ( NFAState* state )
{
	return addOutgoing ( EPSILON, state );
}

bool NFAState::addOutgoing ( uchar_t symbol, NFAStates states )
{
	bool ret = true;
	for each ( auto state in states )
		if ( state->num == 0 )
			ret &= addOutgoing ( symbol, state );
	return ret;
}

NFAState *NFAState::nextState ( uchar_t symbol )
{
	for each ( auto it in outgoing )
		if ( it.first == symbol )
			return it.second;
	return nullptr;
}

string NFAState::toString ( )
{
	string ret = "";
	ret += to_string ( num ) + " ";
	ret += to_string ( incoming.size ( ) ) + " ";
	ret += to_string ( outgoing.size ( ) ) + " ";
	return ret;
}

NFA::NFA ( uchar_t symbol, int num = 1 )
{
	this->expression = symbol;
	NFAState
		*start = new NFAState ( 0, true, false ),
		*end = new NFAState ( num + 1, false, true );
	states.push_back ( start );
	states.push_back ( end );
	startingState = start;
	finalState = end;
	start->addOutgoing ( symbol, end );
	adjustNum ( );
	symbols = "";
}

NFA::NFA ( string infix )
{
	std::stack<NFA*> stack;
	RPN rpn ( infix );
	auto start = chrono::high_resolution_clock::now ( );
	postfix = rpn.postfix;
	for each ( uchar_t ch in postfix )
	{
		if ( isSymbol ( ch ) )
		{
			NFA *nfa = new NFA ( ch );
			stack.push ( nfa );
		}
		else
		{
			if ( ch == KLEENE || ch == POSITIVE || ch == ATMOST_1 )
			{
				NFA *top = stack.top ( );
				stack.pop ( );
				if ( ch == KLEENE )
					top->kleene ( );
				else if ( ch == POSITIVE )
					top->positive ( );
				else
					top->atmost1 ( );
				stack.push ( top );
				continue;
			}
			NFA *left, *right;
			right = stack.top ( );
			stack.pop ( );
			left = stack.top ( );
			stack.pop ( );
			if ( ch == ALTER )
				left->alter ( right );
			else
				left->concat ( right );
			stack.push ( left );
		}
	}
	stack.top ( )->postfix = postfix;
	new ( this ) NFA ( *stack.top ( ) );
	auto end = chrono::high_resolution_clock::now ( );
	std::cout << "NFA built in:\t"
		<< chrono::duration_cast<chrono::milliseconds>( end - start ).count ( )
		<< "ms\t\t" << "States:\t" << states.size ( ) << std::endl;
	buildSymbols ( );
	finalize ( );
	print_graph ( );
}

void NFA::adjustNum ( )
{
	for ( size_t i = 0; i < states.size ( ); ++i )
		states[ i ]->num = i;
}

void NFA::concat ( NFA* next )
{
	finalState->outgoing = next->startingState->outgoing;
	finalState->isFinal = false;
	next->startingState->isStarting = false;
	finalState = next->finalState;
	//expression = "(" + expression + ")";
	expression.append ( next->expression );
	//expression = "(" + expression + ")";
	next->states.erase ( next->states.begin ( ) );
	states = append ( states, next->states );
	adjustNum ( );
#ifdef DEBUGPRINT
	buildSymbols ( );
	cout << "Concatenation:\t" << expression << "\n";
	print_state_table ( );
#endif // DEBUGPRINT
}

void NFA::alter ( NFA* next )
{
	NFAState *start = new NFAState ( 0, true, false );
	start->addOutgoing ( next->startingState );
	start->addOutgoing ( startingState );
	NFAState *end = new NFAState ( 0, false, true );
	end->addIncoming ( next->finalState );
	end->addIncoming ( finalState );
	startingState->isStarting = false;
	next->startingState->isStarting = false;
	finalState->isFinal = false;
	next->finalState->isFinal = false;
	startingState = start;
	finalState = end;
	states = append ( states, next->states );
	states.insert ( states.begin ( ), startingState );
	states.push_back ( finalState );
	//expression = "(" + expression + ")";
	expression.append ( "|" );
	expression.append ( next->expression );
	expression = "(" + expression + ")";
	adjustNum ( );
#ifdef DEBUGPRINT
	buildSymbols ( );
	cout << "Alternation:\t" << expression << "\n";
	print_state_table ( );
#endif
}

void NFA::kleene ( )
{
	NFAState
		*start = new NFAState ( 0, true, false ),
		*end = new NFAState ( 0, false, true );
	finalState->isFinal = false;
	finalState->addOutgoing ( startingState );
	finalState->addOutgoing ( end );
	startingState->isStarting = false;
	startingState->addIncoming ( start );
	startingState = start;
	finalState = end;
	start->addOutgoing ( end );
	states.insert ( states.begin ( ), start );
	states.push_back ( end );
	if ( expression.length ( ) != 1 )
		expression = "(" + expression + ")";
	expression.append ( "*" );
	//expression = "(" + expression + ")";
	adjustNum ( );
#ifdef DEBUGPRINT
	buildSymbols ( );
	cout << "Kleen Closure:\t" << expression << "\n";
	print_state_table ( );
#endif // DEBUGPRINT
}

void NFA::positive ( )
{
	NFAState
		*start = new NFAState ( 0, true, false ),
		*end = new NFAState ( 0, false, true );
	finalState->isFinal = false;
	finalState->addOutgoing ( startingState );
	finalState->addOutgoing ( end );
	startingState->isStarting = false;
	startingState->addIncoming ( start );
	startingState = start;
	finalState = end;
	states.insert ( states.begin ( ), start );
	states.push_back ( end );
	if ( expression.length ( ) != 1 )
		expression = "(" + expression + ")";
	expression.append ( "+" );
	//expression = "(" + expression + ")";
	adjustNum ( );
#ifdef DEBUGPRINT
	buildSymbols ( );
	cout << "Positive Closure:\t" << expression << "\n";
	print_state_table ( );
#endif
}

void NFA::atmost1 ( )						// To do..
{
	startingState->addOutgoing ( finalState );
	if ( expression.length ( ) != 1 )
		expression = "(" + expression + ")";
	expression.append ( "?" );
	adjustNum ( );
}

void NFA::buildSymbols ( )
{
	vector<uchar_t> symbols;
	for each ( uchar_t ch in postfix )
		if ( isSymbol ( ch ) && !contains ( symbols, ch ) )
			symbols.push_back ( ch );
	sort ( symbols.begin ( ), symbols.end ( ) );
	this->symbols = "";
	for each ( auto symbol in symbols )
		this->symbols += symbol;
}

void NFA::build_tables ( )
{
	auto height = states.size ( );
	lookup_table = new NFAState**[ states.size ( ) ];
	for ( size_t i = 0; i != height; ++i )
	{
		auto state = states[ i ];
		lookup_table[ i ] = new NFAState*[ 256 ];
		memset ( lookup_table[ i ], 0, 256 * sizeof lookup_table[ 0 ] );
		for each ( auto e in state->outgoing )		// Filling the useless EPSILON transitions too...
			lookup_table[ i ][ e.first ] = e.second;
	}
	// State table starts.
	size_t WIDTH = 16;
	stringstream ss_state_table;
	ss_state_table << "State" + string ( WIDTH - 8, ' ' );
	symbols.push_back ( EPSILON );
	for each ( uchar_t symbol in symbols )
		ss_state_table << symbol << string ( WIDTH - 1, ' ' );
	ss_state_table << "\n";
	for each ( auto state in states )
	{
		string name = "";
		name += ( state->isStarting ) ? ">" : " ";
		name += ( state->isFinal ) ? "*" : " ";
		name.append ( getKey ( state->num ) );
		ss_state_table << name.append ( WIDTH - name.length ( ) - 5, ' ' );
		for each ( uchar_t symbol in symbols )
		{
			string out = "";
			for each ( auto edge in state->outgoing )
				if ( edge.first == symbol )
					out += getKey ( edge.second->num ) + ",";
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
	symbols.pop_back ( );
	ss_state_table << "\n\n\0";
	state_table = ss_state_table.str ( );
}

void NFA::finalize ( )
{
	build_tables ( );
	NFAStates temp;
	for each ( auto state in states )
	{
		state->outgoingEpsilon.clear ( );
		for each ( auto edge in state->outgoing )
			if ( edge.first == EPSILON )
				state->outgoingEpsilon.push_back ( edge.second );
	}
}

void NFA::print_graph ( )
{
	ofstream oFile;
	oFile.open ( "NFA.gv", ofstream::out | ofstream::trunc );
	oFile << "/*\nExpression:\t" << expression << "\n\n"
		<< state_table
		<< "*/\n\ndigraph NFA\n{\n"
		<< "\tnode [shape=\"circle\"];"
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
	for each ( auto state in states )
	{
		string allMyOutgoing = "{";
		for each ( auto e in state->outgoing )
		{
			oFile << "\ts" << state->num << "->"
				<< "s" << e.second->num << "[label=\""
				<< ( ( e.first == EPSILON ) ? "&#949;" : string ( 1, e.first ) )
				<< "\", penwidth="
				<< ( ( e.second->isFinal ) ? 4.0 : 1.0 )
				<< "];\n";
		}
	}
	oFile << "}";
	oFile.close ( );
}