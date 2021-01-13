/*
------------------------------------- Credits -----------------------------------------------------------------
Generalized National Airspace Trajectory Simulation (GNATS) software
2017-2021 GNATS Development Team at Optimal Synthesis Inc. are:
Team Lead, Software Architecture and Algorithms: Dr. P. K. Menon
Algorithms and Prototyping: Dr. Parikshit Dutta
Java and C++ Code Development: Oliver Chen and Hari N. Iyer
Illustrative Examples in Python and MATLAB: Dr. Parikshit Dutta, Dr. Bong-Jun Yang, Hari Iyer
Illustrative Examples in SciLab and R: Hari Iyer
Acknowledgements: 
GNATS software was developed under the Arizona State University Subaward No. 18-275 under the NASA University Leadership Initiative Prime Contract No. NNX17AJ86A, with Professor Yongming Liu serving as the Principal Investigator. 
Beta Testing outside Optimal Synthesis Inc. was carried out at Arizona State University under the direction of Professor Yongming Liu, at Vanderbilt University under the direction of Professor Sankaran Mahadevan and Professor Pranav Karve, at the Southwest Research Institute under the direction of Dr. Baron Bichon and Dr. Erin DeCarlo, and at Carnegie-Mellon University under the direction of Professor Pingbo Tang.
NASA Technical points-of-contact: Dr. Anupa Bajwa, Dr. Kaushik Datta, Dr. John Cavolowsky, Dr. Kai Goebel
------------------------------------Legacy Source Code--------------------------------------------------------
Legacy Code for the GNATS software was derived from the software packages developed under the following NASA Small Business Innovation Research Projects:
1. 2004-2006 NASA Contract No. NNA05BE64C with Dr. Shon Grabbe of NASA Ames Research Center as the Technical Monitor.
2. 2008-2010 NASA Contract No. NNX08CA02C with Dr. Joseph Rios of Ames Research Center as the Technical Monitor.
3. 2010-2011 NASA Phase III Contract No. NNA10DC12C with Joseph Rios of Ames Research Center as the Technical Monitor.
3. 2016-2018 NASA Contract No. NNX16CL11C with Dr. Nash’at Ahmad of NASA Langley Research Center as the Technical Monitor.
Contributors to these SBIR projects at Optimal Synthesis Inc. were: Dr. P. K. Menon (Principal Investigator), Jason Kwan (Software Engineer), Gerald M. Diaz (Software Engineer), Dr. Monish Tandale (Research Scientist), Dr. Prasenjit Sengupta (Research Scientist), Dr. Sang-Gyun Park (Research Scientist) and Dr. Parikshit Dutta (Research Scientist).
The inspiration for the SBIR projects is derived from the FACET software developed at NASA Ames Research Center by Drs. Banavar Sridhar, Dr. Karl Bilimoria, Dr. Gano Chatterji, Dr. Shon Grabbe and Dr. Kapil Sheth.
---------------------------------------------------------------------------------------------------------------------
*/

/*
 * TrxUtils.cpp
 *
 *  Created on: May 5, 2012
 *      Author: jason
 */

#include "TrxUtils.h"

#include "util_string.h"

#include <deque>
#include <string>
#include <ctype.h> /* for isdigit() */

using std::string;
using std::deque;

namespace osi {

/*
 * Parse the FP_ROUTE record string into a TrxRoute object.
 * The original route string is stored in the TrxRoute.route field.
 */
int parse_trx_route(const string& routeStr, TrxRoute* const route) {

	if(!route) return -1;

	// store the original route string
	route->route = routeStr;

	// TODO: for now we only parse the origin and destination airports
	// and an eta, if present.  later, we need to parse the sids, stars,
	// pars, and airway data.
	string origin;
	string destination;
	long eta = UNSET_LONG;
	long etd = UNSET_LONG;

	// tokenize on period '.'
	deque<string> tokens = tokenize(routeStr, ".");

	// origin airport is first token.
	origin = tokens.at(0);

	// destination airport is last token
	destination = tokens.at(tokens.size()-1);

	// strip the asterisk from origin airport.
	// strip the asterisk from destination airport.
	if(origin.find_first_of("*") != origin.npos) {
		origin.erase(origin.end()-1, origin.end());
	}
	if(destination.find_first_of("*") != destination.npos) {
		destination.erase(destination.end()-1, destination.end());
	}

	// if the origin or destination airport contains '/' then parse eta/etd
	if(origin.find_first_of("/") != origin.npos) {
		deque<string> origToks = tokenize(origin, "/");
		origin = origToks.front();
		etd = atol(origToks.back().c_str());
	}
	if(destination.find_first_of("/") != destination.npos) {
		deque<string> destToks = tokenize(destination, "/");
		destination = destToks.front();
		eta = atol(destToks.back().c_str());
	}

	// if the origin or destination airport is longer than 4 chars
	// then the remaining chars are numbers.  i don't know what the
	// numbers mean.  remove the numeric chars from the end of the string.
	// TODO: if we find out what the numbers mean we can store them
	// appropriately.
	if(origin.length() > 4) {
		string::iterator iter;
		for(iter=origin.end()-1; iter!=origin.begin(); --iter) {
			if(isdigit(*iter)) {
				origin.erase(iter);
			} else {
				break;
			}
		}
	}
	if(destination.length() > 4) {
		string::iterator iter;
		for(iter=destination.end()-1; iter!=destination.begin(); --iter) {
			if(isdigit(*iter)) {
				destination.erase(iter);
			} else {
				break;
			}
		}
	}

	// if origin or destination airport is 3 characters long and doesn't
	// start with 'K' then prefix the airport with 'K'
	if(origin.length() < 4) origin.insert(0, "K");
	if(destination.length() < 4) destination.insert(0, "K");

	// assign route tokens to output struct
	route->origin = origin;
	route->destination = destination;
	route->eta = eta;
	route->etd = etd;

	return 0;
}

}
