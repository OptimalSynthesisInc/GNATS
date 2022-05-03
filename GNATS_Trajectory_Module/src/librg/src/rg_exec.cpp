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

/*
 * main.cpp
 *
 *  Created on: Jul 13, 2012
 *      Author: jason
 */

#define USE_LIBSEARCH 0

#define USE_CIFP 1
#define USE_NAMES_IN_TRX 1
#define ULIDEMO 0
#define STARTFROMBETWEEN 1

#include "AirportLayoutDataLoader.h"
#include "pub_WaypointNode.h"

#include "rg_api.h"
#include "rg_exec.h"

#include "osi_global_constants.h"

#include "geometry_utils.h"

#include "SearchPath.h"

#include "TrxInputStream.h"
#include "TrxInputStreamListener.h"
#include "FixPair.h"
#include "Airport.h"

#include "Fix.h"
#include "Airway.h"
#include "Sid.h"
#include "Star.h"
#include "Approach.h"
#include "ProcKey.h"
#include "util_string.h"
#include "wind_api.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <set>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <deque>

#include <cmath>
#include <cstring>
#include <cctype>
#include <algorithm>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <typeinfo>

#include <omp.h>

#include <dirent.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <getopt.h>
#ifdef __cplusplus
}
#endif

using std::map;
using std::vector;
using std::deque;
using std::set;
using std::string;
using std::cerr;
using std::cout;
using std::endl;
using std::fixed;
using std::setprecision;
using std::setw;
using std::stringstream;
using std::ofstream;
using std::ifstream;
using std::ostream;
using std::sort;

namespace osi {
set<string> unames;

// global option values:
static string rg_airways_file = UNSET_STRING;
static string rg_sids_file = UNSET_STRING;
static string rg_stars_file = UNSET_STRING;
static string rg_airports_file = UNSET_STRING;
static string rg_trx_file = UNSET_STRING;
static string rg_mfl_file = UNSET_STRING;
static string rg_rap_file = UNSET_STRING;
static string rg_airports_list_file = UNSET_STRING;
static string rg_polygon_file = UNSET_STRING;
static string rg_origin = UNSET_STRING;
static string rg_destination = UNSET_STRING;
static string rg_out_dir = ".";
static bool rg_use_new_trx = false;
static bool rg_wind_optimal = false;
static bool rg_get_TOS = false;
static vector<double> rg_scale_factors;
static bool rg_reroute = false;
static PolygonLists rg_scenarios;
static PIREPset rg_pirep;
static vector<string> rg_origins;
static vector<string> rg_destinations;

static int rg_num_fix_pairs = 0;
static int rg_num_trajs = 5;
static int rg_tos_cost_index = 0;
static map<string,double> rg_dir_angle_map;
static string rg_CIFPfile = UNSET_STRING;
static string rg_Sigmetfile = UNSET_STRING;
static string rg_PIREPfile = UNSET_STRING;
static string rg_apconffile = UNSET_STRING;
static string rg_airwayconffile = UNSET_STRING;
static string rg_fixconffile = UNSET_STRING;
static string rg_procconffile = UNSET_STRING;

// Always was giving parser error.
static bool rg_use_sigmetfile = true;
static bool rg_use_PIREPfile = true;
#if USE_CIFP
static int rg_sim_start_hr = 0;
#endif
// new addition to fix procedure type in all TOS
static FixedProc rg_fproc = DEFAULT_FIXED_PROC;
static string rg_callsign = UNSET_STRING;
static double rg_airspeed = UNSET_DOUBLE;
static double rg_altitude = UNSET_DOUBLE;

FixedProc fproc_default = DEFAULT_FIXED_PROC;

vector<string> rg_vector_rap_files;
set<string> rg_airportFilter;

SearchGraph rg_graph;

/*
 * IMPORTANT:
 * Setting g_filter_by_input_airports to true is BUGGY.
 *
 * Setting g_filter_by_input_airports true will cause only the
 * Airports found in the input TRX file or Airport Pairs list
 * to be loaded from the NFD airports.xml file. Otherwise, all
 * 13429 airports will be loaded. By limiting the number of
 * airports we reduce the A* search graph size and hence the
 * memory requirements for the route generator. You may need around
 * 8GB of ram to handle a search space with all 13429 airports.
 * Search speed is also improved by the smaller graph size.
 *
 * However, setting g_filter_by_input_airports to true MAY cause
 * search errors due to unfound airport or fix nodes in the
 * g_airports or g_fixes global hash maps if the airport code appears
 * as a waypoint in a SID, STAR, or airway
 */

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
static inline bool isInteger(const string & s)
{
   if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false ;

   char * p ;
   strtol(s.c_str(), &p, 10) ;

   return (*p == 0) ;
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
static inline deque<string> tokenize(const string& str,
		                              const string& delimiter) {
	deque<string> tokens;
	char* saveptr = NULL;
	char* cstr = (char*)calloc(str.length()+1, sizeof(char));
	strcpy(cstr, str.c_str());
	char* token = strtok_r(cstr, delimiter.c_str(), &saveptr);
	if(token) {
		string s(token);

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


static bool rg_filter_by_input_airports = false;

// used to get lat/lon strings for FP_ROUTE line
static inline string convertDegToDDMM(const double& deg, bool lonFlag) {
	// deg is decimal degrees, north positive lat, east positive lon.
	// converts to string DDDMMSS west positive lon, north positive lat
	double dd = (int)deg;
	double fracDeg = deg-dd;
	double min = fabs(fracDeg) * 60;
	double mm = (int)min;

	// notice we make dd negative to convert east positive lon to
	// west positive lon.
	if(lonFlag) dd *= -1.;

	stringstream sstream;

	if(lonFlag) {
		if(fabs(dd) < 10) sstream << "00";
		else if(fabs(dd) >=10 && fabs(dd) < 100) sstream << "0";
	} else {
		if(fabs(dd) < 10) sstream << "0";
	}
	sstream << (int)dd << (fabs(mm) < 10 ? "0" : "") << (int)mm;

	sstream << (lonFlag ? "W" : "N");

	return sstream.str();
}

static inline bool exists_test(const std::string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}
#if USE_CIFP
static inline double RN(double lat){
	return SEMI_MAJOR_EARTH_FT/sqrt(1-ECCENTRICITY_SQ_EARTH*sin(lat*M_PI/180)*sin(lat*M_PI/180) );
}
#endif

static std::vector<std::string> get_h5_files(std::string path = ".") {
	std::vector<std::string> files;
	files.clear();

    DIR*    dir;
    dirent* pdir;

    struct stat path_stat;
    stat(path.c_str(), &path_stat);

    if (S_ISREG(path_stat.st_mode))
    	return files;

    dir = opendir(path.c_str());

    while ((pdir = readdir(dir)) != NULL) {
    	string fname = pdir->d_name;
    	size_t found = fname.find( ".h5" );

    	if (found != string::npos) {
    		size_t found_alt = fname.find( "grid");
    		if (found_alt == string::npos)
    			files.push_back(path+"/"+fname);
    	}
    }

    sort(files.begin(),files.end());

    return files;
}

// used to get lat/lon strings for TRACK line
static inline string convertDegToDDMMSS(const double& deg, bool lonFlag) {
	// deg is decimal degrees, north positive lat, east positive lon.
	// converts to string DDDMMSS west positive lon, north positive lat
	double dd = (int)deg;
	double fracDeg = deg-dd;
	double min = fabs(fracDeg) * 60;
	double mm = (int)min;
	double fracMin = min-mm;
	double sec = fracMin * 60;
	double ss = (int)sec;

	// notice we make dd negative to convert east positive lon to
	// west positive lon.
	if(lonFlag) dd *= -1.;

	stringstream sstream;
	sstream << (fabs(dd) < 10 ? "0" : "") << (int)dd
			<< (fabs(mm) < 10 ? "0" : "") << (int)mm
			<< (fabs(ss) < 10 ? "0" : "") << (int)ss;

	sstream << (lonFlag ? "W" : "N");

	return sstream.str();
}

class TrxWriter : public TrxInputStreamListener {
public:
	TrxWriter(const string& outfile,
			  const ResultSet* paths,
			  const bool windOptimalFlag=false) {
		out.open(outfile.c_str());
		errorLog.open((outfile+".errors.log").c_str());
		this->paths = const_cast<ResultSet*>(paths);
		this->windOptimalFlag = windOptimalFlag;
	}
	virtual ~TrxWriter() {
		out.close();
		errorLog.close();
	}
	void onTrackTime(long trackTime) {
		out << "TRACK_TIME " << trackTime << endl;
	}
	void onTrack(const TrxRecord& trackRecord) {
		// TODO: fix field order in libtrx
		string acid = trackRecord.acid;
		string type = trackRecord.actype;
		string latstr = convertDegToDDMMSS(trackRecord.latitude, false);
		string lonstr = convertDegToDDMMSS(trackRecord.longitude, true);
		stringstream flightLevel;
		flightLevel << (int)(trackRecord.altitude/100);
		stringstream  heading;
		heading << trackRecord.heading;
		stringstream tas;
		tas << trackRecord.tas;
		string center = trackRecord.center;
		string sector = trackRecord.sector;
		string callsign = (windOptimalFlag ? acid : "");
		string route = getRoute(trackRecord.route.origin, trackRecord.route.destination,
				callsign,trackRecord.latitude,trackRecord.longitude);

		// determine the number of generated routes for the flight.
		// if there is more than 1 route for the flight then we create n
		// TRX records for the flight, where n is the number of routes.
		// the first, nominal, route uses the actual acid string.  each
		// additional TRX record for the flight suffixes the acid with _xx,
		// where xx is the nth route (0-based indexing).  Thus, if there are
		// 3 different routes for AAL001 we would have three TRX records
		// with the following acids: AAL001, AAL001_01, AAL001_02.

		if(route != UNSET_STRING) {
			out << "TRACK " << acid << " " << type << " " << latstr << " " << lonstr
				<< " " << tas.str() << " " << flightLevel.str() << " "
				<< heading.str() << " " << center << " " << sector;// << endl;
			if(rg_use_new_trx) {
			    out << " " << (int)floor(trackRecord.cruiseAltitude / 100.);
			}
			out << endl;
			out << "    FP_ROUTE " << route << endl << endl;
		} else {
			errorLog << "TRACK " << acid << " " << type << " " << latstr << " " << lonstr << " " << tas.str() << " " << flightLevel.str() << " " << heading.str() << " " << center << " " << sector << endl;
			errorLog << "    FP_ROUTE_ERROR " << trackRecord.route.origin << " to " << trackRecord.route.destination << endl;
		}
	}

	string getLatitudeStr(const double& latitude) {
	    // convert decimal degrees to ddmmss string, n positive
	    double degrees;
	    double minutes;
	    double minutes_frac;
	    double seconds;
	    double seconds_frac;

	    minutes_frac = modf(fabs(latitude), &degrees) * 60.;
	    seconds_frac = modf(minutes_frac, &minutes) * 60.;
	    seconds = floor(seconds_frac);

	    int deg = (int)degrees;
	    int min = (int)minutes;
	    int sec = (int)seconds;

        stringstream ss;
        ss << setfill('0') << setw(2) << deg << setw(2) << min << setw(2) << sec;
        return ss.str();
	}

	string getLongitudeStr(const double& longitude) {
	    // convert decimal degrees to dddmmss string, w positive
        double degrees;
        double minutes;
        double minutes_frac;
        double seconds;
        double seconds_frac;

        minutes_frac = modf(fabs(longitude), &degrees) * 60.;
        seconds_frac = modf(minutes_frac, &minutes) * 60.;
        seconds = floor(seconds_frac);

        int sign = 0;
        if(longitude > 0) sign = -1;
        else if(longitude < 0) sign = 1;

        int deg = (int)sign * degrees; // negative to convert to w positive
        int min = (int)minutes;
        int sec = (int)seconds;

        stringstream ss;
        ss << setfill('0') << setw(3) << deg << setw(2) << min << setw(2) << sec;
        return ss.str();
	}

	void writeTrackRecord(const string& callsign,
	                      const string& type,
	                      const string& origin,
	                      const string& destination,
	                      const double& latitude,
	                      const double& longitude,
	                      const double& airspeed,
	                      const double& altitude,
	                      const double& heading,
	                      const string& center,
	                      const string& sector,
	                      const string& routeStr="",
	                      const int& mfl=0) {

	    string latstr = getLatitudeStr(latitude);
	    string lonstr = getLongitudeStr(longitude);

	    cout << latitude << "/" << longitude << " -> " << latstr << "/" << lonstr << endl;

	    stringstream tas_ss;
	    tas_ss << airspeed;

	    stringstream fl_ss;
	    fl_ss << altitude/100.;

	    stringstream hdg_ss;
	    hdg_ss << heading;

	    string route = routeStr=="" ? getRoute(origin, destination, callsign,latitude,longitude) : routeStr;

	    out << "TRACK " << callsign << " " << type << " " << latstr << " " << lonstr << " " << tas_ss.str().c_str() << " " << fl_ss.str().c_str() << " " << hdg_ss.str().c_str() << " " << center << " " << sector;// << endl;
	    if (rg_use_new_trx) {
	        out << " " << mfl;
	    }
	    out << endl;
	    out << "    FP_ROUTE " << route << endl << endl;
	}


	string getRoute(const string& origin, const string& destination, const string& callsign,
			double lat_deg=-1000, double lon_deg=-1000) {

		stringstream ss;

		// iterate over the FixPair keys to obtain the paths
		// and print the paths to the outputs
		//for(unsigned int i=0; i<keys.size(); ++i) {

		// form a key and get the value...
		// if key not found, then perhaps the origin/destination
		// airport ids need to be prefixed with 'K'.
		// try the following combinations:
		// K+origin, destination
		// origin, K+destination
		// K+origin, K+destination
		// until one of them gives a valid key.
		// if none give a valid key then log an error and return
		// an unset string.
		FixPair key(origin, destination, callsign);// = keys.at(i);
		SearchPath path;
		int err = get_result_value(paths, key, &path);
		if(err < 0) {
			key.origin = "K"+origin;
			err = get_result_value(paths, key, &path);
			if(err < 0) {
				key.origin = origin;
				key.destination = "K"+destination;
				err = get_result_value(paths, key, &path);
				if(err < 0) {
					key.origin = "K"+origin;
					err = get_result_value(paths, key, &path);
					if(err < 0) {
						// not found
						return UNSET_STRING;
					}
				}
			}
		}

		int numPoints = path.size();
		//get the index of the nearest point to the
		//lat lon position

		int curr_pos_index = -1;
#if STARTFROMBETWEEN
		double min_dist = MAX_DOUBLE;
		if (lat_deg >-1000 && lon_deg >-1000 ){
			for( int k=0; k < numPoints; ++k ){
				int id = path[k];
				double latitude=0,longitude=0;
				get_fix_location(id, &latitude, &longitude);
				double dist = compute_distance_gc(lat_deg,lon_deg,latitude,longitude);
				if (dist < min_dist){
					min_dist = dist;
					curr_pos_index = k;
				}
			}
		}
#endif
		if (curr_pos_index > 0 ){
			ss << key.origin << "./.";
		}
		else {
			if (path.getSid() == "")
				ss << key.origin << "..";
			else
				ss << key.origin << "." << path.getSid() << ".";
		}

		for(int i=1; i<numPoints-1; ++i) {
			if (i <= curr_pos_index)
				continue;
			// the path is defined as a sequence of fix ids
			// so we need to obtain the fix lat/lon given the
			// fix id.
			//ORIGINAL CODE
			int id = path[i];//path.at(i)->id;
#if (!USE_NAMES_IN_TRX)
			double latitude, longitude;
			get_fix_location(id, &latitude, &longitude);
			string latstr = convertDegToDDMM(latitude, false);
			string lonstr = convertDegToDDMM(longitude, true);

			ss << latstr << "/" << lonstr;
			if ( i < numPoints - 2 )
				ss << "..";
#else
			//PARIKSHIT IMPLEMENTATION TRIAL
			string fixname;
			get_fix_name(id, &fixname);
			ss << fixname;
			if ( i < numPoints-2)
				ss << "..";
#endif
			}
		if (path.getStar() == ""){
			ss << ".." << key.destination;
		}
		else{
			if (path.getApproach() == ""){
				ss <<  "." << path.getStar() << "." << key.destination;
			}
			else{
				ss <<  "." << path.getStar() << "." << path.getApproach()
				   <<  "." << key.destination;
			}

		}



		return ss.str();
	}

private:
	ofstream out;
	ofstream errorLog;
	ResultSet* paths;
	bool windOptimalFlag;
};

// this version of write_trx takes all flights in an existing TRX file
// and writes the corresponding flights' new generated route to a new
// TRX file.
static void write_trx(const string& infile,
		              const string& outfile,
		              const ResultSet* paths,
		              const bool windOptimalFlag=false) {

	TrxWriter writer(outfile, paths, windOptimalFlag);
	TrxInputStream in(infile);
	in.addTrxInputStreamListener(&writer);
	in.parse();
}

// transfer all flights in an existing trx file and write them
// to a new file using the new paths and mfls from the input mfl file
// using the new trx format that includes mfl
static void write_trx(const string& infile,
                      const string& inmflfile,
                      const string& outfile,
                      const ResultSet* paths,
                      const bool windOptimalFlag=false) {

    TrxWriter writer(outfile, paths, windOptimalFlag);
    TrxInputStream in(infile, inmflfile);
    in.addTrxInputStreamListener(&writer);
    in.parse();
}

static void write_trx(const string& outfile,
                      const string& callsign,
                      const string& origin,
                      const string& destination,
                      SearchPath& path,
                      const bool windOptimalFlag=false,
                      const int& mfl=0) {

    string type = "A320";
    double latitude = rg_airports.at("K"+origin)->latitude;
    double longitude = rg_airports.at("K"+origin)->longitude;
    double altitude = rg_airports.at("K"+origin)->elevation;
    double airspeed = 250;
    double heading = 0;
    string center = "ZZZ";
    string sector = "ZZZZZ";

    stringstream ss;
    ss << ("K"+origin) << "..";

    int numPoints = path.size();
    for(int i=0; i<numPoints; ++i) {

        // the path is defined as a sequence of fix ids
        // so we need to obtain the fix lat/lon given the
        // fix id.
        int id = path[i];//path.at(i)->id;
        double latitude, longitude;
        get_fix_location(id, &latitude, &longitude);

        string latstr = convertDegToDDMM(latitude, false);
        string lonstr = convertDegToDDMM(longitude, true);
        ss << latitude << "/" << longitude;

        ss << "..";
    }
    ss << ("K"+destination);

    TrxWriter writer(outfile, NULL, windOptimalFlag);
    if(rg_use_new_trx) {
        writer.writeTrackRecord(callsign, type, origin, destination,
                                latitude, longitude, altitude, airspeed, heading,
                                center, sector, ss.str(), mfl);
    } else {
        writer.writeTrackRecord(callsign, type, origin, destination,
                                latitude, longitude, altitude, airspeed, heading,
                                center, sector, ss.str());
    }
}


class TrxCallsignExtractor : public TrxInputStreamListener {
public:
	TrxCallsignExtractor() {
	}
	virtual ~TrxCallsignExtractor() {
	}
	void onTrackTime(long trackTime){
		(void)trackTime;
	}
	void onTrack(const TrxRecord& trackRecord){
		acids.insert(trackRecord.acid);
	}
	set<string> acids;
};

static void write_mfl(const string& infile,
		              const string& outfile,
		              const string& trxfile) {

	// load the input mfl file to vector<pair<string, int>>
	vector< std::pair<string,int> > mfls;
	ifstream mflin;
	mflin.open(infile.c_str());
	while(mflin.good()) {
		string line;
		getline(mflin, line);
		if(line.size() < 1) continue;
		deque<string> tokens = tokenize(line, " ");
		if(tokens.size() < 1) continue;
		string callsign = tokens.at(0);
		int fl = atof(tokens.at(1).c_str());
		mfls.push_back( std::pair<string, int>(callsign, fl) );
	}
	mflin.close();

	// open trxfile for reading
	// parse the trx files callsigns into set<string>
	TrxCallsignExtractor callsignExtractor;
	TrxInputStream trxin(trxfile);
	trxin.addTrxInputStreamListener(&callsignExtractor);
	trxin.parse();

	// open output mfl for writing
	// iterate over vector<pair<string, int>>.  if callsign is
	// in the trx set of callsigns then write to outfile
	vector< std::pair<string, int> >::iterator iter;
	ofstream mflout;
	mflout.open(outfile.c_str());
	bool firstline = true;
	for(iter=mfls.begin(); iter!=mfls.end(); ++iter) {
		const std::pair<string, int>* p = &(*iter);
		string acid = p->first;
		int fl = p->second;
		set<string>::iterator found = callsignExtractor.acids.find(acid);
		if(found != callsignExtractor.acids.end()) {
			if(firstline) {
				firstline = false;
			} else {
				mflout << endl;
			}
			mflout << acid << " " << fl;
		}
	}
	mflout.close();
}

static void write_mfl(const string& outfile, const string& callsign, const double& mfl) {
    ofstream out;
    out.open(outfile.c_str());
    out << callsign << " " << mfl;
    out.close();
}


/*
 * Print a search path to the specified output stream
 */
static void print_path(int scenario, double scale,
		        const FixPair& key, const SearchPath& path,
		        ostream& out) {

	out << fixed;


	// iterate over each fix id in the path
	int numPoints = path.size();
	for(int i=0; i<numPoints; ++i) {

		// the path is defined as a sequence of fix ids
		// so we need to obtain the fix lat/lon given the
		// fix id.
		int id = const_cast<SearchPath&>(path)[i];//path.at(i)->id;

		double latitude, longitude;
		//cout << "get_fix_location id=" << id << endl;
		get_fix_location(id, &latitude, &longitude);

		string fname;
		get_fix_name(id,&fname);

		// write to the output stream
		out << setprecision(0) << setw(8) << scenario
		    << setprecision(3) << setw(8) << scale
		    << setw(8) << key.origin
		    << setw(8) << key.destination
			<< setw(8) << fname
		    << setprecision(8) << setw(16) << longitude
		    << setprecision(8) << setw(16) << latitude
		    << endl;
	}
	out << endl;
}

/*
 * Print a polygon to the specified output stream
 */
static void print_polygon(int scenario, double scale, const Polygon& poly,
		           ostream& out) {
	out << fixed;
	int num_points = poly.getNumVertices();

	const double* xdata = poly.getXData();
	const double* ydata = poly.getYData();
	for(int i=0; i<num_points; ++i) {
		out << setprecision(0) << setw(8) << scenario
			<< setprecision(3) << setw(8) << scale
			<< setprecision(8) << setw(16) << xdata[i]
			<< setprecision(8) << setw(16) << ydata[i]
			<< endl;
	}
	out << endl;
}

/*
 * TODO:Parikshit ADDER
 * Print a kml path to the specified output stream
 */

static void kml_write_pireps(ostream& out){

	out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n"
		<< " <Document>"
		<< "  <name>PIREPs</name>"
		<< "         <Style id=\"mynewpin\">\n                <IconStyle>\n"
        << "                <scale>0.5</scale>\n                        <color>ff00ddff</color>\n"
		<< " <Icon>\n                                <href>http://maps.google.com/mapfiles/kml/pushpin/blue-pushpin.png</href>\n"
		<< " </Icon>\n               </IconStyle> \n        </Style> "
		<< "  <Folder>\n";


	for (size_t s =0; s< rg_pirep.size(); ++s){
		string wpname = rg_pirep.at(s).getWaypoint();
		double lon = rg_pirep.at(s).getLongitude();
		double lat = rg_pirep.at(s).getLatitude();

		out << "<Placemark>\n<name>" << wpname <<"</name>"
			<<" <styleUrl>#mynewpin</styleUrl>"
			<< "\n  <Point>\n" << "   <coordinates> "
			<< setprecision(10) << lon <<","
			<< setprecision(10) << lat<<","<<0.0
			<< "</coordinates>\n"
			<< " </Point>\n</Placemark>\n";
	}

	out << "</Folder>\n</Document>\n</kml>";

}

//Splash before kml file
static void kml_splash(ostream& out, const bool& polyFlag = false){

	out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n";

	//FOR TOS GENERATION 5 TRAJS
	if (polyFlag){
		out <<"<Document>\n    <name>Weather Polygons</name>\n    <description/>\n"
			<<" <Style id=\"poly-000000-1-77-nodesc-normal\">\n      <LineStyle>\n"
			<<"       <color>ff000000</color>\n        <width>1</width>\n"
			<<"      </LineStyle>\n      <PolyStyle>\n        <color>4d000000</color>\n"
			<<"        <fill>1</fill>\n        <outline>1</outline>\n      </PolyStyle>\n"
			<<"    </Style>\n    <Style id=\"poly-000000-1-77-nodesc-highlight\">\n"
			<<"      <LineStyle>\n        <color>ff000000</color>\n        <width>2</width>\n"
			<<"      </LineStyle>\n      <PolyStyle>\n        <color>4d000000</color>\n"
			<<"        <fill>1</fill>\n        <outline>1</outline>\n      </PolyStyle>\n"
			<<"    </Style>\n    <StyleMap id=\"SIGMETPoly\">\n      <Pair>\n"
			<<"        <key>normal</key>\n        <styleUrl>#poly-000000-1-77-nodesc-normal</styleUrl>\n"
			<<"      </Pair>\n      <Pair>\n        <key>highlight</key>\n"
			<<"        <styleUrl>#poly-000000-1-77-nodesc-highlight</styleUrl>\n"
			<<"      </Pair>\n    </StyleMap>\n    <Style id=\"poly-E65100-1-77-nodesc-normal\">\n"
			<<"      <LineStyle>\n        <color>ff0051e6</color>\n        <width>1</width>\n"
			<<"      </LineStyle>\n      <PolyStyle>\n        <color>4d0051e6</color>\n"
			<<"        <fill>1</fill>\n        <outline>1</outline>\n      </PolyStyle>\n"
			<<"    </Style>\n    <Style id=\"poly-E65100-1-77-nodesc-highlight\">\n      <LineStyle>\n"
			<<"        <color>ff0051e6</color>\n        <width>2</width>\n      </LineStyle>\n"
			<<"      <PolyStyle>\n        <color>4d0051e6</color>\n        <fill>1</fill>\n"
			<<"        <outline>1</outline>\n      </PolyStyle>\n    </Style>\n    <StyleMap id=\"AIRMETPoly\">\n"
			<<"      <Pair>\n        <key>normal</key>\n        <styleUrl>#poly-E65100-1-77-nodesc-normal</styleUrl>\n"
			<<"      </Pair>\n      <Pair>\n        <key>highlight</key>\n"
			<<"        <styleUrl>#poly-E65100-1-77-nodesc-highlight</styleUrl>\n      </Pair>\n"
			<<"    </StyleMap>\n    <Folder>" << endl;

	}else{
		out	<< "<Document> \n <name>Paths</name>\n<description>Examples of paths. "
			<<"Note that the tessellate tag is by default set to 0."
			<<"If you want to create tessellated lines, they must be authored(or edited) directly in KML.\n</description>\n  "
			<<"<Style id=\"yellowLineGreenPoly1\">\n   <LineStyle>\n  <color>50141414</color>\n"
			<<"<width>2</width>\n</LineStyle>\n</Style>\n"
			<<"<Style id=\"yellowLineGreenPoly2\">\n   <LineStyle>\n  <color>501400FF</color>\n"
			<<"<width>2</width>\n</LineStyle>\n</Style>\n"
			<<"<Style id=\"yellowLineGreenPoly3\">\n   <LineStyle>\n  <color>50143C00</color>\n"
			<<"<width>2</width>\n</LineStyle>\n</Style>\n"
			<<"<Style id=\"yellowLineGreenPoly4\">\n   <LineStyle>\n  <color>5014F000</color>\n"
			<<"<width>2</width>\n</LineStyle>\n</Style>\n"
			<<"<Style id=\"yellowLineGreenPoly5\">\n   <LineStyle>\n  <color>501478AA</color>\n"
			<<"<width>2</width>\n</LineStyle>\n</Style>\n";
	}




}
static void print_kml( const SearchPath& path,ostream& out, const string& origin, const string& dest,
		const int& rt_num = 0) {


	out << fixed;
	out << "<Placemark> \n <name>" << origin << " " << dest << "</name> \n <description>Transparent green wall "
		<< "with yellow outlines</description> \n"
		<< "<styleUrl>#yellowLineGreenPoly" << rt_num + 1 << "</styleUrl> \n "
		<< "<LineString>\n  <extrude>1</extrude>\n <tessellate>1</tessellate>\n"
		<< "<altitudeMode>absolute</altitudeMode>\n<coordinates>\n";

	// iterate over each fix id in the path
	int numPoints = path.size();

	//write airport
	if (numPoints > 1){
		int id = const_cast<SearchPath&>(path)[0];
		double latitude, longitude;

		get_fix_location(id, &latitude, &longitude);

		out << setprecision(10) << longitude<<","
		    << setprecision(10) << latitude
		    << endl;
	}

	/*JUST A PLACEHOLDER.
	 * DO NOT PRINT SIDS AND STAR POINTS IN KML RIGHT NOW.
	 * AS MANY AIRPORTS MAY NOT HAVE SIDS AND STARS.
	 * IF YOU ARE COMPLETELY SURE THAT YOUR AIRPORTS WILL HAVE SIDS AND
	 * STARS THEN GO AHEAD AND PRINTS THEM
	 * */
#if 0
	string sidname = path.getSid();
	string starname = path.getStar();
#else
	string sidname = "";
	string starname = "";
#endif

	//write SID
	if (sidname != ""){
		ProcKey key(sidname, origin);
		map<ProcKey, Sid*>::iterator iter = rg_sids.find(key);
		Sid *sid= NULL;
		if (iter != rg_sids.end()){
			sid = iter->second;
		}

		int id = const_cast<SearchPath&>(path)[1];
		string wpname = rg_fix_names[id];
		vector<string> wplist; wplist.clear();
		string prev_transid = "",transid = "";
		while(true){
			transid = "";
			for (map<string,vector<string> >::iterator it = sid->waypoints.begin();
					it != sid->waypoints.end(); ++it){
				if(it->first == prev_transid) continue;
				vector<string> waypts = it->second;
				if (waypts.size()){
					if (waypts.back() == wpname){
						transid = it->first;
						break;
					}
				}
			}

			wplist.insert(wplist.begin(),sid->waypoints[transid].begin(),sid->waypoints[transid].end());
			size_t found = transid.find(origin);
			if (found != string::npos){
				break;
			}
			else if (transid == "" && wplist.size() > 0){
				break;
			}

			wpname = wplist.front();
			prev_transid = transid;

		}

		for (size_t i=0;i<wplist.size(); ++i){
			size_t found = (wplist.at(i)).find(origin);
			if (found != string::npos){
				continue;
			}

			double latitude, longitude;
			get_fix_location(wplist.at(i), &latitude, &longitude);

			// write to the output stream
			out << setprecision(10) << longitude<<","
			    << setprecision(10) << latitude
			    << endl;
		}


	}
	//Sid till here


	for(int i=1; i<numPoints-1; ++i) {

		// the path is defined as a sequence of fix ids
		// so we need to obtain the fix lat/lon given the
		// fix id.
		int id = const_cast<SearchPath&>(path)[i];//path.at(i)->id;
		double latitude, longitude;

		get_fix_location(id, &latitude, &longitude);

		// write to the output stream
		out << setprecision(10) << longitude<<","
		    << setprecision(10) << latitude
		    << endl;
	}


	//write STAR
	if (starname != ""){
		ProcKey key(starname, dest);
		map<ProcKey, Star*>::iterator iter = rg_stars.find(key);
		Star *star= NULL;
		if (iter != rg_stars.end()){
			star = iter->second;
		}

		int id = const_cast<SearchPath&>(path)[path.size()-2];
		string wpname = rg_fix_names[id];
		vector<string> wplist; wplist.clear();
		string prev_transid = "",transid = "";
		while(true){
			transid = "";
			for (map<string,vector<string> >::iterator it = star->waypoints.begin();
					it != star->waypoints.end(); ++it){
				if(it->first == prev_transid) continue;
				vector<string> waypts = it->second;
				if (waypts.size()){
					if (waypts.front() == wpname){
						transid = it->first;
						break;
					}
				}
			}

			wplist.insert(wplist.end(),star->waypoints[transid].begin(),star->waypoints[transid].end());
			//See different from SID
			wpname = wplist.back();
			size_t found = wpname.find(dest);
			if (found != string::npos){
				break;
			}
			else if (transid == "" && wplist.size() > 0){
				break;
			}


			prev_transid = transid;

		}

		for (size_t i=0;i<wplist.size(); ++i){
			size_t found = (wplist.at(i)).find(dest);
			if (found != string::npos){
				continue;
			}

			double latitude, longitude;
			get_fix_location(wplist.at(i), &latitude, &longitude);

			// write to the output stream
			out << setprecision(10) << longitude<<","
			    << setprecision(10) << latitude
			    << endl;
		}


	}
	//Star till here




	//WRITE DEST
	if (numPoints > 1){
		int id = const_cast<SearchPath&>(path)[path.size()-1];//path.at(i)->id;
		double latitude, longitude;

		get_fix_location(id, &latitude, &longitude);

		out << setprecision(10) << longitude<<","
		    << setprecision(10) << latitude
		    << endl;
	}

	out << "</coordinates>\n </LineString>\n </Placemark>\n";
}

static void print_poly_kml(const Polygon& poly,ostream& out,const int& pnum = 0){
	string polytype = "SIGMETPoly";

	if (strncmp(poly.getPolyType().c_str(),"AIRMET",6)==0)
		polytype = "AIRMETPoly";

	out <<"  <Placemark>\n        <name> " << poly.getPolyType() << " " << pnum+1<<"</name>\n"
		<<"        <styleUrl>#"<<polytype<<"</styleUrl>\n"
		<<"        <Polygon>\n          <outerBoundaryIs>\n            <LinearRing>\n"
		<<"              <tessellate>1</tessellate>\n              <coordinates>"<<endl;

	int num_points = poly.getNumVertices();

	const double* xdata = poly.getXData();
	const double* ydata = poly.getYData();
	for(int i = 0;i<num_points;++i){
		out << setprecision(10) << xdata[i] <<","
			<< setprecision(10) << ydata[i] <<","
			<< 0.0 << endl;
	}

	out <<" /</coordinates>\n </LinearRing>\n  </outerBoundaryIs>\n </Polygon>\n </Placemark>\n";
}

/*
 * * TODO:Parikshit ADDER ENDS
 */


/**
 * Compute the difference in timeval structs in seconds.
 */
static double compute_dt(struct timeval &tv_start, struct timeval &tv_end) {
	double dsec = tv_end.tv_sec - tv_start.tv_sec;
	double dusec = tv_end.tv_usec - tv_start.tv_usec;
	double dt = dsec + (dusec / 1e6);
	return dt;
}

/**
 * Parse the comma separated string of scale factors
 * supplied from the command line.
 */
static void parse_scale_factors(string s) {
	char cstr[256];
	memset(cstr, 0, 256*sizeof(char));
	strcpy(cstr, s.c_str());

	double scale;
	char* token = strtok(cstr, ",");
	if(token) {
		scale = atof(token);
		rg_scale_factors.push_back(scale);
	}
	while(token) {
		token = strtok(NULL, ",");
		if(token) {
			scale = atof(token);
			rg_scale_factors.push_back(scale);
		}
	}

	// sort the scale factors ascending
	sort(rg_scale_factors.begin(), rg_scale_factors.end());
}
static void parse_fixed_procedure(string s){
	if (s!="NONE"){
		deque<string> strval = tokenize(s,",");

		if (strval.size() == 3){
			rg_fproc.airport = strval.at(0);
			rg_fproc.procType = strval.at(1);
			rg_fproc.procName = strval.at(2);
		}

	}
}
/**
 * Parse a line of the polygon file.
 * column 1: longitude
 * column 2: latitude
 * column 3: scenario
 */
static void parse_polygon_line(string line,
		                       int* const scenario,
		                       double* const lat,
		                       double* const lon) {
	char buf[256];
	memset(buf, 0, 256*sizeof(char));
	strcpy(buf, line.c_str());

	char* token = strtok(buf, " ");
	if(token) {
		if(lon) *lon = atof(token);
	}
	token = strtok(NULL, " ");
	if(token) {
		if(lat) *lat = atof(token);
	}
	token = strtok(NULL, " ");
	if(token) {
		if(scenario) *scenario = atoi(token);
	}
}

/**
 * Load the airport pairs into global vectors from the file
 * with the specified name
 */
static void load_airports_list(string f) {
	ifstream in;
	in.open(f.c_str());

	string line;

	rg_num_fix_pairs = 0;
	while(in.good()) {
		getline(in, line);

		char buf[256];
		memset(buf, 0, 256*sizeof(char));
		strcpy(buf, line.c_str());

		string origin = UNSET_STRING;
		string destination = UNSET_STRING;

		char* token = strtok(buf, " ");
		if(token) {
			origin = token;
		}
		token = strtok(NULL, " ");
		if(token) {
			destination = token;
		}

		if((origin != UNSET_STRING) && (destination != UNSET_STRING)) {
			rg_origins.push_back(origin);
			rg_destinations.push_back(destination);
			++rg_num_fix_pairs;
		}
	}
	in.close();
}


/*
 * TODO: PARIKSHIT ADDER FOR DIRECT INPUT FROM SIGMETS.
 * FIND SIGMET INFROMATION AT www.aviationweather.gov.
 *
 */
#if USE_CIFP
static void load_dir_angle_map(){
	  double min_div = 22.5;
	  rg_dir_angle_map["N"] = 0;
	  rg_dir_angle_map["NNE"] = min_div;
	  rg_dir_angle_map["NE"] = 2*min_div;
	  rg_dir_angle_map["ENE"] = 3*min_div;
	  rg_dir_angle_map["E"] = 4*min_div;
	  rg_dir_angle_map["ESE"] = 5*min_div;
	  rg_dir_angle_map["SE"] = 6*min_div;
	  rg_dir_angle_map["SSE"] = 7*min_div;
	  rg_dir_angle_map["S"] = 8*min_div;
	  rg_dir_angle_map["SSW"] = 9*min_div;
	  rg_dir_angle_map["SW"] = 10*min_div;
	  rg_dir_angle_map["WSW"] = 11*min_div;
	  rg_dir_angle_map["W"] = 12*min_div;
	  rg_dir_angle_map["WNW"] = 13*min_div;
	  rg_dir_angle_map["NW"] = 14*min_div;
	  rg_dir_angle_map["NNW"] = 15*min_div;
}
static string getMetDesc(const string& line,bool sigmetFlag = false){

	char *dup = strdup(line.c_str());
	char *tok = strtok(dup,":");
	int count = 0;
	string descstr = "";
	while(tok != NULL){
		if (count == 2){
			string s(tok);
			removeSpaces(s);
			if (sigmetFlag)
				descstr = "SIGMET-"+s;
			else
				descstr = "AIRMET-"+s;
		}

		count++;
		tok = strtok(NULL,":");
	}

	free(dup);
	return descstr;

}
static void readWeatherFile(const string& filename,
		vector< vector<string> > &dir,
		vector< vector<string> > &loc,
		vector< vector<int> > &distv,
		vector<string> &MetDesc,
		vector<pair<int,int> > &sthr_ehr){

	dir.clear();
	loc.clear();
	distv.clear();
	MetDesc.clear();
	vector<string> sigmetStr; sigmetStr.clear();
	vector< pair < pair<int,int>,pair<int,int> > > startEndDateTime;startEndDateTime.clear();

	string line;
	ifstream myfile (filename.c_str());

	string comp_keyword = "FROM";
	string first_pt = "";
	string region="";
	string desc_str = "";
	bool MetStart = false;
	bool MetEnd = true;
	int basenum = 1;
	bool start_assigned = false;
	bool end_assigned = false;
	pair<int,int> start_time = make_pair(00,0000);
	pair <int,int> end_time = make_pair(00,2400);
	bool date_assigned = false;
	int obs_date = 0;
	if (myfile.is_open()){

		while( getline(myfile,line)){

			size_t found = line.find( "Type: AIRMET" );
			if( found != string::npos ){
				MetStart = true;MetEnd = true;
				region = "";
				first_pt = "";
				desc_str = getMetDesc(line,false);
			}
			found = line.find( "Type: SIGMET" );
			if( found != string::npos ){
				MetStart = true;MetEnd = true;
				region = "";
				first_pt = "";
				desc_str = getMetDesc(line,true);
			}

			found = line.find("KKCI");
			if( found != string::npos && rg_wind_vec.size() ){
				char *dup = strdup(line.c_str());
				char *tok = strtok(dup," ");
				while(tok != NULL){
					if (atof(tok) && strlen(tok) >= 6){
						int len = strlen(tok);

						string str1 = string(tok).substr(0,2);
						string str2 = string(tok).substr(len-4,4);
						start_time = make_pair(atoi(str1.c_str()),atoi(str2.c_str()));
						start_assigned = true;
						if (!date_assigned){
							obs_date = atoi(str1.c_str());
							date_assigned = true;
						}

					}
					tok = strtok(NULL," ");
				}
				free(dup);
			}

			found = line.find("VALID UNTIL");
			if ( found != string::npos && rg_wind_vec.size() ){
				char *dup = strdup(line.c_str());
				char *tok = strtok(dup," ");
				while(tok != NULL){
					if( atof(tok)){
						if (strlen(tok)>=6){
							int len = strlen(tok);

							string str1 = string(tok).substr(0,2);
							string str2 = string(tok).substr(len-4,4);
							end_time = make_pair(atoi(str1.c_str()),atoi(str2.c_str()));
							end_assigned = true;
						}
						else if (strlen(tok) >= 5 && start_assigned){
							int end_hr = atoi(tok);
							if (start_time.second > end_hr)
								end_time = make_pair( (start_time.first+1)%31, end_hr);
							else
								end_time = make_pair( start_time.first, end_hr);
							end_assigned = true;
						}
					}
					tok = strtok(NULL," ");
				}
				free(dup);
			}

			found = line.find("OUTLOOK VALID");
			if (found != string::npos && !end_assigned && start_assigned && rg_wind_vec.size()){
				char *dup = strdup(line.c_str());
				char *tok = strtok(dup," ");
				while(tok != NULL){
					if (atof(tok)){
						char *tval = strtok(tok,"-");
						int cnter = 0;
						while(tval != NULL){
							if (strlen(tval)>=6){
								int len = strlen(tval);

								string str1 = string(tval).substr(0,2);
								string str2 = string(tval).substr(len-4,4);
								if (cnter == 0){
									start_time = make_pair(atoi(str1.c_str()),atoi(str2.c_str()));
									start_assigned = true;
								}
								else if (cnter == 1){
									end_time = make_pair(atoi(str1.c_str()),atoi(str2.c_str()));
									end_assigned = true;
								}
							}
							cnter++;
							tval = strtok(NULL,"-");
						}
					}
					tok  = strtok(NULL," ");
				}
				free(dup);

			}

			if (rg_wind_vec.size() == 0){
				start_assigned = true;
				end_assigned = true;
			}

			if ( (line.compare(0,4,comp_keyword) == 0 && MetStart) ||
					(!MetEnd && MetStart) ){
				if (MetEnd)
					basenum = 1;
				else
					basenum = 0;
				char *dup = strdup(line.c_str());
				char *token = strtok(dup," ");
				int count =0;
				while (token != NULL){
					if ( (count >= basenum) && (strncmp(token,"TO",2) != 0) )
						region = region + string(token);
					else if ( (count >= basenum) &&
							((strncmp(token,"TO",2) == 0) && (strlen(token) > 2) ) )
						region = region + string(token);
					else if ( (count >= basenum) &&
							((strncmp(token,"TO",2) == 0) && (strlen(token) <= 2)) )
						region = region + string("-");

					if ( (count == basenum || count == basenum+1) && MetEnd){
						size_t dash_loc = string(token).find("-");
						if (dash_loc == string::npos)
							first_pt = first_pt+string(token);
						else
							first_pt = first_pt+string(token).substr(0,dash_loc);
						if (count == basenum+1)
							MetEnd = false;
					}
					count++;
					token = strtok(NULL," ");
				}

				size_t fp_loc = region.find(first_pt,first_pt.length());
				if (fp_loc != string::npos){
					sigmetStr.push_back(region);
					MetStart = false;
					MetEnd = true;
					region = "";
					first_pt = "";
					MetDesc.push_back(desc_str);
					if( start_assigned && end_assigned ){
						startEndDateTime.push_back( make_pair( start_time,end_time ) );
						start_assigned = false; end_assigned = false;
					}
				}
				free(dup);
			}
		}
		myfile.close();
	}

	vector<double> timev;
	int MIN_TIME = 2900;
	for(unsigned int i=0;i<startEndDateTime.size();++i){
		pair<int,int>  startpair= startEndDateTime.at(i).first;
		pair<int,int>  endpair= startEndDateTime.at(i).second;

		int start_date = startpair.first, end_date = endpair.first;
		int start_time = startpair.second, end_time = endpair.second;

		if (start_date != obs_date)
			start_time = start_time+2400;

		if (end_date != obs_date)
			end_time = end_time+2400;

		if (start_time < MIN_TIME )
			MIN_TIME = start_time;
		timev.push_back( (double)(end_time-start_time) );
	}

	MIN_TIME = MIN_TIME/100;
	sthr_ehr.clear();
	for(unsigned int i=0;i<startEndDateTime.size();++i){
		pair<int,int>  startpair= startEndDateTime.at(i).first;
		int start_hr = startpair.second/100-MIN_TIME;
		if (startpair.first != obs_date)
			start_hr = start_hr+24;
		sthr_ehr.push_back( make_pair(start_hr,start_hr+(int)ceil(timev.at(i)/100) ) );
	}

	rg_sim_start_hr = MIN_TIME;


	for (unsigned int k=0;k<sigmetStr.size(); ++k){


		region = sigmetStr.at(k);
		desc_str = MetDesc.at(k);

		char *dup = strdup(region.c_str());
		char *tok = strtok(dup,"-");
		vector<string> dirv;dirv.clear();
		vector<string> locv;locv.clear();
		vector<int> distvv;distvv.clear();
		while(tok != NULL){
			if (string(tok).length() < 4){
				locv.push_back( string(tok).substr(0,3) );
				distvv.push_back(0);
				dirv.push_back("E");

			}
			else{
				int pos = string(tok).length() - 4;
				string dist_dir = string(tok).substr(0,pos+1);
				string comp_dir;
				int dist;
				std::istringstream ss(dist_dir);
				ss >> dist >> comp_dir;
				dirv.push_back(comp_dir);
				distvv.push_back(dist);
				locv.push_back( string(tok).substr(pos+1,3) );
			}

			tok = strtok(NULL,"-");
		}

		dir.push_back(dirv);
		loc.push_back(locv);
		distv.push_back(distvv);
		free(dup);
	}
}


static void getLatLonfromWps(const string &CIFPfilepath,
		vector<string>* const loc,
		vector<string>* const dir,
		vector<int>* const dist,
		vector<double> &latv,
		vector<double> &lonv){
	latv.clear(); lonv.clear();
	unsigned int idx = 0;
	while(idx < loc->size()){

		string wpname = loc->at(idx);
		map<string,Fix*>::iterator itf = rg_fixes.find(wpname);
		if (itf != rg_fixes.end()){
			double lats = itf->second->getLatitude();
			double lons = itf->second->getLongitude();
			latv.push_back(lats);
			lonv.push_back(lons);
		}
		else{
			wpname = "K" + wpname;
			itf = rg_fixes.find(wpname);
			if (itf != rg_fixes.end()){
				double lats = itf->second->getLatitude();
				double lons = itf->second->getLongitude();
				latv.push_back(lats);
				lonv.push_back(lons);
			}
			else {
				map<string,Airport*>::iterator ita = rg_airports.find(wpname);
				if (ita != rg_airports.end()){
					double lats = ita->second->getLatitude();
					double lons = ita->second->getLongitude();
					latv.push_back(lats);
					lonv.push_back(lons);
				}
				else{
					wpname = wpname.substr(1,wpname.length()-1);
					double lat_v= 0, lon_v = 0, ele_v = 0, freq = 0;
					waypoint_type wptype;string reg = "";
					bool exists = getFixFromCIFP(wpname,CIFPfilepath, reg,lat_v,lon_v,ele_v,freq,wptype);
					if (exists){
						latv.push_back(lat_v); lonv.push_back(lon_v);
					}else{
						cout << " Fix not found. Removing it ..." << wpname<<endl;
						loc->erase(loc->begin()+idx);
						dir->erase(dir->begin()+idx);
						dist->erase(dist->begin()+idx);
						--idx;
					}
				}
			}
		}
		++idx;
	}

}
//FIXME: MULTIPLE COMMON DECLARATIONS. FIXIT
static inline double X_ref(double* const lat,double* const lon){
	return RN( *lat ) * cos( *lat * M_PI/180) * cos( *lon * M_PI/180);}
static inline double Y_ref(double* const lat,double* const lon){
	return RN( *lat ) * cos( *lat * M_PI/180) * sin( *lon * M_PI/180);}
static inline double Z_ref(double* const lat){
	return ( ( 1 - ECCENTRICITY_SQ_EARTH ) * RN( *lat ) ) * sin( *lat * M_PI/180);}

static void LatLontoECEF(const double& lat, const double& lon,
					double &x, double &y, double &z){
	double Rn = RN(lat);
	x = Rn * cos(lat*M_PI/180) * cos(lon*M_PI/180);
	y = Rn * cos(lat*M_PI/180) * sin(lon*M_PI/180);
	z = ( (1-ECCENTRICITY_SQ_EARTH) * Rn )*sin(lat*M_PI/180);
}

static void ECEFtoLatLon(const double &x,
		const double &y,
		const double &z,
		double& lat,
		double& lon,
		double &h) {
	if (x != 0) {
		lon = atan2(y, x);

		double p = sqrt(x*x + y*y);

		if (z != 0) {
			double lat_i = atan2(p, z);

			while (true) {
				if (cos(lat_i) != 0) {
					h = p / cos(lat_i) - RN(lat_i*180/M_PI);

					double num = z*(RN(lat_i*180/M_PI) + h);
					double den = p* ( RN(lat_i*180/M_PI) * (1-ECCENTRICITY_SQ_EARTH) + h );

					if (den != 0) {
						lat = atan2(num, den);

						if (fabs(lat - lat_i) < 1e-10)
							break;
						else
							lat_i = lat;
					}
				}
			}
		}
	}
}

static void NEDtoECEF(const double& x_n, const double& y_n, const double& z_n,
				const double& lat_ref, const double& lon_ref,
				const double& x_ref, const double& y_ref, const double& z_ref,
				double& x_i, double& y_i, double& z_i){
	double s_lat = sin(lat_ref*M_PI/180), c_lat = cos(lat_ref*M_PI/180),
			s_lon = sin(lon_ref*M_PI/180), c_lon = cos(lon_ref*M_PI/180);

	x_i = -s_lat*c_lon*x_n - s_lon*y_n - c_lat*c_lon*z_n;
	y_i = -s_lat*s_lon*x_n + c_lon*y_n - c_lat*s_lon*z_n;
	z_i = c_lat*x_n -s_lat*z_n;

	x_i = x_i + x_ref;
	y_i = y_i + y_ref;
	z_i = z_i + z_ref;
}

static void ECEFtoNED(double* const X, double* const Y, double* const Z, double* const ref_lat,
		double* const ref_lon, double* x, double* y, double* z){

	*x = - (*X - X_ref(ref_lat,ref_lon)) * sin( *ref_lat * M_PI/180) * cos( *ref_lon * M_PI/180)
		 - (*Y - Y_ref(ref_lat,ref_lon)) * sin( *ref_lat * M_PI/180) * sin( *ref_lon * M_PI/180)
		 + (*Z - Z_ref(ref_lat)) * cos( *ref_lat * M_PI/180);

	*y = - (*X - X_ref(ref_lat,ref_lon)) * sin( *ref_lon * M_PI/180)
		 + (*Y - Y_ref(ref_lat,ref_lon)) * cos( *ref_lon * M_PI/180);

	*z = - (*X - X_ref(ref_lat,ref_lon)) * cos( *ref_lat * M_PI/180) * cos( *ref_lon * M_PI/180)
		 - (*Y - Y_ref(ref_lat,ref_lon)) * cos( *ref_lat * M_PI/180) * sin( *ref_lon * M_PI/180)
		 - (*Z - Z_ref(ref_lat)) * sin( *ref_lat * M_PI/180);
}

static void NEDtoECEF(double* const x, double* const y, double* const z,double* const ref_lat,
		double* const ref_lon, double* X, double* Y, double* Z){

	*X = - (*x) * sin( *ref_lat * M_PI/180) * cos( *ref_lon * M_PI/180)
		 - (*y) * sin( *ref_lon * M_PI/180)
		 - (*z) * cos( *ref_lat * M_PI/180) * cos( *ref_lon * M_PI/180) + X_ref(ref_lat,ref_lon);

	*Y = - (*x) * sin( *ref_lat * M_PI/180) * sin( *ref_lon * M_PI/180)
		 + (*y) * cos( *ref_lon * M_PI/180)
		 - (*z) * cos( *ref_lat * M_PI/180) * sin( *ref_lon * M_PI/180) + Y_ref(ref_lat,ref_lon);

	*Z =  (*x) * cos( *ref_lat * M_PI/180) - (*z) * sin( *ref_lat * M_PI/180) + Z_ref(ref_lat);
}

static void LatLontoNED(double* const lat, double* const lon, double* const alt,
		double* const ref_lat, double* const ref_lon,	double* x, double* y, double* z){
	double Rn = RN( (*lat) ) + *alt;
	double X = Rn * cos((*lat) * M_PI/180) * cos( (*lon) * M_PI/180);
	double Y = Rn * cos((*lat) * M_PI/180) * sin( (*lon) * M_PI/180);
	double Z = ( ( 1 - ECCENTRICITY_SQ_EARTH ) * Rn )*sin( (*lat) * M_PI/180);
	ECEFtoNED(&X,&Y,&Z,ref_lat,ref_lon,x,y,z);


}

static void NEDtoLatLon(double* const x, double* const y, double* const z,double* const ref_lat,
		double* const ref_lon,	double* lat, double* lon, double* h){
	double X=0,Y=0,Z=0;
	NEDtoECEF(x,y,z,ref_lat,ref_lon,&X,&Y,&Z);


	*lon = atan2(Y,X);

	double p = sqrt(X*X + Y*Y);

	double lat_i = atan2( p,Z );
	if (lat_i>= M_PI)
		lat_i = lat_i-M_PI;
	else if (lat_i <= -M_PI)
		lat_i = lat_i +M_PI;

	while(true){

		*h = p/cos(lat_i)- RN(lat_i*180/M_PI);

		double num = Z * (RN(lat_i*180/M_PI) + (*h) );
		double den = p * ( RN(lat_i*180/M_PI)*(1-ECCENTRICITY_SQ_EARTH) + (*h) );
		(*lat) = atan2(num, den);

		if (*lat>= M_PI)
			*lat = *lat-M_PI;
		else if (*lat <= -M_PI)
			*lat = *lat +M_PI;

		if (fabs( (*lat) - lat_i) <1e-6){
			break;
		}
		else if ( (fabs(*lat-M_PI/2) < 1e-6 || fabs(lat_i-M_PI/2) < 1e-6) ||
				(fabs(*lat+M_PI/2) < 1e-6 || fabs(lat_i+M_PI/2) < 1e-6) ){
			break;
		}
		else{
			lat_i = *lat;
		}
	}

	*lat = *lat * 180/M_PI;
	*lon = *lon * 180/M_PI;
}
//FIXME: MULTIPLE COMMON DECLARATIONS TILL HERE. FIXIT

static void getPointPosition(const double& lat_r, const double& lon_r, const double& ang,
					const double& dist,	double& lat_p, double& lon_p){


	//Convert ref to ECEF
	double X_ref = 0, Y_ref = 0, Z_ref = 0;
	LatLontoECEF(lat_r, lon_r, X_ref, Y_ref, Z_ref);

	//Get coordinates in NED
	double x_ned = dist*cos(ang*M_PI/180), y_ned = dist*cos(ang*M_PI/180);

	//Get coordinates in ECEF
	double x_i = 0.0, y_i = 0.0, z_i = 0.0;
	NEDtoECEF(x_ned, y_ned,0, lat_r, lon_r, X_ref, Y_ref, Z_ref, x_i, y_i, z_i);

	//Get lat lon values
	double h;
	ECEFtoLatLon(x_i, y_i, z_i, lat_p, lon_p, h);

	lat_p = lat_p*180/M_PI;
	lon_p = lon_p*180/M_PI;
}



static void getPolygonEdges(const vector<string> &dir,
		const vector<int> &distv,
		const vector<double> &latv,
		const vector<double> &lonv,
		vector<double> &lat_ydata,
		vector<double> &lon_xdata){

	for(unsigned int s=0; s<dir.size(); ++s){
		double ang = rg_dir_angle_map[dir.at(s)];
		double lati = 0,loni = 0;

		getPointPosition(latv.at(s), lonv.at(s),ang,distv.at(s)*nauticalMilesToFeet,
				lati,loni);
		lat_ydata.push_back(lati); lon_xdata.push_back(loni);
	}

}



static void processWeatherPolys(const string &CIFPfilepath, const string &Sigmetfilepath) {
	load_dir_angle_map();
	vector<vector<string> > dir;
	vector<vector<string> > loc;
	vector<vector<int> > distv;
	vector<string> MetDesc;
	vector<pair<int,int> > sthr_ehr;

	readWeatherFile(Sigmetfilepath,dir,loc,distv,MetDesc,sthr_ehr);

	vector< vector<double> > latv;latv.clear();
	vector < vector<double> > lonv;lonv.clear();
	for (unsigned int k =0;k<loc.size();++k) {
		vector<string>* locv = &loc.at(k);
		vector<string>* dirv = &dir.at(k);
		vector<int>* distvv = &distv.at(k);
		vector<double> latvv;latvv.clear();
		vector<double> lonvv;lonvv.clear();

		getLatLonfromWps(CIFPfilepath,locv,dirv,distvv,latvv,lonvv);

		latv.push_back(latvv);lonv.push_back(lonvv);
	}

	rg_scenarios.clear();

	PolygonSet polygonSet;
//	rg_scenarios.resize(2);//AIRMET and SIGMET

	vector<vector<pair<double,double> > > PolyEdges;PolyEdges.clear();
	for (unsigned int s = 0; s < loc.size(); ++s) {
		vector<string> dirv = dir.at(s);
		vector<int> distvv = distv.at(s);
		vector<double> latvv = latv.at(s);
		vector<double> lonvv = lonv.at(s);
		vector<double> lat_ydata;lat_ydata.clear();
		vector<double> lon_xdata;lon_xdata.clear();
		getPolygonEdges(dirv,distvv,latvv,lonvv,lat_ydata,lon_xdata);
		if (lat_ydata.size() > 0 && lon_xdata.size() > 0 ) {
			Polygon polygon(lon_xdata, lat_ydata, lat_ydata.size());
			polygon.setPolyType(MetDesc.at(s));
			polygon.setStartHour(sthr_ehr.at(s).first);
			polygon.setEndHour(sthr_ehr.at(s).second);
			polygonSet.push_back(polygon);
		}
	}
//DIFFERENT SCENARIOS FOR AIRMET AND SIGMET. WE CAN COMBINE THEM INTO ONE.
	rg_scenarios.push_back(polygonSet);

	polygonSet.clear();
}

//TODO: Parikshit adder
static void processWeatherPolysforNATS(const string &CIFPfilepath,
		const string &Sigmetfilepath,
		PolygonSet& polygonSet) {

	load_dir_angle_map();
	vector<vector<string> > dir;
	vector<vector<string> > loc;
	vector<vector<int> > distv;
	vector<string> MetDesc;
	vector<pair<int,int> > sthr_ehr;

	readWeatherFile(Sigmetfilepath,dir,loc,distv,MetDesc,sthr_ehr);

	vector< vector<double> > latv;latv.clear();
	vector < vector<double> > lonv;lonv.clear();
	for (unsigned int k =0;k<loc.size();++k) {
		vector<string>* locv = &loc.at(k);
		vector<string>* dirv = &dir.at(k);
		vector<int>* distvv = &distv.at(k);
		vector<double> latvv;latvv.clear();
		vector<double> lonvv;lonvv.clear();

		getLatLonfromWps(CIFPfilepath,locv,dirv,distvv,latvv,lonvv);

		latv.push_back(latvv);lonv.push_back(lonvv);
	}

	vector<vector<pair<double,double> > > PolyEdges;PolyEdges.clear();
	for (unsigned int s = 0; s < loc.size(); ++s) {
		vector<string> dirv = dir.at(s);
		vector<int> distvv = distv.at(s);
		vector<double> latvv = latv.at(s);
		vector<double> lonvv = lonv.at(s);
		vector<double> lat_ydata;lat_ydata.clear();
		vector<double> lon_xdata;lon_xdata.clear();
		getPolygonEdges(dirv,distvv,latvv,lonvv,lat_ydata,lon_xdata);
		if (lat_ydata.size() > 0 && lon_xdata.size() > 0 ) {
			Polygon polygon(lon_xdata, lat_ydata, lat_ydata.size());
			polygon.setPolyType(MetDesc.at(s));
			polygon.setStartHour(sthr_ehr.at(s).first);
			polygon.setEndHour(sthr_ehr.at(s).second);
			polygonSet.push_back(polygon);
		}
	}
}
//end parikshit adder

static void removeSubstrs(string& s, string& p) {
  string::size_type n = p.length();
  for (string::size_type i = s.find(p);
      i != string::npos;
      i = s.find(p))
      s.erase(i, n);
}

template<typename T>
static int modRound(const T& val) {
	int fl_val = (int)floor(val / 100);
	int rem = (int)val%100;
	if (rem >= 30)
		return fl_val+1;
	else
		return fl_val;
}

static void readPIREPs( PIREPset* const pirep_vec,
		const string& filename) {
	string line;
	ifstream myfile (filename.c_str());
	pirep_vec->clear();

	int counter = 0; //LINE COUNTER

	if (myfile.is_open()) {
		while ( getline(myfile,line)) {
			string wpname;
			bool urgent_flag = false;
			double obs_tm = -100.0;
			double ang = 0.0;
			double dist = 0.0;
			int flt_lev = 0;
			string phenom = "TB";
			int severity = 1;
			int cnt = 0;//TOKEN COUNTER
			bool error_flag = false;

			char *dup = strdup(line.c_str());
			char *tok = strtok(dup,"/");

			while (tok != NULL) {
				//LOOP CHECKER
				if (cnt == 0) {
					string str(tok);
					istringstream iss(str);
					int cntr = 0;
					do {
						string subst;
						iss >> subst;
						if (cntr == 0)
							wpname = subst;
						if (cntr == 1) {
							if (strncmp(subst.c_str(),"UA",2) == 0)
								urgent_flag = false;
							else
								urgent_flag = true;
						}
						if (cntr == 2 && subst.length()) {
							error_flag = true;

							break;
						}
						cntr++;
					} while(iss);
				}

				if (cnt == 1) {
					string str(tok);
					istringstream iss(str);
					int cntr = 0;
					do {
						string subst;
						iss >> subst;
						if (cntr == 1) {
							size_t found = subst.find_first_of("0123456789");
							if (found != string::npos) {
									if (found == 0) {
										error_flag = true;

										break;
									}
								string wp = subst.substr(0, found);
								if (wp.length() <= 4) {
									wpname = wp ;
								}
								string num = subst.substr(found, subst.length()-found);
								assert(num.length() == 6);

								if (num.length() > 3) {
									string ang_str = num.substr(0, 3);
									string dist_str = num.substr(3, num.length());

									//PIREP IS IN STATUTE MILES
									ang = atof( ang_str.c_str() );
									dist = atof(dist_str.c_str() )*MilesToFeet;
								}
							}
							else {
								if (subst.length() <= 4)
									wpname = subst;
								ang = 0.0;dist = 0.0;
							}
						}
						if (cntr == 2 && subst.length() == 6 && atof(subst.c_str()) ) {
							string ang_str = subst.substr(0, 3);
							string dist_str = subst.substr(3, subst.length());

							//PIREP IS IN STATUTE MILES
							ang = atof( ang_str.c_str() );
							dist = atof(dist_str.c_str() )*MilesToFeet;
						}
						cntr++;
					} while (iss);
				}

				if (cnt == 2 ) {
					string str(tok);
					istringstream iss(str);
					int cntr = 0;
					do {
						if (rg_wind_vec.size() == 0) {
							obs_tm = 0.0;
							break;
						}
						string subst;
						iss >> subst;
						if (cntr == 1) {
							double tm = atof(subst.c_str());
							if (tm != 0)
								obs_tm = tm;
							else {
								try {
									obs_tm = pirep_vec->back().getObsTime();
								} catch(int _exep) {
									obs_tm = 0.0;
								}
							}
						}//if (cntr == 1){
						cntr++;
					} while (iss);
				}//if (cnt == 2)

				if (cnt == 3) {
					string flt_lev_str(tok);
					string fl = "FL";
					removeSubstrs(flt_lev_str,fl);
					if ( atoi( flt_lev_str.c_str() ) ){
						flt_lev =  atoi( flt_lev_str.c_str() ) ;
					}
					else {
						if (flt_lev_str == "UNKN")
							flt_lev =  -4000 ;
						else
							flt_lev =  2 ; // climb and descent did not know what to file.
					}
				}
				if (cnt > 3) {
					if (urgent_flag)
						severity = 3;
					else
						severity = 1;
					string rest_str(tok);
					size_t found = rest_str.find("IC ");
					if (found != std::string::npos) {
						phenom = "IC";
						size_t subfound = rest_str.find("LGT");
						if (subfound != string::npos)
							severity = 1;
						subfound = rest_str.find("MOD");
						if (subfound != string::npos)
							severity = 2;
						subfound = rest_str.find("SEV");
						if (subfound != string::npos)
							severity = 3;
					}

					found = rest_str.find("TB ");
					if (found != std::string::npos) {
						phenom = "TB";
						size_t subfound = rest_str.find("LGT");
						if (subfound != string::npos)
							severity = 1;
						subfound = rest_str.find("MOD");
						if (subfound != string::npos)
							severity = 2;
						subfound = rest_str.find("SEV");
						if (subfound != string::npos)
							severity = 3;
					}

					found = rest_str.find("LLWS ");
					if (found != std::string::npos) {
						phenom = "LLWS";
						severity = 3;
					}
				}
				if (error_flag)
					break;

				cnt++;
				tok = strtok(NULL, "/");
			}//while(tok != NULL){

			if (!error_flag) {
				PIREP pirep(wpname,
						urgent_flag,
						modRound<double>(obs_tm),
						ang,
						dist,
						flt_lev,
						phenom,
						severity);
				pirep_vec->push_back(pirep);
			}//if (!error_flag){

			free(dup);
			counter++;
		}//while( getline(myfile,line)){
	}//	if (myfile.is_open()){

	myfile.close();
}

static void getPIREPposition(const string& CIFPfile, PIREPset* const pirepvec) {
	PIREPset::iterator it = pirepvec->begin();

	while (true) {
		bool eraseflag = false;
		if (it == pirepvec->end())
			break;
		string wpname = it->getWaypoint();
		Fix* fix1 = NULL;

		get_fix(wpname, &fix1, false, true);

		if (fix1 != NULL) {
			double lats = fix1->getLatitude();
			double lons = fix1->getLongitude();
			double latp = 0, lonp = 0;

			getPointPosition(lats, lons, it->getAngle(), it->getDistance(), latp, lonp);

			it->setLatitude(latp);
			it->setLongitude(lonp);
		}
		else {
			double lats = 0, lons = 0, eles = 0, freq = 0;
			waypoint_type wptype;
			string reg = "";
			bool exists = getFixFromCIFP(wpname, CIFPfile, reg, lats, lons, eles, freq,wptype);

			if (exists) {
				double latp = 0, lonp = 0;

				getPointPosition(lats, lons, it->getAngle(), it->getDistance(), latp, lonp);

				it->setLatitude(latp);
				it->setLongitude(lonp);
			}
			else
				eraseflag = true;
		}

		if (eraseflag)
			pirepvec->erase(it);
		else
			it++;
	}
}

static void processPIREPs(const string& CIFPfile, const string& PIREPfilepath) {
	rg_pirep.clear();

	readPIREPs(&rg_pirep, PIREPfilepath);

	getPIREPposition(CIFPfile, &rg_pirep);
}


static void processSimStartTime(){
	int min_time =numeric_limits<int>::max();
	for(size_t s = 0; s < rg_pirep.size(); ++s){
		int tm = rg_pirep.at(s).getObsTime();
		if( tm < min_time)
			min_time = tm ;
	}

	if (min_time > rg_sim_start_hr){
		for(size_t s = 0; s < rg_pirep.size(); ++s){
			int tm = rg_pirep.at(s).getObsTime();
			rg_pirep.at(s).setObsTime(tm - rg_sim_start_hr);
		}
	}
	else{
		int d_time = rg_sim_start_hr - min_time;
		for (size_t s =0; s < rg_scenarios.size(); ++s)
			for (size_t r = 0; r < rg_scenarios.at(s).size(); ++r){
				Polygon *p = &rg_scenarios.at(s).at(r);
				p->setStartHour(p->getStartHour()+d_time);
				p->setEndHour(p->getEndHour()+d_time);
			}
		rg_sim_start_hr = min_time;
	}
}
#endif
/*
 * TODO: PARIKSHIT ADDER ENDS
 */

/**
 * Load the scenario polygons from the file with the specified name
 */
static void load_scenarios(string f) {
	// the polygon file should define polygons in 3 columns:
	// longitude    latitude    scenario
	// individual polygons should be separated by blank lines

	if (!rg_use_sigmetfile) {
		ifstream in;
		in.open(f.c_str());

		string line;

		vector<double> xdata;
		vector<double> ydata;

		int scenario = 0;

		PolygonSet polygonSet;

		int scen = -1;

		rg_scenarios.clear(); // Reset

		while (in.good()) {
			// read the line
			getline(in, line);

			double lat = MAX_DOUBLE, lon = MAX_DOUBLE;

			if (line.length() < 1) {
				if (scen != scenario) {
					rg_scenarios.push_back(polygonSet);
					polygonSet.clear();
					scenario = scen;
				}

				if (xdata.size() > 0 && ydata.size() > 0) {
					Polygon polygon(xdata, ydata, xdata.size());
					polygonSet.push_back(polygon);
				}

				xdata.clear();
				ydata.clear();
			} else {
				// parse the line
				parse_polygon_line(line, &scen, &lat, &lon);

				xdata.push_back(lon);
				ydata.push_back(lat);
			}
		}

		// insert the last scenario
		rg_scenarios.push_back(polygonSet);
		polygonSet.clear();

		scenario = scen;

		in.close();
	}
}
//todo:parikshit adder for nats
static void load_scenarios_for_NATS(string f,
		PolygonSet& polygonSet) {
	// the polygon file should define polygons in 3 columns:
	// longitude    latitude    scenario
	// individual polygons should be separated by blank lines

		ifstream in;
		in.open(f.c_str());

		string line;

		vector<double> xdata;
		vector<double> ydata;

		int scen = 1;

		while (in.good()) {
			// read the line
			getline(in, line);

			double lat = MAX_DOUBLE, lon = MAX_DOUBLE;

			if (line.length() < 1) {

				if (xdata.size() > 0 && ydata.size() > 0) {
					Polygon polygon(xdata, ydata, xdata.size());
					polygonSet.push_back(polygon);
				}

				xdata.clear();
				ydata.clear();
			} else {
				// parse the line
				parse_polygon_line(line, &scen, &lat, &lon);
				xdata.push_back(lon);
				ydata.push_back(lat);
			}
		}

		in.close();
}


static void print_splash() {
    printf("\n");
    printf("********************************************************************************\n");
    printf("*                               Route Generator                                *\n");
    printf("*                                 Version 1.0                                  *\n");
    printf("*                                                                              *\n");
    printf("*                            Optimal Synthesis Inc.                            *\n");
    printf("*                            95 First St, Suite 240                            *\n");
    printf("*                             Los Altos, CA  94022                             *\n");
    printf("********************************************************************************\n");
    printf("\n");
}


/**
 * Display the help/usage dialog
 */
static void print_usage(char* progname) {
	cout << endl;
	cout << "Usage: " << progname << " [options] " << endl << endl;
	cout << "  Options:" << endl;
	cout << "    --airways-file=<file>   -a <file>    airway data XML file" << endl;
	cout << "    --airports-file=<file>  -P <file>    airports data XML file" << endl;
	cout << "    --sids-file=<file>      -S <file>    sids data XML file" << endl;
	cout << "    --stars-file=<file>     -T <file>    stars data XML file" << endl;
	cout << "    --trx-file=<file>       -t <file>    TRX file" << endl;
	cout << "    --mfl-file=<file>       -m <file>    MFL file" << endl;
	cout << "    --rap-file=<file>       -R <file>    RUC/RAP wind file" << endl;
	cout << "    --airports-list=<file>  -A <file>    airport pairs list" << endl;
	cout << "    --polygon-file=<file>   -p <file>    polygon file" << endl;
	cout << "    --origin=<arg>          -o <arg>     origin airport" << endl;
	cout << "    --destination=<arg>     -d <arg>     destination airport" << endl;
	cout << "    --scale-factors=<list>  -s <list>    comma separated list of scale factors" << endl;
	cout << "    --reroute               -r           reroute instead of replan" << endl;
	cout << "    --wind-optimal          -w           use wind optimal cost function" << endl;
	cout << "                                         instead of default cost function" << endl;
	cout << "    --filter-by-airports    -F           if set then filter NAS data" << endl;
	cout << "                                         for Airports to load only the" << endl;
	cout << "                                         elements for airports found in" << endl;
	cout << "                                         the input airport list or TRX file." << endl;
	cout << "    --num-tos=<arg>         -z <arg>     the number of trajectory options." << endl;
	cout << "    --tos-cost-index=<arg>  -c <arg>     TOS cost index" << endl;
	cout << "    --out-dir=<dir>         -D <dir>     output directory" <<  endl;
	cout << "    --new-trx               -n           write new format TRX output file that" << endl;
	cout << "                                         includes the cruise altitude instead" << endl;
	cout << "                                         of writing a separate MFL file." << endl;
	cout << "    --help                  -h           print this message" << endl;
	cout << endl;
}

/**
 * Create a directory and the required parent directories.
 * Similar to bash mkdir -p
 */
static bool mkpath( std::string path ) {
    bool bSuccess = false;
#ifndef _INC__MINGW_H
    int nRC = ::mkdir( path.c_str(), 0775 );
#else
    int nRC = ::mkdir( path.c_str());
#endif
    if( nRC == -1 )
    {
        switch( errno )
        {
            case ENOENT:
                //parent didn't exist, try to create it
                if( mkpath( path.substr(0, path.find_last_of('/')) ) )
                    //Now, try to create again.
#ifndef _INC__MINGW_H
                    bSuccess = 0 == ::mkdir( path.c_str(), 0775 );
#else
                	bSuccess = 0 == ::mkdir( path.c_str());
#endif
                else
                    bSuccess = false;
                break;
            case EEXIST:
                //Done!
                bSuccess = true;
                break;
            default:
                bSuccess = false;
                break;
        }
    }
    else
        bSuccess = true;
    return bSuccess;
}

/**
 * Parse command line args into global variables
 */
static void parse_args(int argc, char* argv[]) {

	int c;

	while(true) {

		int option_index = 0;

		// {name, has_arg, flag, val}
		// no_argument = 0
		// required_argument = 1
		// optional_argument = 2
		static struct option opts[] = {
				{"airways-file", 1, 0, 'a'},
				{"airports-file", 1, 0, 'P'},
				{"sids-file", 1, 0, 'S'},
				{"stars-file", 1, 0, 'T'},
				{"trx-file", 1, 0, 't'},
				{"mfl-file", 1, 0, 'm'},
				{"rap-file", 1, 0, 'R'},
				{"airports-list", 1, 0, 'A'},
				{"origin", 1, 0, 'o'},
				{"destination", 1, 0, 'd'},
				{"scale-factors", 1, 0, 's'},
				{"reroute", 0, 0, 'r'},
				{"wind-optimal", 0, 0, 'w'},
				{"filter-by-airports", 0, 0, 'F'},
				{"out-dir", 1, 0, 'D'},
				{"new-trx", 0, 0, 'n'},
				{"num-tos",1, 0, 'z'},
				{"tos-cost-index",1, 0, 'c'},
				{"CIFP-file", 1, 0, 'C'},
				{"Sigmet-file", 1, 0, 'M'},
				{"PIREP-file", 1, 0, 'I'},
				{"AP-conf", 1, 0, 'B'},
				{"AW-conf", 1, 0, 'E'},
				{"Fix-conf", 1, 0, 'G'},
				{"Proc-conf", 1, 0, 'H'},
				{"fixed-procedure",1,0,'f'},
				{"polygon-file", 1, 0, 'p'},
				{"help", 0, 0, 'h'},
				{0, 0, 0, 0}
		};

		string optstr = "a:P:S:T:t:m:R:A:o:d:s:grwFD:nz:c:C:M:I:B:E:G:H:f:p:h";



		c = getopt_long(argc, argv, optstr.c_str(), opts, &option_index);
		if(c == -1) {
			break;
		}
		int err = -1;
		stringstream ss;
		switch(c) {
		case 'a':
			rg_airways_file = optarg;
			break;
		case 'P':
			rg_airports_file = optarg;
			break;
		case 'S':
			rg_sids_file = optarg;
			break;
		case 'T':
			rg_stars_file = optarg;
			break;
		case 't':
			rg_trx_file = optarg;
			break;
		case 'm':
			rg_mfl_file = optarg;
			break;
		case 'R':
			rg_rap_file = optarg;
			break;
		case 'A':
			rg_airports_list_file = optarg;
			load_airports_list(rg_airports_list_file);
			break;
		case 'o':
			rg_origin = optarg;
			break;
		case 'd':
			rg_destination = optarg;
			break;
		case 's':
			parse_scale_factors(string(optarg));
			for(unsigned int i=0; i<rg_scale_factors.size(); ++i) {
				ss << rg_scale_factors.at(i)
						<< (i<rg_scale_factors.size()-1 ? "," : "\n");
			}
			break;
		case 'r':
			rg_reroute = true;
			break;
		case 'w':
			rg_wind_optimal = true;
			break;
		case 'F':
		    rg_filter_by_input_airports = true;
		    break;
		case 'D':
			rg_out_dir = optarg;
			rg_output_directory = rg_out_dir;
			err = mkpath(rg_out_dir.c_str());
			if(err != 1) {
				cerr << "ERROR: failed to create output directory ("
						<< err << ")" << endl;
				exit(err);
			}
			break;
		case 'n':
		    rg_use_new_trx = true;
		    break;
		case 'z':
			rg_num_trajs = atoi(optarg);
			rg_get_TOS  = (rg_num_trajs<=1) || (!rg_wind_optimal) ? false : true;
			break;
		case 'c':
		    rg_tos_cost_index = atoi(optarg);
		    break;
		case 'C':
			rg_CIFPfile = optarg;
		    if ( !exists_test(rg_CIFPfile) )
		    	rg_CIFPfile = "src/rg/share/rg/nas/CIFP_201609/FAACIFP18";
		    break;
		case 'M':
			rg_Sigmetfile = optarg;
		    if ( !exists_test(rg_Sigmetfile) )
		    	rg_use_sigmetfile = false;
		    break;
		case 'I':
			rg_PIREPfile = optarg;
		    if ( !exists_test(rg_PIREPfile) )
		    	rg_use_PIREPfile = false;
		    break;
		case 'B':
			rg_apconffile = optarg;
			if ( !exists_test(rg_apconffile))
				rg_apconffile ="src/rg/share/rg/nas/airport.config";
			break;
		case 'E':
			rg_airwayconffile = optarg;
		    if ( !exists_test(rg_airwayconffile))
		    	rg_airwayconffile ="src/rg/share/rg/nas/airway.config";
		    break;
		case 'G':
			rg_fixconffile = optarg;
		    if ( !exists_test(rg_fixconffile))
		    	rg_fixconffile ="src/rg/share/rg/nas/enrtfix.config";
		    break;
		case 'H':
			rg_procconffile = optarg;
		    if ( !exists_test(rg_procconffile))
		    	rg_procconffile ="src/rg/share/rg/nas/Proc.config";
		    break;
		case 'f':
			parse_fixed_procedure(string(optarg));
			break;
		case 'p':
			rg_polygon_file = optarg;
			load_scenarios(rg_polygon_file);
			break;
		case 0:
		case 'h':
		default:
			print_usage(argv[0]);
			exit(-1);
		}
	}

}

/**
 * Parse command line args into global variables
 */
static void parse_args_alt(int argc, char* argv[]) {

	int c;

	while(true) {

		int option_index = 0;

		// {name, has_arg, flag, val}
		// no_argument = 0
		// required_argument = 1
		// optional_argument = 2
		static struct option opts[] = {
				{"callsign", 1, 0, 'a'},
				{"airspeed", 1, 0, 'P'},
				{"altitude", 1, 0, 'S'},
				{"rap-file", 1, 0, 'R'},
				{"origin", 1, 0, 'o'},
				{"destination", 1, 0, 'd'},
				{"scale-factors", 1, 0, 's'},
				{"reroute", 0, 0, 'r'},
				{"wind-optimal", 0, 0, 'w'},
				{"filter-by-airports", 0, 0, 'F'},
				{"out-dir", 1, 0, 'D'},
				{"num-tos",1, 0, 'z'},
				{"tos-cost-index",1, 0, 'c'},
				{"CIFP-file", 1, 0, 'C'},
				{"Sigmet-file", 1, 0, 'M'},
				{"PIREP-file", 1, 0, 'I'},
				{"AP-conf", 1, 0, 'B'},
				{"AW-conf", 1, 0, 'E'},
				{"Fix-conf", 1, 0, 'G'},
				{"Proc-conf", 1, 0, 'H'},
				{"fixed-procedure",1,0,'f'},
				{"polygon-file", 1, 0, 'p'},
				{"help", 0, 0, 'h'},
				{0, 0, 0, 0}
		};

		string optstr = "a:P:S:R:o:d:s:grwFD:z:c:C:M:I:B:E:G:H:f:p:h";



		c = getopt_long(argc, argv, optstr.c_str(), opts, &option_index);
		if(c == -1) {
			break;
		}
		int err = -1;
		stringstream ss;
		switch(c) {
		case 'a':
			rg_callsign = optarg;
			break;
		case 'P':
			rg_airspeed = atof(optarg);
			break;
		case 'S':
			rg_altitude = atof(optarg);
			break;
		case 'R':
			rg_rap_file = optarg;
			cout << "rg_rap_file=" << rg_rap_file << endl;
			break;
		case 'o':
			rg_origin = optarg;
			break;
		case 'd':
			rg_destination = optarg;
			break;
		case 's':
			parse_scale_factors(string(optarg));
			for(unsigned int i=0; i<rg_scale_factors.size(); ++i) {
				ss << rg_scale_factors.at(i)
						<< (i<rg_scale_factors.size()-1 ? "," : "\n");
			}
			break;
		case 'r':
			rg_reroute = true;
			break;
		case 'w':
			rg_wind_optimal = true;
			break;
		case 'F':
		    rg_filter_by_input_airports = true;
		    break;
		case 'D':
			rg_out_dir = optarg;
			rg_output_directory = rg_out_dir;
			err = mkpath(rg_out_dir.c_str());
			if(err != 1) {
				cerr << "ERROR: failed to create output directory ("
						<< err << ")" << endl;
				exit(err);
			}
			break;
		case 'z':
			rg_num_trajs = atoi(optarg);
			rg_get_TOS  = (rg_num_trajs<=1) || (!rg_wind_optimal) ? false : true;
			break;
		case 'c':
		    rg_tos_cost_index = atoi(optarg);
		    break;
		case 'C':
			rg_CIFPfile = optarg;
		    if ( !exists_test(rg_CIFPfile) )
		    	rg_CIFPfile = "src/rg/share/rg/nas/CIFP_201609/FAACIFP18";
		    break;
		case 'M':
			rg_Sigmetfile = optarg;
		    if ( !exists_test(rg_Sigmetfile) )
		    	rg_use_sigmetfile = false;
		    break;
		case 'I':
			rg_PIREPfile = optarg;
		    if ( !exists_test(rg_PIREPfile) )
		    	rg_use_PIREPfile = false;
		    break;
		case 'B':
			rg_apconffile = optarg;
			if ( !exists_test(rg_apconffile))
				rg_apconffile ="src/rg/share/rg/nas/airport.config";
			break;
		case 'E':
			rg_airwayconffile = optarg;
		    if ( !exists_test(rg_airwayconffile))
		    	rg_airwayconffile ="src/rg/share/rg/nas/airway.config";
		    break;
		case 'G':
			rg_fixconffile = optarg;
		    if ( !exists_test(rg_fixconffile))
		    	rg_fixconffile ="src/rg/share/rg/nas/enrtfix.config";
		    break;
		case 'H':
			rg_procconffile = optarg;
		    if ( !exists_test(rg_procconffile))
		    	rg_procconffile ="src/rg/share/rg/nas/Proc.config";
		    break;
		case 'f':
			parse_fixed_procedure(string(optarg));
			break;
		case 'p':
			rg_polygon_file = optarg;
			load_scenarios(rg_polygon_file);
			break;
		case 0:
		case 'h':
		default:
			print_usage(argv[0]);
			exit(-1);
		}
	}

}

void getAcidFilterList(vector<string>* const acidList) {
	if(!acidList) return;

	acidList->push_back("USA1614");
	acidList->push_back("USA264");
	acidList->push_back("CHQ3179");
	acidList->push_back("ASH2915");
	acidList->push_back("NWA1178");
	acidList->push_back("TRS765");
	acidList->push_back("CHQ3432");
	acidList->push_back("N500RP");
	acidList->push_back("DAL816");
	acidList->push_back("TCF7562");
	acidList->push_back("UAL422");
	acidList->push_back("SKW3839");
	acidList->push_back("DAL510");
	acidList->push_back("CHQ3001");
	acidList->push_back("SKW3793");
	acidList->push_back("NKS936");
	acidList->push_back("JBU124");
	acidList->push_back("SKW6643");
	acidList->push_back("USA246");
	acidList->push_back("FDX3719");

	acidList->push_back("N188AK");

	acidList->push_back("USA4");
	acidList->push_back("FLG3760");
	acidList->push_back("TCF7564");
	acidList->push_back("N555HD");
	acidList->push_back("AAL2465");
	acidList->push_back("TRS366");
}

void filterFlights(vector<FixPair>& allFlights, vector<string>& acidList) {
	// remove from allFlights entries that don't have acid in acidList.
	// sort acidList for fast searching with binary_search
	std::sort(acidList.begin(), acidList.end());
	for(int i=(int)allFlights.size()-1; i>=0; --i) {
		string acid = allFlights.at(i).callsign;
		if(!std::binary_search(acidList.begin(), acidList.end(), acid)) {
			allFlights.erase(allFlights.begin()+i);
		}
	}
}

static string getCommercialAirportCode(const string& code, const int codeLen=4) {
	if(codeLen == 4) {
		// if the code is less than 4 characters and does not contain
		// any numbers then assume it is commercial.  prefix it with K
		// and return it.
		if(code.length() < 4) {
			for(unsigned int i=0; i<code.length(); ++i) {
				if(isdigit(code.at(i))) {
					return code;
				}
			}
			return "K"+code;
		}
		// otherwise return the existing code
		return code;
	} else {
		// if the code is equal to 4 characters and does not contain
		// numbers and starts with K then return substring of last 3 chars.
		if(code.length() == 4) {
			if(code.at(0) == 'K') {
				return code.substr(1, 3);
			}
		}
		return code;
	}
}

static string get_runway_name(const string& inpstring){
	string rwname = "";

	deque<string> tokens = tokenize(inpstring,"_");

	for(size_t s=0;s<tokens.size(); ++s){
		if ( (tokens.at(s).compare(0,2,"RW") ==0)
			&&(isInteger(tokens.at(s).substr(2,2))) ){
			rwname = rwname + tokens.at(s);
		}
	}

	rwname = rwname +"-"+tokens.at(tokens.size()-1);
	return rwname;
}

#if ULIDEMO
static int parseForDemo(const string& filename,ResultSets* const results){

	string trackfname = filename + "/FlightTrack.dat";
	string routefname = filename + "/FlightRoute.dat";
	string line;
	ifstream myfile (routefname.c_str());

	ResultSetKey rkey(-1,1);
	FixPair fp("","","SWA1897");
	SearchPath spath;spath.clear();
	double ini_heading = 0;
	if (myfile.is_open()){
		int counter = 0;
		string wpname = "";
		while( getline(myfile,line)){
			deque<string> tokens = tokenize(line," ");
			wpname = tokens.at(0);
			if (counter == 0){
				map<string,Airport*>::iterator ita = g_airports.find(wpname);
				if (ita != g_airports.end()){
					fp.origin = wpname;
				}
				else{
					cout <<" ORIGIN NOT FOUND.. EXITING"<< endl;
					exit(-1);
				}
				if (isDouble(tokens.at(3)))
					ini_heading = atof(tokens.at(3).c_str());
				counter++;
				continue;
			}

			Fix* fix = NULL; int fid = get_fix(wpname,&fix);

			if (fid < 0){
				cout << tokens.at(0)<<" NOT FOUND.. EXITING "<<endl;
				exit(-1);
			}
			spath.push_back(fid);
			counter++;
		}
		map<string,Airport* >::iterator ita = g_airports.find(wpname);
		if (ita != g_airports.end()){
			fp.destination = wpname;
		}
		else{
			cout <<" DESTINATION NOT FOUND.. EXITING"<< endl;
			exit(-1);
		}
	}
	myfile.close();

	ifstream myfile2 (trackfname.c_str());
	double altitude = 0;
	string latitude ="",longitude = "",heading = "",fltlevel = "",airspeed = "" ;
	if(myfile2.is_open()){
		while( getline(myfile2,line)){

			deque<string> tokens = tokenize(line," ");
			int altidx = 0;
			for (size_t s= 0; s<tokens.size(); ++s){
				string tok = tokens.at(s);
				if (isDouble(tok) && tok.find(":") == string::npos){
					if (latitude == "") latitude = tok;
					else if (longitude == "") longitude = tok;
					else if (heading == "") heading = tok;
				}
				if (tok == "Level" || tok == "Climbing" || tok == "Descending"){
					if (fltlevel == "") {
						fltlevel = tokens.at(s-1);
						removeSpaces(fltlevel);
						fltlevel.erase(std::remove(fltlevel.begin(), fltlevel.end(), ','), fltlevel.end());
					}

					if (airspeed == "") {
						airspeed = tokens.at(s-3);
						airspeed.erase(std::remove(airspeed.begin(), airspeed.end(), ','), airspeed.end());
					}
					altidx = s-2;
					break;
				}
			}

			if(altidx > 0){
				string altstr = tokens.at(altidx);
				altstr.erase(std::remove(altstr.begin(), altstr.end(), ','), altstr.end());
				double altnow = atof(altstr.c_str());
				if (altnow > altitude){
					altitude = altnow;
				}
			}

		}

	}


	string lat = convertDegToDDMMSS(atof(latitude.c_str()),false);
	string lon = convertDegToDDMMSS(atof(longitude.c_str()),true);

	stringstream flightLevel;
	flightLevel << (int)(  atof( fltlevel.c_str() )/100);
	stringstream  headstr;
	headstr << ini_heading;
	stringstream tas;
	tas << atof(airspeed.c_str());

	fp.altitude = altitude; fp.airspeed = atof(airspeed.c_str());
	ResultSet rset; rset.insert(make_pair(fp,spath));
	results->insert(make_pair(rkey,rset));

	stringstream trx_ss;
	stringstream mfl_ss;
	stringstream scenario_ss;

	trx_ss << g_out_dir << "/rg_out_0_0.trx";
	mfl_ss << g_out_dir << "/rg_out_0_0.mfl";

	string outfile = trx_ss.str();
	string mfloutfile = mfl_ss.str();

	//First Write TRX

	ofstream out;out.open(outfile.c_str());
	out << "TRACK_TIME 1111225555" << endl; //JUST RANDOM DO NOT KNOW WHAT IT MEANS
	out << "TRACK " << fp.callsign << " B733 " << lat << " " << lon << " "
		<< tas.str() << " " << flightLevel.str() << " " << headstr.str()
		<< " ZOA ZOA33" << endl;
	out << "FP_ROUTE ";

	if (spath.getSid() == "")
		out << fp.origin << "..";
	else
		out << fp.origin << "." << spath.getSid() << ".";

	// iterate over each fix id in the path
	int numPoints = spath.size();
	for(int i=0; i<numPoints-1; ++i) {




		// the path is defined as a sequence of fix ids
		// so we need to obtain the fix lat/lon given the
		// fix id.
		//ORIGINAL CODE
		int id = spath[i];//path.at(i)->id;
#if (!USE_NAMES_IN_TRX)
		double lati, longi;
		get_fix_location(id, &lati, &longi);
		string wpnm = "";
		get_fix_name(id,&wpnm);

		string latstr = convertDegToDDMM(lati, false);
		string lonstr = convertDegToDDMM(longi, true);

		out << latstr << "/" << lonstr;
		out << "..";
#else
		string fixname;
		get_fix_name(id, &fixname);
		ss << fixname;
		if ( i < numPoints-2)
			ss << "..";
#endif
	}
	if (spath.getStar() == "")
		out << fp.destination;
	else
		out <<  "." << spath.getSid() << "." << fp.destination;

	out.close();

	//Next Write MFL
	ofstream mflout;mflout.open(mfloutfile.c_str());
	mflout << fp.callsign << " " << (int)(fp.altitude/100) << endl;
	mflout.close();

	return 1;
}
#endif



static void check_trx_mfl_existence(){
	if (!exists_test(rg_trx_file)){
		//Creating trx files
		rg_trx_file = "share/rg/trx/TRX_creation_demo.trx";

		ofstream out;
		out.open( rg_trx_file.c_str() );

		time_t timer;time(&timer);
		stringstream ss;
		ss <<timer;

		out <<"TRACK_TIME " << ss.str() << endl;
		out <<"TRACK ";

		cout <<"Please enter the aircraft id: ";
		string acid; cin >> acid;

		cout <<"Please enter the aircraft type:";
		string actype; cin >> actype;

		cout <<"Please enter the initial latitude in decimals:";
		double lat; cin >> lat;string lat_str = convertDegToDDMMSS(lat, false);

		cout <<"Please enter the initial longitude in decimals:";
		double lon; cin >> lon;string lon_str = convertDegToDDMMSS(lon, true);

		cout<<"Please enter initial speed in knots:";
		double spd; cin >> spd;

		cout<<"Please enter initial flight level:";
		double ftlev; cin >> ftlev;

		cout <<"Please enter the initial heading:";
		double hdg; cin >> hdg;

		cout <<"Please enter the origin:";
		string origin; cin >> origin;

		cout <<"Please enter the destination:";
		string destination; cin >> destination;

		out << acid << " " << actype << " " << lat_str.substr(0,lat_str.length()-1) << " " << lon_str.substr(0,lon_str.length()-1) << " "
			<< spd << " " << ftlev << " " << hdg << " " <<origin << " " << origin << 01 << endl;
		out <<"    FP_ROUTE " << origin << ".."<< destination << endl;

		out.close();

		rg_mfl_file = "share/rg/trx/TRX_creation_demo.mfl";

		out.open( rg_mfl_file.c_str() );

		cout <<"Please enter the maximum flight level:";
		int max_flt_level; cin >> max_flt_level;

		out << acid << " " << max_flt_level;
		out.close();


	}
}


static void get_intercept_point(double* const lat1, double* const lon1, double* const alt1,
							double* const course1_deg,
							double* const lat2, double* const lon2, double* const alt2,
							double* const course2_deg,
							double* lat_intercept, double* lon_intercept, double* h){

	double ref_lat = *lat1;
	double ref_lon = *lon1;


	double x1=0,y1=0,z1=0;
	LatLontoNED(lat1,lon1,alt1,&ref_lat,&ref_lon,&x1,&y1,&z1);



	double x2=0,y2=0,z2=0;
	LatLontoNED(lat2,lon2,alt2,&ref_lat,&ref_lon,&x2,&y2,&z2);

	double m1 = tan( (*course1_deg * M_PI/180.0) );
	double m2 = tan( (*course2_deg * M_PI/180.0));

	double den = (m1-m2);
	double x_intercept = ( (y2-m2*x2) - (y1-m1*x1) ) / den;
	double y_intercept = ( m1*(y2-m2*x2) - m2*(y1-m1*x1) ) / den;
	double z_intercept = (z1+z2)/2; //JUST A PLACE HOLDER PUT 3D LATER

	NEDtoLatLon(&x_intercept, &y_intercept, &z_intercept, &ref_lat,
				&ref_lon,	lat_intercept, lon_intercept, h);

}

void get_proc_properties(const string& procType){


	if (procType == "SID"){
		map<ProcKey,Sid*>::iterator sid_it = rg_sids.begin();
		for (;sid_it != rg_sids.end(); ++sid_it){
			ProcKey key = sid_it->first;

			cout << "Sid name = "<< key.name << ", airport = " << key.airport << endl;
			Sid* sid = sid_it->second;

			map<string, vector<string> > waypoints = sid->waypoints;
			map<string, vector<pair<string,string> > > path_term = sid->path_term;
			map<string, vector<pair<string,string> > > alt_desc = sid->alt_desc;
			map<string, vector<pair<string,double> > > alt_1 = sid->alt_1;
			map<string, vector<pair<string,double> > > alt_2 = sid->alt_2;
			map<string, vector<pair<string,double> > > spd_limit = sid->spd_limit;
			map<string, vector<pair<string,string> > > recco_nav = sid->recco_nav;
			map<string, vector<pair<string,double> > > theta = sid->theta;
			map<string, vector<pair<string,double> > > rho = sid->rho;
			map<string, vector<pair<string,double> > > mag_course = sid->mag_course;
			map<string, vector<pair<string,double> > > rt_dist = sid->rt_dist;

			map<string,vector<string> >::iterator it  = waypoints.begin();
			for ( ;it != waypoints.end(); ++it){
				string legname = it->first;
				cout << "Leg name = " << legname << endl;
				vector<string> wpmap = it->second;
				vector<pair<string,string> > path_term_map = path_term[legname];
				vector<pair<string,string> > alt_desc_map = alt_desc[legname];
				vector<pair<string,double> > alt_1_map = alt_1[legname];
				vector<pair<string,double> > alt_2_map = alt_1[legname];
				vector<pair<string,double> > spd_limit_map = spd_limit[legname];
				vector<pair<string,string> > recco_nav_map = recco_nav[legname];
				vector<pair<string,double> > theta_map = theta[legname];
				vector<pair<string,double> > rho_map = rho[legname];
				vector<pair<string,double> > mag_course_map = mag_course[legname];
				vector<pair<string,double> > rt_dist_map = rt_dist[legname];
				for (size_t k=0; k< wpmap.size(); ++k){
					cout << "Wp name = " << wpmap.at(k)
						 <<" path term = "<< path_term_map.at(k).second
						 <<" alt desc = "<< alt_desc_map.at(k).second
						 <<" alt 1 = "<< alt_1_map.at(k).second
						 <<" alt 2 = "<< alt_2_map.at(k).second
						 <<" spd limit = "<< spd_limit_map.at(k).second
						 <<" recco_nav = "<< recco_nav_map.at(k).second
						 <<" theta = "<< theta_map.at(k).second
						 <<" rho = "<< rho_map.at(k).second
						 <<" mag_course = "<< mag_course_map.at(k).second
						 <<" rt_dist = " <<rt_dist_map.at(k).second
						 << endl;
				}
				cout <<endl;

			}
			cout <<endl;



		}
	}


}

/**
 * Load background data files of RouteGenerator
 */
int load_rg_data_files_NATS(const string rap_file,
		const string cifp_file,
		const string& airportConfigFile,
		const string& airwayConfigFile,
		const string& fixConfigFile) {
	rg_vector_rap_files.clear();

	rg_rap_file = rap_file;
	rg_CIFPfile = cifp_file;

	rg_apconffile = airportConfigFile;
	rg_airwayconffile = airwayConfigFile;
	rg_fixconffile = fixConfigFile;

    // ========================================================================

    cout << "RouteGenerator: Loading airport data..." << endl;
    // Prepare g_airport variables
    load_rg_airports();

    // ========================================================================

    cout << "RouteGenerator: Loading airway data..." << endl;
    // Prepare g_airways data
    load_airways_alt(rg_CIFPfile, rg_airwayconffile);

    // ========================================================================

    cout << "RouteGenerator: Loading SID data..." << endl;
    load_procs_alt(rg_CIFPfile, "SID", DEFAULT_FIXED_PROC);

    cout << "RouteGenerator: Loading STAR data..." << endl;
    load_procs_alt(rg_CIFPfile, "STAR", DEFAULT_FIXED_PROC);

    cout << "RouteGenerator: Loading APPROACH data..." << endl;
    load_procs_alt(rg_CIFPfile, "APPROACH", DEFAULT_FIXED_PROC);

    // ========================================================================

	rg_vector_rap_files = get_h5_files(rg_rap_file);
    read_wind_grid_all_hdf5(rg_vector_rap_files, &rg_wind_vec);

    cout << "RouteGenerator: Loading wind files." << endl;
    // load rap wind
    if (rg_wind_vec.size() == 0)
        read_wind_grid_hdf5(rg_rap_file, &rg_wind);

    // ========================================================================

    // find the unique set of names of the fixes used by
    // airways, sids, stars, and approaches. use this as a filter
    // set for loading fixes

    map<ProcKey, Star*>::iterator star_it;
    map<ProcKey, Sid*>::iterator sid_it;
    map<ProcKey, Approach*>::iterator app_it;
    map<string, Airway*>::iterator air_it;
    map<string, Airport*>::iterator ap_it;

    for (ap_it=osi::rg_airports.begin(); ap_it!=osi::rg_airports.end(); ++ap_it) {
        Airport* ap = ap_it->second;
        unames.insert(ap->name);
    }

    for (air_it=osi::rg_airways.begin(); air_it!=osi::rg_airways.end(); ++air_it) {
        Airway* air = air_it->second;
        for (unsigned int i=0; i<air->waypointNames.size(); ++i) {
            unames.insert(air->waypointNames.at(i));
        }
    }

    for (star_it=osi::rg_stars.begin(); star_it!=osi::rg_stars.end(); ++star_it) {
        Star* star = star_it->second;
        map< string, vector<string> >::iterator wp_it;
        for (wp_it=star->waypoints.begin(); wp_it!=star->waypoints.end(); ++wp_it) {
            vector<string>& wps = wp_it->second;
            for (unsigned int i=0; i<wps.size(); ++i) {
                unames.insert(wps.at(i));
            }
        }
    }

    for (sid_it=osi::rg_sids.begin(); sid_it!=osi::rg_sids.end(); ++sid_it) {
        Sid* sid = sid_it->second;
        map< string, vector<string> >::iterator wp_it;
        for (wp_it=sid->waypoints.begin(); wp_it!=sid->waypoints.end(); ++wp_it) {
            vector<string>& wps = wp_it->second;
            for (unsigned int i=0; i<wps.size(); ++i) {
                unames.insert(wps.at(i));
                size_t pos = wps.at(i).find("NO_WP_NAME_");
                if (pos != string::npos) {
                    string rwname = get_runway_name(wps.at(i));
                    unames.insert(rwname);
                }
            }
        }
    }

    for (app_it = osi::rg_approach.begin(); app_it != osi::rg_approach.end(); ++app_it) {
        Approach* app = app_it->second;
        map< string, vector<string> >::iterator wp_it;
        for (wp_it = app->waypoints.begin(); wp_it != app->waypoints.end(); ++wp_it) {
            vector<string>& wps = wp_it->second;
            for (unsigned int i = 0; i < wps.size(); ++i) {
                unames.insert(wps.at(i));
            }
        }
    }

    rg_supplemental_airway_seq = 0; // Reset

    return 0;
}

int load_rg_weather_files_NATS(
		const string cifp_file,
		const string& polygonFile,
		const string& sigmetFile,
		const string& pirepFile) {
	rg_CIFPfile = cifp_file;
    rg_polygon_file = polygonFile;
    rg_Sigmetfile = sigmetFile;
    rg_PIREPfile = pirepFile;

	if (sigmetFile == "NONE") {
		rg_use_sigmetfile = false;
	} else {
		if (!exists_test(sigmetFile)) {
			rg_use_sigmetfile = false;
		} else {
			rg_use_sigmetfile = true;
		}
	}
	if (pirepFile == "NONE") {
		rg_use_PIREPfile = false;
	} else {
		if (!exists_test(pirepFile)) {
			rg_use_PIREPfile = false;
		} else {
			rg_use_PIREPfile = true;
		}
	}
	if (polygonFile != "NONE") {
		if (!rg_use_sigmetfile) {
			// ok to use polygon file
			cout << "RouteGenerator: Loading polygon file: " << polygonFile << endl;
			load_scenarios(rg_polygon_file);
		}
	}

	// ========================================================================

	/*
	 * LOAD WEATHER POLYGONS HERE. IF YOU ARE LOADING SIGMETS.
	 */
	if (rg_use_sigmetfile)
		processWeatherPolys(rg_CIFPfile, rg_Sigmetfile);

	if (rg_use_PIREPfile) {
		processPIREPs(rg_CIFPfile, rg_PIREPfile);
		if (rg_wind_vec.size())
			processSimStartTime();
	}
}

/**
 * Release memory
 */
void release_rg_resources() {
	cout << "RouteGenerator: Cleaning Up..." << endl;

	unload_rg_airways();
	unload_rg_airports();

	unload_rg_sids();
	unload_rg_stars();

	rg_fixes_by_id.clear();

	rg_fix_names.clear();
	rg_fix_ids.clear();

	map<ProcKey, Approach*>::iterator ite_rg_approach;
	for (ite_rg_approach = rg_approach.begin(); ite_rg_approach != rg_approach.end(); ite_rg_approach++) {
		if (ite_rg_approach->second != NULL) {
			delete ite_rg_approach->second;
		}
	}
	rg_approach.clear();
}

int runRg(int argc, char* argv[]) {

    print_splash();
	parse_args(argc, argv);

	set<string> airportFilter; airportFilter.clear();
	set<FixPair> airportPairs;  // shortest-path airport pairs
	vector<FixPair> flights;    // wind-optimal flights
	vector<string> rap_files; rap_files.clear();

	bool useOriginAndDestination = false;
	bool useAirportsFile = false;
	bool useTrxFile = false;

	(void)useOriginAndDestination;
	(void)useAirportsFile;

	//New adder if exists test fails then create your own trx file

	check_trx_mfl_existence();

	if (!rg_wind_optimal) {

		// make sure we have valid inputs:
		// one of the following must be specified (in order of priority):
		// 1) origin AND destination
		// 2) airports-file
		// 3) trx-file
		// if more than one is specified, then the highest priority is used.
		if ((rg_origin != UNSET_STRING) && (rg_destination != UNSET_STRING)) {
			useOriginAndDestination = true;

			airportPairs.insert(FixPair(rg_origin, rg_destination));
		}
		else if(rg_airports_list_file != UNSET_STRING) {
			useAirportsFile = true;

			for(int i=0; i<rg_num_fix_pairs; ++i) {
				airportPairs.insert(FixPair(rg_origins.at(i),
				                    rg_destinations.at(i)));
			}
		}
		else if(rg_trx_file != UNSET_STRING) {
			useTrxFile = true;

			// detect if the input trx is a new format and set rg_use_new_trx
			// flag if it wasn't explicitly specified by input args
			if(is_new_trx_format(rg_trx_file)) {
			    rg_use_new_trx = true;
			}

			parse_trx_airport_pairs(rg_trx_file, &airportPairs);

		} else {
			cerr << "ERROR: invalid origin/destination input" << endl;
			exit(-1);
		}

	} else {

		// for wind optimal routing, we require the use of trx+mfl files.
		if(rg_trx_file==UNSET_STRING) {
			cerr << "ERROR: invalid TRX file: " << rg_trx_file << endl;
			exit(-1);
		}

        // detect if the input trx is a new format and set rg_use_new_trx
        // flag if it wasn't explicitly specified by input args
        if(is_new_trx_format(rg_trx_file)) {
            rg_use_new_trx = true;
        }

		if(rg_mfl_file==UNSET_STRING && !rg_use_new_trx) {
			cerr << "ERROR: invalid MFL file: " << rg_mfl_file << endl;
			exit(-1);
		}
		// for wind optimal routing, we require a ruc/rap wind file.
		if(rg_rap_file==UNSET_STRING) {
			cerr << "ERROR: invalid RUC/RAP file: " << rg_rap_file << endl;
			exit(-1);
		}

		useTrxFile = true;

		if(rg_use_new_trx) {
		    cout << "Using new TRX format." << endl;
		}

		//TODO:PARIKSHIT ADDER TO GET WIND FILES FOR ALL TIMES.
		rap_files = get_h5_files(rg_rap_file);
		read_wind_grid_all_hdf5(rap_files, &rg_wind_vec);
		//ADDER ENDS
		cout << "Finished reading wind files." <<endl;
		// load rap wind
		if (rg_wind_vec.size() == 0)
			read_wind_grid_hdf5(rg_rap_file, &rg_wind);

		parse_trx_flights(rg_trx_file, rg_mfl_file, &flights);

		parse_trx_airport_pairs(rg_trx_file, &airportPairs);
	}

    // compute the set of unique airport ids to use as a filter set
    // when loading airports, sids, and stars.

	if(rg_filter_by_input_airports) {

        set<FixPair>::iterator apiter;
        for(apiter=airportPairs.begin(); apiter!=airportPairs.end(); ++apiter) {

            // domestic commercial airports in the airports.xml file all use
            // four letter codes starting with K, for example, KSFO and not
            // SFO.
            string origin3 = getCommercialAirportCode(apiter->origin, 3);
            string dest3 = getCommercialAirportCode(apiter->destination, 3);
            string origin4 = getCommercialAirportCode(apiter->origin, 4);
            string dest4 = getCommercialAirportCode(apiter->destination, 4);
            airportFilter.insert(origin3);
            airportFilter.insert(dest3);
            airportFilter.insert(origin4);
            airportFilter.insert(dest4);
        }

	}

	// ensure that all airports in the airportPairs list are 4-letter
	// commercial airport codes starting with K in order to avoid any
	// confusion with navaids/fixes that have the same name.  San Francisco
	// has a navaid SFO, for example, and we don't want to confuse it
	// with the airport object KSFO.
	set<FixPair> airportPairsCopy;
	airportPairsCopy.insert(airportPairs.begin(), airportPairs.end());
	airportPairs.clear();
	set<FixPair>::iterator pair_iter;
	for(pair_iter=airportPairsCopy.begin(); pair_iter!=airportPairsCopy.end();
	        ++pair_iter) {
		string o = getCommercialAirportCode(pair_iter->origin, 4);
		string d = getCommercialAirportCode(pair_iter->destination, 4);
		airportPairs.insert(FixPair(o,d));
	}

	// convert the stl vector of scale factors into a raw array
	// of doubles, as required by the API functions.
	int numScalings = rg_scale_factors.size();
	double scaling[rg_scale_factors.size()];
	for(int i=0; i<numScalings; ++i) {
		scaling[i] = rg_scale_factors.at(i);
	}

	// start timing
	struct timeval tv_start, tv_init, tv_compute, tv_write, tv_end;
	gettimeofday(&tv_start, NULL);

	// load the NFD airway data file.
	// don't forget to unload the data when we're done!
	// the airway data must be loaded before calling any other API
	// functions from TrxPathUtil.h since many of the functions will
	// make use of the global airway and fix data.
	cout << endl;
#if (!USE_CIFP)
    cout << "Loading airport data..." << endl;

    if(g_filter_by_input_airports) {
        load_airports(g_airports_file, &airportFilter);
    } else {
        load_airports(g_airports_file, NULL);
    }


    cout << "Loading airway data..." << endl;
    load_airways(g_airways_file);


    cout << "Loading sid data..." << endl;
    load_sids(g_sids_file, NULL);


    cout << "Loading star data..." << endl;
    load_stars(g_stars_file, NULL);

#else


/** TODO: PARIKSHIT ADDER FOR DIRECT CIFP INPUT*/

    cout << "Loading airport data..." << endl;
    load_airports_alt(rg_CIFPfile,&airportFilter, rg_apconffile);

    cout << "Loading airway data..." << endl;
    load_airways_alt(rg_CIFPfile, rg_airwayconffile);

    cout << "Loading sid data..." << endl;

    load_procs_alt(rg_CIFPfile,"SID", rg_fproc);

    cout << "Loading star data..." << endl;

    load_procs_alt(rg_CIFPfile,"STAR", rg_fproc);

    cout << "Loading approach data..." << endl;

    load_procs_alt(rg_CIFPfile,"APPROACH", rg_fproc);




    // find the unique set of names of the fixes used by
    // airways, sids, stars, and approaches. use this as a filter
    // set for loading fixes
    set<string> unames;
    map<ProcKey, Star*>::iterator star_it;
    map<ProcKey, Sid*>::iterator sid_it;
    map<ProcKey, Approach*>::iterator app_it;
    map<string, Airway*>::iterator air_it;
    map<string, Airport*>::iterator ap_it;

    for(ap_it=osi::rg_airports.begin(); ap_it!=osi::rg_airports.end(); ++ap_it) {
        Airport* ap = ap_it->second;
        unames.insert(ap->name);

    }

    for(air_it=osi::rg_airways.begin(); air_it!=osi::rg_airways.end(); ++air_it) {
        Airway* air = air_it->second;
        for(unsigned int i=0; i<air->waypointNames.size(); ++i) {
            unames.insert(air->waypointNames.at(i));
        }
    }

    for(star_it=osi::rg_stars.begin(); star_it!=osi::rg_stars.end(); ++star_it) {
       Star* star = star_it->second;
       map< string, vector<string> >::iterator wp_it;
       for(wp_it=star->waypoints.begin(); wp_it!=star->waypoints.end(); ++wp_it) {
           vector<string>& wps = wp_it->second;
           for(unsigned int i=0; i<wps.size(); ++i) {
               unames.insert(wps.at(i));
           }
       }
    }

    for(sid_it=osi::rg_sids.begin(); sid_it!=osi::rg_sids.end(); ++sid_it) {
        Sid* sid = sid_it->second;
        map< string, vector<string> >::iterator wp_it;
        for(wp_it=sid->waypoints.begin(); wp_it!=sid->waypoints.end(); ++wp_it) {
            vector<string>& wps = wp_it->second;
            for(unsigned int i=0; i<wps.size(); ++i) {
                unames.insert(wps.at(i));
                size_t pos = wps.at(i).find("NO_WP_NAME_");
                if (pos != string::npos){
                	string rwname = get_runway_name(wps.at(i));
                	unames.insert(rwname);
                }

            }
        }
    }

    for(app_it=osi::rg_approach.begin(); app_it!=osi::rg_approach.end(); ++app_it) {
        Approach* app = app_it->second;
        map< string, vector<string> >::iterator wp_it;
        for(wp_it=app->waypoints.begin(); wp_it!=app->waypoints.end(); ++wp_it) {
            vector<string>& wps = wp_it->second;
            for(unsigned int i=0; i<wps.size(); ++i) {
                unames.insert(wps.at(i));
            }
        }
    }

    cout << "Loading Fix data..." << endl;
    load_fixes(rg_CIFPfile, rg_fixconffile, &airportFilter, &unames);

    cout << "Running star approach map..." << endl;
    star_approach_map();



    /*
     * LOAD WEATHER POLYGONS HERE. IF YOU ARE LOADING SIGMETS.
     */
    if (rg_use_sigmetfile)
        processWeatherPolys(rg_CIFPfile, rg_Sigmetfile);
    if (rg_use_PIREPfile){
        processPIREPs(rg_CIFPfile, rg_PIREPfile);
        if (rg_wind_vec.size() )
            processSimStartTime();
    }
#endif


    /** TODO: PARIKSHIT ADDER ENDS HERE*/
	// build the network connectivity from airways
	cout << "Building network connectivity..." << endl;
	SearchGraph graph;
	get_airway_connectivity(&graph, rap_files.size());


	// no longer need sids and stars so unload
	//TODO:PARIKSHIT ADDER: REMOVED THEM TO END FOR GENERATION OF RUNWAY TO
	//RUNWAY TRAJECTORY.


	gettimeofday(&tv_init, NULL);

	// compute the shortest path between the specified airport pair
	// you would call this function repeatedly for each pair of
	// origin/destination airports.  This version of get_paths()
	// takes in the weather scenarios and scale factors.  It will find
	// the shortest path around the polygons in each scenario at each
	// scale factor.  For example, if there are 3 polygons in two
	// scenarios (the original + the perturbed) and 3 scale factors
	// (1x, 2x, 3x).  It will produce 6 paths:
	// scenario 1 around 1x (possibly concave) polygons
	// scenario 1 around 2x convex polygons
	// scenario 1 around 3x convex polygons
	// scenario 2 around 1x (possibly convcave) polygons
	// scenario 2 around 2x convex polygons
	// scenario 2 around 3x convex polygons

	ResultSets results;
	ResultTrajs t_results; //For TOS
	PolygonSets resultPolys;

	tos_gen_method t_method = REM_ENTIRE_PATH;

	bool replanFlag = !rg_reroute;
#if 0
	for (unsigned int k = 0;k<rg_scenarios.size();++k){
		for (unsigned int s = 0 ;s<rg_scenarios.at(k).size();++s){
			Polygon *poly = &rg_scenarios.at(k).at(s);
			int n = poly->getNumVertices();
			const double* lons = poly->getXData();
			const double* lats = poly->getYData();
			cout<< "Scenario = "<< k+1 << " poly type = "<<poly->getPolyType() << endl;
			cout<<" start hour " << poly->getStartHour() << " end hour = " << poly->getEndHour() <<endl;
			for(int j=0; j<n; ++j) {
				cout << setprecision(10) << lons[j] <<","
					 << setprecision(10) << lats[j] <<","
					 << 0.0 << endl;
			}
			cout << endl;
		}
	}
#endif

#if ULIDEMO
	cout << "Running Demo for ULI and Exiting..." << endl;
	parseForDemo("share/rg/ULIDemo",&results);
	rg_get_TOS = false;
#else

	if(!rg_wind_optimal) {
		cout << "Getting shortest paths..." << endl << endl;
		get_paths(graph, airportPairs, rg_scenarios, scaling, numScalings,
			  &results, &resultPolys, replanFlag);
	} else {
		if (!rg_get_TOS){
			cout << "Getting wind optimal paths..." << endl << endl;
			get_wind_optimal_paths(graph, flights, &rg_wind, &rg_wind_vec, rg_scenarios, scaling,
					numScalings, &results, &resultPolys, replanFlag);
		}
		else{
			cout << "Getting wind optimal trajectory option sets..." << endl;
			get_wind_optimal_TOS(graph, flights, &rg_wind, &rg_wind_vec, &rg_pirep, rg_scenarios, scaling,
					numScalings, &t_results, &resultPolys, rg_num_trajs, replanFlag,
					t_method, rg_fproc);
		}
	}
#endif
	gettimeofday(&tv_compute, NULL);

	// do something useful with the results...
	// here, we will iterate through the result set and print the
	// data to stdout.  we will also print the paths to a delimited
	// text file so that the paths may be plotted in gnuplot, matlab,
	// octave, or some other plotting package.
	// first, iterate over each scenario's result set at each scale
	cout << endl << "Writing Output Files..." << endl;
	stringstream outPath;
	stringstream polysOutPath;
	stringstream kmlOutPath;
	stringstream kmlPolyOutPath;
	stringstream kmlPIREPOutPath;

	outPath << rg_out_dir << "/paths_out.dat";
	polysOutPath << rg_out_dir << "/polys_out.dat";
	kmlOutPath << rg_out_dir << "/paths_out.kml"; //TODO :PARIKSHIT ADDER
	kmlPolyOutPath << rg_out_dir << "/polys_out.kml"; //TODO :PARIKSHIT ADDER
	kmlPIREPOutPath << rg_out_dir << "/pireps_out.kml";

	ofstream out;
	ofstream polysOut;
	ofstream kmlOut;//TODO :PARIKSHIT ADDER
	ofstream kmlPolyOut;//TODO :PARIKSHIT ADDER
	ofstream kmlPIREPOut;

	out.open( outPath.str().c_str() );
	polysOut.open( polysOutPath.str().c_str() );
	kmlOut.open( kmlOutPath.str().c_str() );//TODO :PARIKSHIT ADDER
	kmlPolyOut.open( kmlPolyOutPath.str().c_str() );//TODO :PARIKSHIT ADDER
	kmlPIREPOut.open( kmlPIREPOutPath.str().c_str() );
	kml_splash( kmlOut );
	kml_splash(kmlPolyOut,true);
	kml_write_pireps( kmlPIREPOut );


	vector<ResultSetKey> resultKeys;

	if (!rg_get_TOS){
		get_result_sets_keys<ResultSets>(results, &resultKeys);

		int pathcounter = 0;
		vector<ResultSetKey>::iterator resultKeyIter;
		for(resultKeyIter=resultKeys.begin(); resultKeyIter!=resultKeys.end();
				++resultKeyIter) {
			ResultSetKey resultSetKey = *resultKeyIter;
			int scenario = resultSetKey.scenario;
			double scale = resultSetKey.scale;

			// obtain the ResultSet for the scenario,scale combination
			const ResultSet* result = get_result_set<ResultSets,ResultSet>(scenario, scale, results);
			if(!result) {
				cerr << "ERROR: no results for scenario " << scenario << " at"
						" scale level " << scale << endl;
				continue;
			}

			// find the result's paths, keyed by FixPairs.
			vector<FixPair> keys;

			int err = get_result_keys<ResultSet>(result, &keys);
			if(err < 0) {
				cerr << "ERROR: failed to obtain result set for "
						"scenario " << scenario << " at scale level "
						<< scale << endl;
				continue;
			}

			// iterate over the FixPair keys to obtain the paths
			// and print the paths to the outputs
			for(unsigned int i=0; i<keys.size(); ++i) {
				FixPair key = keys.at(i);
				SearchPath path;
				err = get_result_value(result, key, &path);
				if(err < 0) {
					cerr << "ERROR: failed to obtain path for scenario "
							<< scenario << " at scale level "
							<< scale << " for "
							<< key.origin << "->" << key.destination << endl;
					continue;
				}

				print_path(scenario, scale, key, path, out);
				int pathnum = (rg_num_trajs == 0 )?(pathcounter%5):(pathcounter%rg_num_trajs);

				print_kml(path, kmlOut,  key.origin, key.destination, pathnum );
				pathcounter++;
				if(!useTrxFile) {

					stringstream trx_ss;
					stringstream mfl_ss;
					stringstream scenario_ss;
					if(scenario < 0) {
						scenario_ss << "nom";
					} else {
						scenario_ss << scenario;
					}
					trx_ss << rg_out_dir << "/rg_out_" << pathnum << "_" << scenario_ss.str()
	                		<< "_" << scale << ".trx";
					mfl_ss << rg_out_dir << "/rg_out_" << pathnum << "_" << scenario_ss.str()
	                		<< "_" << scale << ".mfl";

					string outfile = trx_ss.str();
					string mfloutfile = mfl_ss.str();

					if(rg_use_new_trx) {

					} else {
                        write_trx(outfile, "OSI123", rg_origin, rg_destination, path,
                                rg_wind_optimal);
                        write_mfl(mfloutfile, "OSI123", 330);
					}
				}
			}

			// write trx for the current set of paths
			if(useTrxFile) {
				stringstream trx_ss;
				stringstream mfl_ss;
				stringstream scenario_ss;
				if(scenario < 0) {
					scenario_ss << "nom";
				} else {
					scenario_ss << scenario;
				}
				trx_ss << rg_out_dir << "/rg_out_" << pathcounter << "_" << scenario_ss.str()
			    		   << "_" << scale << ".trx";
				mfl_ss << rg_out_dir << "/rg_out_" << pathcounter << "_" << scenario_ss.str()
			    		   << "_" << scale << ".mfl";

				string outfile = trx_ss.str();
				string mfloutfile = mfl_ss.str();

				if(rg_use_new_trx) {

				} else {
                    if(rg_trx_file != UNSET_STRING) {
                        write_trx(rg_trx_file, outfile, result, rg_wind_optimal);
                    }
                    if(rg_trx_file != UNSET_STRING && rg_mfl_file != UNSET_STRING) {
                        write_mfl(rg_mfl_file, mfloutfile, outfile);
                    }
				}
			}

			// print polygons to out file
			// watch out, the nominal scenario has id of -10
			if(scenario >= 0) {
				PolygonSetKey polyKey(scenario, scale);
				const PolygonSet* polySet = &(resultPolys.at(polyKey));
				for(unsigned int i=0; i<polySet->size(); ++i) {
					const Polygon* poly = &(polySet->at(i));
					print_polygon(scenario, scale, *poly, polysOut);
					print_poly_kml(*poly,kmlPolyOut,i);
				}
			}
		}
	}//if (!g_get_TOS)
	else{

		get_result_sets_keys<ResultTrajs>(t_results, &resultKeys);

		int pathcounter = 0;
		vector<ResultSetKey>::iterator resultKeyIter;
		for(resultKeyIter=resultKeys.begin(); resultKeyIter!=resultKeys.end();
				++resultKeyIter) {
			ResultSetKey resultSetKey = *resultKeyIter;
			int scenario = resultSetKey.scenario;

			if (scenario <0 && numScalings) continue;
			double scale = resultSetKey.scale;

			// obtain the ResultSet for the scenario,scale combination
			const ResultTraj* result = get_result_set<ResultTrajs,ResultTraj>(scenario, scale, t_results);
			if(!result) {
				cerr << "ERROR: no results for scenario " << scenario << " at"
						" scale level " << scale << endl;
				continue;
			}

			// find the result's paths, keyed by FixPairs.
			vector<FixPair> keys;

			int err = get_result_keys<ResultTraj>(result, &keys);
			if(err < 0) {
				cerr << "ERROR: failed to obtain result set for "
						"scenario " << scenario << " at scale level "
						<< scale << endl;
				continue;
			}

			// iterate over the FixPair keys to obtain the paths
			// and print the paths to the outputs
			for(unsigned int i=0; i<keys.size(); ++i) {
				FixPair key = keys.at(i);
				vector<SearchPath> paths;
				err = get_result_value_TOS(result, key, &paths);
				if(err < 0) {
					cerr << "ERROR: failed to obtain path for scenario "
							<< scenario << " at scale level "
							<< scale << " for "
							<< key.origin << "->" << key.destination << endl;
					continue;
				}

				for(unsigned int pcnt =0;pcnt < paths.size(); ++pcnt){
					SearchPath path = paths.at(pcnt);
					print_path(scenario, scale, key, path, out);
					print_kml(path, kmlOut,key.origin, key.destination, (pathcounter%rg_num_trajs));
					pathcounter++;
					if(!useTrxFile) {
						stringstream trx_ss;
						stringstream mfl_ss;
						stringstream scenario_ss;
						if(scenario < 0) {
							scenario_ss << "nom";
						} else {
							scenario_ss << scenario;
						}
						trx_ss << rg_out_dir << "/rg_out_" << scenario_ss.str()
								<< "_" << scale << ".trx";
						mfl_ss << rg_out_dir << "/rg_out_" << scenario_ss.str()
								<< "_" << scale << ".mfl";

						string outfile = trx_ss.str();
						string mfloutfile = mfl_ss.str();

						if(rg_use_new_trx) {
						    write_trx(outfile, "OSI123", rg_origin, rg_destination, path,
						                                        rg_wind_optimal, 330);
						} else {
                            write_trx(outfile, "OSI123", rg_origin, rg_destination, path,
                                    rg_wind_optimal);
                            write_mfl(mfloutfile, "OSI123", 330);
						}
					}
				}

			}

			// write trx for the current set of paths
			if(useTrxFile) {
				for (int nt = 0;nt<rg_num_trajs;++nt){
					stringstream trx_ss;
					stringstream mfl_ss;
					stringstream scenario_ss;
					if(scenario < 0) {
						scenario_ss << "nom";
					} else {
						scenario_ss << scenario;
					}
					trx_ss << rg_out_dir << "/rg_out_" << nt << "_" << scenario_ss.str()
							<< "_" << scale << ".trx";
					mfl_ss << rg_out_dir << "/rg_out_" << nt << "_" << scenario_ss.str()
							<< "_" << scale << ".mfl";
					string outfile = trx_ss.str();
					string mfloutfile = mfl_ss.str();


//					NEED TO IMPLEMENT NEW VERSION OF THIS.THIS IS THE BEST I COULD DO.

					ResultSet* resset = new ResultSet();
					ResultTraj::const_iterator itv;
					for(itv = result->begin();itv != result->end(); ++itv){
						//SEE THIS FROM HERE
						if( (unsigned int)nt >= itv->second.size())
							continue;
						FixPair *fp =const_cast<FixPair*> (&(itv->first));
						SearchPath *sp = const_cast<SearchPath*> (&( (itv->second).at(nt) ) );
						resset->insert(std::pair<FixPair, SearchPath>(*fp,*sp));
					}
					if(rg_use_new_trx) {
					    write_trx(rg_trx_file, rg_mfl_file, outfile, resset, rg_wind_optimal);
					} else {
					    write_trx(rg_trx_file, outfile, resset, rg_wind_optimal);
					    write_mfl(rg_mfl_file, mfloutfile, outfile);
					}
					delete resset;
				}
			}

			// print polygons to out file
			// watch out, the nominal scenario has id of -10
			if(scenario >= 0) {
				PolygonSetKey polyKey(scenario, scale);
				const PolygonSet* polySet = &(resultPolys.at(polyKey));
				for(unsigned int i=0; i<polySet->size(); ++i) {
					const Polygon* poly = &(polySet->at(i));
					print_polygon(scenario, scale, *poly, polysOut);
					print_poly_kml(*poly,kmlPolyOut,i);
				}
			}
		}
	}

	kmlOut << "</Document>\n	</kml>"<<endl;
	kmlOut.close();
	kmlPolyOut<<"</Folder>\n</Document>\n</kml>" << endl;
	kmlPIREPOut.close();
	kmlPolyOut.close();
	polysOut.close();
	out.close();


	gettimeofday(&tv_write, NULL);

	cout << "Cleaning Up..." << endl;

	// free the NFD airway data.
	// you can no longer call any of the API functions, including
	// get_fix_*() functions and get_airway_*() functions after
	// calling unload_airways().
	unload_rg_airways();
	unload_rg_airports();

	unload_rg_sids();
	unload_rg_stars();

	gettimeofday(&tv_end, NULL);

	cout << "Done." << endl;

	// compute run times
	double dt_init = compute_dt(tv_start, tv_init);
	double dt_compute = compute_dt(tv_init, tv_compute);
	double dt_write = compute_dt(tv_compute, tv_write);
	double dt_cleanup = compute_dt(tv_write, tv_end);
	double dt = compute_dt(tv_start, tv_end);

	cout << endl;
	cout << "Program execution time: " << dt << " seconds" << endl;
	cout << "        Initialization: " << dt_init << " seconds" << endl;
	cout << "            Processing: " << dt_compute << " seconds" << endl;
	cout << "              Printing: " << dt_write << " seconds" << endl;
	cout << "               Cleanup: " << dt_cleanup << " seconds" << endl;
	cout << endl;

	cout << "Good bye." << endl;

	return 0;
}

void supplement_airways_by_flightPlans(waypoint_node_t* waypoint_node_ptr) {
	if (waypoint_node_ptr != NULL) {
		supplement_airways_by_flightPlans_logic(waypoint_node_ptr);
	}
}

#if 0
  int runRgOptimized(int argc, char* argv[], ResultTrajs& t_results){

    print_splash();

    cout << "argc=" << argc << endl;
    cout << "argv=" << endl;
    for(int i=0; i<argc; ++i) {
      cout << "  " << string(argv[i]) << endl;
    }
    parse_args_alt(argc, argv);

    set<string> airportFilter; airportFilter.clear();
    airportFilter.insert(rg_origin);airportFilter.insert(rg_destination);

    set<FixPair> airportPairs;  // shortest-path airport pairs
    vector<FixPair> flights;    // wind-optimal flights
    vector<string> rap_files; rap_files.clear();

    FixPair fp(rg_origin,rg_destination,rg_callsign);
    fp.airspeed = g_airspeed;
    fp.altitude = g_altitude;
    airportPairs.insert(fp);
    flights.push_back(fp);

    bool useOriginAndDestination = false;
    bool useAirportsFile = false;

    (void)useOriginAndDestination;
    (void)useAirportsFile;


    //TODO:PARIKSHIT ADDER TO GET WIND FILES FOR ALL TIMES.
    rap_files = get_h5_files(g_rap_file);
    read_wind_grid_all_hdf5(rap_files, &g_wind_vec);
    //ADDER ENDS
    cout << "Finished reading wind files." <<endl;
    // load rap wind
    if (g_wind_vec.size() == 0)
      read_wind_grid_hdf5(g_rap_file, &g_wind);



    // convert the stl vector of scale factors into a raw array
    // of doubles, as required by the API functions.
    int numScalings = g_scale_factors.size();
    double scaling[g_scale_factors.size()];
    for(int i=0; i<numScalings; ++i) {
      scaling[i] = g_scale_factors.at(i);
    }



    // start timing
    struct timeval tv_start, tv_init, tv_compute, tv_write, tv_end;
    gettimeofday(&tv_start, NULL);

    // load the NFD airway data file.
    // don't forget to unload the data when we're done!
    // the airway data must be loaded before calling any other API
    // functions from TrxPathUtil.h since many of the functions will
    // make use of the global airway and fix data.
    cout << endl;

    /** TODO: PARIKSHIT ADDER FOR DIRECT CIFP INPUT*/

    cout << "Loading airport data..." << endl;
    load_airports_alt(CIFPfile,&airportFilter,apconffile);

    cout << "Loading airway data..." << endl;
    load_airways_alt(CIFPfile,airwayconffile);

    cout << "Loading sid data..." << endl;

    load_procs_alt(CIFPfile,"SID",fproc);

    cout << "Loading star data..." << endl;

    load_procs_alt(CIFPfile,"STAR",fproc);

    cout << "Loading approach data..." << endl;

    load_procs_alt(CIFPfile,"APPROACH",fproc);





    // find the unique set of names of the fixes used by
    // airways, sids, stars, and approaches. use this as a filter
    // set for loading fixes
    set<string> unames;
    map<ProcKey, Star*>::iterator star_it;
    map<ProcKey, Sid*>::iterator sid_it;
    map<ProcKey, Approach*>::iterator app_it;
    map<string, Airway*>::iterator air_it;
    map<string, Airport*>::iterator ap_it;

    for(ap_it=osi::g_airports.begin(); ap_it!=osi::g_airports.end(); ++ap_it) {
      Airport* ap = ap_it->second;
      unames.insert(ap->name);

    }

    for(air_it=osi::g_airways.begin(); air_it!=osi::g_airways.end(); ++air_it) {
      Airway* air = air_it->second;
      for(unsigned int i=0; i<air->waypointNames.size(); ++i) {
	unames.insert(air->waypointNames.at(i));
      }
    }

    for(star_it=osi::g_stars.begin(); star_it!=osi::g_stars.end(); ++star_it) {
      Star* star = star_it->second;
      map< string, vector<string> >::iterator wp_it;
      for(wp_it=star->waypoints.begin(); wp_it!=star->waypoints.end(); ++wp_it) {
	vector<string>& wps = wp_it->second;
	for(unsigned int i=0; i<wps.size(); ++i) {
	  unames.insert(wps.at(i));
	}
      }
    }

    for(sid_it=osi::g_sids.begin(); sid_it!=osi::g_sids.end(); ++sid_it) {
      Sid* sid = sid_it->second;
      map< string, vector<string> >::iterator wp_it;
      for(wp_it=sid->waypoints.begin(); wp_it!=sid->waypoints.end(); ++wp_it) {
	vector<string>& wps = wp_it->second;
	for(unsigned int i=0; i<wps.size(); ++i) {
	  unames.insert(wps.at(i));
	  size_t pos = wps.at(i).find("NO_WP_NAME_");
	  if (pos != string::npos){
	    string rwname = get_runway_name(wps.at(i));
	    unames.insert(rwname);
	  }

	}
      }
    }

    for(app_it=osi::g_approach.begin(); app_it!=osi::g_approach.end(); ++app_it) {
      Approach* app = app_it->second;
      map< string, vector<string> >::iterator wp_it;
      for(wp_it=app->waypoints.begin(); wp_it!=app->waypoints.end(); ++wp_it) {
	vector<string>& wps = wp_it->second;
	for(unsigned int i=0; i<wps.size(); ++i) {
	  unames.insert(wps.at(i));
	}
      }
    }

    cout << "Loading Fix data..." << endl;
    load_fixes(CIFPfile, fixconffile, &airportFilter, &unames);

    /*
     * LOAD WEATHER POLYGONS HERE. IF YOU ARE LOADING SIGMETS.
     */
    if (g_use_sigmetfile)
      processWeatherPolys(CIFPfile,Sigmetfile);
    if (g_use_PIREPfile){
      processPIREPs(CIFPfile,PIREPfile);
      if (g_wind_vec.size() )
	processSimStartTime();
    }

    /** TODO: PARIKSHIT ADDER ENDS HERE*/
    // build the network connectivity from airways
    cout << "RouteGenerator: Building network connectivity..." << endl;
    SearchGraph graph;
    get_airway_connectivity(&graph, rap_files.size());


    gettimeofday(&tv_init, NULL);

    // compute the shortest path between the specified airport pair
    // you would call this function repeatedly for each pair of
    // origin/destination airports.  This version of get_paths()
    // takes in the weather scenarios and scale factors.  It will find
    // the shortest path around the polygons in each scenario at each
    // scale factor.  For example, if there are 3 polygons in two
    // scenarios (the original + the perturbed) and 3 scale factors
    // (1x, 2x, 3x).  It will produce 6 paths:
    // scenario 1 around 1x (possibly concave) polygons
    // scenario 1 around 2x convex polygons
    // scenario 1 around 3x convex polygons
    // scenario 2 around 1x (possibly convcave) polygons
    // scenario 2 around 2x convex polygons
    // scenario 2 around 3x convex polygons

    ResultSets results;
    //ResultTrajs t_results; //For TOS
    PolygonSets resultPolys;

    tos_gen_method t_method = REM_ENTIRE_PATH;
    //This is the edge number to start when removing edges to create TOS in
    // REM_ENTIRE_PATH method
    int offset_for_removing_edges = 0;

    bool replanFlag = !g_reroute;

    if(!g_wind_optimal) {
      cout << "Getting shortest paths..." << endl << endl;
      get_paths(graph, airportPairs, g_scenarios, scaling, numScalings,
		&results, &resultPolys, replanFlag);
    } else {
      if (!g_get_TOS){
	cout << "Getting wind optimal paths..." << endl << endl;
	get_wind_optimal_paths(graph, flights, &g_wind, &g_wind_vec, g_scenarios, scaling,
			       numScalings, &results, &resultPolys, replanFlag);
      }
      else{
	cout << "Getting wind optimal trajectory option sets..." << endl << endl;
	get_wind_optimal_TOS(graph, flights, &g_wind, &g_wind_vec, &g_pirep, g_scenarios, scaling,
			     numScalings, &t_results, &resultPolys, g_num_trajs, replanFlag,
			     t_method,fproc,offset_for_removing_edges);
      }
    }

    gettimeofday(&tv_compute, NULL);

    bool enableFileOut = false;
    if(enableFileOut) {
      // do something useful with the results...
      // here, we will iterate through the result set and print the
      // data to stdout.  we will also print the paths to a delimited
      // text file so that the paths may be plotted in gnuplot, matlab,
      // octave, or some other plotting package.
      // first, iterate over each scenario's result set at each scale
      cout << endl << "Writing Output Files..." << endl;
      stringstream outPath;
      stringstream polysOutPath;
      stringstream kmlOutPath;
      stringstream kmlPolyOutPath;
      stringstream kmlPIREPOutPath;

      outPath << g_out_dir << "/paths_out.dat";
      polysOutPath << g_out_dir << "/polys_out.dat";
      kmlOutPath << g_out_dir << "/paths_out.kml"; //TODO :PARIKSHIT ADDER
      kmlPolyOutPath << g_out_dir << "/polys_out.kml"; //TODO :PARIKSHIT ADDER
      kmlPIREPOutPath << g_out_dir << "/pireps_out.kml";

      ofstream out;
      ofstream polysOut;
      ofstream kmlOut;//TODO :PARIKSHIT ADDER
      ofstream kmlPolyOut;//TODO :PARIKSHIT ADDER
      ofstream kmlPIREPOut;

      out.open( outPath.str().c_str() );
      polysOut.open( polysOutPath.str().c_str() );
      kmlOut.open( kmlOutPath.str().c_str() );//TODO :PARIKSHIT ADDER
      kmlPolyOut.open( kmlPolyOutPath.str().c_str() );//TODO :PARIKSHIT ADDER
      kmlPIREPOut.open( kmlPIREPOutPath.str().c_str() );
      kml_splash( kmlOut );
      kml_splash(kmlPolyOut,true);
      kml_write_pireps( kmlPIREPOut );


      vector<ResultSetKey> resultKeys;

      if (!g_get_TOS){
	get_result_sets_keys<ResultSets>(results, &resultKeys);

	int pathcounter = 0;
	vector<ResultSetKey>::iterator resultKeyIter;
	for(resultKeyIter=resultKeys.begin(); resultKeyIter!=resultKeys.end();
	    ++resultKeyIter) {
	  ResultSetKey resultSetKey = *resultKeyIter;
	  int scenario = resultSetKey.scenario;
	  double scale = resultSetKey.scale;

	  // obtain the ResultSet for the scenario,scale combination
	  const ResultSet* result = get_result_set<ResultSets,ResultSet>(scenario, scale, results);
	  if(!result) {
	    cerr << "ERROR: no results for scenario " << scenario << " at"
	      " scale level " << scale << endl;
	    continue;
	  }

	  // find the result's paths, keyed by FixPairs.
	  vector<FixPair> keys;

	  int err = get_result_keys<ResultSet>(result, &keys);
	  if(err < 0) {
	    cerr << "ERROR: failed to obtain result set for "
	      "scenario " << scenario << " at scale level "
		 << scale << endl;
	    continue;
	  }

	  // iterate over the FixPair keys to obtain the paths
	  // and print the paths to the outputs
	  for(unsigned int i=0; i<keys.size(); ++i) {
	    FixPair key = keys.at(i);
	    SearchPath path;
	    err = get_result_value(result, key, &path);
	    if(err < 0) {
	      cerr << "ERROR: failed to obtain path for scenario "
		   << scenario << " at scale level "
		   << scale << " for "
		   << key.origin << "->" << key.destination << endl;
	      continue;
	    }

	    print_path(scenario, scale, key, path, out);
	    int pathnum = (g_num_trajs == 0 )?(pathcounter%5):(pathcounter%g_num_trajs);

	    print_kml(path, kmlOut,  key.origin, key.destination, pathnum );
	    pathcounter++;
	  }


	  // print polygons to out file
	  // watch out, the nominal scenario has id of -10
	  if(scenario >= 0) {
	    PolygonSetKey polyKey(scenario, scale);
	    const PolygonSet* polySet = &(resultPolys.at(polyKey));
	    for(unsigned int i=0; i<polySet->size(); ++i) {
	      const Polygon* poly = &(polySet->at(i));
	      print_polygon(scenario, scale, *poly, polysOut);
	      print_poly_kml(*poly,kmlPolyOut,i);
	    }
	  }
	}
      }//if (!g_get_TOS) ends
      else{

	get_result_sets_keys<ResultTrajs>(t_results, &resultKeys);

	int pathcounter = 0;
	vector<ResultSetKey>::iterator resultKeyIter;
	for(resultKeyIter=resultKeys.begin(); resultKeyIter!=resultKeys.end();
	    ++resultKeyIter) {
	  ResultSetKey resultSetKey = *resultKeyIter;
	  int scenario = resultSetKey.scenario;

	  if (scenario <0 && numScalings) continue;
	  double scale = resultSetKey.scale;

	  // obtain the ResultSet for the scenario,scale combination
	  const ResultTraj* result = get_result_set<ResultTrajs,ResultTraj>(scenario, scale, t_results);
	  if(!result) {
	    cerr << "ERROR: no results for scenario " << scenario << " at"
	      " scale level " << scale << endl;
	    continue;
	  }

	  // find the result's paths, keyed by FixPairs.
	  vector<FixPair> keys;

	  int err = get_result_keys<ResultTraj>(result, &keys);
	  if(err < 0) {
	    cerr << "ERROR: failed to obtain result set for "
	      "scenario " << scenario << " at scale level "
		 << scale << endl;
	    continue;
	  }

	  // iterate over the FixPair keys to obtain the paths
	  // and print the paths to the outputs
	  for(unsigned int i=0; i<keys.size(); ++i) {
	    FixPair key = keys.at(i);
	    vector<SearchPath> paths;
	    err = get_result_value_TOS(result, key, &paths);
	    if(err < 0) {
	      cerr << "ERROR: failed to obtain path for scenario "
		   << scenario << " at scale level "
		   << scale << " for "
		   << key.origin << "->" << key.destination << endl;
	      continue;
	    }

	    for(unsigned int pcnt =0;pcnt < paths.size(); ++pcnt){
	      SearchPath path = paths.at(pcnt);
	      print_path(scenario, scale, key, path, out);
	      print_kml(path, kmlOut,key.origin, key.destination, (pathcounter%g_num_trajs));
	      pathcounter++;
	    }

	  }

	  // print polygons to out file
	  // watch out, the nominal scenario has id of -10
	  if(scenario >= 0) {
	    PolygonSetKey polyKey(scenario, scale);
	    const PolygonSet* polySet = &(resultPolys.at(polyKey));
	    for(unsigned int i=0; i<polySet->size(); ++i) {
	      const Polygon* poly = &(polySet->at(i));
	      print_polygon(scenario, scale, *poly, polysOut);
	      print_poly_kml(*poly,kmlPolyOut,i);
	    }
	  }
	}
      }

      kmlOut << "</Document>\n	</kml>"<<endl;
      kmlOut.close();
      kmlPolyOut<<"</Folder>\n</Document>\n</kml>" << endl;
      kmlPIREPOut.close();
      kmlPolyOut.close();
      polysOut.close();
      out.close();

      gettimeofday(&tv_write, NULL);
    }

    cout << "Cleaning Up..." << endl;

    // free the NFD airway data.
    // you can no longer call any of the API functions, including
    // get_fix_*() functions and get_airway_*() functions after
    // calling unload_airways().
    unload_airways();
    unload_airports();

    unload_sids();
    unload_stars();

    gettimeofday(&tv_end, NULL);

    cout << "Done." << endl;

    // compute run times
    double dt_init = compute_dt(tv_start, tv_init);
    double dt_compute = compute_dt(tv_init, tv_compute);
    double dt_write = compute_dt(tv_compute, tv_write);
    double dt_cleanup = compute_dt(tv_write, tv_end);
    double dt = compute_dt(tv_start, tv_end);

    cout << endl;
    cout << "Program execution time: " << dt << " seconds" << endl;
    cout << "        Initialization: " << dt_init << " seconds" << endl;
    cout << "            Processing: " << dt_compute << " seconds" << endl;
    cout << "              Printing: " << dt_write << " seconds" << endl;
    cout << "               Cleanup: " << dt_cleanup << " seconds" << endl;
    cout << endl;

    cout << "Good bye." << endl;

    return 0;
}
#endif /* 0 */


int runRgOptimized(const string& origin,
		   const string& destination,
		   const string& procName,
		   const double& cruiseAlt,
		   const double& cruiseSpd,
		   const string& rapFile,
		   const string& cifpFile,
		   const string& polygonFile,
		   const string& sigmetFile,
		   const string& pirepFile,
		   const string& airportConfigFile,
		   const string& airwayConfigFile,
		   const string& fixConfigFile,
		   const string& procConfigFile, 
		   const bool& rerouteFlag,
		   const bool& windOptimalFlag,
		   const bool& filterAirports,
		   const int& numOpts,
		   const double& costIndex,
		   ResultTrajs& t_results,
		   vector<string>& routes,
		   vector<double>& costs,
		   vector< vector<double> >& latitudes,
		   vector< vector<double> >& longitudes,
		   vector< vector<double> >& polygonLatitudes,
		   vector< vector<double> >& polygonLongitudes) {
    print_splash();

    rg_origin = origin;
    rg_destination = destination;
    rg_altitude = cruiseAlt;
    rg_airspeed = cruiseSpd;
    rg_rap_file = rapFile;
    rg_CIFPfile = cifpFile;

    rg_polygon_file = polygonFile;
    rg_Sigmetfile = sigmetFile;
    rg_PIREPfile = pirepFile;

    rg_apconffile = airportConfigFile;
    rg_airwayconffile = airwayConfigFile;
    rg_fixconffile = fixConfigFile;
    rg_procconffile = procConfigFile;
    rg_reroute = rerouteFlag;
    rg_wind_optimal = windOptimalFlag;
    rg_filter_by_input_airports = filterAirports;
    rg_num_trajs = numOpts;
    rg_get_TOS = true;
    rg_tos_cost_index = costIndex;
    rg_scale_factors.push_back(1);

    // dummy callsign
    rg_callsign = "OSI123";

    // TODO: this needs to be input
    rg_fproc.airport = destination;//"KLAX";
    rg_fproc.procType = "STAR";
    rg_fproc.procName = procName;//"SADDE";

    // if both sigmet and polygon files are specified
    // then use sigmet and ignore polygons
    if (sigmetFile == "NONE") {
        rg_use_sigmetfile = false;
    } else {
        if(!exists_test(sigmetFile)) {
            rg_use_sigmetfile = false;
        } else {
            rg_use_sigmetfile = true;
        }
    }
    if (pirepFile == "NONE") {
        rg_use_PIREPfile = false;
    } else {
        if(!exists_test(pirepFile)) {
            rg_use_PIREPfile = false;
        } else {
            rg_use_PIREPfile = true;
        }
    }
    if (polygonFile != "NONE") {
        if(!rg_use_sigmetfile) {
            // ok to use polygon file
            cout << "Loading polygon file: " << polygonFile << endl;
            load_scenarios(rg_polygon_file);
        }
    }

    cout << "rg_origin=" << rg_origin << endl;
    cout << "rg_destination=" << rg_destination << endl;
    cout << "rg_CIFPfile=" << rg_CIFPfile << endl;
    cout << "rg_apconffile=" << rg_apconffile << endl;

    cout << "rg_airwayconffile=" << rg_airwayconffile << endl;
    cout << "rg_fixconffile=" << rg_fixconffile << endl;
    cout << "rg_procconffile=" << rg_procconffile << endl;

	cout << "rg_reroute=" << rg_reroute << endl;
    cout << "rg_wind_optimal=" << rg_wind_optimal << endl;
    cout << "rg_filter_by_input_airports=" << rg_filter_by_input_airports << endl;
    cout << "rg_num_trajs=" << rg_num_trajs << endl;
    cout << "rg_get_TOS=" << rg_get_TOS << endl;
	cout << "rg_tos_cost_index=" << rg_tos_cost_index << endl;

    set<string> airportFilter; 
    airportFilter.clear();
    airportFilter.insert(rg_origin);
    airportFilter.insert(rg_destination);

    set<FixPair> airportPairs;  // shortest-path airport pairs
    vector<FixPair> flights;    // wind-optimal flights
    vector<string> rap_files; 
    rap_files.clear();

    FixPair fp(rg_origin,rg_destination,rg_callsign);
    fp.airspeed = rg_airspeed;
    fp.altitude = rg_altitude;
    airportPairs.insert(fp);
    flights.push_back(fp);

    bool useOriginAndDestination = false;
    bool useAirportsFile = false;

    (void)useOriginAndDestination;
    (void)useAirportsFile;


    //TODO:PARIKSHIT ADDER TO GET WIND FILES FOR ALL TIMES.
    rap_files = get_h5_files(rg_rap_file);
    read_wind_grid_all_hdf5(rap_files, &rg_wind_vec);
    //ADDER ENDS
    cout << endl;
    cout << "Finished reading wind files." <<endl;
    // load rap wind
    if (rg_wind_vec.size() == 0)
        read_wind_grid_hdf5(rg_rap_file, &rg_wind);

    // convert the stl vector of scale factors into a raw array
    // of doubles, as required by the API functions.
    int numScalings = rg_scale_factors.size();
    double scaling[rg_scale_factors.size()];
    for (int i=0; i<numScalings; ++i) {
        scaling[i] = rg_scale_factors.at(i);
    }



    // start timing
    struct timeval tv_start;
    struct timeval tv_init;
    struct timeval tv_compute;
    struct timeval tv_write;
    struct timeval tv_end;

    gettimeofday(&tv_start, NULL);

    // load the NFD airway data file.
    // don't forget to unload the data when we're done!
    // the airway data must be loaded before calling any other API
    // functions from TrxPathUtil.h since many of the functions will
    // make use of the global airway and fix data.
    cout << endl;

    /** TODO: PARIKSHIT ADDER FOR DIRECT CIFP INPUT*/

    cout << "Loading airport data..." << endl;
    load_airports_alt(rg_CIFPfile, &airportFilter, rg_apconffile);

    cout << "Loading airway data..." << endl;
    load_airways_alt(rg_CIFPfile, rg_airwayconffile);

    cout << "Loading sid data..." << endl;

    load_procs_alt(rg_CIFPfile, "SID", rg_fproc);

    cout << "Loading star data..." << endl;

    load_procs_alt(rg_CIFPfile, "STAR", rg_fproc);

    cout << "Loading approach data..." << endl;

    load_procs_alt(rg_CIFPfile, "APPROACH", rg_fproc);





    // find the unique set of names of the fixes used by
    // airways, sids, stars, and approaches. use this as a filter
    // set for loading fixes
    set<string> unames;
    map<ProcKey, Star*>::iterator star_it;
    map<ProcKey, Sid*>::iterator sid_it;
    map<ProcKey, Approach*>::iterator app_it;
    map<string, Airway*>::iterator air_it;
    map<string, Airport*>::iterator ap_it;

    for(ap_it=osi::rg_airports.begin(); ap_it!=osi::rg_airports.end(); ++ap_it) {
        Airport* ap = ap_it->second;
        unames.insert(ap->name);

    }

    for(air_it=osi::rg_airways.begin(); air_it!=osi::rg_airways.end(); ++air_it) {
        Airway* air = air_it->second;
        for(unsigned int i=0; i<air->waypointNames.size(); ++i) {
            unames.insert(air->waypointNames.at(i));
        }
    }

    for(star_it=osi::rg_stars.begin(); star_it!=osi::rg_stars.end(); ++star_it) {
        Star* star = star_it->second;
        map< string, vector<string> >::iterator wp_it;
        for(wp_it=star->waypoints.begin(); wp_it!=star->waypoints.end(); ++wp_it) {
            vector<string>& wps = wp_it->second;
            for(unsigned int i=0; i<wps.size(); ++i) {
                unames.insert(wps.at(i));
            }
        }
    }

    for(sid_it=osi::rg_sids.begin(); sid_it!=osi::rg_sids.end(); ++sid_it) {
        Sid* sid = sid_it->second;
        map< string, vector<string> >::iterator wp_it;
        for(wp_it=sid->waypoints.begin(); wp_it!=sid->waypoints.end(); ++wp_it) {
            vector<string>& wps = wp_it->second;
            for(unsigned int i=0; i<wps.size(); ++i) {
                unames.insert(wps.at(i));
                size_t pos = wps.at(i).find("NO_WP_NAME_");
                if (pos != string::npos){
                    string rwname = get_runway_name(wps.at(i));
                    unames.insert(rwname);
                }

            }
        }
    }

    for(app_it=osi::rg_approach.begin(); app_it!=osi::rg_approach.end(); ++app_it) {
        Approach* app = app_it->second;
        map< string, vector<string> >::iterator wp_it;
        for(wp_it=app->waypoints.begin(); wp_it!=app->waypoints.end(); ++wp_it) {
            vector<string>& wps = wp_it->second;
            for(unsigned int i=0; i<wps.size(); ++i) {
                unames.insert(wps.at(i));
            }
        }
    }

    cout << "Loading Fix data..." << endl;
    load_fixes(rg_CIFPfile, rg_fixconffile, &airportFilter, &unames);

    //	NEEDED FOR TERMINAL AREA PROCESSING
    //    Insert intercept points for VI,CI leg
    //    insert_intercept_points();


    //    NEEDED FOR TERMINAL AREA PROCESSING
    //    cout << "Running star approach map..." << endl;
    //    star_approach_map();



    /*
     * LOAD WEATHER POLYGONS HERE. IF YOU ARE LOADING SIGMETS.
     */
    if (rg_use_sigmetfile)
        processWeatherPolys(rg_CIFPfile, rg_Sigmetfile);
    if (rg_use_PIREPfile){
        processPIREPs(rg_CIFPfile, rg_PIREPfile);
        if (rg_wind_vec.size() )
            processSimStartTime();
    }

    /** TODO: PARIKSHIT ADDER ENDS HERE*/
    // build the network connectivity from airways
    cout << "RouteGenerator: Building network connectivity..." << endl;
    SearchGraph graph;
    get_airway_connectivity(&graph, rap_files.size());


    gettimeofday(&tv_init, NULL);

    // compute the shortest path between the specified airport pair
    // you would call this function repeatedly for each pair of
    // origin/destination airports.  This version of get_paths()
    // takes in the weather scenarios and scale factors.  It will find
    // the shortest path around the polygons in each scenario at each
    // scale factor.  For example, if there are 3 polygons in two
    // scenarios (the original + the perturbed) and 3 scale factors
    // (1x, 2x, 3x).  It will produce 6 paths:
    // scenario 1 around 1x (possibly concave) polygons
    // scenario 1 around 2x convex polygons
    // scenario 1 around 3x convex polygons
    // scenario 2 around 1x (possibly convcave) polygons
    // scenario 2 around 2x convex polygons
    // scenario 2 around 3x convex polygons

    ResultSets results;

    PolygonSets resultPolys;

    tos_gen_method t_method = REM_ENTIRE_PATH;
    //This is the edge number to start when removing edges to create TOS in
    // REM_ENTIRE_PATH method
    int offset_for_removing_edges = 0;

    bool replanFlag = !rg_reroute;

    if (!rg_wind_optimal) {
        cout << "Getting shortest paths..." << endl << endl;
        get_paths(graph, airportPairs, rg_scenarios, scaling, numScalings,
                  &results, &resultPolys, replanFlag);
    } else {
        if (!rg_get_TOS){
            cout << "Getting wind optimal paths..." << endl << endl;
            get_wind_optimal_paths(graph, flights, &rg_wind, &rg_wind_vec, rg_scenarios, scaling,
                                   numScalings, &results, &resultPolys, replanFlag);
        }
        else{
            cout << "Getting wind optimal trajectory option sets..." << endl << endl;
            get_wind_optimal_TOS(graph, flights, &rg_wind, &rg_wind_vec, &rg_pirep, rg_scenarios, scaling,
                                 numScalings, &t_results, &resultPolys, rg_num_trajs, replanFlag,
                                 t_method, rg_fproc,offset_for_removing_edges);
        }
    }

    // ========================================================================

    vector<ResultSetKey> resultKeys;
    get_result_sets_keys<ResultTrajs>(t_results, &resultKeys);
    int pathcounter = 0;

    vector<ResultSetKey>::iterator resultKeyIter;
    for (resultKeyIter=resultKeys.begin(); resultKeyIter!=resultKeys.end(); ++resultKeyIter) {
        ResultSetKey resultSetKey = *resultKeyIter;
        int scenario = resultSetKey.scenario;

        if(scenario < 0 && numScalings) continue;
        double scale = resultSetKey.scale;

        // obtain the ResultSet for the scenario,scale combination
        const ResultTraj* result = get_result_set<ResultTrajs,ResultTraj>(scenario, scale, t_results);
        if(!result) {
            cerr << "ERROR: no results for scenario " << scenario << " at"
                    " scale level " << scale << endl;
            continue;
        }

        // find the result's paths, keyed by FixPairs.
        vector<FixPair> keys;

        int err = get_result_keys<ResultTraj>(result, &keys);
        if(err < 0) {
            cerr << "ERROR: failed to obtain result set for "
                    "scenario " << scenario << " at scale level "
                    << scale << endl;
            continue;
        }

        // iterate over the FixPair keys to obtain the paths
        // and print the paths to the outputs
        for(unsigned int i=0; i<keys.size(); ++i) {
            FixPair key = keys.at(i);
            vector<SearchPath> paths;
            err = get_result_value_TOS(result, key, &paths);
            if(err < 0) {
                cerr << "ERROR: failed to obtain path for scenario "
                        << scenario << " at scale level "
                        << scale << " for "
                        << key.origin << "->" << key.destination << endl;
                continue;
            }

            for(unsigned int pcnt=0; pcnt < paths.size(); ++pcnt){

                SearchPath path = paths.at(pcnt);

                vector<double> lats;
                vector<double> lons;

                stringstream ss;

                for(int j=0; j<path.size(); ++j) {
                    int id = const_cast<SearchPath&>(path)[j];

                    double latitude, longitude;
                    get_fix_location(id, &latitude, &longitude);

                    string fname;
                    get_fix_name(id, &fname);
                    ss << fname;
                    if(j < path.size()-1) ss << "..";

                    lats.push_back(latitude);
                    lons.push_back(longitude);
                }

                string route = ss.str();
                routes.push_back(route);

                double cost = path.getPathCost();
                costs.push_back(cost);

                latitudes.push_back(lats);
                longitudes.push_back(lons);

                pathcounter++;
            }
        }

        // output the polygons
        if (scenario >= 0) {
            PolygonSetKey polyKey(scenario, scale);
            const PolygonSet* polySet = &(resultPolys.at(polyKey));

            for(unsigned int i=0; i<polySet->size(); ++i) {

                const Polygon* poly = &(polySet->at(i));
                int num_points = poly->getNumVertices();
                const double* xdata = poly->getXData();
                const double* ydata = poly->getYData();

                vector<double> polyLats;
                vector<double> polyLons;

                for(int j=0; j<num_points; ++j) {
                    polyLats.push_back(ydata[j]);
                    polyLons.push_back(xdata[j]);
                }

                polygonLatitudes.push_back(polyLats);
                polygonLongitudes.push_back(polyLons);
            }
        }
    }

    // ========================================================================

    gettimeofday(&tv_compute, NULL);

    bool enableFileOut = false;
    if (enableFileOut) {
        // do something useful with the results...
        // here, we will iterate through the result set and print the
        // data to stdout.  we will also print the paths to a delimited
        // text file so that the paths may be plotted in gnuplot, matlab,
        // octave, or some other plotting package.
        // first, iterate over each scenario's result set at each scale
        cout << endl << "Writing Output Files..." << endl;
        stringstream outPath;
        stringstream polysOutPath;
        stringstream kmlOutPath;
        stringstream kmlPolyOutPath;
        stringstream kmlPIREPOutPath;

        outPath << rg_out_dir << "/paths_out.dat";
        polysOutPath << rg_out_dir << "/polys_out.dat";
        kmlOutPath << rg_out_dir << "/paths_out.kml"; //TODO :PARIKSHIT ADDER
        kmlPolyOutPath << rg_out_dir << "/polys_out.kml"; //TODO :PARIKSHIT ADDER
        kmlPIREPOutPath << rg_out_dir << "/pireps_out.kml";

        ofstream out;
        ofstream polysOut;
        ofstream kmlOut;//TODO :PARIKSHIT ADDER
        ofstream kmlPolyOut;//TODO :PARIKSHIT ADDER
        ofstream kmlPIREPOut;

        out.open( outPath.str().c_str() );
        polysOut.open( polysOutPath.str().c_str() );
        kmlOut.open( kmlOutPath.str().c_str() );//TODO :PARIKSHIT ADDER
        kmlPolyOut.open( kmlPolyOutPath.str().c_str() );//TODO :PARIKSHIT ADDER
        kmlPIREPOut.open( kmlPIREPOutPath.str().c_str() );
        kml_splash( kmlOut );
        kml_splash(kmlPolyOut,true);
        kml_write_pireps( kmlPIREPOut );

        vector<ResultSetKey> resultKeys;

        if (!rg_get_TOS) {
            get_result_sets_keys<ResultSets>(results, &resultKeys);

            int pathcounter = 0;
            vector<ResultSetKey>::iterator resultKeyIter;
            for(resultKeyIter=resultKeys.begin(); resultKeyIter!=resultKeys.end();
                    ++resultKeyIter) {
                ResultSetKey resultSetKey = *resultKeyIter;
                int scenario = resultSetKey.scenario;
                double scale = resultSetKey.scale;

                // obtain the ResultSet for the scenario,scale combination
                const ResultSet* result = get_result_set<ResultSets,ResultSet>(scenario, scale, results);
                if(!result) {
                    cerr << "ERROR: no results for scenario " << scenario << " at"
                            " scale level " << scale << endl;
                    continue;
                }

                // find the result's paths, keyed by FixPairs.
                vector<FixPair> keys;

                int err = get_result_keys<ResultSet>(result, &keys);
                if(err < 0) {
                    cerr << "ERROR: failed to obtain result set for "
                            "scenario " << scenario << " at scale level "
                            << scale << endl;
                    continue;
                }

                // iterate over the FixPair keys to obtain the paths
                // and print the paths to the outputs
                for(unsigned int i=0; i<keys.size(); ++i) {
                    FixPair key = keys.at(i);
                    SearchPath path;
                    err = get_result_value(result, key, &path);
                    if(err < 0) {
                        cerr << "ERROR: failed to obtain path for scenario "
                                << scenario << " at scale level "
                                << scale << " for "
                                << key.origin << "->" << key.destination << endl;
                        continue;
                    }

                    print_path(scenario, scale, key, path, out);
                    int pathnum = (rg_num_trajs == 0 )?(pathcounter%5):(pathcounter%rg_num_trajs);

                    print_kml(path, kmlOut,  key.origin, key.destination, pathnum );
                    pathcounter++;
                }


                // print polygons to out file
                // watch out, the nominal scenario has id of -10
                if(scenario >= 0) {
                    PolygonSetKey polyKey(scenario, scale);
                    const PolygonSet* polySet = &(resultPolys.at(polyKey));
                    for(unsigned int i=0; i<polySet->size(); ++i) {
                        const Polygon* poly = &(polySet->at(i));
                        print_polygon(scenario, scale, *poly, polysOut);
                        print_poly_kml(*poly,kmlPolyOut,i);
                    }
                }
            }
        }//if (!rg_get_TOS) ends
        else {

            get_result_sets_keys<ResultTrajs>(t_results, &resultKeys);

            int pathcounter = 0;
            vector<ResultSetKey>::iterator resultKeyIter;
            for(resultKeyIter=resultKeys.begin(); resultKeyIter!=resultKeys.end();
                    ++resultKeyIter) {
                ResultSetKey resultSetKey = *resultKeyIter;
                int scenario = resultSetKey.scenario;

                if (scenario <0 && numScalings) continue;
                double scale = resultSetKey.scale;

                // obtain the ResultSet for the scenario,scale combination
                const ResultTraj* result = get_result_set<ResultTrajs,ResultTraj>(scenario, scale, t_results);
                if(!result) {
                    cerr << "ERROR: no results for scenario " << scenario << " at"
                            " scale level " << scale << endl;
                    continue;
                }

                // find the result's paths, keyed by FixPairs.
                vector<FixPair> keys;

                int err = get_result_keys<ResultTraj>(result, &keys);
                if(err < 0) {
                    cerr << "ERROR: failed to obtain result set for "
                            "scenario " << scenario << " at scale level "
                            << scale << endl;
                    continue;
                }

                // iterate over the FixPair keys to obtain the paths
                // and print the paths to the outputs
                for(unsigned int i=0; i<keys.size(); ++i) {
                    FixPair key = keys.at(i);
                    vector<SearchPath> paths;
                    err = get_result_value_TOS(result, key, &paths);
                    if(err < 0) {
                        cerr << "ERROR: failed to obtain path for scenario "
                                << scenario << " at scale level "
                                << scale << " for "
                                << key.origin << "->" << key.destination << endl;
                        continue;
                    }

                    for(unsigned int pcnt =0;pcnt < paths.size(); ++pcnt){
                        SearchPath path = paths.at(pcnt);
                        print_path(scenario, scale, key, path, out);
                        print_kml(path, kmlOut,key.origin, key.destination, (pathcounter%rg_num_trajs));
                        pathcounter++;
                    }

                }

                // print polygons to out file
                // watch out, the nominal scenario has id of -10
                if(scenario >= 0) {
                    PolygonSetKey polyKey(scenario, scale);
                    const PolygonSet* polySet = &(resultPolys.at(polyKey));
                    for(unsigned int i=0; i<polySet->size(); ++i) {
                        const Polygon* poly = &(polySet->at(i));
                        print_polygon(scenario, scale, *poly, polysOut);
                        print_poly_kml(*poly,kmlPolyOut,i);
                    }
                }
            }
        }

        kmlOut << "</Document>\n	</kml>"<<endl;
        kmlOut.close();
        kmlPolyOut<<"</Folder>\n</Document>\n</kml>" << endl;
        kmlPIREPOut.close();
        kmlPolyOut.close();
        polysOut.close();
        out.close();

        gettimeofday(&tv_write, NULL);
    }

    // ========================================================================

    cout << endl;
    cout << "Cleaning Up..." << endl;

    // free the NFD airway data.
    // you can no longer call any of the API functions, including
    // get_fix_*() functions and get_airway_*() functions after
    // calling unload_airways().
    unload_rg_airways();
    unload_rg_airports();

    unload_rg_sids();
    unload_rg_stars();

    gettimeofday(&tv_end, NULL);

    cout << "Done." << endl;

    // compute run times
    double dt_init = compute_dt(tv_start, tv_init);
    double dt_compute = compute_dt(tv_init, tv_compute);

    double dt_cleanup = compute_dt(tv_compute, tv_end);
    double dt = compute_dt(tv_start, tv_end);

    cout << endl;
    cout << "Program execution time: " << dt << " seconds" << endl;
    cout << "        Initialization: " << dt_init << " seconds" << endl;
    cout << "            Processing: " << dt_compute << " seconds" << endl;

    cout << "               Cleanup: " << dt_cleanup << " seconds" << endl;
    cout << endl;

    cout << "Good bye." << endl;

    return 0;
}

int runRgOptimized_NATS(const string& callsign,
		const string& origin,
		const string& destination,
		const string& procName,
		const double& cruiseAlt,
		const double& cruiseSpd,
		const string& polygonFile,
		const string& sigmetFile,
		const string& pirepFile,
		const string& fixConfigFile,
		const string& procConfigFile,
		const bool& rerouteFlag,
		const bool& windOptimalFlag,
		const bool& filterAirports,
		const int& numOpts,
		const double& costIndex,
		ResultTrajs& t_results,
		vector<string>& routes,
		vector<double>& costs,
		vector< vector<double> >& latitudes,
		vector< vector<double> >& longitudes,
		vector< vector<double> >& polygonLatitudes,
		vector< vector<double> >& polygonLongitudes) {
    rg_origin = origin;
    rg_destination = destination;

    rg_altitude = cruiseAlt;
    rg_airspeed = cruiseSpd;

    rg_polygon_file = polygonFile;
    rg_Sigmetfile = sigmetFile;
    rg_PIREPfile = pirepFile;

    rg_fixconffile = fixConfigFile;
    rg_procconffile = procConfigFile;

    rg_reroute = rerouteFlag;
    rg_wind_optimal = windOptimalFlag;
    rg_filter_by_input_airports = filterAirports;
    rg_num_trajs = numOpts;
    rg_get_TOS = true;
    rg_tos_cost_index = costIndex;
    rg_scale_factors.push_back(1);

    // if both sigmet and polygon files are specified
    // then use sigmet and ignore polygons
    if (sigmetFile == "NONE") {
        rg_use_sigmetfile = false;
    } else {
        if (!exists_test(sigmetFile)) {
            rg_use_sigmetfile = false;
        } else {
            rg_use_sigmetfile = true;
        }
    }
    if (pirepFile == "NONE") {
        rg_use_PIREPfile = false;
    } else {
        if (!exists_test(pirepFile)) {
            rg_use_PIREPfile = false;
        } else {
            rg_use_PIREPfile = true;
        }
    }
    if (polygonFile != "NONE") {
        if (!rg_use_sigmetfile) {
            // ok to use polygon file
            load_scenarios(rg_polygon_file);
        }
    }

	rg_airportFilter.clear();
	rg_airportFilter.insert(rg_origin);
	rg_airportFilter.insert(rg_destination);

	// ========================================================================

	set<FixPair> airportPairs;  // shortest-path airport pairs
    vector<FixPair> flights;    // wind-optimal flights

    FixPair fp(rg_origin, rg_destination, callsign);
    fp.airspeed = rg_airspeed;
    fp.altitude = rg_altitude;
    airportPairs.insert(fp);
    flights.push_back(fp);

    // ========================================================================

    // convert the stl vector of scale factors into a raw array
    // of doubles, as required by the API functions.
    int numScalings = rg_scale_factors.size();
    double scaling[rg_scale_factors.size()];
    for (int i = 0; i < numScalings; ++i) {
        scaling[i] = rg_scale_factors.at(i);
    }

    // ========================================================================

    load_fixes(rg_CIFPfile, rg_fixconffile, &rg_airportFilter, &unames);

    // ========================================================================

    SearchGraph graph;
    get_airway_connectivity(&graph, rg_vector_rap_files.size());

    // ========================================================================

    ResultSets results;

    PolygonSets resultPolys;

    tos_gen_method t_method = REM_ENTIRE_PATH;
    //This is the edge number to start when removing edges to create TOS in
    // REM_ENTIRE_PATH method
    int offset_for_removing_edges = 0;

    bool replanFlag = !rg_reroute;

    if (!rg_wind_optimal) {
        cout << "RouteGenerator: Aircraft " << callsign << ": Getting shortest paths..." << endl << endl;
        get_paths(graph, airportPairs, rg_scenarios, scaling, numScalings,
                  &results, &resultPolys, replanFlag);
    } else {
        if (!rg_get_TOS) {
            cout << "RouteGenerator: Aircraft " << callsign << ": Getting wind optimal paths..." << endl << endl;
            get_wind_optimal_paths(graph, flights, &rg_wind, &rg_wind_vec, rg_scenarios, scaling,
                                   numScalings, &results, &resultPolys, replanFlag);
        }
        else {
            cout << "RouteGenerator: Aircraft " << callsign << ": Getting wind optimal trajectory option sets..." << endl << endl;
            get_wind_optimal_TOS(graph,
            		flights,
					&rg_wind,
					&rg_wind_vec,
					&rg_pirep,
					rg_scenarios,
					scaling,
					numScalings,
					&t_results,
					&resultPolys,
					rg_num_trajs,
					replanFlag,
					t_method,
					rg_fproc,
					offset_for_removing_edges);
        }
    }

    // ========================================================================

    vector<ResultSetKey> resultKeys;
    get_result_sets_keys<ResultTrajs>(t_results, &resultKeys);
    int pathcounter = 0;

    vector<ResultSetKey>::iterator resultKeyIter;
    for (resultKeyIter = resultKeys.begin(); resultKeyIter != resultKeys.end(); ++resultKeyIter) {
        ResultSetKey resultSetKey = *resultKeyIter;
        int scenario = resultSetKey.scenario;

        if(scenario < 0 && numScalings) continue;
        double scale = resultSetKey.scale;

        // obtain the ResultSet for the scenario,scale combination
        const ResultTraj* result = get_result_set<ResultTrajs,ResultTraj>(scenario, scale, t_results);
        if (!result) {
            cerr << "RouteGenerator: ERROR: no results for scenario " << scenario << " at"
                    " scale level " << scale << endl;
            continue;
        }

        // find the result's paths, keyed by FixPairs.
        vector<FixPair> keys;

        int err = get_result_keys<ResultTraj>(result, &keys);
        if (err < 0) {
            cerr << "RouteGenerator: ERROR: failed to obtain result set for "
                    "scenario " << scenario << " at scale level "
                    << scale << endl;
            continue;
        }

        // iterate over the FixPair keys to obtain the paths
        // and print the paths to the outputs
        for (unsigned int i = 0; i < keys.size(); ++i) {
            FixPair key = keys.at(i);
            vector<SearchPath> paths;
            err = get_result_value_TOS(result, key, &paths);
            if (err < 0) {
                cerr << "RouteGenerator: ERROR: failed to obtain path for scenario "
                        << scenario << " at scale level "
                        << scale << " for "
                        << key.origin << "->" << key.destination << endl;
                continue;
            }

            for (unsigned int pcnt = 0; pcnt < paths.size(); ++pcnt) {

                SearchPath path = paths.at(pcnt);

                vector<double> lats;
                vector<double> lons;

                stringstream ss;

                for (int j = 0; j < path.size(); ++j) {
                    int id = const_cast<SearchPath&>(path)[j];//path.at(i)->id;

                    double latitude, longitude;
                    get_fix_location(id, &latitude, &longitude);

                    string fname;
                    get_fix_name(id, &fname);
                    ss << fname;
                    if (j < path.size()-1) ss << "..";

                    lats.push_back(latitude);
                    lons.push_back(longitude);
                }

                string route = ss.str();
                routes.push_back(route);

                double cost = path.getPathCost();
                costs.push_back(cost);

                latitudes.push_back(lats);
                longitudes.push_back(lons);

                pathcounter++;
            }
        }

        // output the polygons
        if (scenario >= 0) {
            PolygonSetKey polyKey(scenario, scale);
            const PolygonSet* polySet = &(resultPolys.at(polyKey));

            for (unsigned int i = 0; i < polySet->size(); ++i) {
                const Polygon* poly = &(polySet->at(i));
                int num_points = poly->getNumVertices();
                const double* xdata = poly->getXData();
                const double* ydata = poly->getYData();

                vector<double> polyLats;
                vector<double> polyLons;

                for (int j = 0; j < num_points; ++j) {
                    polyLats.push_back(ydata[j]);
                    polyLons.push_back(xdata[j]);
                }

                polygonLatitudes.push_back(polyLats);
                polygonLongitudes.push_back(polyLons);
            }
        }
    }

    // ========================================================================

    bool enableFileOut = false;
    if (enableFileOut) {
        // do something useful with the results...
        // here, we will iterate through the result set and print the
        // data to stdout.  we will also print the paths to a delimited
        // text file so that the paths may be plotted in gnuplot, matlab,
        // octave, or some other plotting package.
        // first, iterate over each scenario's result set at each scale
        cout << endl << "RouteGenerator: Writing Output Files..." << endl;
        stringstream outPath;
        stringstream polysOutPath;
        stringstream kmlOutPath;
        stringstream kmlPolyOutPath;
        stringstream kmlPIREPOutPath;

        outPath << rg_out_dir << "/paths_out.dat";
        polysOutPath << rg_out_dir << "/polys_out.dat";
        kmlOutPath << rg_out_dir << "/paths_out.kml"; //TODO :PARIKSHIT ADDER
        kmlPolyOutPath << rg_out_dir << "/polys_out.kml"; //TODO :PARIKSHIT ADDER
        kmlPIREPOutPath << rg_out_dir << "/pireps_out.kml";

        ofstream out;
        ofstream polysOut;
        ofstream kmlOut;//TODO :PARIKSHIT ADDER
        ofstream kmlPolyOut;//TODO :PARIKSHIT ADDER
        ofstream kmlPIREPOut;

        out.open( outPath.str().c_str() );
        polysOut.open( polysOutPath.str().c_str() );
        kmlOut.open( kmlOutPath.str().c_str() );//TODO :PARIKSHIT ADDER
        kmlPolyOut.open( kmlPolyOutPath.str().c_str() );//TODO :PARIKSHIT ADDER
        kmlPIREPOut.open( kmlPIREPOutPath.str().c_str() );
        kml_splash( kmlOut );
        kml_splash(kmlPolyOut,true);
        kml_write_pireps( kmlPIREPOut );

        vector<ResultSetKey> resultKeys;

        if (!rg_get_TOS) {
            get_result_sets_keys<ResultSets>(results, &resultKeys);

            int pathcounter = 0;
            vector<ResultSetKey>::iterator resultKeyIter;
            for (resultKeyIter=resultKeys.begin(); resultKeyIter!=resultKeys.end();
                    ++resultKeyIter) {
                ResultSetKey resultSetKey = *resultKeyIter;
                int scenario = resultSetKey.scenario;
                double scale = resultSetKey.scale;

                // obtain the ResultSet for the scenario,scale combination
                const ResultSet* result = get_result_set<ResultSets,ResultSet>(scenario, scale, results);
                if (!result) {
                    cerr << "RouteGenerator: ERROR: no results for scenario " << scenario << " at"
                            " scale level " << scale << endl;
                    continue;
                }

                // find the result's paths, keyed by FixPairs.
                vector<FixPair> keys;

                int err = get_result_keys<ResultSet>(result, &keys);
                if (err < 0) {
                    cerr << "RouteGenerator: ERROR: failed to obtain result set for "
                            "scenario " << scenario << " at scale level "
                            << scale << endl;
                    continue;
                }

                // iterate over the FixPair keys to obtain the paths
                // and print the paths to the outputs
                for (unsigned int i=0; i<keys.size(); ++i) {
                    FixPair key = keys.at(i);
                    SearchPath path;
                    err = get_result_value(result, key, &path);
                    if (err < 0) {
                        cerr << "RouteGenerator: ERROR: failed to obtain path for scenario "
                                << scenario << " at scale level "
                                << scale << " for "
                                << key.origin << "->" << key.destination << endl;
                        continue;
                    }

                    print_path(scenario, scale, key, path, out);
                    int pathnum = (rg_num_trajs == 0 )?(pathcounter%5):(pathcounter%rg_num_trajs);

                    print_kml(path, kmlOut,  key.origin, key.destination, pathnum );
                    pathcounter++;
                }

                // print polygons to out file
                // watch out, the nominal scenario has id of -10
                if (scenario >= 0) {
                    PolygonSetKey polyKey(scenario, scale);
                    const PolygonSet* polySet = &(resultPolys.at(polyKey));
                    for (unsigned int i=0; i<polySet->size(); ++i) {
                        const Polygon* poly = &(polySet->at(i));
                        print_polygon(scenario, scale, *poly, polysOut);
                        print_poly_kml(*poly,kmlPolyOut,i);
                    }
                }
            }
        }//if (!rg_get_TOS) ends
        else {
            get_result_sets_keys<ResultTrajs>(t_results, &resultKeys);

            int pathcounter = 0;
            vector<ResultSetKey>::iterator resultKeyIter;
            for (resultKeyIter=resultKeys.begin(); resultKeyIter!=resultKeys.end();
                    ++resultKeyIter) {
                ResultSetKey resultSetKey = *resultKeyIter;
                int scenario = resultSetKey.scenario;

                if (scenario <0 && numScalings) continue;
                double scale = resultSetKey.scale;

                // obtain the ResultSet for the scenario,scale combination
                const ResultTraj* result = get_result_set<ResultTrajs,ResultTraj>(scenario, scale, t_results);
                if (!result) {
                    cerr << "RouteGenerator: ERROR: no results for scenario " << scenario << " at"
                            " scale level " << scale << endl;
                    continue;
                }

                // find the result's paths, keyed by FixPairs.
                vector<FixPair> keys;

                int err = get_result_keys<ResultTraj>(result, &keys);
                if (err < 0) {
                    cerr << "RouteGenerator: ERROR: failed to obtain result set for "
                            "scenario " << scenario << " at scale level "
                            << scale << endl;
                    continue;
                }

                // iterate over the FixPair keys to obtain the paths
                // and print the paths to the outputs
                for (unsigned int i=0; i<keys.size(); ++i) {
                    FixPair key = keys.at(i);
                    vector<SearchPath> paths;
                    err = get_result_value_TOS(result, key, &paths);
                    if (err < 0) {
                        cerr << "RouteGenerator: ERROR: failed to obtain path for scenario "
                                << scenario << " at scale level "
                                << scale << " for "
                                << key.origin << "->" << key.destination << endl;
                        continue;
                    }

                    for (unsigned int pcnt =0;pcnt < paths.size(); ++pcnt){
                        SearchPath path = paths.at(pcnt);
                        print_path(scenario, scale, key, path, out);
                        print_kml(path, kmlOut,key.origin, key.destination, (pathcounter%rg_num_trajs));
                        pathcounter++;
                    }

                }

                // print polygons to out file
                // watch out, the nominal scenario has id of -10
                if (scenario >= 0) {
                    PolygonSetKey polyKey(scenario, scale);
                    const PolygonSet* polySet = &(resultPolys.at(polyKey));
                    for (unsigned int i=0; i<polySet->size(); ++i) {
                        const Polygon* poly = &(polySet->at(i));
                        print_polygon(scenario, scale, *poly, polysOut);
                        print_poly_kml(*poly,kmlPolyOut,i);
                    }
                }
            }
        }

        kmlOut << "</Document>\n	</kml>"<<endl;
        kmlOut.close();
        kmlPolyOut<<"</Folder>\n</Document>\n</kml>" << endl;
        kmlPIREPOut.close();
        kmlPolyOut.close();
        polysOut.close();
        out.close();
    }

    // ========================================================================

    map<string, Fix*>::iterator fiter;

	for (fiter = rg_fixes.begin(); fiter != rg_fixes.end(); ++fiter) {
		Fix* f = fiter->second;
		if (f) {
			delete f;
		}
	}

    rg_fixes.clear();
    rg_fixes_by_id.clear();
    rg_fix_ids.clear();
    rg_fix_names.clear();
}


int runRgForNATS(const string& polygonFile,
		const string& sigmetFile,
		const string& pirepFile,
		const vector<double>& lats_deg,
		const vector<double>& lons_deg,
		vector<double>& lat_reroute_deg,
		vector<double>& lon_reroute_deg,
		vector<pair<int,int> >& similar_idx_wpts,
		const string &CIFPfilepath) {
    rg_polygon_file = polygonFile;
    rg_Sigmetfile = sigmetFile;
    rg_PIREPfile = pirepFile;

    // if both sigmet and polygon files are specified
    // then use sigmet and ignore polygons
    if (rg_Sigmetfile == "NONE") {
        rg_use_sigmetfile = false;
    } else {
    	if (!exists_test(rg_Sigmetfile)) {
            rg_use_sigmetfile = false;
        } else {
            rg_use_sigmetfile = true;
        }
    }

    PolygonSet weatherpolygons;
    if (polygonFile != "NONE") {
        if (!rg_use_sigmetfile) {
        	load_scenarios_for_NATS(rg_polygon_file,
        			weatherpolygons);
        }
        else{
        	if (exists_test(CIFPfilepath)){
        		processWeatherPolysforNATS(CIFPfilepath,rg_Sigmetfile,weatherpolygons);
        	}
        	else{
        		printf("Please specify the path of CIFP file if using NOAA SIGMETs.\n");
        	}
        }
    }

    weatherAvoidanceRoutesForNATS(weatherpolygons, lats_deg, lons_deg, lat_reroute_deg, lon_reroute_deg,
    		similar_idx_wpts);

    return 1;

}

int getWeatherPolygons(const double& lat_deg, const double& lon_deg, const double& alt_ft, const double& nmi_rad,
					const vector<double>& fp_lat_deg,
					const vector<double>& fp_lon_deg,
					const string& polygonFile,
					const string& sigmetFile,
					const string& pirepFile,
					const string& CIFPfilepath,
					vector<Polygon>& polygons_ahead) {

    rg_polygon_file = polygonFile;
    rg_Sigmetfile = sigmetFile;

    rg_PIREPfile = pirepFile;

    // if both sigmet and polygon files are specified
    // then use sigmet and ignore polygons
    if (rg_Sigmetfile == "NONE") {
        rg_use_sigmetfile = false;
    } else {
    	if (!exists_test(rg_Sigmetfile)) {
            rg_use_sigmetfile = false;
        } else {
            rg_use_sigmetfile = true;
        }
    }



    PolygonSet weatherpolygons;
    if (polygonFile != "NONE" || !rg_use_sigmetfile) {
    	if (exists_test(rg_polygon_file))  {
        	load_scenarios_for_NATS(rg_polygon_file,
        			weatherpolygons);
    	}
    	else{
    		printf("Please specify either polygon file or sigmet file.\n");
    	}
    }
    else{

    	if (exists_test(CIFPfilepath)){

    		processWeatherPolysforNATS(CIFPfilepath,rg_Sigmetfile,weatherpolygons);
    	}
    	else{
    		printf("Please specify the path of CIFP file if using NOAA SIGMETs.\n");
    	}
    }

    find_intersecting_polygons(lat_deg, lon_deg, alt_ft, nmi_rad, fp_lat_deg, fp_lon_deg, weatherpolygons, polygons_ahead);

	return 1;
}

} /* namespace osi */
