/*
------------------------------------- Credits -----------------------------------------------------------------
Generalized National Airspace Trajectory Simulation (GNATS) software
2017-2021 GNATS Development Team at Optimal Synthesis Inc. are:
Team Lead, Software Architecture and Algorithms: Dr. P. K. Menon
Algorithms and Prototyping: Dr. Parikshit Dutta
Java and C++ Code Development: Oliver Chen and Hari N. Iyer
Illustrative Examples in Python and MATLAB: Dr. Parikshit Dutta, Dr. Bong-Jun Yang, Hari Iyer
Illustrative Examples in SciLab and R: Hari Iyer
Vedik Jayaraj (Summer Intern) helped digitize 39 US airports together with the Arrival-Departure procedures and helped in beta testing of GNATS.
Acknowledgements: 
GNATS software was developed under the Arizona State University Subaward No. 18-275 under the NASA University Leadership Initiative Prime Contract No. NNX17AJ86A, with Professor Yongming Liu serving as the Principal Investigator. 
Beta Testing outside Optimal Synthesis Inc. was carried out at Arizona State University under the direction of Professor Yongming Liu, at Vanderbilt University under the direction of Professor Sankaran Mahadevan and Professor Pranav Karve, at the Southwest Research Institute under the direction of Dr. Baron Bichon and Dr. Erin DeCarlo, and at Carnegie-Mellon University under the direction of Professor Pingbo Tang.
NASA Technical points-of-contact: Dr. Anupa Bajwa, Dr. Kaushik Datta, Dr. John Cavolowsky, Dr. Kai Goebel
------------------------------------Legacy Source Code--------------------------------------------------------
Legacy Code for the GNATS software was derived from the software packages developed under the following NASA Small Business Innovation Research Projects:
1. 2004-2006 NASA Contract No. NNA05BE64C with Dr. Shon Grabbe of NASA Ames Research Center as the Technical Monitor.
2. 2008-2010 NASA Contract No. NNX08CA02C with Dr. Joseph Rios of Ames Research Center as the Technical Monitor.
3. 2010-2011 NASA Phase III Contract No. NNA10DC12C with Dr. Joseph Rios of Ames Research Center as the Technical Monitor.
4. 2016-2018 NASA Contract No. NNX16CL11C with Dr. Nash’at Ahmad of NASA Langley Research Center as the Technical Monitor.

Contributors to these SBIR projects at Optimal Synthesis Inc. were: Dr. P. K. Menon (Principal Investigator), Jason Kwan (Software Engineer), Gerald M. Diaz (Software Engineer), Dr. Monish Tandale (Research Scientist), Dr. Prasenjit Sengupta (Research Scientist), Dr. Sang-Gyun Park (Research Scientist) and Dr. Parikshit Dutta (Research Scientist).
The inspiration for the SBIR projects is derived from the FACET software developed at NASA Ames Research Center by Dr. Banavar Sridhar, Dr. Karl Bilimoria, Dr. Gano Chatterji, Dr. Shon Grabbe and Dr. Kapil Sheth.

Dr. Victor H. L. Cheng of Optimal Synthesis Inc. provided the digitized data for 40 major US Airports
---------------------------------------------------------------------------------------------------------------------
*/

#ifndef __UTIL_STRING_H__
#define __UTIL_STRING_H__

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <deque>

#include <iostream>

#ifdef _WIN32
//#ifndef strtok_r
//// strtok is supposedly re-entrant on win32...
//#define strtok_r(s,d,p) strtok(s,d);
//#endif

// the win32 pthread.h included by libgosafe has
// definitions for strtok_r, so use those to
// avoid multiple definitions.
#include "pthread.h"
#endif

using namespace std;

bool startsWith (char* base, char* str);
bool endsWith (const char* base, const char* str);
int indexOf (char* base, char* str);
int indexOf_shift (char* base, char* str, int startIndex);
int lastIndexOf (char* base, char* str);
void subString (char* dest, const char* input, int offset, int len);
char* trimwhitespace(char *str);
size_t trimwhitespace(char *out, size_t len, const char *str);

/**
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Function:
 *   trim()
 *
 * Description:
 *   remove the leading and trailing whitespace from a string.  This function
 *   will modify the original input string.
 *
 * Input:
 *   none
 *
 * In/Out:
 *   str - the std string to be trimmed
 *
 * Return:
 *   the reference to the trimmed string
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
inline string& trim(string& str) {
  string::size_type pos = str.find_last_not_of(' ');
  if(pos != string::npos) {
    str.erase(pos + 1);
    pos = str.find_first_not_of(' ');
    if(pos != string::npos) str.erase(0, pos);
  }
  else str.erase(str.begin(), str.end());
  return str;
}
/*
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Function:
 *   tokenize()
 *
 * Description:
 *   Tokenize a string into a vector of tokens
 *   Tokenizer from:
 *     www.oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html
 *
 * Input:
 *   str - the string to be tokenized
 *   delimiter - the character that separates tokens
 *
 * In/Out:
 *   none
 *
 * Return:
 *   a STL deque of strings containing the tokens.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
inline deque<string> tokenize(const string& str,
		                              const string& delimiter) {
	deque<string> tokens;
	char* saveptr = NULL;
	char* cstr = (char*)calloc(str.length()+1, sizeof(char));
	strcpy(cstr, str.c_str());
	char* token = strtok_r(cstr, delimiter.c_str(), &saveptr);
	if(token) {
		string s(token);
		//trim(s);
		tokens.push_back(s);
	}

	while(token) {
		token = strtok_r(NULL, delimiter.c_str(), &saveptr);

		if(token) {
			string s(token);

			tokens.push_back(s);
		}
	}

	free(cstr);

	return tokens;
}

/**
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Function:
 *   fromString
 *
 * Description:
 *   Convert a string to a primitive type
 *
 * Inputs:
 *   s - the string to convert
 *   f - the base of conversion: std::dec, std::hex, etc.
 *
 * In/Out:
 *   t - the variable to store the converted string value
 *
 * Return:
 *   true on success, false on failure
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
template <class T>
static inline bool fromString(T& t, const string& s,
							  ios_base& (*f)(ios_base&)) {
	istringstream iss(s);
	return !(iss >> f >> t).fail();
}

/**
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Function:
 *   isInteger
 *
 * Description:
 *   Check if a c++ string is a int
 *
 * Inputs:
 *   s - the string to convert
 *
 * In/Out:
 *   None
 *
 * Return:
 *   true on success, false on failure
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
inline bool isInteger(const string & s)
{
   if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false ;

   char * p ;
   strtol(s.c_str(), &p, 10) ;

   return (*p == 0) ;
}

/** PARIKSHIT ADDER
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Function:
 *   isDouble
 *
 * Description:
 *   Check if a c++ string is a double
 *
 * Inputs:
 *   s - the string to convert
 *
 * In/Out:
 *   None
 *
 * Return:
 *   true on success, false on failure
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
inline bool isDouble(const std::string& s)
{
    char* end = 0;
    double val = strtod(s.c_str(), &end);
    return end != s.c_str() && val != HUGE_VAL;
}

void toUppercase(char * srcArray);

#endif
