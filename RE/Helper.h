#include <vector>
#include <windows.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <atomic>
#include <chrono>

#define SETCOLOUR(c)		SetConsoleTextAttribute( GetStdHandle ( STD_OUTPUT_HANDLE ), (c) )
#define NULLIFY(dest,size)	memset((dest), 0, (size) * sizeof (dest)[ 0 ] )

typedef enum Colour
{
	BLACK,
	DARK_BLUE,
	DARK_GREEN,
	DARK_CYAN,
	DARK_RED,
	DARK_MAGENTA,
	DARK_YELLOW,
	LIGHT_GRAY,
	DARK_GRAY,
	LIGHT_BLUE,
	LIGHT_GREEN,
	LIGHT_CYAN,
	LIGHT_RED,
	LIGHT_MAGENTA,
	LIGHT_YELLOW,
	WHITE
}Colour;

using namespace std;

template<typename T>
static vector<T> append ( vector<T> original, vector<T> toAppend )
{
	original.insert (
		original.end ( ),
		toAppend.begin ( ),
		toAppend.end ( )
		);
	return original;
}

template<typename T>
static bool contains ( vector<T> vec, T x )
{
	for each ( auto it in vec )
		if ( it == x )
			return true;
	return false;
}

static string inline getKey ( int num )
{
	return ( "s" + to_string ( num ) );
}

static string
_aA_zZ = "(q|w|e|r|t|y|u|i|o|p|a|s|d|f|g|h|j|k|l|z|x|c|v|b|n|m|Q|W|E|R|T|Y|U|I|O|P|A|S|D|F|G|H|J|K|L|Z|X|C|V|B|N|M)",
_0_9 = "(0|1|2|3|4|5|6|7|8|9)";

static void replaceAll ( string& str, const string& from, const string& to )
{
	if ( from.empty ( ) )
		return;
	size_t start_pos = 0;
	while ( ( start_pos = str.find ( from, start_pos ) ) != string::npos )
	{
		str.replace ( start_pos, from.length ( ), to );
		start_pos += to.length ( ); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

static string shrink ( string& str )
{
	if ( str.find_first_of ( ',' ) == string::npos )
		return str;
	vector<uchar_t> symbols;
	for each ( uchar_t symbol in str )
		if ( symbol != ',' )
			symbols.push_back ( symbol );
	sort ( symbols.begin ( ), symbols.end ( ) );
	str = "";
	auto size = symbols.size ( );
	for ( size_t i = 0; i <= size - 2; ++i )
	{
		auto j = i;
		while ( symbols[ j ] == symbols[ j + 1 ] - 1 && j != size - 2 )
			++j;
		if ( symbols[ j ] == symbols.back ( ) - 1 )
			++j;
		if ( i == j )
		{
			str.push_back ( symbols[ i ] );
			str.push_back ( ',' );
			continue;
		}
		str.push_back ( '[' );
		str.push_back ( symbols[ i ] );
		str.push_back ( '-' );
		str.push_back ( symbols[ j ] );
		str.push_back ( ']' );
		str.push_back ( ',' );
		i = j;
	}
	if ( symbols[ size - 2 ] != symbols.back ( ) - 1 )
		str.push_back ( symbols.back ( ) );
	else
		str.pop_back ( );
	return str;
}

static string expand ( string regex )
{
	while ( true )
	{
		auto start = regex.find_first_of ( '[' );
		if ( start == string::npos )
			break;
		auto end = regex.find_first_of ( ']', start );
		string sub = regex.substr ( start, end - start + 1 );
		if ( sub[ 1 ] != '\\' && sub[ 2 ] == '-' )				// Character range.
		{
			string replacement = "(";
			uchar_t
				from = sub[ 1 ],
				to = sub[ 3 ];
			if ( from > to )
			{
				from = to;
				to = sub[ 1 ];
			}
			for ( ; from != to + 1; ++from )
			{
				replacement.push_back ( from );
				replacement.push_back ( '|' );
			}
			replacement.pop_back ( );
			replacement.push_back ( ')' );
			replaceAll ( regex, sub, replacement );
		}
		else													// Character set.
		{
			string replacement = "(";
			auto size = sub.length ( ) - 1;
			for ( size_t i = 1; i < size; ++i )
			{
				if ( sub[ i ] == '\\' )
					replacement.append ( sub.substr ( i++, 2 ) );
				else
					replacement.push_back ( sub[ i ] );
				replacement.push_back ( '|' );
			}
			replacement.pop_back ( );
			replacement.push_back ( ')' );
			replaceAll ( regex, sub, replacement );
		}
	}
	while ( true )
	{
		auto start = regex.find_first_of ( '{' );
		if ( start == string::npos )
			break;
		auto end = regex.find_first_of ( '}', start );
		string sub = regex.substr ( start, end - start + 1 );
		size_t min, max, comma = sub.find ( ',' );
		if ( comma == string::npos )
		{
			min = stoi ( sub.substr ( 1, sub.length ( ) - 2 ) );
			max = 0;
		}
		else
		{
			min = stoi ( sub.substr ( 1, comma - 1 ) );
			max = stoi ( sub.substr ( comma + 1, sub.length ( ) - comma - 1 ) );
		}
		auto mid = start - 1;
		string replacement = "";
		if ( regex[ mid ] != ')' )
		{
			replacement = string ( min, regex[ mid ] );
			start = mid - 1;
			if ( max > min )
			{
				max -= min;
				string temp = "";
				temp.push_back ( regex[ mid ] );
				temp.push_back ( '?' );
				while ( max-- )
					replacement.append ( temp );
			}
		}
		else
		{
			auto szRegex = regex.c_str ( );
			int depth = 1;
			for ( start = mid - 1; depth; --start )
				if ( szRegex[ start ] == ')' )
					++depth;
				else if ( szRegex[ start ] == '(' )
					--depth;
			auto temp = regex.substr ( start + 1, mid - start );
			auto backup = min;
			while ( backup-- )
				replacement.append ( temp );
			if ( max > min )
			{
				max -= min;
				temp.push_back ( '?' );
				while ( max-- )
					replacement.append ( temp );
			}
		}
		replacement = "(" + replacement + ")";
		replaceAll ( regex, regex.substr ( start + 1, end - start ), replacement );
	}
	replaceAll ( regex, "\\.", ASCII_DOT );
	replaceAll ( regex, ".", "(" + _aA_zZ + "|" + _0_9 + ")" );
	replaceAll ( regex, "|", ASCII_ALTER );
	replaceAll ( regex, "*", ASCII_KLEENE );
	replaceAll ( regex, "+", ASCII_POSITIVE );
	replaceAll ( regex, "?", ASCII_ATMOST_1 );
	replaceAll ( regex, "\\"ASCII_ALTER, "|" );
	replaceAll ( regex, "\\"ASCII_KLEENE, "*" );
	replaceAll ( regex, "\\"ASCII_POSITIVE, "+" );
	replaceAll ( regex, "\\"ASCII_ATMOST_1, "?" );
	replaceAll ( regex, "\\-", "-" );
	replaceAll ( regex, "\\n", "\n" );
	replaceAll ( regex, "(", ASCII_PARANTHESIS_OPEN );
	replaceAll ( regex, ")", ASCII_PARANTHESIS_CLOSE );
	replaceAll ( regex, "\\"ASCII_PARANTHESIS_OPEN, "(" );
	replaceAll ( regex, "\\"ASCII_PARANTHESIS_CLOSE, ")" );
	replaceAll ( regex, ASCII_DOT, "." );
	return regex;
}

static Colour pickColour ( )
{
	int c;
	for ( unsigned i = 0; i < 16; ++i )
	{
		SETCOLOUR ( i );
		cout << i << " ";
	}
	SETCOLOUR ( LIGHT_GRAY );
	cout << "\nChoice:\t";
	cin >> c;
	return (Colour) ( c % 16 );
}

static bool file_exists ( const std::string& name )
{
	ifstream f ( name.c_str ( ) );
	bool result = f.good ( );
	f.close ( );
	return result;
}