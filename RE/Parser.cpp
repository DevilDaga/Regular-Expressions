#include "Parser.h"

//awawawawawawawawaw

Parser::Parser ( string infix, Colour colMatch, Colour colOther )
	: DFA ( expand ( infix ) ), colMatch ( colMatch ), colOther ( colOther )
{ }

Parser::Parser ( string infix, Colour col )
	: DFA ( expand ( infix ) ), colMatch ( col ), colOther ( LIGHT_GRAY )
{ }

Parser::Parser ( string infix )
	: DFA ( expand ( infix ) ), colMatch ( LIGHT_GREEN ), colOther ( LIGHT_GRAY )
{ }

vector<match> Parser::parse ( string str )
{
	auto length = str.length ( );
	vector<match> result;
	result.reserve ( 0xFFFF );						// Could go as high as 2 ^ ( n - 1 ) + 1
	DFAState *active;
	const char *szStr = str.c_str ( );
	for ( size_t i = 0; i != length; ++i )
	{
		const char *ch = szStr + i;
		active = startingState;
		string valid = "";
		do
		{
			auto nextState = LOOKUP ( active )[ *ch ];
			if ( nextState )
			{
				active = nextState;
				valid.push_back ( *ch );
				if ( active->isFinal )
					result.push_back ( make_pair ( valid, i ) );
				++ch;
			}
			else
				break;
		} while ( true );
	}
	return result;
}

bool Parser::verify ( string str )
{
	auto length = str.length ( );
	if ( length < minLength || length > maxLength )
		return false;
	auto active = startingState;
	auto szStr = str.c_str ( );
	for ( size_t i = 0; i != length; ++i )
	{
		active = LOOKUP ( active )[ szStr[ i ] ];
		if ( !active )
			return false;
	}
	return active->isFinal;
}

void Parser::print_matches ( string toMatch )
{
	vector<match> matches = parse ( toMatch );
	auto count = matches.size ( );
	if ( !count )
		cout << "No matches found.\n";
	else
	{
		cout << "Following " << count << " matche" <<
			( ( count != 1 ) ? "s" : "" ) << " were found:\nPos\tString\n";
		for each ( auto it in matches )
			cout << it.second << "\t" << it.first << "\n";
	}
}

void Parser::highlight_matches ( string toMatch )
{
	vector<match> matches = parse ( toMatch ), temp;
	if ( matches.size ( ) )
	{
		for ( size_t i = 0; i < matches.size ( ) - 1; ++i )		// Get rid of matches that are subsets of bigger matches.
		{
			match biggest = matches[ i ];
			auto j = i + 1;
			while ( j != matches.size ( ) &&
					matches[ i ].second + matches[ i ].first.length ( ) ==
					matches[ j ].second + matches[ j ].first.length ( ) )
					++j;
			i = j - 1;
			temp.push_back ( biggest );
		}
		temp.push_back ( matches.back ( ) );
		matches = temp;
	}
	size_t cur = 0;
	for each ( auto it in matches )				// Got this after too much trial and error..  Prove it's correctness?
	{
		SETCOLOUR ( colOther );
		if ( cur < it.second )
		{
			cout << toMatch.substr ( cur, it.second - cur );
			cur = it.second;
		}
		else if ( cur - it.second < it.first.length ( ) )
		{
			SETCOLOUR ( colMatch );
			cout << it.first.substr ( cur - it.second );
			cur = it.second + it.first.length ( );
		}
		if ( it.second == cur )
		{
			SETCOLOUR ( colMatch );
			cur += it.first.length ( );
			cout << it.first;
			continue;
		}
	}
	SETCOLOUR ( colOther );
	cout << toMatch.substr ( cur ) << endl;
	SETCOLOUR ( LIGHT_GRAY );
}

string Parser::getRandom ( )				// Warning: might return a VERY large string
{
	return getRandom ( rand ( ) );
}

string Parser::getRandom ( size_t max )		// 'max' is more like an estimate..   THIS IS BAD! REDO!
//	EDIT: THIS is NP-Complete!
{
	if ( max < minLength )
		return "";
	else if ( max > maxLength )
		max = maxLength;
	auto next = startingState;
	string result = "";
	result.reserve ( max + 6 );
	auto temp = next->outgoing;
	std::random_shuffle ( temp.begin ( ), temp.end ( ) );
	bool giveUp = false;
	for ( size_t i = 0; i < max - MINDIST ( next ); ++i )
	{
		if ( ( max - i ) > MAXDIST ( next ) )
			break;
		if ( !temp.size ( ) )						// Finalize
		{
			if ( !result.length ( ) )		// Seems redundant now
			{
				giveUp = true;
				temp = next->outgoing;
				--i;
				break;
			}
			next = next->prevState ( result.back ( ) );
			result.pop_back ( );
			break;
		}
		auto e = temp.back ( );
		if ( MINDIST ( e.second ) >= max - i ||
			 MAXDIST ( e.second ) < max - i - 1 )
		{
			--i;
			temp.pop_back ( );
			continue;
		}
		result.push_back ( e.first );
		next = e.second;
		temp = next->outgoing;
		std::random_shuffle ( temp.begin ( ), temp.end ( ) );
	}
	//auto required = max - result.length ( );
	//auto backup = MINDIST ( next );
	for ( auto i = max - result.length ( ); i != ~0 && i < -1; --i )
	{
		temp = next->outgoing;
		std::random_shuffle ( temp.begin ( ), temp.end ( ) );
		for each ( auto e in temp )				// Randomize this too.	EDIT: Done
		{
			//if ( MINDIST ( e.second ) == i - 1 )
			if ( MINDIST ( e.second ) == MINDIST ( next ) - 1 ||
				 ( MAXDIST ( e.second ) == MAXDIST ( next ) - 1 && MAXDIST ( next ) == i ) )
			{
				result.push_back ( e.first );
				next = e.second;
				break;
			}
		}
	}
	//if ( result.length ( ) != max )
	//	__debugbreak ( );
	return result;
}