#include "RPN.h"

int RPN::precedence ( uchar_t ch )
{
	switch ( ch )
	{
		case ATMOST_1:
		case POSITIVE:
		case KLEENE: return 4;
		case CONCAT: return 3;
		case ALTER: return 2;
		case PARANTHESIS_OPEN: return 1;
		default: return 0;
	}
}

bool isSymbol ( uchar_t ch )
{
	switch ( ch )
	{
		case ATMOST_1:
		case KLEENE:
		case POSITIVE:
		case CONCAT:
		case ALTER:
		case PARANTHESIS_OPEN:
		case PARANTHESIS_CLOSE:
			return false;
	}
	return true;
}

RPN::RPN ( std::string infix )
{
	std::stack<uchar_t> operators;
	std::string temp = "";
	if ( !infix.length ( ) )
	{
		postfix = "";
		return;
	}
	for ( size_t i = 0; i < infix.length ( ) - 1; ++i )
	{
		temp.append ( infix.substr ( i, 1 ) );
		uchar_t ch_1 = infix[ i + 1 ];
		if ( isSymbol ( ch_1 ) || ch_1 == PARANTHESIS_OPEN )
		{
			uchar_t ch = infix[ i ];
			if ( isSymbol ( ch ) )
				temp.push_back ( CONCAT );
			else if ( ch == KLEENE || ch == POSITIVE ||
					  ch == ATMOST_1 || ch == PARANTHESIS_CLOSE )
					  temp.push_back ( CONCAT );
		}
	}
	temp.append ( infix.substr ( infix.length ( ) - 1, 1 ) );
	this->infix = infix = temp;
	postfix = "";
	uchar_t ch, ch_1;
	int index = 0, k = 0, l = infix.length ( );
	while ( index != l )
	{
		ch = infix[ index++ ];
		ch_1 = infix[ index ];
		if ( index != l && ch != PARANTHESIS_CLOSE &&
			 (
			 ch_1 == KLEENE || ch == KLEENE ||
			 ch_1 == POSITIVE || ch == POSITIVE ||
			 ch_1 == ATMOST_1 || ch == ATMOST_1 )
			 )
			 postfix += ch;
		else if ( ch == PARANTHESIS_OPEN )
			operators.push ( ch );
		else if ( ch == PARANTHESIS_CLOSE )							// Pop everythning in brackets
		{
			while ( operators.top ( ) != PARANTHESIS_OPEN )
			{
				postfix += operators.top ( );
				operators.pop ( );
			}
			operators.pop ( );							// Get rid of the last PARANTHESIS_OPEN
		}
		else if ( isSymbol ( ch ) )					// Symbol.
			postfix += ch;
		else if ( !operators.empty ( ) && precedence ( operators.top ( ) ) >= precedence ( ch ) )
		{
			index--;									// To compare the same uchar_t with the next StackTop
			postfix += operators.top ( );
			operators.pop ( );
		}
		else
		{
			operators.push ( ch );
		}
	}
	while ( !operators.empty ( ) )
	{
		postfix += operators.top ( );
		operators.pop ( );
	}
}