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

//int main ( )
//{
//	cout << "[a-z]\t" << _a_z
//		<< "\n[A-Z]\t" << _A_Z
//		<< "\n[aA-zZ]\t" << _aA_zZ
//		<< "\n[0-9]\t" << _0_9 << endl;
//	string infix = "a+(b*|c)|d*aacd";
//	//cout << "Enter the RE:\t";
//	//cin >> infix;
//	string regex = " a | an | the |I |(";
//	regex += _aA_zZ;
//	regex += "+(ing|ed|th|nd|ly) )| " + _aA_zZ + "*as";
//	regex = " a | an | the |I |[aA-zZ]+(ing|ed|ant|th|nd|ly|en|st|re|er) | [aA-zZ]*(as|on) ";
//	//regex = "[a-z]*ing ";
//	auto parser = new Parser ( regex, Colour::DARK_CYAN, Colour::BLACK );
//	parser->print_graph ( );
//	cout << parser->getRandom ( 10 ) << endl;
//	string toMatch = ( "I was walking to my car when I noticed my pants were on fire. I turned to my friend and said,  \"I think were going to need an oven and fast. \"  He grabbed me with his soggy fingers and said..  \"Snap out of it, Man. \"  Then he proceeded to chant and walk in circles around me.  I looked down, My pants were still on fire, but by this time it had spread to my Uterus and three other ligaments. I was getting pretty agitated that no one seemed to care about my third degree burns, or my soon imminent death, but I went along with his plan still hoping he'd manage to save me through his voo-doo witchcraft.  After I came out of my trance from the fire I noticed that I was now in the kitchen of my grandma's house baking a cake..  I Screamed  \"WHAT THE F***? \"  My Grandma ran in and slapped the s*** outta me for cursing.  Then she proceeded to say,  \"I swear if you talk like that 5 more times, I'll beat the living s*** outta you. \"  I was quite confused as to what was going on and what I did to deserve this true life mad lib. I just stood there in silence with a look of uttermost confusion.. and I starred at the women who had once been a kind sweet old lady, who wouldn't even kill a baby piglet, now turned into this vicious beast of a monster with veins protruding from her neck like a porcupines quills on a midsummer day.  She gasped, and then with the blink of an eye turned back into the sweet old lady I had once known.   \"Would you like some cookies, dear? \"   \"Ummm Yes, grandma. \"  As she went to retrieve the cookies, I was planning my escape.. Looking around pondering what unimaginable thing would happen next if I stayed. As my grandma neared the corner with her plate of cookies, I ran to the door as fast as my burning legs would take me.  As I made my silent escape I heard grandma yell,  \"You forgot your cookies dear \"  Little did she know I didn't give a s*** about those cookies. As I ran I thought about my previous life and how this whole day had been more interesting than my Whole entire life on earth had.. I began to ponder if this was karma kicking my a** for just sitting on the laptop all day typing short stories for little to no pay. When I decided to stop and catch my breath, I wiped the sweat from my face and looked up to see what else this new world had to offer.  Far off in the distance I could see a huge building, maybe a hotel or some sort of jail. I wasn't too sure.. but I marked that location off my list, the last thing I needed was to go to a jail and get killed by a bunch of mobsters.. I mean Hell my pants were already on Fire.  I decided to turn to my left and see what my next choice would be.  There were flying cantaloupes, rainbows and songs of happiness near by, I mean I was a little frightened by the flying fruit but I'll take this any day over Prison inmates.  I skipped closer and closer to the festivities and when I arrived I seen all my friends I had went to high school with there were holding hands and singing Kumbayah around the camp ice.. Yes It was a giant block of ice situated on three wood logs.. I felt much more comforted here than I did at my grandmas. I took a deep breath of relief and I thought Maybe, this day is getting better. I joined hands and with Germany and Tokyo and began to sing with everyone else, but as soon as I Belched out my voice changed to an annoying high pitched squeal.. Similar to ringing in your ears.  Everyone turned toward me and gave me the death stare and I knew I had screwed up once again, they all walked in slow motion towards me saying the same familiar chant I had heard earlier, before anyone could reach me I awoke in a frantic sweaty rush in my bed.. My legs were no longer on fire and I felt slightly normal again. I noticed that my mom, a preacher, and several other family members were standing around me sobbing and chanting.. I said.  \"What's going on? \" They informed me that I had been possessed by a spirit named Robert that liked to make people crazy by making there dreams seem similar to real life, only completely insane. They told me that I had been very lucky to be through such a traumatic experience and live to tell about it, without needing to be put in a psych ward. I turned to the preacher and said.   \"I think were gonna need an oven and fast. \"  That's when they knew I was a goner." );
//	//cout << toMatch << "\n\n";
//	//parser->highlight_matches ( toMatch );
//	unsigned long long sum = 0;
//	//for ( int i = 0; i < 2000; ++i )
//	//	sum += parser->getRandom ( i ).length ( );
//	cout << sum / 2000 << endl;
//	//__debugbreak ( );
//	cin.ignore ( 2 );
//	return 0;
//}