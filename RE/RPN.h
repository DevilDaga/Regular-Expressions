#include <stack>
#include <string>

typedef unsigned char uchar_t;

#define ASCII_EPSILON					"\xEE"
#define ASCII_CONCAT					"\xFF"
#define ASCII_ALTER						"\xFE"
#define ASCII_DOT						"\xFD"
#define ASCII_KLEENE					"\xFC"
#define ASCII_POSITIVE					"\xFB"
#define ASCII_ALL_ALPHA_NUM				"\xFA"
#define ASCII_ALL_DIGITS				"\xF9"
#define ASCII_PARANTHESIS_OPEN			"\xF8"
#define ASCII_PARANTHESIS_CLOSE			"\xF7"
#define ASCII_ALL_ALPHA_LOWER			"\xF6"
#define ASCII_ALL_ALPHA_UPPER			"\xF5"
#define ASCII_ATMOST_1					"\xF4"

const unsigned char
EPSILON = (uchar_t) 238,					// 0xEE
CONCAT = (uchar_t) 255,						// 0xFF
ALTER = (uchar_t) 254,						// 0xFE
DOT = (uchar_t) 253,						// 0xFD
KLEENE = (uchar_t) 252,						// 0xFC
POSITIVE = (uchar_t) 251,					// 0xFB
ALL_ALPHA_NUM = (uchar_t) 250,				// 0xFA
ALL_DIGITS = (uchar_t) 249,					// 0xF9
PARANTHESIS_OPEN = (uchar_t) 248,			// 0xF8
PARANTHESIS_CLOSE = (uchar_t) 247,			// 0xF7
ALL_ALPHA_LOWER = (uchar_t) 246,			// 0xF6
ALL_ALPHA_UPPER = (uchar_t) 245,			// 0xF5
ATMOST_1 = (uchar_t) 244;					// 0xF4

bool isSymbol ( uchar_t );

class RPN
{
private:
	int precedence ( uchar_t );
public:
	std::string infix, postfix;

	RPN ( std::string );
};