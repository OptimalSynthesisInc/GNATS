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

/**
 * FlightPlanParser.cpp
 * 
 * Flight plan string parser.  This class parses a flight plan
 * string into vectors of latitudes and longitudes.
 *
 * Author: jason
 * Date: January 19, 2013
 */

#include "AirportLayoutDataLoader.h"

#include "FlightPlanParser.h"
#include "FlightPlan.h"

#include "RouteElement.h"

#include "PointWGS84.h"
#include "geometry_utils.h"

#include "NatsPar.h"
#include "NatsSid.h"
#include "NatsStar.h"
#include "NatsApproach.h"
#include "NatsAirway.h"
#include "NatsWaypoint.h"
#include "NatsAirport.h"

#include "json.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <iostream>
#include <vector>
#include <deque>
#include <list>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include <cmath>

#include <typeinfo>

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::deque;
using std::lower_bound;
using std::upper_bound;
using std::stringstream;
using std::find;
using std::list;

using namespace osi;
using json = nlohmann::json;

// static prototypes
static void trim_dots(string& str);
static NatsPar* parse_par(string& str, const vector<NatsPar>& g_pars);
static NatsApproach* parse_approach(string& str, const vector<NatsApproach>& g_approaches,
		const string& ap = "KSFO");
static NatsStar* parse_star(string& str, const vector<NatsStar>& g_stars,
		const string& ap = "KSFO");
static NatsSid* parse_sid(string& str, const vector<NatsSid>& g_sids,
		const string& ap = "KSFO");

static void checkWaypointList(vector<string>& wplist);
static void removestr(string& wp, string rem);
template<typename T>
static void populateElement(T* const proc, const deque<string>& tokens,
		const vector<NatsWaypoint>& waypoints, const vector<NatsAirport>& airports,
		const string& rwname, const string& initial_target_waypoint,
		FpRouteElement& elem, vector<string>& legs);
static void populateApproachElement(NatsApproach* const app, const string& star_end,
        const vector<NatsWaypoint>& waypoints,FpRouteElement& elem,vector<string>& legs);
bool is_rwy_found(string& str){
	size_t found1 = str.find("-");
	size_t found2 = str.find("RW");

	if (found1 != string::npos && found2 != string::npos)
		return true;
	else
		return false;
}

static inline bool isInteger(const string & s)
{
   if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false ;

   char * p ;
   strtol(s.c_str(), &p, 10) ;

   return (*p == 0) ;
}

static inline string& trim(string& str) {
  string::size_type pos = str.find_last_not_of(' ');
  if(pos != string::npos) {
    str.erase(pos + 1);
    pos = str.find_first_not_of(' ');
    if(pos != string::npos) str.erase(0, pos);
  }
  else str.erase(str.begin(), str.end());
  return str;
}

#define DEBUG_PARSE2 0

FlightPlanParser::FlightPlanParser() {
}

FlightPlanParser::~FlightPlanParser() {
}

#if 0
void FlightPlanParser::parse(const string& fpstr, 
		vector<NatsPar>& g_pars,
		vector<NatsStar>& g_stars,
		vector<NatsSid>& g_sids,
		vector<NatsAirway>& g_airways,
		vector<NatsWaypoint>& g_waypoints,
		vector<NatsAirport>& g_airports,
		FlightPlan* const fpout) {

	// process the flight plan route strings...
	// we will tokenize the route string using '.' as separator
	// 1) remove and store the first token.  its the origin airport
	// 2) remove and store the last token.  its the destination airport/eta.
	// 3) create an empty string vector of wp tokens (may be names or lat/lon)
	// 4) for the remaining substring...
	//    a) check to see if the remaining substring corresponds to a PAR.
	//       if yes then add the PAR waypoint sequence to the vector of wp
	//       and remove the entire PAR identifier from the route substring.
	//       else...
	//         check to see if the remaining substring corresponds to a STAR.
	//         if yes then add the STAR wp sequence to the vector of wp
	//         and remove the entire STAR identifier from the route substring.
	//         else...
	//           check to see if the remaining substring corresponds to a SID.
	//           if yes then add the SID wp sequence to the vector of wp and
	//           remove the entire SID identifier from the route substring.
	//           else...
	//             the remaining substring is not a PAR, STAR, or SID.
	//             pop the next token from the beginning of the substring.
	//             check if the popped token corresponds to an Airway.
	//             if yes then add the Airway wp sequence to the vector of wp
	//             else...
	//             the token is assumed to be a waypoint. add it to wp vec.
	//    b) goto (4) and repeat until no more tokens in remaining substring.
	// 5) we now have a vector of waypoints (either named or lat/lon strings,
	//    or radial fixes).  note: radial fixes are specified as bearing
	//    and distance from a named fix.
	//    Iterate over the vector of waypoints and convert each waypoint str
	//    to a lat/lon point in decimal degrees (type double).

	if(!fpout) return;

	string fp = fpstr;
	fpout->routeString = fpstr;

	// 1) parse the origin airport
	int firstdot = fp.find_first_of(".");
	string origin = fp.substr(0, firstdot);
	fpout->origin = origin;

	// 2) parse the destination airport/eta
	int lastdot = fp.find_last_of(".");
	string destAndEta = fp.substr(lastdot+1, string::npos);
	string destination;
	if(destAndEta.length() == 3) {
		destination = destAndEta;
	} else if(destAndEta.length() == 4) {
		// could be XXX* or KXXX
		if(destAndEta.at(3) == '*') {
			destination = destAndEta.substr(0,3);
		} else {
			destination = destAndEta.substr(1);
		}
	} else {
		// destintation and eta: [airport]/eta
		int pos = destAndEta.find_first_of("/");
		destination = destAndEta.substr(0, pos);
	}
	fpout->destination = destination;

	// we will now work a copy with origin/destination removed.
	// also remove any leading/trailing '.' or '/' characters
	int sublen = lastdot - firstdot;
	string sub = fp.substr(firstdot+1, sublen);

	if(sub.length() > 0) {
		while(sub.at(0) == '.' || sub.at(0) == '/') {
			if(sub.length() > 0) {
				sub.erase(0,1);
				if(sub.length() < 1) break;
			}
		}
	}
	if(sub.length() > 0) {
		while(sub.at(sub.length()-1) == '.' || sub.at(sub.length()-1) == '/') {
			if(sub.length() > 0) {
				sub.erase(sub.length()-1);
				if(sub.length() < 1) break;
			}
		}
	}

	// 3) create the empty vector of waypoint strings
	deque<string> tokens;
	deque<FpRouteElement> elements;
	cout << "origin: " << origin << endl;
	cout << "destination: " << destination << endl;
	cout << "sub: " << sub << endl;

	NatsPar* par = NULL;
	NatsStar* star = NULL;
	NatsSid* sid = NULL;

	// find the departure route
	sid = parse_sid(sub, g_sids);

	// find the arrival route. try par first, then star if no par found
	par = parse_par(sub, g_pars);
	if(!par) {
		star = parse_star(sub, g_stars);
	}

	// some short flight plans may have a sid followed immediately by a
	// par or star.  in these cases, we need to make sure that there is
	// no overlap in the complete departure/arrival route.  to do this,
	// we find the first common waypoint (with respect to the ends of the
	// departure/arrival procedure)
	// that appears in both the sid and par/star.  we remove all points
	// after the common point from the sid and we remove all points
	// before (and including) the common point from the par/star.
	// if there are multiple common points we give priority to the
	// arrival route. Example:
	// sid: PRFUM, 12237, 13820, 10273, CADDU, HOBES, 14744, DRK
	// par: BLD, BLD108030, PRFUM, SINHO, BLD108093, 10273, DRK, HATRK, MAIER, WEBAD, COYOT, BRUSR
	// the first common point starting at the ends of the procedures is DRK
	// we would use the following:
	// sid: PRFUM, 12237, 13820, 10273, CADDU, HOBES, 14744, DRK
	// par: HATRK, MAIER, WEBAD, COYOT, BRUSR
	int arrival_index = 0;
	int departure_index = (sid ? sid->waypoints.size()-1 : 0);

	if(sid) {
		for(int i=(int)sid->waypoints.size()-1; i>=0; --i) {
			vector<string>::iterator wpiter;
			if(par) {
				wpiter = find(par->waypoints.begin(), par->waypoints.end(), sid->waypoints.at(i));
				if(wpiter != par->waypoints.end()) {
					departure_index = i;
					arrival_index = wpiter - par->waypoints.begin() + 1;
					break;
				}
			} else if(star) {
				wpiter = find(star->waypoints.begin(), star->waypoints.end(), sid->waypoints.at(i));
				if(wpiter != star->waypoints.end()) {
					departure_index = i;
					arrival_index = wpiter - star->waypoints.begin() + 1;
					break;
				}
			} else {
				// no arrival route, so use entire sid.
				break;
			}
		}
	}

	// assemble the tokens:

	// push the sid waypoints to the back of tokens, if sid was found
	if(sid) {
		for(int i=0; i<=departure_index; ++i) {
			tokens.push_back(sid->waypoints.at(i));
		}
	} else {
		tokens.push_back(origin);
	}

	// push the remaining route tokens to the back of tokens
	char* saveptr = NULL;
	char* cstr = (char*)calloc(sub.length()+1, sizeof(char));
	strcpy(cstr, sub.c_str());
	char* tok = strtok_r(cstr, ".", &saveptr);
	while(tok) {
		tokens.push_back(string(tok));
		tok = strtok_r(NULL, ".", &saveptr);
	}
	free(cstr);

	// push the arrival route waypoints to the back of tokens, if arrival
	// route was found
	if(par) {
		for(unsigned int i=arrival_index; i<par->waypoints.size(); ++i) {
			tokens.push_back(par->waypoints.at(i));
		}
	} else if(star) {
		for(unsigned int i=arrival_index; i<star->waypoints.size(); ++i) {
			tokens.push_back(star->waypoints.at(i));
		}
	}

	// append the route with the destination airport, if necessary.
	// stars may end with the airport.  pars may not.
	// the destination airport as given in the TRX route may be
	// a 3-letter acronym for domestic flights, or a 4-letter
	// acronym starting with K for international flights.
	string dest3Letter = (destination.length() > 3 ?
			destination.substr(1) : 
			destination);
	if(tokens.back() != dest3Letter) {
		tokens.push_back(dest3Letter);
	}

	cout << "Route tokens: [ ";
	for(unsigned int ri=0; ri<tokens.size(); ++ri) {
		cout << tokens.at(ri) << " ";
	}
	cout << "]" << endl;

	// process the route tokens and find airway tokens.
	// if an airway token is found then insert the airway subarray from
	// the preceding waypoint to the next waypoint.  begin processing
	// from the first token.
	for(unsigned int j=1; j<tokens.size()-1; ++j) {
		string token = tokens.at(j);

		// the airway may be specified as V27.  here, we parse the
		// number from the name and prefix it with 0 if it is less.
		// than 100.  this is because the airway names in the
		// Airways.crypt file are 4 characters long, ie V027.
		string airwayType = token.substr(0,1);
		string airwayNum = token.substr(1);
		int airwayInt = atoi(airwayNum.c_str());
		stringstream ss;
		ss << airwayType;
		if(airwayInt < 100) {
			ss << "0";
		}
		ss << airwayNum;

		NatsAirway key;
		key.name = ss.str();
		vector<NatsAirway>::iterator awIter = lower_bound(g_airways.begin(),
				g_airways.end(),
				key);
		NatsAirway* airway = NULL;
		if(awIter != g_airways.end()) {
			if(awIter->name == key.name) {
				airway = &(*awIter);
			}
		}

		// if we find a matching airway name, look for the subarray
		// from the preceeding token to the following waypoint token.
		// airway waypoint sequences are bi-directional, so we need to
		// search both directions.
		//*
		if(airway) {
			string airwayStart = tokens.at(j-1);
			string airwayEnd = tokens.at(j+1);
			vector<string> airwaySegment;

			for(int k=(j-1); k>=0; --k) {
				vector<string>::iterator it = find(airway->route.begin(), airway->route.end(), tokens.at(k));
				if(it != airway->route.end()) {
					// remove tokens from k to j-1
					tokens.erase(tokens.begin()+k, tokens.begin()+j-1);
					airwayStart = airway->route.at((int)(it - airway->route.begin()));

					break;
				}
			}
			for(unsigned int k=j+1; k<tokens.size(); ++k) {
				vector<string>::iterator it = find(airway->route.begin(), airway->route.end(), tokens.at(k));
				if(it != airway->route.end()) {
					// remove tokens from j+1 to k
					tokens.erase(tokens.begin()+j+1, tokens.begin()+k);
					airwayEnd = airway->route.at((int)(it - airway->route.begin()));

					break;
				}
			}

			cout << "get airway segment: start=" << airwayStart << ", end=" << airwayEnd << endl;

			airway->getRouteSegment(airwayStart, airwayEnd, &airwaySegment);
			if(airwaySegment.size() > 0) airwaySegment.erase(airwaySegment.begin());
			if(airwaySegment.size() > 0) airwaySegment.erase(airwaySegment.end()-1);

			// if airway segment was found then insert the segment at i and
			// remove the original airway token. update the loop counter
			// to point at the airwayEnd token.
			tokens.erase(tokens.begin()+j);
			tokens.insert(tokens.begin()+j,
					airwaySegment.begin(),
					airwaySegment.end());
			deque<string>::iterator it = find(tokens.begin(),
					tokens.end(),
					airwayEnd);
			j=it - tokens.begin();
		} else {
			// remove the original airway name token from the tokens array.
			// decrement the loop counter to reflect the change in size.
		}
	}

	cout << "Route tokens: [ ";
	for(unsigned int ri=0; ri<tokens.size(); ++ri) {
		cout << tokens.at(ri) << " ";
	}
	cout << "]" << endl;

	// remove duplicate consecutive fixes.  some PARs may contain
	// duplicate fixes,  for example, one of the BSR..OSI PARs
	// in zoa_par.crypt (there's more than one)
	// contains two consecutive BSR fixes.
	for(unsigned int ri=1; ri<tokens.size(); ++ri) {
		if(tokens.at(ri-1) == tokens.at(ri)) {
			tokens.erase(tokens.begin()+ri,tokens.begin()+ri+1);
		}
	}

	// at this point, we should have a vector of route tokens
	// that contains only waypoint names or points specified
	// as a radial location from a waypoint (such as EUGEN099017).
	// look up the waypoint lat/lon from the global waypoint data.
	// and build vectors of lat/lon points.
	// we distinguish radial distance points by string length.
	// all named waypoints are either 3 or 5 characters.
	// if a named waypoint is not in Waypoints.crypt then we
	// remove the waypoint from the flight plan.
	vector<double> latitudes;
	vector<double> longitudes;
	for(unsigned int ri=0; ri<tokens.size(); ++ri) {

		if(tokens.at(ri).length() <= 5) {
			// if named waypoint 3 or 5 chars
			string fix = tokens.at(ri);
			double fixLat=-999999;
			double fixLon=-999999;
			NatsWaypoint wpKey;
			wpKey.name = fix;
			vector<NatsWaypoint>::iterator wpIter =
					lower_bound(g_waypoints.begin(),
							g_waypoints.end(),
							wpKey);
			if(wpIter != g_waypoints.end() && wpIter->name == wpKey.name) {
				fixLat = wpIter->latitude;
				fixLon = wpIter->longitude;
			} else {
				// note: airport codes in Airports.crypt are 4-letter codes
				// starting with K, for example KSFO as opposed to SFO
				int fixlen = fix.length();
				NatsAirport apKey;
				apKey.code = (fixlen < 4 ? "K"+fix : fix);
				vector<NatsAirport>::iterator apIter =
						lower_bound(g_airports.begin(),
								g_airports.end(),
								apKey);
				if(apIter != g_airports.end() && apIter->code == apKey.code) {
					fixLat = apIter->latitude;
					fixLon = apIter->longitude;
				}
			}

			if(fixLat==-999999 || fixLon==-999999) {
				// fix not found in g_waypoints or g_airports. ignore it.
				continue;
			}

			latitudes.push_back(fixLat);
			longitudes.push_back(fixLon);

			fpout->route.push_back(PointWGS84(fixLat, fixLon));
		} else {
			// else if radial point...
			//  parse out the fix, radial bearing, and dist. components
			//  lookup the fix in g_waypoints
			//  if fix not found in g_waypoints, try g_airports
			//  if fix still not found, drop it from flight plan, otherwise
			//  if fix is found then compute lat/lon and push to lat/lon vectors
			int fixlen = tokens.at(ri).length() - 6;
			string fix = tokens.at(ri).substr(0, fixlen);
			string bearingStr = tokens.at(ri).substr(fixlen, 3);
			string distStr = tokens.at(ri).substr(fixlen+3, 3);
			double bearing = atof(bearingStr.c_str()) * M_PI/180.; // radians
			double dist = atof(distStr.c_str()); // miles

			double fixLat=-999999;
			double fixLon=-999999;
			NatsWaypoint wpKey;
			wpKey.name = fix;
			vector<NatsWaypoint>::iterator wpIter =
					lower_bound(g_waypoints.begin(),
							g_waypoints.end(),
							wpKey);
			if(wpIter != g_waypoints.end() && wpIter->name == wpKey.name) {
				fixLat = wpIter->latitude;
				fixLon = wpIter->longitude;
			} else {
				// note: airport codes in Airports.crypt are 4-letter codes
				// starting with K, for example KSFO as opposed to SFO
				NatsAirport apKey;
				apKey.code = (fixlen < 4 ? "K"+fix : fix);
				vector<NatsAirport>::iterator apIter =
						lower_bound(g_airports.begin(),
								g_airports.end(),
								apKey);
				if(apIter != g_airports.end() && apIter->code == apKey.code) {
					fixLat = apIter->latitude;
					fixLon = apIter->longitude;
				}
			}

			if(fixLat==-999999 || fixLon==-999999) {
				// fix not found in g_waypoints or g_airports. ignore it.
				continue;
			}

			// fix lat/lon from degrees to rad
			double fixLatRad = fixLat*M_PI/180.;

			double milesToFeet = 5280;
			double radiusEarthFeet = 20925524.9;
			double rangeAngleRad = dist * milesToFeet / radiusEarthFeet;
			double latRad = asin(sin(fixLatRad) * cos(rangeAngleRad) +
					cos(fixLatRad) * sin(rangeAngleRad) *
					cos(bearing));
			double latDeg = latRad*180/M_PI;
			double lonDeg = fixLon + asin(sin(rangeAngleRad) * sin(bearing) /
					cos(latRad))*180/M_PI;

			latitudes.push_back(latDeg);
			longitudes.push_back(lonDeg);

			fpout->route.push_back(PointWGS84(latDeg, lonDeg));
		}
	}

	// final filtering of flight plan waypoints.
	// here, we remove waypoints that are 'far away' from the flight plan
}
#endif

static const NatsWaypoint* get_waypoint(const string& name,
		                               const vector<NatsWaypoint>& waypoints) {
	vector<NatsWaypoint>::const_iterator key_iter;
	NatsWaypoint wp_key;
	wp_key.name = name;
	key_iter = lower_bound(waypoints.begin(), waypoints.end(), wp_key);

	if(key_iter != waypoints.end() && key_iter->name==name) {
		return &(*key_iter);
	}
	return NULL;
}

static void get_lat_lon_point(const string wp, double* const lat, double* const lon) {
	// wp is a lat/lon specification:
	// we want decimal degrees, north positive, east positive.
	// the TRX file usually gives ddmmN/dddmmW, north and west positive.
	// directions may or may not be specified if no direction specified
	// we assume north positive and west positive.
	size_t slash_pos = wp.find_first_of("/");
	string latstr = wp.substr(0, slash_pos);
	string lonstr = wp.substr(slash_pos+1);
	int lat_ddmm = atoi(latstr.c_str());
	int lon_dddmm = atoi(lonstr.c_str());

	*lat = floor(lat_ddmm / 100.) + ((double)(lat_ddmm%100)) / 60.;
	*lon = floor(lon_dddmm / 100.) + ((double)(lon_dddmm%100)) / 60.;

	// check if the string contains directions (N,S,E,W)
	size_t s_pos = wp.find_first_of("S");
	size_t e_pos = wp.find_first_of("E");

	// if east is not explicitly specified or west is explicitly
	// specified then convert lon to east positive.
	if(e_pos >= wp.length()) {
		*lon = -(*lon);
	}

	// if south is explicitly specified then convert lat to
	// north positive.
	if(s_pos < wp.length()) {
		*lat = -(*lat);
	}
}

static void get_intersection(const FpRouteElement& a,
		                     const FpRouteElement& b,
		                     const vector<NatsWaypoint>& waypoints,
		                     string* const intersection,
		                     int* const index_a,
		                     int* const index_b) {

	// if a and b are both single points they must be an exact match

	if(a.sequence.size() == 1 && b.sequence.size() == 1) {
		if( a.sequence.back() == b.sequence.back() ) {
			*index_a = 0;
			*index_b = 0;
			*intersection = a.sequence.back();
		} else {
			*index_a = -1;
			*index_b = -1;
			*intersection = "";
		}
		return;
	}



	// search for the intersection. if no intersection is found then
	// find the nearest point in b if b's sequence is > 1.
	for(unsigned int i=0; i<a.sequence.size(); ++i) {
		// the key is the name of the waypoint.  we cut out the
		// bearing and dist info if it is a radial fix. for example,
		// if the a.sequence.at(i) value is AUDIO021003 then
		// we use AUDIO.
		string key = a.sequence.at(i).substr(0,
				a.sequence.at(i).find_first_of("0123456789"));
		vector<string>::const_iterator it = find(b.sequence.begin(),
				                                 b.sequence.end(),
				                                 key);
		if(it != b.sequence.end() && (*it)==key) {
			*intersection = *it;
			*index_a = i;
			*index_b = it - b.sequence.begin();

			return;
		}
	}

	// no intersection found.
	// find the 'nearest' intersection. to do this, we take the endpoint
	// of the shorter route and compute the minimum distance from the
	// longer route.  here, shorter/longer refers to the number of elements
	// in the route vector.
	// it could be that the intersection wasn't found because the route
	// didn't contain the named key waypoint, or that the key waypoint was
	// specified as a lat/lon pair.  if the key waypoint is a lat/lon pair
	// then we construct a dummy waypoint with the desired lat/lon location
	// and use it for distance calculations.
	string key;
	string smin = "";
	int imin = -1;
	bool found_key = false;
	const vector<string>* vec;
	if(a.sequence.size() < b.sequence.size()) {
		key = a.sequence.back().substr(0, a.sequence.back().find_first_of("0123456789"));
		vec = &(b.sequence);
	} else {
		key = b.sequence.back().substr(0, b.sequence.back().find_first_of("0123456789"));
		vec = &(a.sequence);
	}
	double key_lat = 0;
	double key_lon = 0;
	// if key contains '/' then it might be a lat/lon pair
	size_t slash_pos = key.find_first_of("/");
	if(slash_pos < key.length()) {
		get_lat_lon_point(key, &key_lat, &key_lon);
		found_key = true;
	} else {

		//BYPASS THE WAYPOINT CREATION LOGIC IN OLD CARPAT
		//FLY THE ORIGINAL ROUTE
		*intersection = "";
		*index_a = -1;
		*index_b = -1;
		return ;
		//IF YOU COMMENT OUT THE ABOVE THREE LINES YOU WILL GET THE ORIGINAL CODE
		//THIS BYPASSING WAS DONE TO SOLVE ERIN'S PROBLEM.
		// BYPASS LOGIC TILL HERE.

		const NatsWaypoint* nats_key = get_waypoint(key, waypoints);
		if(nats_key) {
			key_lat = nats_key->latitude;
			key_lon = nats_key->longitude;
			found_key = true;
		}
	}
	if(found_key) {
		double dmin = numeric_limits<double>::max();
		for(unsigned int i=0; i<vec->size(); ++i) {
			const NatsWaypoint* wp = get_waypoint(vec->at(i), waypoints);
			if(wp) {
				double d = compute_distance_gc(key_lat,
											   key_lon,
											   wp->latitude,
											   wp->longitude, 0);
				if(d < dmin) {
					dmin = d;
					imin = i;
					smin = wp->name;
				}
			}
		}
		if(smin != "") {
			if(a.sequence.size() < b.sequence.size()) {
				*index_a = a.sequence.size()-1;
				*index_b = imin;
				*intersection = smin;

				return;
			} else {
				*index_a = imin;
				*index_b = b.sequence.size()-1;
				*intersection = smin;

				return;
			}
		}
	}

	// something went wrong if we get to here.
	*intersection = "";
	*index_a = -1;
	*index_b = -1;
}

bool verify_taxi_plans(const string string_acid,
		const string string_origin_airport,
		const string string_destination_airport,
		const string route_str
		) {
	bool retValue = true; // Default

	vector<string> vector_taxi_plan;
	vector_taxi_plan.clear();

	int pos_1st_left_arrow = -1;
	int pos_2nd_left_arrow = -1;
	int pos_1st_right_arrow = -1;
	int pos_2nd_right_arrow = -1;

	pos_1st_left_arrow = route_str.find("<");
	pos_1st_right_arrow = route_str.find(">", pos_1st_left_arrow+1);
	pos_2nd_left_arrow = route_str.find("<", pos_1st_right_arrow+1);
	pos_2nd_right_arrow = route_str.find(">", pos_2nd_left_arrow+1);

	string tmpRunwayName;

	string string_departing_taxi_plan = route_str.substr(pos_1st_left_arrow+1, (pos_1st_right_arrow - pos_1st_left_arrow - 1));
	string string_landing_taxi_plan = route_str.substr(pos_2nd_left_arrow+1, (pos_2nd_right_arrow - pos_2nd_left_arrow - 1));

	string tmpTaxi_plan_str;

	string tmpValue_Id;
	string tmpValue_Latitude;
	string tmpValue_Longitude;

	map<string, AirportNode> tmpMap_waypoint_node;

	bool flag_taxi_plan_validity = true;

	// Process departing taxi plan
	tmpTaxi_plan_str.assign(string_departing_taxi_plan);

	trim(tmpTaxi_plan_str);
	if (tmpTaxi_plan_str.length() > 0) {
		if (map_ground_waypoint_connectivity.find(string_origin_airport) == map_ground_waypoint_connectivity.end()) {
			printf("Aircraft %s: Origin airport %s surface layout data does not exist.\n", string_acid.c_str(), string_origin_airport.c_str());
			flag_taxi_plan_validity = false;
			retValue = false;
		} else {
			tmpMap_waypoint_node = map_ground_waypoint_connectivity.at(string_origin_airport).map_waypoint_node;

			if (tmpTaxi_plan_str.find_first_of("{") != 0) {
				printf("Aircraft %s: Departing taxi plan is not valid.\n", string_acid.c_str());
				flag_taxi_plan_validity = false;
			}
			while (tmpTaxi_plan_str.find_first_of("{") == 0) {
				tmpValue_Id.clear(); // Reset
				tmpValue_Latitude.clear(); // Reset
				tmpValue_Longitude.clear(); // Reset

				int tmpPos_parenthesis_L = tmpTaxi_plan_str.find_first_of("{");
				int tmpPos_parenthesis_R = tmpTaxi_plan_str.find_first_of("}");
				if ((tmpPos_parenthesis_L < 0) || (tmpPos_parenthesis_R < 0)) {
					printf("Aircraft %s: Departing taxi plan is not valid.\n", string_acid.c_str());

					flag_taxi_plan_validity = false;
					retValue = false;

					break;
				}
				string tmpJsonStr = tmpTaxi_plan_str.substr(tmpPos_parenthesis_L, (tmpPos_parenthesis_R - tmpPos_parenthesis_L + 1));

				json jsonObj = json::parse(tmpJsonStr);

				if (jsonObj.contains("id")) {
					tmpValue_Id = jsonObj.at("id").get<string>();
				}

				if (jsonObj.contains("latitude")) {
					tmpValue_Latitude = jsonObj.at("latitude").get<string>();
				}
				if (jsonObj.contains("longitude")) {
					tmpValue_Longitude = jsonObj.at("longitude").get<string>();
				}

				if (tmpValue_Id.length() > 0) {
					if (tmpMap_waypoint_node.find(tmpValue_Id) == tmpMap_waypoint_node.end()) {
						printf("Aircraft %s: Departing taxi plan contains invalid waypoint %s.\n", string_acid.c_str(), tmpValue_Id.c_str());
						retValue = false;
					} else {
						vector_taxi_plan.push_back(tmpValue_Id);
					}
				}

				if (tmpTaxi_plan_str.length() > (tmpPos_parenthesis_R+1)) {
					tmpTaxi_plan_str.erase(0, tmpPos_parenthesis_R+1);
				} else {
					break;
				}

				if (tmpTaxi_plan_str.find_first_of(",")+1 > 0) {
					tmpTaxi_plan_str.erase(0, tmpTaxi_plan_str.find_first_of(",")+1);
				}

				trim(tmpTaxi_plan_str);
				if (tmpTaxi_plan_str.find_first_of("{") != 0) {
					printf("Aircraft %s: Departing taxi plan is not valid.\n", string_acid.c_str());

					flag_taxi_plan_validity = false;
					retValue = false;

					break;
				}
			} // end - while

			if (flag_taxi_plan_validity) {
				if (vector_taxi_plan.size() <= 2) {
					printf("Aircraft %s: Departing taxi plan does not contain enough waypoints.\n", string_acid.c_str());
				}
			}
		}
	}

	// =========================================================================

	flag_taxi_plan_validity = true; // Reset

	// Process landing taxi plan
	tmpTaxi_plan_str.assign(string_landing_taxi_plan);

	trim(tmpTaxi_plan_str);
	if (tmpTaxi_plan_str.length() > 0) {
		if (map_ground_waypoint_connectivity.find(string_destination_airport) == map_ground_waypoint_connectivity.end()) {
			printf("Aircraft %s: Destination airport %s surface layout data does not exist.\n", string_acid.c_str(), string_destination_airport.c_str());
			flag_taxi_plan_validity = false;
			retValue = false;
		} else {
			tmpMap_waypoint_node = map_ground_waypoint_connectivity.at(string_destination_airport).map_waypoint_node;

			if (tmpTaxi_plan_str.find_first_of("{") != 0) {
				printf("Aircraft %s: Landing taxi plan is not valid.\n", string_acid.c_str());
				flag_taxi_plan_validity = false;
				retValue = false;
			}
			while (tmpTaxi_plan_str.find_first_of("{") == 0) {
				tmpValue_Id.clear(); // Reset
				tmpValue_Latitude.clear(); // Reset
				tmpValue_Longitude.clear(); // Reset

				int tmpPos_parenthesis_L = tmpTaxi_plan_str.find_first_of("{");
				int tmpPos_parenthesis_R = tmpTaxi_plan_str.find_first_of("}");
				if ((tmpPos_parenthesis_L < 0) || (tmpPos_parenthesis_R < 0)) {
					printf("Aircraft %s: Landing taxi plan is not valid.\n", string_acid.c_str());

					flag_taxi_plan_validity = false;
					retValue = false;

					break;
				}
				string tmpJsonStr = tmpTaxi_plan_str.substr(tmpPos_parenthesis_L, (tmpPos_parenthesis_R - tmpPos_parenthesis_L + 1));

				json jsonObj = json::parse(tmpJsonStr);

				if (jsonObj.contains("id")) {
					tmpValue_Id = jsonObj.at("id").get<string>();
				}

				if (jsonObj.contains("latitude")) {
					tmpValue_Latitude = jsonObj.at("latitude").get<string>();
				}
				if (jsonObj.contains("longitude")) {
					tmpValue_Longitude = jsonObj.at("longitude").get<string>();
				}

				if (tmpValue_Id.length() > 0) {
					if (tmpMap_waypoint_node.find(tmpValue_Id) == tmpMap_waypoint_node.end()) {
						printf("Aircraft %s: Landing taxi plan contains invalid waypoint %s.\n", string_acid.c_str(), tmpValue_Id.c_str());
						retValue = false;
					} else {
						vector_taxi_plan.push_back(tmpValue_Id);
					}
				}

				if (tmpTaxi_plan_str.length() > (tmpPos_parenthesis_R+1)) {
					tmpTaxi_plan_str.erase(0, tmpPos_parenthesis_R+1);
				} else {
					break;
				}

				if (tmpTaxi_plan_str.find_first_of(",")+1 > 0) {
					tmpTaxi_plan_str.erase(0, tmpTaxi_plan_str.find_first_of(",")+1);
				}

				trim(tmpTaxi_plan_str);
				if (tmpTaxi_plan_str.find_first_of("{") != 0) {
					printf("Aircraft %s: Landing taxi plan is not valid.\n", string_acid.c_str());

					flag_taxi_plan_validity = false;
					retValue = false;

					break;
				}
			} // end - while

			if (flag_taxi_plan_validity) {
				if (vector_taxi_plan.size() <= 2) {
					printf("Aircraft %s: Landing taxi plan does not contain enough waypoints.\n", string_acid.c_str());

					retValue = false;
				}
			}
		}
	}

	return retValue;
}

static void removestr(string& wp, string rem){


	while(true){
		size_t pos = wp.find(rem);
		if (pos != string::npos){
			wp.erase(pos,rem.length());
		}
		else{
			break;
		}
	}
}


static bool get_in_air_flight_plan(const string& fp, string &sub){

	string dsd = ("./.");
	bool starts_from_middle = false;
	size_t dotslash_dot_pos = fp.find("./.");
	if (dotslash_dot_pos != string::npos) {
		starts_from_middle = true;
	}

	int firstdot = fp.find_first_of(".");
	int lastdot = fp.find_last_of(".");


	if (starts_from_middle){
		size_t pos_1st_leftArrow = fp.find(".<");
		size_t fpstart = dotslash_dot_pos+dsd.length();

		int sublen = pos_1st_leftArrow-fpstart;
		if (sublen > 0){
			sub = fp.substr(fpstart, sublen);
		}else if (sublen == 0){
			sub = "";
		}else{
			printf("Wrong flight plan %s\n",fp.c_str());
		}
	}
	else{
		string dra = ">.", dla = ".<";
		size_t pos_1st_rightArrow = fp.find(">.");
		size_t pos_2nd_leftArrow = fp.find(".<", pos_1st_rightArrow+1);
		int sublen = pos_2nd_leftArrow
				-pos_1st_rightArrow-dra.length();
		if (sublen > 0 ){
			sub = fp.substr(pos_1st_rightArrow+dra.length(), sublen);
		}else{
			printf("Wrong flight plan %s\n",fp.c_str());
		}
	}
	return starts_from_middle;
}

bool FlightPlanParser::parse(const string& acid,
		 const string& fpstr,
		 const vector<NatsSid>& sids,
		 const vector<NatsPar>& pars,
		 const vector<NatsStar>& stars,
		 const vector<NatsApproach>& approaches,
		 const vector<NatsAirway>& airways,
		 const vector<NatsWaypoint>& waypoints,
		 const vector<NatsAirport>& airports,
		 FlightPlan* fpout,
		 const double altitude,
		 const double cruiseAltitude) {
	bool retValue = true; // Default

	if (!fpout)
		return false;

	FlightPlan tmpFp;

	string fp = fpstr;
	tmpFp.routeString = fpstr;

	// 1) parse the origin airport
	int firstdot = fp.find_first_of(".");
	string origin;
	if (fp.find("FP_ROUTE ") != string::npos) {
		origin = fp.substr(strlen("FP_ROUTE "), firstdot-strlen("FP_ROUTE "));
	} else {
		origin = fp.substr(0, firstdot);
	}

	NatsAirport tmporigin; tmporigin.code = origin;
	vector<NatsAirport>::const_iterator itap = find(airports.begin(),
											   airports.end(),
											   tmporigin);
	if (itap == airports.end()){
		printf("Aircraft %s: Origin %s not found.\n",acid.c_str(), origin.c_str());
		retValue = false;
		return retValue;
	}
	else{
		origin = itap->code;
	}

	if (origin.length() == 0) {
		printf("Aircraft %s: Origin airport not found.\n", acid.c_str());
		retValue = false;
		return retValue;
	}

	if( map_ground_waypoint_connectivity.find(origin)
			== map_ground_waypoint_connectivity.end()){
		printf("Aircraft %s: Origin %s not found.\n",acid.c_str(), origin.c_str());
		retValue = false;
		return retValue;
	}

	tmpFp.origin = origin;

	// 2) parse the destination airport/eta
	int lastdot = fp.find_last_of(".");
	string destAndEta = fp.substr(lastdot+1, string::npos);
	string destination;
	if (destAndEta.length() == 3) {
		destination = destAndEta;
	} else if (destAndEta.length() == 4) {
		// could be XXX* or KXXX
		if (destAndEta.at(3) == '*') {
			destination = destAndEta.substr(0, 3);
		}
		else {
			destination = destAndEta;
		}
	} else {
		// destintation and eta: [airport]/eta
		int pos = destAndEta.find_first_of("/");
		destination = destAndEta.substr(0, pos);
	}

	NatsAirport tmpdest; tmpdest.code = destination;
	itap = find(airports.begin(),airports.end(),tmpdest);
	if (itap == airports.end()){
		printf("Aircraft %s: Destination %s not found.\n",acid.c_str(), destination.c_str());
		retValue = false;
		return retValue;
	}
	else{
		destination = itap->code;
	}


	if (destination.length() == 0) {
		printf("Aircraft %s: Destination airport not found.\n", acid.c_str());
		retValue = false;
		return retValue;
	}

	if( map_ground_waypoint_connectivity.find(destination)
			== map_ground_waypoint_connectivity.end()){
		printf("Aircraft %s: Destination %s not found.\n",acid.c_str(), destination.c_str());
		retValue = false;
		return retValue;
	}

	tmpFp.destination = destination;



	// =========================================================================
	/*
	 * 1) Get the "in-air" portion of the flight plan.
	 * In air flight plans can be
	 * ori.<>.runway1.sid1.wp1..wp2..wp3.star1.approach1.runway2.<>.dest
	 * ori.<>.runway1.sid1.wp1..wp2.jr1.wp3..wp4.star1.approach1.runway1.<>.dest
	 * ori./.sidwp1.sid1.wp1..wp2..wp3..wp4.star1.approach1.runway1.<>.dest
	 * ori./.wp1.jr1.wp2.jr2.wp3.star1.approach1.runway1.<>.dest
	 * ori./.wp1..wp2..wp3..wp4.star1.approach1.runway1.<>.dest
	 * ori./.starwp1.star1.approach1.runway1.<>.dest
	 * ori./.appwp1.approach1.runway1.<>.dest
	 * ori./.runway1.<>.dest
	 * ori./..<>.dest
	 * ori./.wp1..wp2..<>.dest (needs some work)
	 * And combination of 1st and 2nd FP types can be inserted too.
	 */

	string sub = "";
	bool starts_from_middle = get_in_air_flight_plan(fp,sub);

	string departure_runway = "";
	string initial_target = "";
	string arrival_runway = "";

	//1.) First get initial target waypoint/ departure runway
	size_t first_dot_pos = sub.find_first_of('.');
	if (first_dot_pos != string::npos){
		//flight is in departure taxi/ takeoff/climb/cruise/descent/initial approach mode
		initial_target = sub.substr(0, first_dot_pos);
		sub = sub.substr(first_dot_pos+1);
		// Check if the initial target waypoint is a runway format :RWxx
		if (initial_target.substr(0,2) == "RW"
			&& isInteger(initial_target.substr(2, 2))
			) {
			departure_runway = initial_target;
		}
	}
	else {
		//First target waypoint is the landing runway v_2 point
		initial_target = sub;
		arrival_runway = initial_target;
	}

	//2.) Next get the arrival runway
	size_t last_dot_pos = sub.find_last_of('.');
	if (last_dot_pos != string::npos && last_dot_pos < sub.length()-1){
		size_t rwlen = sub.length()-last_dot_pos;
		arrival_runway = sub.substr(last_dot_pos+1, rwlen);
		sub = sub.substr(0, sub.length()-rwlen);
	}

	if (sub.length() > 0) {
		while (sub.at(0) == '.' || sub.at(0) == '/') {
			if (sub.length() > 0) {
				sub.erase(0,1);
				if (sub.length() < 1) break;
			}
		}
	}

	if (sub.length() > 0) {
		while (sub.at(sub.length()-1) == '.' || sub.at(sub.length()-1) == '/') {
			if (sub.length() > 0) {
				sub.erase(sub.length()-1);
				if (sub.length() < 1) break;
			}
		}
	}

	// 3.) Get the sid /stars / approach
	deque<string> tokens;
	tokens.clear();

	deque<FpRouteElement> elements;
	elements.clear();

	NatsStar* star = NULL;
	NatsSid* sid = NULL;
	NatsApproach* approach = NULL;

	// find the arrival route. try par first, then star if no par found

	approach = parse_approach(sub, approaches, destination);
	star = parse_star(sub, stars, destination);

	// find the departure route
	sid = parse_sid(sub, sids, origin);

	//4.) push the remaining route tokens to the back of tokens
	//    Gives you enroute points.
	string first_enrt_wp = "", last_enrt_wp = "";
	//if there is a no sid but there is a star and sub is not empty
	//it means the first target waypoint is enroute
	if (!sid && star && sub != ""){
		first_enrt_wp = initial_target;
		tokens.push_back(initial_target);
	}
	else if(!sid && !star && !approach && sub != ""){
		 //Probably the ori./.wp1..wp2..<>.dest
		first_enrt_wp = initial_target;
		tokens.push_back(initial_target);
	}
	else if (!sid && !star && !approach && sub == ""){
		 //handle the ori./.wp1..<>.dest
		if (initial_target.substr(0,2) != "RW"
			|| !isInteger(initial_target.substr(2, 2) )
			){
			//If initial target wp is not the destination runway then
			first_enrt_wp = initial_target;
			tokens.push_back(initial_target);
		}

	}

	char* saveptr = NULL;
	char* cstr = (char*)calloc(sub.length()+1, sizeof(char));
	strcpy(cstr, sub.c_str());
	char* tok = strtok_r(cstr, ".", &saveptr);
	while (tok) {
		if (first_enrt_wp == "") {
			first_enrt_wp = string(tok);
		}

		tokens.push_back(string(tok));
		tok = strtok_r(NULL, ".", &saveptr);
	}
	free(cstr);

	if (!tokens.empty()){
		last_enrt_wp = tokens.back();
	}

	if (first_enrt_wp == last_enrt_wp
			&& sid == NULL
			&& star == NULL
			&& approach == NULL ){
		//That means there is no sid/star or approach everything starts from
		//runway or we have only one enroute waypoint
		if ((4 <= initial_target.length())
				&& (initial_target.substr(0,2) == "RW"
						|| isInteger(initial_target.substr(2, 2)))
			) {
			// here we check if everything does start from runway
				first_enrt_wp = ""; last_enrt_wp = "";
		}
	}

	// =========================================================================

	FpRouteElement origElem;
	origElem.identifier = origin;
	origElem.sequence.push_back(origin);
	elements.push_front(origElem);

	// start the element sequence with the SID
	vector<string> sid_leg, star_leg, app_leg;
	if (sid) {
		FpRouteElement elem;
		elem.identifier = sid->name;

		string tmpRWName = "";
		if (departure_runway != "") {
			tmpRWName = departure_runway + "-" + origin;
		}

		populateElement<NatsSid>(sid, tokens, waypoints, airports,
				tmpRWName, initial_target, elem, sid_leg);
		if (!elem.sequence.empty()) {
			elements.push_back(elem);
		}
		else {
			sid = NULL;
		}
	}

	// iterate over the tokens and add the route elements for each
	// to the deque of elements.  remaining elements may be
	// airways, waypoints, or airports. there shouldn't be any
	// sid/star/par tokens in the remaining route as they have already
	// been parsed out.
	for (unsigned int j=0; j<tokens.size(); ++j) {
		string token = tokens.at(j);

		// the airway may be specified as V27.  here, we parse the
		// number from the name and prefix it with 0 if it is less.
		// than 100.  this is because the airway names in the
		// Airways.crypt file are 4 characters long, ie V027.
		string airwayType = token.substr(0,1);
		string airwayNum = token.substr(1);
		int airwayInt = atoi(airwayNum.c_str());
		stringstream ss;
		ss << airwayType;
		ss << airwayNum;

		NatsAirway airway_key;
		airway_key.name = ss.str();
		vector<NatsAirway>::const_iterator awIter = find(airways.begin(),
				                                           airways.end(),
				                                           airway_key);
		if (awIter != airways.end() && awIter->name == airway_key.name) {
			if (j < 1 ){
				cout << " Wrong position of jet route. Please put atleast one waypoint before jet route" << endl;
				exit(0);
			}
			const NatsAirway* airway = &(*awIter);

			string aw_start = tokens.at(j-1);
			string aw_end = tokens.at(j+1);
			if (
					find(airway->route.begin(),airway->route.end(),aw_start) == airway->route.end()
				){
				printf("ERROR: Airway %s does not have waypoint %s. Please fix flight plan.\n",
						airway->name.c_str(),aw_start.c_str());
				return false;
			}
			else if (
					find(airway->route.begin(),airway->route.end(),aw_end) == airway->route.end()
				){
				printf("ERROR:Airway %s does not have waypoint %s. Please fix flight plan.\n",
						airway->name.c_str(),aw_end.c_str());
				return false;
			}

			vector<string> airway_seg;
			airway->getRouteSegment(aw_start, aw_end,&airway_seg);

			FpRouteElement elem;
			elem.identifier = airway->name;
			elem.sequence.insert(elem.sequence.end(),
								airway_seg.begin(),
								airway_seg.end());
			elements.push_back(elem);
		} else {
			// this is a waypoint or airport.
			// we must search the waypoint and airports lists for
			// the waypoint to make sure it actually exists in the database.
			NatsWaypoint wp_key;
			wp_key.name = token;
			vector<NatsWaypoint>::const_iterator wpIter = lower_bound(waypoints.begin(),
					                                                  waypoints.end(),
					                                                  wp_key);
			if (wpIter != waypoints.end() && wpIter->name == wp_key.name) {
				const NatsWaypoint* wp = &(*wpIter);
				FpRouteElement elem;
				elem.identifier = wp->name;
				elem.sequence.push_back(wp->name);
				elements.push_back(elem);
			} else {

				NatsAirport ap_key;
				ap_key.name = token;
				vector<NatsAirport>::const_iterator apIter = lower_bound(airports.begin(),
						                                                  airports.end(),
						                                                  ap_key);
				if (apIter != airports.end() && apIter->name == ap_key.name) {
					const NatsAirport* ap = &(*apIter);
					FpRouteElement elem;
					elem.identifier = ap->name;
					elem.sequence.push_back(ap->name);
					elements.push_back(elem);
				} else {
					// could be a fix/dist/bearing or lat/lon point
					FpRouteElement elem;
					elem.identifier = token;
					elem.sequence.push_back(token);
					elements.push_back(elem);
				}
			}
		}
	} // end - for


	// finish the element sequence with STAR
	if (star) {
		FpRouteElement elem;

		string targ_wp = initial_target;
		if (sid) targ_wp = "";
		populateElement<NatsStar>(star, tokens, waypoints, airports,"",
				targ_wp,elem, star_leg);
		if (!elem.sequence.empty()) {
			elements.push_back(elem);
		}
		else {
			star = NULL;
		}

		//TODO:Parikshit adder
		if (approach && star) {
			FpRouteElement elemap;
			populateApproachElement(approach, elem.sequence.back(),
					waypoints, elemap, app_leg);

			if (!elemap.sequence.empty()) {
				elements.push_back(elemap);
			}
			else {
				approach = NULL;
			}
		}
	}
	else if (approach && !star){
		// This means that the flight plan is starting from approach
		FpRouteElement elemap;

		populateApproachElement(approach, initial_target,
				waypoints, elemap, app_leg);

		if (!elemap.sequence.empty()) {
			elements.push_back(elemap);
		}
		else {
			approach = NULL;
		}
	}


	FpRouteElement destElem;
	destElem.identifier = destination;
	destElem.sequence.push_back(destination);
	elements.push_back(destElem);

#if DEBUG_PARSE2
	cout << endl;
	cout << "origin: " << origin << endl;
	cout << "destination: " << destination << endl;
	cout << "sid: " << (sid ? sid->name : "") << endl;
	cout << "star: " << (star ? star->name : "") << endl;
	cout << "approach: " << (approach ? approach->name : "") << endl;
	cout << "tokens:";
	for (unsigned int i=0; i<tokens.size(); ++i) {
		cout << " " << tokens.at(i) << " ";
	}
	cout << endl << "elements size:" << elements.size()<<", ";;
	cout << "elements:";
	for (unsigned int i=0; i<elements.size(); ++i) {
		if (i != 0)
			cout << ",";
		cout << " " << elements.at(i).identifier << " ";
	}
	cout << endl;
#endif

	if (sid) {
		if (star == NULL) {
			printf("Aircraft %s: STAR procedure is not defined.\n", acid.c_str());

			retValue = false;
		}
	}

	if ((sid) || (star)) {
		if (approach == NULL) {
			printf("Aircraft %s: Approach procedure is not defined.\n", acid.c_str());

			retValue = false;
		}
	}

	// ========================================================================

	if (retValue) {
		// iterate over the element sequence and merge the sequences
		// for a route, we use a subsequence beginning with the intersection
		// with the previous element and ending with the intersection with
		// the next element.
		deque<string> route;
		route.clear(); // Clear

		if (elements.size() > 1) {
			for (unsigned int i=1; i<elements.size(); ++i) {
				string intersect_ab = "";
				string intersect_bc = "";
				int index_a = -1;
				int index_b = -1;
				int index_b2 = -1;
				int index_c = -1;

				// determine the intersection (start) of elements i-1 and i
				get_intersection(elements.at(i-1), elements.at(i), waypoints,
								 &intersect_ab, &index_a, &index_b);

				// intersection found. trim the end of element i-1 sequence
				// up to index_a. otherwise, use entire element i-1 sequence.
				if (index_a >= 0) {
					elements.at(i-1).sequence.erase(elements.at(i-1).sequence.begin()+index_a,
													elements.at(i-1).sequence.end());
				}

				// intersection found. trim beginning of element i sequence up
				// to index_b.
				if (index_b >= 0) {
					elements.at(i).sequence.erase(elements.at(i).sequence.begin(),
												  elements.at(i).sequence.begin()+index_b);
				}

				// add the elements i-1 sequence to the route sequence
				route.insert(route.end(),
							 elements.at(i-1).sequence.begin(),
							 elements.at(i-1).sequence.end());
			} // end - for loop

			// add the last element sequence to the route sequence
			route.insert(route.end(),
						 elements.back().sequence.begin(),
						 elements.back().sequence.end());

		} else if (elements.size() == 1) {
			// if there is only 1 element then we use the whole sequence
			route.insert(route.end(),
						 elements.back().sequence.begin(),
						 elements.back().sequence.end());
		}

		// remove any consecutive duplicates from the route sequence
		deque<string>::iterator uiter = unique(route.begin(), route.end());
		size_t newsize = uiter-route.begin();
		route.resize( newsize );

#if DEBUG_PARSE2
	cout << endl;
	cout << "route:";
	for (unsigned int i=0; i<route.size(); ++i) {
		cout << " " << route.at(i) << " ";
	}
	cout << "\n" << endl;
#endif

		// iterate over the route tokens and compute the lat/lon positions
		// of each point. store these to the output flight plan object
		bool enroute_flag = false;
		bool climb_flag = true;
		bool descent_flag = false;
		bool approach_flag = false;

		if (first_enrt_wp == "" && last_enrt_wp == ""){
			enroute_flag = false;
			climb_flag = false;
			if (star){
				descent_flag = true;
			}
			else if (approach){
				descent_flag = false;
				approach_flag = true;
			}
			else{
				descent_flag = false;
				approach_flag = false;
			}
		}

		if (
				enroute_flag == false
			  && climb_flag == false
			  && descent_flag == false
			  && approach_flag == false
			  && route.size() <= 3


			){
			string arr_runway = route.at(1);
			if (
					arr_runway.substr(0,2) == "RW"
					&& isInteger(arr_runway.substr(2,2))
					){
				route.at(1) = route.at(1)+"-"+destination;
			}

		}

		for (unsigned int i = 1; i < route.size()-1; ++i) {

			string path_n_terminator = "";
			string alt_desc = "NONE";
			double alt_1 = -10000;
			double alt_2 = -10000;
			double speed_lim = -10000;
			//New adders to WGS84.
			string wp = route.at(i);
			string procname = "";
			string proctype = "";

			string recco_navaid="NONE";
			double theta=-1000;
			double rho=-1000;
			double mag_course=-1000;
			double rt_dist=-1000;
			string spdlim_desc="NONE";




			size_t slash_pos = wp.find_first_of("/");

			size_t dash_pos = wp.find_first_of("-");

			if (climb_flag && sid) {
				bool found_flag = false;

				for (size_t k = 0; k < sid_leg.size(); ++k) {
					proctype = "SID";
					procname = sid->name;

					vector<string> wplist = sid->wp_map[sid_leg.at(k)];
					vector<string>::iterator it = find(wplist.begin(),
							wplist.end(), wp);

					//CONSTRAINTS
					vector<pair<string,string> > path_term_v = sid->path_term[sid_leg.at(k)];
					vector<pair<string,string> > alt_desc_v = sid->alt_desc[sid_leg.at(k)];
					vector<pair<string,double> > alt_1_v = sid->alt_1[sid_leg.at(k)];
					vector<pair<string,double> > alt_2_v = sid->alt_2[sid_leg.at(k)];
					vector<pair<string,double> > spd_limit_v = sid->spd_limit[sid_leg.at(k)];

					//FOR NATS
					vector<pair<string,string> > recco_navaid_v = sid->recco_nav[sid_leg.at(k)];
					vector<pair<string,double> > theta_v = sid->theta[sid_leg.at(k)];
					vector<pair<string,double> > rho_v = sid->rho[sid_leg.at(k)];
					vector<pair<string,double> > mag_course_v = sid->mag_course[sid_leg.at(k)];
					vector<pair<string,double> > rt_dist_v = sid->rt_dist[sid_leg.at(k)];
					vector<pair<string,string> > spdlim_desc_v = sid->spdlim_desc[sid_leg.at(k)];

					if (it != wplist.end()) {
						int idx = it - wplist.begin();

						//YOU NEED TO CHECK THE PATH TERM OF THE PREVIOUS LEG

						if (!tmpFp.route.empty()){
							string prevpt = tmpFp.route.back().path_n_terminator;
							if (prevpt == "VI"
								|| prevpt == "CI"
								|| prevpt == "VM"
								){
								if ( path_term_v[idx].second == "CF"
									|| path_term_v[idx].second == "DF"){

								}
								else{
									continue;
								}

							}
						}


						wp = *it;
						//MOLEN 8 from RW10 in SFO
						if (i == 1){
							//First point
							if (
								initial_target.substr(0,2) == "RW"
								&& isInteger(initial_target.substr(2, 2))
								) {

								//If the initial target point is the runway
								if (!is_rwy_found(wp)){
									//No runway found in the first wp.
									//Then see if the leg has runway as a transition point
									//If the leg has runway as a transition point then
									// replace the current waypoint with the initial target waypoint
									string trans_ =
											sid->route_to_trans_rttype[sid_leg.at(k)].first;
									if (initial_target.substr(0,4) ==
											trans_.substr(0,4)){
										//create acceptable format for a runway waypoint.
										wp = initial_target+"-"+origin;
										dash_pos = wp.find_first_of("-");
									}//if (initial_target.substr(0,4) ==
								}//if (!is_rwy_found(wp)){

							}//if (	initial_target.substr(0,2) == "RW"

							if (wp.find("ALL") != string::npos
									&& is_rwy_found(wp)){
								if (departure_runway != ""){
									if (
											departure_runway.substr(0,2) == "RW"
												&& isInteger(departure_runway.substr(2, 2))
												) {
										wp = departure_runway+'-'+origin;
									}

								}
							}
						}

						path_n_terminator = path_term_v[idx].second;
						alt_desc = alt_desc_v[idx].second;
						alt_1 = alt_1_v[idx].second;
						alt_2 = alt_2_v[idx].second;
						speed_lim = spd_limit_v[idx].second;

						//NATS ADDER
						recco_navaid = recco_navaid_v[idx].second;
						theta = theta_v[idx].second;
						rho = rho_v[idx].second;
						mag_course = mag_course_v[idx].second;
						rt_dist = rt_dist_v[idx].second;
						spdlim_desc = spdlim_desc_v[idx].second;

						found_flag = true;

						break;
					}

				}
				//CANNOT HANDLE MANUAL LEGS RIGHT NOW.
				//IF FIRST POINT THEN RUNWAY IS ASSIGNED
				if (path_n_terminator == "VM" && i > 1){
					continue;
				}

				if (!found_flag) {
					climb_flag = false;
					enroute_flag = true;
					proctype = "ENROUTE";
				}
			} // end - climb_flag && sid

			if (wp == first_enrt_wp) {
				climb_flag = false;
				enroute_flag = true;
				if (!sid)
					proctype = "ENROUTE";
			}

			if (enroute_flag) {
				if (wp != first_enrt_wp){
					proctype = "ENROUTE";
				}
			}

			if (wp == last_enrt_wp){
				enroute_flag = false;
				descent_flag = true;
			}

			if (descent_flag && star && wp != last_enrt_wp) {

				bool found_flag = false;
				for (size_t k=0; k<star_leg.size(); ++k) {
					proctype = "STAR";
					procname = star->name;

					vector<string> wplist = star->wp_map[star_leg.at(k)];
					vector<string>::iterator it = find(wplist.begin(),wplist.end(),
							wp);

					//CONSTRAINTS
					vector<pair<string,string> > path_term_v = star->path_term[star_leg.at(k)];
					vector<pair<string,string> > alt_desc_v = star->alt_desc[star_leg.at(k)];
					vector<pair<string,double> > alt_1_v = star->alt_1[star_leg.at(k)];
					vector<pair<string,double> > alt_2_v = star->alt_2[star_leg.at(k)];
					vector<pair<string,double> > spd_limit_v = star->spd_limit[star_leg.at(k)];

					//FOR NATS
					vector<pair<string,string> > recco_navaid_v = star->recco_nav[star_leg.at(k)];
					vector<pair<string,double> > theta_v = star->theta[star_leg.at(k)];
					vector<pair<string,double> > rho_v = star->rho[star_leg.at(k)];
					vector<pair<string,double> > mag_course_v = star->mag_course[star_leg.at(k)];
					vector<pair<string,double> > rt_dist_v = star->rt_dist[star_leg.at(k)];
					vector<pair<string,string> > spdlim_desc_v = star->spdlim_desc[star_leg.at(k)];

					if (it != wplist.end()) {
						int idx = it- wplist.begin();

						wp = *it;
						path_n_terminator = path_term_v[idx].second;
						alt_desc = alt_desc_v[idx].second;
						alt_1 = alt_1_v[idx].second;
						alt_2 = alt_2_v[idx].second;
						speed_lim = spd_limit_v[idx].second;

						//NATS ADDER
						recco_navaid = recco_navaid_v[idx].second;
						theta = theta_v[idx].second;
						rho = rho_v[idx].second;
						mag_course = mag_course_v[idx].second;
						rt_dist = rt_dist_v[idx].second;
						spdlim_desc = spdlim_desc_v[idx].second;

						found_flag = true;

						break;
					}
				}

				if (!found_flag && slash_pos >= wp.length() ) {
					descent_flag = false;
					approach_flag = true;
				}
			} // end - descent_flag && star

			if (approach_flag && approach) {
				bool found_flag = false;
				for (size_t k=0; k<app_leg.size(); ++k) {
					proctype = "APPROACH";
					procname = approach->name;
					vector<string> wplist = approach->wp_map[app_leg.at(k)];
					vector<string>::iterator it = find(wplist.begin(),wplist.end(),wp);

					//CONSTRAINTS
					vector<pair<string,string> > path_term_v = approach->path_term[app_leg.at(k)];
					vector<pair<string,string> > alt_desc_v = approach->alt_desc[app_leg.at(k)];
					vector<pair<string,double> > alt_1_v = approach->alt_1[app_leg.at(k)];
					vector<pair<string,double> > alt_2_v = approach->alt_2[app_leg.at(k)];
					vector<pair<string,double> > spd_limit_v = approach->spd_limit[app_leg.at(k)];

					//FOR NATS
					vector<pair<string,string> > recco_navaid_v = approach->recco_nav[app_leg.at(k)];
					vector<pair<string,double> > theta_v = approach->theta[app_leg.at(k)];
					vector<pair<string,double> > rho_v = approach->rho[app_leg.at(k)];
					vector<pair<string,double> > mag_course_v = approach->mag_course[app_leg.at(k)];
					vector<pair<string,double> > rt_dist_v = approach->rt_dist[app_leg.at(k)];
					vector<pair<string,string> > spdlim_desc_v = approach->spdlim_desc[app_leg.at(k)];

					if (it != wplist.end()) {
						wp = *it;

						int idx = it- wplist.begin();
						path_n_terminator = path_term_v[idx].second;
						alt_desc = alt_desc_v[idx].second;
						alt_1 = alt_1_v[idx].second;
						alt_2 = alt_2_v[idx].second;
						speed_lim = spd_limit_v[idx].second;

						recco_navaid = recco_navaid_v[idx].second;
						theta = theta_v[idx].second;
						rho = rho_v[idx].second;
						mag_course = mag_course_v[idx].second;
						rt_dist = rt_dist_v[idx].second;
						spdlim_desc = spdlim_desc_v[idx].second;
						found_flag = true;

						break;
					}
				}

				if (!found_flag) {
					if (destination == wp
						|| destination == "K"+wp
						|| destination == "P"+wp
						|| "K" + destination == wp
						|| "P" + destination == wp
						) {
					}
					else {
						cerr << "Waypoint after approach " << wp << endl;
					}
				}

			} // end - approach_flag && approach
			//PARIKSHIT ADDER TILL HERE


			if (wp.length() <= 5 ||
					(dash_pos < wp.length() ) ) {
				// wp is a named waypoint or a runway specification:
				// Find waypoint for waypoint database
				// check if it is a runway transition.
				// If runway transition then remove "-NO_WP_NAME" string
				// Put the appropriate airport name after runway

				size_t find_str = wp.find("-NO_WP_NAME");
				if (find_str != string::npos){
					removestr(wp,"-NO_WP_NAME");
				}

				if ((dash_pos < wp.length()
							&& wp.substr(0,2) == "RW")){
					if (wp.compare(4,1,"B") == 0){
						wp = departure_runway + "-" + origin;
					}

				}

				if (wp.find(arrival_runway) != string::npos && arrival_runway != "") {
					proctype = "APPROACH";
				}

				// wp is a named waypoint 3, 4, or 5 characters
				double fixLat=-999999;
				double fixLon=-999999;

				NatsWaypoint wpKey;
				wpKey.name = wp;

				vector<NatsWaypoint>::const_iterator wpIter =
						find(waypoints.begin(),
								    waypoints.end(),
								    wpKey);
				if (wpIter != waypoints.end() && wpIter->name == wpKey.name) {
					fixLat = wpIter->latitude;
					fixLon = wpIter->longitude;

				} else {
					// note: airport codes in Airports.crypt are inconsistent.
					// some start with K (like KSFO not SFO), others don't
					// start with K (like LHB not KLHB). we'll try both.
					// typically if an airport has a number in the code then
					// id doesn't usually prefix with K in the file.  try
					// the unprefixed code first.
					int fixlen = wp.length();
					NatsAirport apKey;
					apKey.code = wp;
					vector<NatsAirport>::const_iterator apIter =
							lower_bound(airports.begin(),
									    airports.end(),
									    apKey);

					if (apIter != airports.end() && apIter->code == apKey.code) {
						fixLat = apIter->latitude;
						fixLon = apIter->longitude;
					} else if(fixlen < 4) {
						apKey.code = (fixlen < 4 ? "K"+wp : wp);
						apIter = lower_bound(airports.begin(),
										     airports.end(),
										     apKey);
						if (apIter != airports.end() && apIter->code == apKey.code) {
							fixLat = apIter->latitude;
							fixLon = apIter->longitude;
						}
					}
				}

				if (fixLat==-999999 || fixLon==-999999) {
					// fix not found in g_waypoints or g_airports. ignore it.
					cout << "	Fix not found: " << wp << endl;
					continue;
				}

				tmpFp.route.push_back(PointWGS84(fixLat, fixLon));
				tmpFp.route.back().path_n_terminator = path_n_terminator;
				tmpFp.route.back().alt_desc = alt_desc;
				tmpFp.route.back().alt_1 = alt_1;
				tmpFp.route.back().alt_2 = alt_2;
				tmpFp.route.back().speed_lim = speed_lim;
				tmpFp.route.back().wpname = wp;
				tmpFp.route.back().procname = procname;
				tmpFp.route.back().proctype = proctype;

				tmpFp.route.back().recco_navaid = recco_navaid;
				tmpFp.route.back().theta = theta;
				tmpFp.route.back().rho = rho;
				tmpFp.route.back().mag_course = mag_course;
				tmpFp.route.back().rt_dist = rt_dist;
				tmpFp.route.back().spdlim_desc = spdlim_desc;
				tmpFp.route.back().wp_cat = 1;

				if (wp == last_enrt_wp) {
					enroute_flag = false;
					descent_flag = true;
				}
			} else if (slash_pos < wp.length()) {
				// wp is a lat/lon specification:
				// we want decimal degrees, north positive, east positive.
				// the TRX file usually gives ddmmN/dddmmW, north and west positive.
				// directions may or may not be specified if no direction specified
				// we assume north positive and west positive.
				string latstr = wp.substr(0, slash_pos);
				string lonstr = wp.substr(slash_pos+1);

				int lat_ddmm = atoi(latstr.c_str());
				int lon_dddmm = atoi(lonstr.c_str());

				int lat_ddmmss;
				int lon_ddmmss;

				ostringstream tmpSS;

				tmpSS.str("");
				tmpSS << lat_ddmm;

				// Check latitude string digits
				if (tmpSS.str().length() < 6) {
					lat_ddmmss = lat_ddmm * 100; // Prepare data for DDMMSS
				}

				tmpSS.str("");
				tmpSS << lon_dddmm;

				// Check longitude string digits
				if (tmpSS.str().length() < 6) {
					lon_ddmmss = lon_dddmm * 100; // Prepare data for DDMMSS
				}

				double val_lat_dd = floor(lat_ddmmss / 10000.);
				double val_lon_dd = floor(lon_ddmmss / 10000.);

				double val_lat_mm = floor((lat_ddmmss - val_lat_dd * 10000) / 100) / 60;
				double val_lon_mm = floor((lon_ddmmss - val_lon_dd * 10000) / 100) / 60;

				double val_lat_ss = (lat_ddmmss % 100) / 3600;
				double val_lon_ss = (lon_ddmmss % 100) / 3600;

				double lat = val_lat_dd + val_lat_mm + val_lat_ss;
				double lon = val_lon_dd + val_lon_mm + val_lon_ss;

				// check if the string contains directions (N,S,E,W)
				size_t s_pos = wp.find_first_of("S");
				size_t e_pos = wp.find_first_of("E");

				// if east is not explicitly specified or west is explicitly
				// specified then convert lon to east positive.
				if (e_pos >= wp.length()) {
					lon = -lon;
				}

				// if south is explicitly specified then convert lat to
				// north positive.
				if (s_pos < wp.length()) {
					lat = -lat;
				}

	            tmpFp.route.push_back(PointWGS84(lat, lon));
				tmpFp.route.back().path_n_terminator = path_n_terminator;
				tmpFp.route.back().alt_desc = alt_desc;
				tmpFp.route.back().alt_1 = alt_1;
				tmpFp.route.back().alt_2 = alt_2;
				tmpFp.route.back().speed_lim = speed_lim;
				tmpFp.route.back().wpname = wp;
				tmpFp.route.back().procname = procname;
				tmpFp.route.back().proctype = proctype;

				tmpFp.route.back().recco_navaid = recco_navaid;
				tmpFp.route.back().theta = theta;
				tmpFp.route.back().rho = rho;
				tmpFp.route.back().mag_course = mag_course;
				tmpFp.route.back().rt_dist = rt_dist;
				tmpFp.route.back().spdlim_desc = spdlim_desc;
				tmpFp.route.back().wp_cat = 2;
			} else {
				// wp is a fix+bearing+distance
				//  parse out the fix, radial bearing, and dist. components
				//  lookup the fix in g_waypoints
				//  if fix not found in g_waypoints, try g_airports
				//  if fix still not found, drop it from flight plan, otherwise
				//  if fix is found then compute lat/lon and push to lat/lon vectors
				int fixlen = wp.length() - 6;
				string fix = wp.substr(0, fixlen);
				string bearingStr = wp.substr(fixlen, 3);
				string distStr = wp.substr(fixlen+3, 3);
				double bearing = atof(bearingStr.c_str()) * M_PI/180.; // radians
				double dist = atof(distStr.c_str()); // miles

				double fixLat = -999999;
				double fixLon = -999999;
				NatsWaypoint wpKey;
				wpKey.name = fix;
				wp = fix;
				vector<NatsWaypoint>::const_iterator wpIter =
						lower_bound(waypoints.begin(),
								    waypoints.end(),
								    wpKey);

				if (wpIter != waypoints.end() && wpIter->name == wpKey.name) {
					fixLat = wpIter->latitude;
					fixLon = wpIter->longitude;
				} else {
					// note: airport codes in Airports.crypt are 4-letter codes
					// starting with K, for example KSFO as opposed to SFO
					NatsAirport apKey;
					apKey.code = (fixlen < 4 ? "K"+fix : fix);
					vector<NatsAirport>::const_iterator apIter =
							lower_bound(airports.begin(),
									airports.end(),
									apKey);
					if (apIter != airports.end() && apIter->code == apKey.code) {
						fixLat = apIter->latitude;
						fixLon = apIter->longitude;
					}
				}

				if (fixLat==-999999 || fixLon==-999999) {
					// fix not found in g_waypoints or g_airports. ignore it.
					cout << "	Radial fix not found: " << fix << endl;

					continue;
				}

				// fix lat/lon from degrees to rad
				double fixLatRad = fixLat*M_PI/180.;
				double milesToFeet = 5280;
				double radiusEarthFeet = 20925524.9;
				double rangeAngleRad = dist * milesToFeet / radiusEarthFeet;
				double latRad = asin(sin(fixLatRad) * cos(rangeAngleRad) +
						cos(fixLatRad) * sin(rangeAngleRad) *
						cos(bearing));
				double latDeg = latRad*180/M_PI;
				double lonDeg = fixLon + asin(sin(rangeAngleRad) * sin(bearing) /
						cos(latRad))*180/M_PI;

				tmpFp.route.push_back(PointWGS84(latDeg, lonDeg));
				tmpFp.route.back().path_n_terminator = path_n_terminator;
				tmpFp.route.back().alt_desc = alt_desc;
				tmpFp.route.back().alt_1 = alt_1;
				tmpFp.route.back().alt_2 = alt_2;
				tmpFp.route.back().speed_lim = speed_lim;
				tmpFp.route.back().wpname = wp;
				tmpFp.route.back().procname = procname;
				tmpFp.route.back().proctype = proctype;

				tmpFp.route.back().recco_navaid = recco_navaid;
				tmpFp.route.back().theta = theta;
				tmpFp.route.back().rho = rho;
				tmpFp.route.back().mag_course = mag_course;
				tmpFp.route.back().spdlim_desc = spdlim_desc;
				tmpFp.route.back().rt_dist = rt_dist;
			}
		} // end - Loop of route-- for (unsigned int i = 1; i < route.size()-1; ++i) {

		if (route.size() > 2) {

			bool only_enroute_flag = true;
			for (unsigned int i=1; i<tmpFp.route.size()-1; ++i) {

				if (tmpFp.route.at(i).proctype != "ENROUTE"){
					 only_enroute_flag = false;
					 break;
				 }
			}

			if (only_enroute_flag){
				//IF THE FLIGHT PLAN CONTAINS ONLY ENROUTE POINTS THEN ADD
				// POINT ON THE SURFACE FOR THE ARRIVAL AIRPORT

				NatsAirport dest_ap;
				dest_ap.code =destination;

				vector<NatsAirport>::const_iterator itap =
						find(airports.begin(), airports.end(), dest_ap);

				if (itap != airports.end()){

					tmpFp.route.push_back(PointWGS84(itap->latitude, itap->longitude));
					tmpFp.route.back().path_n_terminator = "";
					tmpFp.route.back().alt_desc = "";
					tmpFp.route.back().alt_1 = itap->elevation;
					tmpFp.route.back().alt_2 = -1000;
					tmpFp.route.back().speed_lim = -1000;
					tmpFp.route.back().wpname = itap->code+"_GROUND";
					tmpFp.route.back().procname = "AIRPORT";
					tmpFp.route.back().proctype = "AIRPORT_GROUND";
					tmpFp.route.back().recco_navaid = itap->code;
					tmpFp.route.back().theta = -1000;
					tmpFp.route.back().rho = -1000;
					tmpFp.route.back().mag_course = -1000;
					tmpFp.route.back().spdlim_desc = "";
					tmpFp.route.back().rt_dist = -1000;

				}

			}
		}

#if DEBUG_PARSE2
		cout << "fp:";
		for (unsigned int i=0; i<tmpFp.route.size(); ++i) {
			cout << " [" << tmpFp.route.at(i).wpname << "/"
					<< tmpFp.route.at(i).proctype
					<< setprecision(10)
					<< "/" << tmpFp.route.at(i).latitude
					<< "/" << tmpFp.route.at(i).longitude
					<< "/" << tmpFp.route.at(i).path_n_terminator
					<< "/" << tmpFp.route.at(i).mag_course
					<< "] "
					<< endl;
		}
		cout << endl;
#endif
	}

	if (retValue) {
		if (0 < initial_target.length()) {
			tmpFp.initial_target.assign(initial_target);
		}

		*fpout = tmpFp;
	}

	return retValue;
}

bool FlightPlanParser::parse(const string& acid,
		 const string& fpstr,
		 FlightPlan& fpout,
		 const double altitude,
		 const double cruiseAltitude) {
	bool retValue = true; // Default

	FlightPlan tmpFp;

	string fp = fpstr;

	tmpFp.routeString = fpstr;

	// 1) parse the origin airport
	int firstDot = fp.find_first_of('}.<');

	string jsonString_origin;
	if (fp.find("FP_ROUTE ") != string::npos) {
		jsonString_origin = fp.substr(strlen("FP_ROUTE "), firstDot-1-strlen("FP_ROUTE "));
	} else {
		jsonString_origin = fp.substr(0, firstDot-1);
	}

	string origin_code;
	string origin_name;
	double origin_lat = 0.0;
	double origin_lon = 0.0;
	double origin_alt = 0.0;

	json jsonObj;

	jsonObj = json::parse(jsonString_origin);

	if ((jsonObj.contains("ap_code")) && (jsonObj.contains("ap_name")) && (jsonObj.contains("lat")) && (jsonObj.contains("lon")) && (jsonObj.contains("alt"))) {
		origin_code = jsonObj.at("ap_code").get<string>();
		origin_name = jsonObj.at("ap_name").get<string>();
		origin_lat = convertLatLonString_to_deg(jsonObj.at("lat").get<string>().c_str());
		origin_lon = convertLatLonString_to_deg(jsonObj.at("lon").get<string>().c_str());
		origin_alt = jsonObj.at("alt").get<double>();
	} else {
		printf("Aircraft %s: Origin airport data is not valid.\n", acid.c_str());
		retValue = false;
		return retValue;
	}

	tmpFp.origin.assign(origin_code);
	tmpFp.origin_name.assign(origin_name);
	tmpFp.origin_latitude = origin_lat;
	tmpFp.origin_longitude = origin_lon;
	tmpFp.origin_altitude = origin_alt;

	// 2) parse the destination airport/eta
	int lastDot = fp.find_last_of('>.{');
	string jsonString_destination = fp.substr(lastDot, string::npos);

	string destination_code;
	string destination_name;
	double destination_lat = 0.0;
	double destination_lon = 0.0;
	double destination_alt = 0.0;

	jsonObj = json::parse(jsonString_destination);

	if ((jsonObj.contains("ap_code")) && (jsonObj.contains("ap_name")) && (jsonObj.contains("lat")) && (jsonObj.contains("lon")) && (jsonObj.contains("alt"))) {
		destination_code = jsonObj.at("ap_code").get<string>();
		destination_name = jsonObj.at("ap_name").get<string>();
		destination_lat = convertLatLonString_to_deg(jsonObj.at("lat").get<string>().c_str());
		destination_lon = convertLatLonString_to_deg(jsonObj.at("lon").get<string>().c_str());
		destination_alt = jsonObj.at("alt").get<double>();
	} else {
		printf("Aircraft %s: Destination airport data is not valid.\n", acid.c_str());
		retValue = false;
		return retValue;
	}

	tmpFp.destination.assign(destination_code);
	tmpFp.destination_name.assign(destination_name);
	tmpFp.destination_latitude = destination_lat;
	tmpFp.destination_longitude = destination_lon;
	tmpFp.destination_altitude = destination_alt;

	int secondDot = fp.find_first_of('>.RW<', firstDot+1);
	int thirdDot = fp.find_first_of('>.<', secondDot+1);
	int fourthDot = fp.find_first_of('>.RW<', thirdDot+1);
	int fifthDot = fp.find_first_of('>.<', fourthDot+1);

	string jsonString_taxiPlan_departing = fp.substr(firstDot+1, secondDot-4-(firstDot+1));

	string jsonString_runway_departing = fp.substr(secondDot+1, thirdDot-2-(secondDot+1));

	string jsonString_flightPlan = fp.substr(thirdDot+1, fourthDot-4-(thirdDot+1));

	string jsonString_runway_landing = fp.substr(fourthDot+1, fifthDot-2-(fourthDot+1));

	string jsonString_taxiPlan_landing = fp.substr(fifthDot+1, lastDot-2-(fifthDot+1));

	tmpFp.departing_taxiPlanString.assign(jsonString_taxiPlan_departing);
	tmpFp.landing_taxiPlanString.assign(jsonString_taxiPlan_landing);
	tmpFp.flightPlanString.assign(jsonString_flightPlan);
	tmpFp.departing_runwayString.assign(jsonString_runway_departing);
	tmpFp.landing_runwayString.assign(jsonString_runway_landing);

	// =========================================================================

	vector<PointWGS84> vector_flight_plan;

	bool flag_plan_validity;

	string tmp_plan_str;
	string tmpValue_WpName;
	double tmpValue_Latitude;
	double tmpValue_Longitude;
	double tmpValue_Altitude;
	string tmpValue_Phase;

	flag_plan_validity = true; // Reset

	// Process flight plan
	tmp_plan_str.assign(tmpFp.flightPlanString);

	trim(tmp_plan_str);
	if (tmp_plan_str.length() > 0) {
		if (tmp_plan_str.find_first_of("{") != 0) {
			printf("             Flight plan is not valid.\n");
			flag_plan_validity = false;

			return false;
		}
		while (tmp_plan_str.find_first_of("{") == 0) {
			tmpValue_WpName.clear(); // Reset
			tmpValue_Latitude = 0.0; // Reset
			tmpValue_Longitude = 0.0; // Reset
			tmpValue_Phase.clear(); // Reset

			int tmpPos_parenthesis_L = tmp_plan_str.find_first_of("{");
			int tmpPos_parenthesis_R = tmp_plan_str.find_first_of("}");
			if ((tmpPos_parenthesis_L < 0) || (tmpPos_parenthesis_R < 0)) {
				printf("             Flight plan is not valid.\n");
				vector_flight_plan.clear();
				flag_plan_validity = false;

				return false;
			}

			string tmpJsonStr = tmp_plan_str.substr(tmpPos_parenthesis_L, (tmpPos_parenthesis_R - tmpPos_parenthesis_L + 1));

			jsonObj = json::parse(tmpJsonStr);

			if (jsonObj.contains("wp_name")) {
				tmpValue_WpName = jsonObj.at("wp_name").get<string>();
			}

			if (jsonObj.contains("lat")) {
				tmpValue_Latitude = convertLatLonString_to_deg(jsonObj.at("lat").get<string>().c_str());
			}
			if (jsonObj.contains("lon")) {
				tmpValue_Longitude = convertLatLonString_to_deg(jsonObj.at("lon").get<string>().c_str());
			}
			if (jsonObj.contains("alt")) {
				tmpValue_Altitude = jsonObj.at("alt").get<double>();
			}

			if (jsonObj.contains("phase")) {
				tmpValue_Phase = jsonObj.at("phase").get<string>();
			}

			if ((tmpValue_WpName.length() > 0) && (tmpValue_Phase.length() > 0)) {
				vector_flight_plan.push_back(PointWGS84());

				vector_flight_plan.back().wpname.assign(tmpValue_WpName);
				vector_flight_plan.back().latitude = tmpValue_Latitude;
				vector_flight_plan.back().longitude = tmpValue_Longitude;
				vector_flight_plan.back().alt = tmpValue_Altitude;
				vector_flight_plan.back().phase.assign(tmpValue_Phase);
			} else {
				printf("Aircraft %s: Airborne flight plan not valid.\n", acid.c_str());

				vector_flight_plan.clear();
				flag_plan_validity = false;

				retValue = false;
				return retValue;
			}

			if (tmp_plan_str.length() > (tmpPos_parenthesis_R+1)) {
				tmp_plan_str.erase(0, tmpPos_parenthesis_R+1);
			} else {
				break;
			}

			if (tmp_plan_str.find_first_of(",")+1 > 0) {
				tmp_plan_str.erase(0, tmp_plan_str.find_first_of(",")+1);
			}

			trim(tmp_plan_str);
			if (tmp_plan_str.find_first_of("{") != 0) {
				printf("             Flight plan is not valid.\n");
				vector_flight_plan.clear();
				flag_plan_validity = false;

				return false;
			}
		}

		tmpFp.route = vector_flight_plan;
	}

	if (retValue) {
		fpout = tmpFp;
	}

	return retValue;
}

#if 0
static void pop_route_element(string& fp_str,
		                      const vector<NatsSid>& sids,
		                      const vector<NatsPar>& pars,
		                      const vector<NatsStar>& stars,
		                      const vector<NatsAirway>& airways,
		                      const vector<NatsWaypoint>& waypoints,
		                      const vector<NatsAirport>& airports,
		                      vector<FpRouteElement>& prevElements,
		                      FpRouteElement* const element) {
	// this function searches the string for route element identifiers
	// starting from the first token.  we process two tokens at a time
	// since the general format of the string is as follows:
	// airport.route.waypoint.route.waypoint ... airport
	// we start with the airports removed.
	// thus, a token pair consists of route.waypoint where the
	// route token indicates a waypoint sequence and the waypoint token
	// indicates the termination point in the sequence. if the termination
	// waypoint token is not found in the route sequence then we use
	// the route sequence to its end point.  the route sequence should
	// start at the termination point of the previous sequence.
	// a route may be a single waypoint
	size_t dot_pos1 = fp_str.find_first_of(".");
	size_t dot_pos2 = fp_str.find_first_of(".", dot_pos1+1);
	string route_token = fp_str.substr(0, dot_pos1);
	string term_token = fp_str.substr(dot_pos1+1, dot_pos2-dot_pos1-1);
	cout << "route_token=" << route_token << ", term_token=" << term_token << ", dot1=" << dot_pos1 << ", dot2=" << dot_pos2 << endl;

	// if the route token is blank or '/' then we
	// use the term token as the output sequence

	// if the route token is not blank or '/' but the term token is
	// blank then we either we take the entire route or the route
	// is actually a single waypoint.

	// if the route token is not blank or '/' and the term token is
	// not blank then we

	// determine if the tokens indicate a SID, STAR, PAR, Airway, or waypoint
	const NatsSid* sid = NULL;
	const NatsPar* par = NULL;
	const NatsStar* star = NULL;
	const NatsAirway* airway = NULL;
	const NatsWaypoint* waypoint = NULL;
	const NatsAirport* airport = NULL;

	// check if SID
	NatsSid sidKey;
	sidKey.id = route_token + "." + term_token;
	vector<NatsSid>::const_iterator sid_iter = lower_bound(sids.begin(),
			                                          sids.end(),
			                                          sidKey);

	// if we found the SID then determine the subsequence up to the
	// termination point and set the output subsequence; remove the
	// route and term tokens then return.
	if(sid_iter != sids.end() && sid_iter->id == sidKey.id) {
		sid = &(*sid_iter);

		vector<string>::const_iterator it = find(sid->waypoints.begin(),
				                                 sid->waypoints.end(),
				                                 term_token.substr(0, term_token.find_first_of("0123456789")));
		element->identifier = (route_token == "" || route_token == "/") ? term_token : route_token;
		element->sequence.insert(element->sequence.end(),
				                 sid->waypoints.begin(), it);
		fp_str = dot_pos2 >= fp_str.length() ? "" : fp_str.substr(dot_pos2+1);
		cout << "sid" << endl;
		return;
	}

	// check if PAR
	NatsPar parKey;
	parKey.identifier = route_token;
	vector<NatsPar>::const_iterator par_iter = lower_bound(pars.begin(),
			                                          pars.end(),
			                                          parKey);

	// if we found the PAR then determine the subsequence from the
	// termination point of the previous route element to the end of
	// the PAR; remove the route and term tokens then return.
	if(par_iter != pars.end() && par_iter->identifier == parKey.identifier) {
		par = &(*par_iter);
		string start_token = prevElements.back().get_terminator();
		vector<string>::const_iterator it = find(par->waypoints.begin(),
				                           par->waypoints.end(),
				                           start_token);
		element->identifier = (route_token == "" || route_token == "/") ? term_token : route_token;
		element->sequence.insert(element->sequence.end(),
				                 it, par->waypoints.end());
		cout << "par" << endl;
		fp_str = dot_pos2 >= fp_str.length() ? "" : fp_str.substr(dot_pos2+1);
		return;
	}

	// check if STAR
	NatsStar starKey;
	starKey.name = route_token;
	vector<NatsStar>::const_iterator star_iter = lower_bound(stars.begin(),
			                                            stars.end(),
			                                            starKey);

	// if we found the STAR then determine the subsequence from the
	// termination point of the previous route element to the end of
	// the STAR; remove the route and term tokens then return.
	if(star_iter != stars.end() && star_iter->name == starKey.name) {
		star = &(*star_iter);
		string start_token = prevElements.back().get_terminator();
		vector<string>::const_iterator it = find(star->waypoints.begin(),
				                           star->waypoints.end(),
				                           start_token);
		element->identifier = (route_token == "" || route_token == "/") ? term_token : route_token;
		element->sequence.insert(element->sequence.end(),
				                 it, star->waypoints.end());
		cout << "star" << endl;
		fp_str = dot_pos2 >= fp_str.length() ? "" : fp_str.substr(dot_pos2+1);
		return;
	}

	// check if Airway
	// the airway may be specified as V27.  here, we parse the
	// number from the name and prefix it with 0 if it is less.
	// than 100.  this is because the airway names in the
	// Airways.crypt file are 4 characters long, ie V027.
	string airwayType = route_token.substr(0,1);
	string airwayNum = route_token.substr(1);
	int airwayInt = atoi(airwayNum.c_str());
	stringstream ss;
	ss << airwayType;
	if(airwayInt < 100) {
		ss << "0";
	}
	ss << airwayNum;
	NatsAirway airwayKey;
	airwayKey.name = ss.str();
	cout << "airway key: " << airwayKey.name << endl;
	cout << "airways.size: " << airways.size() << endl;

	vector<NatsAirway>::const_iterator airway_iter = lower_bound(airways.begin(),
				                                            airways.end(),
				                                            airwayKey);

	// if we found the Airway then determine the subsequence from the
	// termination point of the previous route element to the termination
	// point of this element; remove the route and term tokens then return.
	if(airway_iter != airways.end() && airway_iter->name == airwayKey.name) {
		airway = &(*airway_iter);
		string start_token = prevElements.back().get_terminator();
		vector<string>::const_iterator start_it = find(airway->route.begin(),
				                                 airway->route.end(),
				                                 start_token);
		vector<string>::const_iterator end_it = find(airway->route.begin(),
				                               airway->route.end(),
				                               term_token);
		element->identifier = (route_token == "" || route_token == "/") ? term_token : route_token;
		element->sequence.insert(element->sequence.begin(),
				                 start_it, end_it);
		cout << "airway: sequence size=" << element->sequence.size() << endl;
		fp_str = dot_pos2 >= fp_str.length() ? "" : fp_str.substr(dot_pos2+1);
		return;
	}

	// if the route token is a waypoint then the following term token
	// should be blank.  if the route token is blank or '/' then the term token
	// must not be blank.

	// check if waypoint
	NatsWaypoint waypointKey;
	waypointKey.name = route_token;
	vector<NatsWaypoint>::const_iterator wp_iter = lower_bound(waypoints.begin(),
			                                              waypoints.end(),
			                                              waypointKey);

	// if we found the Waypoint then add it to the output sequence.
	// if there is also a term token specified then add it to the output.
	// remove the route and term tokens then return.
	if(wp_iter != waypoints.end() && wp_iter->name == waypointKey.name) {
		waypoint = &(*wp_iter);
		element->identifier = (route_token == "" || route_token == "/") ? term_token : route_token;
		element->sequence.push_back(route_token);
		cout << "waypoint 1" << endl;
		if(term_token.length() > 0) {
			element->sequence.push_back(term_token);
			cout << "waypoint 2" << endl;
		}
		fp_str = dot_pos2 >= fp_str.length() ? "" : fp_str.substr(dot_pos2+1);
		return;
	}

	// check if airport
	NatsAirport airportKey;
	airportKey.name = route_token;
	vector<NatsAirport>::const_iterator ap_iter = lower_bound(airports.begin(),
			                                                   airports.end(),
			                                                   airportKey);

	// treat airpror like a waypoint
	if(ap_iter != airports.end() && ap_iter->name == airportKey.name) {
		airport = &(*ap_iter);
		element->identifier = (route_token == "" || route_token == "/") ? term_token : route_token;
		element->sequence.push_back(route_token);
		cout << "airport 1" << endl;
		if(term_token.length() > 0) {
			element->sequence.push_back(term_token);
			cout << "airport 2" << endl;
		}
		fp_str = dot_pos2 >= fp_str.length() ? "" : fp_str.substr(dot_pos2+1);
		return;
	}
}

void FlightPlanParser::parse2(const string& fpstr,
	     const vector<NatsSid>& sids,
	     const vector<NatsPar>& pars,
	     const vector<NatsStar>& stars,
	     const vector<NatsAirway>& airways,
	     const vector<NatsWaypoint>& waypoints,
	     const vector<NatsAirport>& airports,
	     FlightPlan* const fpout) {

	vector<FpRouteElement> route_elements;

	// this function parses the flight plan string into a FlightPlan
	// object as follows:
	//   1. Remove and store the first token. its the origin airport
	//   2. Remove and store the last token. its the destination airport
	//
	//   The remaining tokens form the route.
	//
	//   3. Check if the leading tokens indicate a SID. if yes, then
	//      pop the SID tokens from the front, obtain the SID sequence
	//      and add the SID the route
	//   4. Check if the trailing tokens indicate a PAR. if yes, then
	//      pop the PAR tokens from the back, obtain the PAR sequence
	//      and add the PAR to the route. if no, then check if the trailing
	//      tokens indicate a STAR. if yes then pop the STAR tokens from
	//      the back, obtain the STAR sequence and add the STAR to the route


	if(!fpout) return;

	string fp = fpstr;
	fpout->routeString = fpstr;

	// 1) parse the origin airport
	int firstdot = fp.find_first_of(".");
	string origin = fp.substr(0, firstdot);
	fpout->origin = origin;

	// 2) parse the destination airport/eta
	int lastdot = fp.find_last_of(".");
	string destAndEta = fp.substr(lastdot+1, string::npos);
	string destination;
	if(destAndEta.length() == 3) {
		destination = destAndEta;
	} else if(destAndEta.length() == 4) {
		// could be XXX* or KXXX
		if(destAndEta.at(3) == '*') {
			destination = destAndEta.substr(0,3);
		} else {
			destination = destAndEta.substr(1);
		}
	} else {
		// destintation and eta: [airport]/eta
		int pos = destAndEta.find_first_of("/");
		destination = destAndEta.substr(0, pos);
	}
	fpout->destination = destination;

	// we will now work a copy with origin/destination removed.
	// also remove any leading/trailing '.' or '/' characters.
	// the substring may start with / to indicate that there is
	// no departure route
	int sublen = lastdot - firstdot;
	string sub = fp.substr(firstdot+1, sublen);
	if(sub.length() > 0) {
		size_t start_pos = sub.find_first_not_of(".");
		size_t end_pos = sub.find_last_not_of("./");
		sub = sub.substr(start_pos, end_pos+1);
	}

	int count = 0;
	while(sub.length() > 0) {
		cout << "sub=" << sub << endl;
		FpRouteElement new_element;
		pop_route_element(sub, sids, pars, stars, airways, waypoints, airports,
				          route_elements, &new_element);
		route_elements.push_back(new_element);
		count++;
		if(count > 20) break;
	}

	cout << "Route elements:" << endl;
	for(unsigned int i=0; i<route_elements.size(); ++i) {
		cout << " " << route_elements.at(i).identifier << " ";
	}
	cout << endl;

	cout << "Sequences:" << endl;
	for(unsigned int i=0; i<route_elements.size(); ++i) {
		cout << route_elements.at(i).identifier << ":";
		for(unsigned int j=0; j<route_elements.at(i).sequence.size(); ++j) {
			cout << " " << route_elements.at(i).sequence.at(j) << " ";
		}
		cout << endl;
	}

	exit(0);
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Static impl.
////////////////////////////////////////////////////////////////////////////////

static void trim_dots(string& str) {
  // trim leading and trailing dots from the string
  str.erase(0, str.find_first_not_of("."));
  str.erase(str.find_last_not_of(".")+1);
}

// tokenize the string to find a segment that matches a PAR identifier.
// if a match is found then return the pointer to the par and remove
// the PAR identifier sequence from the end of the string.
static NatsPar* parse_par(string& str, const vector<NatsPar>& g_pars) {
	NatsPar* par = NULL;
	string remaining = str;
	unsigned int pos = str.length();
	int ptr=0;
	while(pos > 0 && !par) {
		pos = remaining.find_first_of(".")+1;
		if(pos >= remaining.length()) break;
		remaining = remaining.substr(pos);
		ptr += pos;
		NatsPar key;
		key.identifier = remaining;
		vector<NatsPar>::const_iterator iter = lower_bound(g_pars.begin(),
				g_pars.end(),
				key);
		if(iter != g_pars.end()) {
			if(iter->identifier == remaining) {
				par = const_cast<NatsPar*>(&(*iter));
			}
		}
	}

	if(par) {
		if(str.length() > 0 && ptr > 0) str.erase(ptr-1);
		trim_dots(str);
	}

	return par;
}

// tokenize the string to find a token that matches a STAR identifier.
// if a match is found then return the pointer to the star and remove
// the STAR identifier sequence from the end of the string.
static NatsApproach* parse_approach(string& str,
		const vector<NatsApproach>& g_approaches, const string& ap){

	NatsApproach* approach = NULL;
	string remaining = str;
	unsigned int pos = str.length();
	int ptr=0;
	while(pos > 0 && !approach) {
		pos = remaining.find_first_of(".")+1;
		if(pos >= remaining.length()) break;
		remaining = remaining.substr(pos);
		ptr += pos;

		NatsApproach key;
		key.name = remaining;
		key.id = ap;

		vector<NatsApproach>::const_iterator iter = find(g_approaches.begin(),
						g_approaches.end(),
						key);
		if(iter != g_approaches.end()) {
			if(iter->name == remaining) {
				approach = const_cast<NatsApproach*>(&(*iter));
			}
		}
	}

	if (approach) {
		if (ptr)
			str.erase(ptr-1);
		else
			str.erase();
		trim_dots(str);
	}
#if DEBUG_PARSE2
	else{
		cout << endl;
		cout <<" Substring = " << str<< endl;
		cout << "NO Approaches Found\n";
	}
#endif
	return approach;
}

// tokenize the string to find a token that matches a STAR identifier.
// if a match is found then return the pointer to the star and remove
// the STAR identifier sequence from the end of the string.
static NatsStar* parse_star(string& str, const vector<NatsStar>& g_stars,
		const string& ap) {

	NatsStar* star = NULL;
	string remaining = str;
	unsigned int pos = str.length();
	int ptr=0;
	while(pos > 0 && !star) {
		pos = remaining.find_first_of(".")+1;
		if(pos >= remaining.length()) break;
		remaining = remaining.substr(pos);
		ptr += pos;

		NatsStar key;
		key.name = remaining;
		key.id = ap;
		
		vector<NatsStar>::const_iterator iter = find(g_stars.begin(),
				g_stars.end(),
				key);
		if(iter != g_stars.end()) {
			if(iter->name == remaining) {
				star = const_cast<NatsStar*>(&(*iter));
			}
		}
		else{
			if (isInteger( (key.name).substr(key.name.length()-1,1) ) ){
				(key.name).erase((key.name).length()-1,1);
			}
			else{
				continue;
			}

			size_t len = key.name.length();
			for (iter = g_stars.begin();iter != g_stars.end(); ++iter){
				if ( (strncmp(key.name.c_str(),iter->name.c_str(),len) == 0)
						&& (key.id == iter->id
						|| "K"+key.id == iter->id
						|| "P"+key.id == iter->id

						) ){
					star = const_cast<NatsStar*>(&(*iter));
					break;
				}
			}
		}

	}
	if(star) {
		if (ptr)
			str.erase(ptr-1);
		else
			str.erase();
		trim_dots(str);
	}
#if DEBUG_PARSE2
	else{
		cout <<" Substring = " << str<< endl;
		cout << "NO STARs Found\n";
	}
#endif
	return star;
}


// tokenize the string to find a token that matechs a SID identifier.
// if a match is found then return the pointer to the sid and remove
// the SID identifier sequence from the front of the string.
static NatsSid* parse_sid(string& str, const vector<NatsSid>& g_sids, const string& ap) {


	NatsSid* sid = NULL;
	string remaining = str;
	int pos = remaining.length();
	while(pos > 0 && !sid) {
		pos = remaining.find_last_of(".");
		remaining = remaining.substr(0, pos);

		NatsSid key;
		key.name = remaining;
		key.id = ap;

		vector<NatsSid>::const_iterator iter = find(g_sids.begin(),
				g_sids.end(),
				key);
		if(iter != g_sids.end()) {
			if(iter->name == remaining) {
				sid = const_cast<NatsSid*>(&(*iter));
			}
		}
		else{
			if (isInteger( (key.name).substr(key.name.length()-1,1) ) ){
				(key.name).erase((key.name).length()-1,1);
			}
			else{
				continue;
			}

			size_t len = key.name.length();
			for (iter = g_sids.begin();iter != g_sids.end(); ++iter){
				if ( (strncmp(key.name.c_str(),iter->name.c_str(),len) == 0)
						&& (key.id == iter->id
							|| "K"+key.id == iter->id
							|| "P"+key.id == iter->id
							) ){
					sid = const_cast<NatsSid*>(&(*iter));

					break;
				}
			}
		}
	}

	if (sid) {
		str.erase(0, pos);
		// remove leading dots
		trim_dots(str);
	}
#if DEBUG_PARSE2
	else{
		cout <<" Substring = " << str << endl;
		cout << "NO Sids Found\n\n";
	}
#endif

	return sid;
}

//check whether a NO_WP_NAME waypoint is in the list
static void checkWaypointList(vector<string>& wplist){


	vector<string>::iterator it  = wplist.begin();

	while( it != wplist.end() ){
		size_t found = (*it).find("NO_WP_NAME");
		if (found != string::npos){
			wplist.erase(it);
		}
		else{
			++it;
		}
	}

}


template<typename T>
static void populateElement(T* const proc, const deque<string>& tokens,
		const vector<NatsWaypoint>& waypoints,	const vector<NatsAirport>& airports,
		const string& rwname, const string& initial_target_waypoint,
		FpRouteElement& elem,vector<string>& legs){
	legs.clear();


	elem.identifier = proc->name;
	string curr_wp = "";
	if (tokens.size()){
		if (typeid(NatsSid) == typeid(T)){
			curr_wp = tokens.front();
		}
		else if (typeid(NatsStar) == typeid(T)){
			curr_wp = tokens.back();
		}
	}
	else{
		//NO SID NO ENROUTE ONLY STAR
		curr_wp = initial_target_waypoint;
	}


	double wp_found = false;

	for (size_t k=0;k<proc->waypoints.size();++k){
		if (curr_wp == proc->waypoints.at(k)){
			wp_found = true;
			break;
		}
	}

	// If lat lon waypoint get the nearest waypoint as the next waypoint.
	size_t slash_pos= curr_wp.find_first_of("/");
	if ( slash_pos < curr_wp.length()
			|| (!wp_found) ){

		double latf=0,lonf=0;

		//Search of the it is a lat/lon pair point
		get_lat_lon_point(curr_wp, &latf, &lonf);

		if(!wp_found &&
				slash_pos >= curr_wp.length()
				){
			//Means that the waypoint is not a lat/lon waypoint
			//It is a waypoint that is not in SID/STAR.
			NatsWaypoint tmp_wp; tmp_wp.name = curr_wp;
			vector<NatsWaypoint>::const_iterator it =
					find(waypoints.begin(), waypoints.end(),
									tmp_wp);
			if (it != waypoints.end()){
				latf = it->latitude;
				lonf = it->longitude;
			}
			else{
				NatsAirport tmp_ap; tmp_ap.code = curr_wp;
				vector<NatsAirport>::const_iterator it = find(airports.begin(),
																airports.end(),
																tmp_ap);
				if (it != airports.end()){
					latf = it->latitude;
					lonf = it->longitude;
				}
				else{
					return ;
				}
			}
		}

		double m_dist = 1e10; int idx = -1;
		for (size_t k = 0;k < proc->waypoints.size(); ++k){
			NatsWaypoint tmp_wp; tmp_wp.name = proc->waypoints.at(k);
			vector<NatsWaypoint>::const_iterator it = find(waypoints.begin(),
															waypoints.end(),
															tmp_wp);
			double latt=0,lont=0;
			if (it != waypoints.end()){
				latt = it->latitude;
				lont = it->longitude;
			}
			double dist = compute_distance_gc(latf,  lonf, latt,  lont, 0);
			if ( dist < m_dist){
				idx = (int)k;
				m_dist = dist;
			}
		}
		if (idx >= 0){
			curr_wp = proc->waypoints[(size_t)idx];
		}
		else{
			cerr << "Error no waypoints in procedure " << proc->name << endl;
		}

		//Check whether the waypoint added is at the edge of sid/star
		for (map<string,vector<string> >::iterator it = proc->wp_map.begin();
				it != proc->wp_map.end(); ++it){
			vector<string> wplist = it->second;
			if (wplist.empty()) continue;
			vector<string>::iterator its = find(wplist.begin(),wplist.end(),
					curr_wp);
			if (its != wplist.end()){

				if (typeid(T) == typeid(NatsStar)){
					if (curr_wp != wplist.front()){
						curr_wp = wplist.front();
					}
				}
				else{
					if (curr_wp != wplist.back()){
						curr_wp = wplist.back();
					}
				}
				break;
			}
		}
	}

	if (
		   is_rwy_found(curr_wp)
		&& isInteger(curr_wp.substr(2,2))
		&& rwname !=""
			){

		//If the current waypoint is runway_name
		//then again go through all the waypoints in the sid leg and
		//find the actual leg
		size_t dash_pos = rwname.find_first_of("-");
		int comparison_length = dash_pos;
		if (!isInteger(rwname.substr(comparison_length-1,1)) ){
			comparison_length = comparison_length-1;
		}
		if (strncmp(rwname.c_str(),
				curr_wp.c_str(),
				comparison_length) ){


			//we need to change the curr_wp now
			//as it does not match the runway
			//loop through all the wp_map waypoints and
			// then change the curr_wp to one
			// at the end of a sid leg so that it can
			// be accounted as current waypoint (curr_wp).

			for (map<string,vector<string> >::iterator it = proc->wp_map.begin();
					it != proc->wp_map.end(); ++it){
				vector<string> wplist = it->second;
				if (wplist.empty()) continue;
				string cand_curr_wp = wplist.back(); // for SID. This wont trigger for STAR

				if (strncmp(rwname.c_str(),
						cand_curr_wp.c_str(),
						comparison_length) == 0){
					curr_wp = cand_curr_wp;
					break;
				}

			}//for (map<string,vector<string> >::iterator it = proc->wp_map.begin();
		}//if (strncmp(rwname.c_str(),
	}//	if (  is_rwy_found(curr_wp)

	map<string,vector<string> > wp_map = proc->wp_map;
	map<string,vector<string> >::iterator iter = wp_map.begin();

	// Put waypoints in consecutive legs of the sid/star one after another
	// untill the end of sid/star is reached

	while(iter != wp_map.end()){
		vector<string> wplist = iter->second;

		if (rwname != ""){
			string legname = iter->first;
			if (is_rwy_found(legname) ){
				if (legname.substr(2,3) != "ALL"){
					int comparison_length = rwname.length();
					if (legname.compare(4,1,"B") == 0){ //Indicates both runways possible
						string rw_dir = rwname.substr(4,1);
						legname.replace(4,1,rw_dir);
					}

					if (strncmp(rwname.c_str(),
							legname.c_str(),
							comparison_length)){
						++iter;
						continue;
					}
				}
			}
		}

		if (wplist.empty()) {
			++iter;
			continue;
		}

		string wp_term = wplist.front();

		if (typeid(NatsSid) == typeid(T)){
			wp_term =wplist.back();
		}

		if ( curr_wp.compare(0, curr_wp.size(), wp_term) == 0 ){
			legs.push_back(iter->first);

			//Put Sid legs
			if (typeid(NatsSid) == typeid(T)){
				if (curr_wp == wplist.front() ){
					++iter;
					continue;
				}


				//This will only happen in SID when a leg starts at the middle.
				vector<string>::iterator itiwp = find(wplist.begin(),wplist.end(),
												initial_target_waypoint);

				if (itiwp != wplist.end()){
					elem.sequence.insert(elem.sequence.begin(),
							itiwp, wplist.end());
					curr_wp = (*itiwp);
					break;
				}
				else{
					elem.sequence.insert(elem.sequence.begin(),
								wplist.begin(), wplist.end());
					curr_wp = wplist.front();
				}
			}
			//Put star legs in sequence
			else{
				if (curr_wp == wplist.back() ){
					++iter;
					continue;
				}
				elem.sequence.insert(elem.sequence.end(),
											wplist.begin(), wplist.end());
				curr_wp = wplist.back();
			}
			iter = wp_map.begin();
		}
		else{
			++iter;
		}
	}

	if (elem.sequence.empty()){

		// If still the sequence is empty then it means the curr_wp is in
		// the middle of the leg.
		//Search the leg it is in the middle of and then insert that leg.

		//1) Search the leg
		int pos = 0;
		for(iter = wp_map.begin(); iter != wp_map.end(); ++iter){
			vector<string>::iterator it = find(iter->second.begin(),
					iter->second.end(),curr_wp);
			if (it != iter->second.end()){
				pos = it-iter->second.begin();
				legs.push_back(iter->first);
				break;
			}
		}

		bool break_if_current_leg = false;
		//2)Insert the leg
		if ((iter != wp_map.end()) && (iter->second.size() > 0)) {
			vector<string> wplist = iter->second;

			//Put Sid legs
			if (typeid(NatsSid) == typeid(T)){

				//This will only happen in SID when a leg starts at the middle.
				vector<string>::iterator itiwp = find(wplist.begin(),wplist.end(),
												initial_target_waypoint);
				if (
						( itiwp != wplist.end() )
						&& ( pos >= itiwp-wplist.begin() )
					){


					elem.sequence.insert(elem.sequence.begin(),
							itiwp, wplist.end()-pos);
					curr_wp = (*itiwp);
					break_if_current_leg = true;
				}else{
					elem.sequence.insert(elem.sequence.begin(),
							wplist.begin(),wplist.end()-pos);

					curr_wp = wplist.front();
				}
			}
			//Put star legs in sequence
			else {
				elem.sequence.insert(elem.sequence.end(),
											wplist.begin()+pos, wplist.end());
				curr_wp = wplist.back();
			}

			//3)Insert legs till end.
			iter = wp_map.begin();
			while(iter != wp_map.end() && (!break_if_current_leg) ){
				vector<string> wplist = iter->second;

				//Insert the wplist with runway in it
				if (rwname != ""){

					string legname = iter->first;

					if (is_rwy_found(legname) ){
						if (legname.substr(2,3) != "ALL"){
							int comparison_length = rwname.length();
							if (legname.compare(4,1,"B") == 0){ //Indicates both runways possible
								string rw_dir = rwname.substr(4,1);
								legname.replace(4,1,rw_dir);
							}

							if (strncmp(rwname.c_str(),
									legname.c_str(),
									comparison_length)
									&& legname.substr(2,3) != "ALL"){
								++iter;
								continue;
							}
						}
					}
				}


				if (wplist.empty()) {
					++iter;
					continue;
				}

				string wp_term = wplist.front();

				if (typeid(NatsSid) == typeid(T)){
					wp_term =wplist.back();
				}

				if ( curr_wp.compare(0, curr_wp.size(), wp_term) == 0 ){
					legs.push_back(iter->first);

					//Put Sid legs
					if (typeid(NatsSid) == typeid(T)){
						if (curr_wp == wplist.front() ){
							++iter;
							continue;
						}
						//This will only happen in SID when a leg starts at the middle.
						vector<string>::iterator itiwp = find(wplist.begin(),wplist.end(),
														initial_target_waypoint);

						if (itiwp != wplist.end()){
							elem.sequence.insert(elem.sequence.begin(),
									itiwp, wplist.end());
							curr_wp = (*itiwp);
							break;
						}
						else{
							elem.sequence.insert(elem.sequence.begin(),
										wplist.begin(), wplist.end());
							curr_wp = wplist.front();
						}
					}
					//Put star legs in sequence
					else {
						if (curr_wp == wplist.back() ){
							++iter;
							continue;
						}
						elem.sequence.insert(elem.sequence.end(),
													wplist.begin(), wplist.end());
						curr_wp = wplist.back();
					}

					iter = wp_map.begin();
				}
				else{
					++iter;
				}
			}//			while(iter != wp_map.end() && (!break_if_current_leg) ){
		}//if ((iter != wp_map.end()) && (iter->second.size() > 0)) {
	}//if (elem.sequence.empty()){

	if (!elem.sequence.empty()){
		if (elem.sequence.back() == proc->id
				||"K"+ elem.sequence.back() == proc->id
				|| "P"+ elem.sequence.back() == proc->id
				|| "C"+ elem.sequence.back() == proc->id){
			elem.sequence.pop_back();
		}
		else if(elem.sequence.front() == proc->id
				||"K"+ elem.sequence.front() == proc->id
				|| "P"+ elem.sequence.front() == proc->id
				|| "C"+ elem.sequence.front() == proc->id){
			elem.sequence.erase(elem.sequence.begin());
		}
	}
	else{
		cout <<"ERROR in " << __LINE__ << " in " << __FILE__ << ". "
			  <<" Could not find or connect procedure." << endl;
		return ;
	}

	if (!elem.sequence.empty()) {

		if (rwname != ""){
			if (typeid(NatsSid) == typeid(T)){
				string last_wp = elem.sequence.front();

				if (!is_rwy_found(last_wp)){
					map<string,vector<string> >::iterator it = wp_map.begin();
					for(;it != wp_map.end(); ++it){
						string legname = it->first;
						if (!is_rwy_found(legname))
							continue;
						vector<string>::iterator itr =
								find(legs.begin(),legs.end(),legname);
						if (itr != legs.end()){
							continue;
						}
						int comparison_length = rwname.length();
						if (legname.compare(4,1,"B") == 0){ //Indicates both runways possible
							string rw_dir = rwname.substr(4,1);
							legname.replace(4,1,rw_dir);
						}
						if (strncmp(rwname.c_str(),
								legname.c_str(),
								comparison_length) == 0
								){
							vector<string> wplist = it->second;

							vector<string>::iterator its = find(wplist.begin(),
									wplist.end(),last_wp);
							size_t pos = 0;
							if (its != wplist.end()){
								pos = its-wplist.begin();
							}
							elem.sequence.insert(elem.sequence.begin(),
									wplist.begin(),
									wplist.end()-pos);
							 legs.push_back(it->first);


						}//if (strncmp(rwname.c_str(),

					}//for(;it != wp_map.end(); ++it){
				}//if (!is_rwy_found(last_wp)){

			}//if (typeid(NatsSid) == typeid(T)){

		}//if (rwname != ""){
	}//	if (!elem.sequence.empty()){


}

static void populateApproachElement(NatsApproach* const app,
		const string& star_end,
        const vector<NatsWaypoint>& waypoints,
		FpRouteElement& elem,vector<string>& legs){
	legs.clear();
	elem.sequence.clear();
	elem.identifier = app->name;
	NatsWaypoint star_wp; star_wp.name = star_end;

	vector<NatsWaypoint>::const_iterator iter = find(waypoints.begin(),
			waypoints.end(),star_wp);

	double lat=0,lon=0;
	if (iter != waypoints.end()) {
		lat = iter->latitude;
		lon = iter->longitude;
	} else {
		//it might be a lat/lon waypoint
		//Search of the it is a lat/lon pair point
		get_lat_lon_point(star_end, &lat, &lon);
		if (lat == 0 && lon == 0){
			cerr << "	STAR end not found" << endl;
			return ;
		}
	}

	string ap = app->id;
	map<string,vector<string> >::iterator it  = app->wp_map.begin();


	string head_wp = "";
	bool found_star_end = false;
	// first insert leg with runway
	while (it != app->wp_map.end()){
		vector<string> wplist = it->second;

		vector<string>::iterator its = find_if(wplist.begin(),wplist.end(),
												is_rwy_found);
		if (its != wplist.end() && elem.sequence.empty() ){
			legs.push_back(it->first);

			// Now check if star end is in the wplist
			vector<string>::iterator itstarend = find(wplist.begin(),wplist.end(),
											star_end);
			if (itstarend != wplist.end()){
				elem.sequence.insert(elem.sequence.begin(),
									itstarend, wplist.end());
				found_star_end = true;
				break;
			}
			else{
				//Do the normal thing
				elem.sequence.insert(elem.sequence.begin(),
					wplist.begin(), wplist.end());
				head_wp = wplist.front();
				break;
			}
		}
		++it;
	}

	//next insert leg going from the end of runway leg to the approach boundary
	while(found_star_end){


		it = app->wp_map.begin();
		vector<string> idxlist; idxlist.clear();
		for(; it != app->wp_map.end(); ++it){
			if (it->second.back() == head_wp){
				idxlist.push_back( it->first );
			}

		}

		if (idxlist.size() == 0){
			break;
		}
		else if (idxlist.size() == 1){
			vector<string> wplist = app->wp_map[idxlist[0]];
			legs.push_back(idxlist[0]);
			// Now check if star end is in the wplist
			vector<string>::iterator itstarend = find(wplist.begin(),wplist.end(),
											star_end);
			if (itstarend != wplist.end()){
				elem.sequence.insert(elem.sequence.begin(),
									itstarend, wplist.end());
				found_star_end = true;
				break;
			}
			else{
				elem.sequence.insert(elem.sequence.begin(),
						wplist.begin(), wplist.end());
				head_wp = wplist.front();
			break;
			}
		}
		else {
			size_t numel = idxlist.size();
			bool found = false;
			double min_dist = 1e20;
			int idx = -1;
			for ( size_t s = 0; s < numel ;++s){
				vector<string> wplist = app->wp_map[idxlist[s]];

				NatsWaypoint wp; wp.name = wplist.front();
				vector<string>::iterator itstarend = find(wplist.begin(),
														wplist.end(),
														star_end);
				if (itstarend != wplist.end()){
					legs.push_back(idxlist[s]);
					elem.sequence.insert(elem.sequence.begin(),
										itstarend, wplist.end());
					found = true;
					break;
				}
				else{
					iter = find(waypoints.begin(),
							waypoints.end(),wp);

					double wplat = iter->latitude;
					double wplon = iter->longitude;
					double dist = compute_distance_gc(lat,  lon,  wplat,  wplon, 0);

					if (dist < min_dist){
						min_dist = dist;
						idx = (int)s;
					}
				}
			}

			if (found){
				break;
			}
			else {
				vector<string> wplist = app->wp_map[idxlist[(size_t)idx]];
				legs.push_back(idxlist[(size_t)idx]);
				elem.sequence.insert(elem.sequence.begin(),
						wplist.begin(), wplist.end());
				break;
			}
		}

	}

	//remove everything after approach

	while(true){
		if (!elem.sequence.empty()){
			string wpname = elem.sequence.back();
			if ( is_rwy_found(wpname)){
				break;
			}
			else{
				elem.sequence.pop_back();
			}
		}
		else{
			break;
		}
	}
}
