#define _CRT_SECURE_NO_WARNINGS
#include "Parser.h"
#include <stdio.h>
#include <random>
#include <chrono>
#include <atomic>

// http://hackingoff.com/compilers/regular-expression-to-nfa-dfa
// To verify simple DFAs

#define	CLEARSTREAM			do { cin.clear ( ); fflush ( stdin ); } while ( false )

int main ( )
{
	string infix, toMatch, random;
	Parser *parser;
	int choice = -1;
	srand ( (unsigned) time ( 0 ) );
	while ( true )
	{
		cout << "[a-z]\t:\t" << "Character range\t\t\teg. [A-Z]"
			<< "\n[xyz]\t:\t" << "Character set\t\t\teg. [abcdxyz]"
			<< "\n\\\t:\t" << "Special character\t\teg. \\+ \\? \\* \\-"
			<< "\n{n}\t:\t" << "Exactly n times\t\t\teg. [abc]{5}"
			<< "\n{n, m}\t:\t" << "At least \'n\' and at most \'m\' times"
			<< "\n?\t:\t" << "0 or 1 times. Same as {0, 1}\teg. (abc)?"
			<< endl;
		cout << "Enter the RE:\t";
		CLEARSTREAM;
		getline ( cin, infix );
		cout << "Building the NFA and DFA..." << endl;
		auto start = chrono::high_resolution_clock::now ( );
		parser = new Parser ( infix );
		auto end = chrono::high_resolution_clock::now ( );
		cout << "Everything built in:\t"
			<< chrono::duration_cast<chrono::milliseconds>( end - start ).count ( )
			<< "ms" << endl;

		cout << "Done!\nState tables and graphs saved to *.gv files!" << endl;
		choice = -1;
		while ( choice != 5 )
		{
			cout << "1:\tVerify a string.\n"
				<< "2:\tGenerate random valid string.\n"
				<< "3:\tPrint all matches in a given string.\n"
				<< "4:\tHighlight all matches in a given string.\n"
				<< "5:\tChange the RE.\n"
				<< "6:\tGenerate \'N\' random valid strings.\n"
#ifdef _DEBUG
				<< "7:\tTest Case.\n"
#endif // _DEBUG
				<< "0:\tEXIT\n"
				<< "Choice:\t";
			CLEARSTREAM;
			cin >> choice;
			if ( !choice )
				return 0;
			switch ( choice )
			{
				case 1:
					cout << "Input the string:" << endl;
					CLEARSTREAM;
					toMatch = "";
					getline ( cin, toMatch );
					if ( parser->verify ( toMatch ) )
						cout << "Valid." << endl;
					else
						cout << "Invalid." << endl;
					break;
				case 2:
					cout << "Enter the length:\t";
					size_t estimate;
					cin >> estimate;
					if ( estimate < parser->minLength )
					{
						cout << "The minimum length is:\t"
							<< parser->minLength << endl;
						break;
					}
					if ( estimate > parser->maxLength )
					{
						cout << "The maximum length is:\t"
							<< parser->maxLength << endl;
						break;
					}
					random = parser->getRandom ( estimate );
					if ( estimate != random.length ( ) )
						cout << "Rounded off to " << random.length ( )
						<< " characters." << endl;
					cout << random << endl;
					break;
				case 3:
					cout << "Input the string:" << endl;
					CLEARSTREAM;
					toMatch = "";
					getline ( cin, toMatch );
					parser->print_matches ( toMatch );
					CLEARSTREAM;
					break;
				case 4:
					cout << "Choose the colour for highlighting:\t";
					parser->colMatch = pickColour ( );
					cout << "Choose the colour for other symbols:\t";
					parser->colOther = pickColour ( );
					cout << "Input the string:" << endl;
					CLEARSTREAM;
					toMatch = "";
					getline ( cin, toMatch );
					parser->highlight_matches ( toMatch );
					CLEARSTREAM;
					break;
				case 5:
					break;
				case 6:
					size_t count;
					cout << "Enter \'N\':\t";
					CLEARSTREAM;
					cin >> count;
					size_t rand_size;
					while ( count-- )
					{
						auto maxLength = min ( parser->maxLength, 100U );
						rand_size = rand ( ) % ( maxLength - parser->minLength + 1 );
						rand_size += parser->minLength;
						cout << parser->getRandom ( rand_size ) << endl;
					}
					break;
#ifdef _DEBUG
				case 7:
					break;
#endif // _DEBUG
				default:
					cout << "INVALID INPUT.\n"
						<< "Try again." << endl;
			}
		}
	}
	return 0;
}