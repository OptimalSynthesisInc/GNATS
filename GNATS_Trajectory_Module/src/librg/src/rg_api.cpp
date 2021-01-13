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

#include "AirportLayoutDataLoader.h"
#include "rg_api.h"
#include "TrxInputStream.h"
#include "TrxInputStreamListener.h"
#include "TrxRecord.h"
#include "Fix.h"
#include "Airway.h"
#include "Sid.h"
#include "Star.h"
#include "Approach.h"
#include "Airport.h"
#include "ProcKey.h"
#include "FixPair.h"
#include "LookupGrid.h"
#include "AstarSearch.h"
#include "DepthFirstSearch.h"
#include "Polygon.h"
#include "osi_global_constants.h"
#include "geometry_utils.h"

#include "rg_exec.h"
#include "util_string.h"

#include <string>
using std::string;

#include "wind_api.h"

#include <string>
#include <set>
#include <deque>
#include <vector>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <cmath>
#include <sstream>
#include <map>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <typeinfo>

#include <time.h>

#include <omp.h>

#include "rapidxml/rapidxml.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::set;
using std::deque;
using std::vector;
using std::map;
using std::string;
using std::stringstream;
using std::setw;
using std::setprecision;

using namespace rapidxml;

#ifndef NDEBUG
#define LOG_GET_SHORTEST_PATHS_POLYGON 0
#endif

#define LOG_SCENARIO_PATHS  0
#define USE_LIBSEARCH       0
#define ENABLE_SID_STAR     1


#ifndef WIND_SCALE_FACTOR
#define WIND_SCALE_FACTOR 1
#endif

#ifndef MIN_CRUISE_AIRSPEED_KNOTS
#define MIN_CRUISE_AIRSPEED_KNOTS 400
#endif

#define ENABLE_COMPARE_SP_TO_WO 0

#define USE_MULTIPLIER 1



namespace osi {

//First pair: Airport Star pair, end point of star mapped
//Second pair: Airport Approach pair, linking point of approach
typedef map< pair<ProcKey,string>, pair<ProcKey,string> > STARAPPmap;

STARAPPmap g_star_app_map;

static const double DEFAULT_ALTITUDE_FT = 35000;

static ofstream error_log;
static const string error_log_name = "rg_errors.log";
static bool cost_compute_flag = true;


static double g_max_wind_magnitude = 0;
static double g_max_cost = 0;

#if USE_MULTIPLIER
static double g_multiplier = 2;
#endif

static omp_lock_t paths_lock;

string rg_output_directory = ".";
WindGrid rg_wind;
vector<WindGrid> rg_wind_vec;

int rg_supplemental_airway_seq = 0;

/*
 * Convert lat lon in CIFP file.
 */
void convertLatLon(const string& inpstr, double& val,bool latFlag){

	int multval = 1;
	if (latFlag){
		if ( inpstr.compare(0,1,"S") == 0)
			multval = -1;
		int deg = atoi( inpstr.substr(1,2).c_str());
		int min = atoi( inpstr.substr(3,2).c_str());
		int sec = atoi( inpstr.substr(5,4).c_str());
		val = (double)deg + (double)min/60.0 + (double)sec*0.01/3600.0;
		val = multval* val;
	}
	else{
		if ( inpstr.compare(0,1,"W") == 0)
			multval = -1;
		int deg = atoi( inpstr.substr(1,3).c_str());
		int min = atoi( inpstr.substr(4,2).c_str());
		int sec = atoi( inpstr.substr(6,4).c_str());
		val = (double)deg + (double)min/60.0 + (double)sec*0.01/3600.0;
		val = multval* val;

	}

}

/*
 * Convert Elev in CIFP file.
 */
void convertElev(const string& inpstr, double& val){

	size_t len = inpstr.length();
	int multival = 1;
	if (inpstr.compare(0,1,"-") == 0)
		multival = -1;
	val = 1.00*atoi(inpstr.substr(1,len-1).c_str());
	val = multival*val;
}

/*
 * Convert Mag Var in CIFP file.
 */
void convertMagVarDec(const string &inp_str,double &val){
	size_t len = inp_str.length();
	double mult = 1.0;
	if ( inp_str.compare(0,1,"E") == 0)
		mult = -1.0;
	string angstr = inp_str.substr(1,len-1);
	val = mult * atof(angstr.c_str())/10.0;
}


/*
 * Remove spaces in the string
 */
void removeSpaces(string& input){
  input.erase(std::remove(input.begin(),input.end(),' '),input.end());
}

/**
 * gc_distance cost function
 */
double gc_distance_cost_function(const double& lat1, const double& lon1, const double& alt1,
			                     const double& lat2, const double& lon2, const double& alt2,
			                     const double& spd) {
	(void)spd;
	double alt = (alt1+alt2)/2.;
	return compute_distance_gc(lat1, lon1, lat2, lon2, alt);
}

/**
 * wind optimal cost function, spd in ft/sec
 */
double wind_optimal_cost_function(const double& lat1, const double& lon1, const double& alt1,
                                  const double& lat2, const double& lon2, const double& alt2,
                                  const double& spd,
                                  //TODO:PARIKSHIT ADDER
                                  WindGrid* const g_wind,
                                  vector<WindGrid>* const g_wind_vec
                                  //TODO:PARIKSHIT ADDER ENDS
                                  ) {

	// compute the link vector, D, from p1 to p2 using gc distance
	// divide link into n segments. and compute the segment length, di.
	// for each segment di,
	//   compute the midpoint and obtain the ruc/rap wind at the midpoint
	//   resolve the wind vector along D
	//   compute track speed at midpoint as vi=vcruise+vwind
	// compute the link transit time as T=sum(ti)

	double alt = ( (alt1+alt2)/2.) ; // ft
	double D = compute_distance_gc(lat1, lon1, lat2, lon2, alt); // ft
	double dnom = 3.0 * nauticalMilesToFeet;
	double n = ceil(D/dnom);
	double d = D/n;

	double T = 0;

	double heading = compute_heading_gc(lat1, lon1, lat2, lon2);

	while(heading < 0) heading += 360.;
	while(heading > 360) heading -= 360.;

	double headingRad = heading*M_PI/180.;

	double latmid;
	double lonmid;
	double s = d/2;

	for(int i=0; i<(int)n; ++i) {

		compute_location_gc(lat1, lon1, s, heading, &latmid, &lonmid, alt);

		// get windFieldComponents true for ft/sec, false for m/sec
		double windNorth=0, windEast=0;

		if (g_wind_vec->size())
			g_wind_vec->at(0).getWind(latmid, lonmid, alt, &windNorth, &windEast);
		else
			g_wind->getWind(latmid, lonmid, alt, &windNorth, &windEast);

		// compute wind vectors along-track and cross-track
		//TODO: WIND SPEED IS IN KNOTS. SEE THE WIND_TOOL MAIN FILE FOR REFERENCE.
		double w_at = (windNorth*cos(headingRad) + windEast*sin(headingRad)) * KnotsToFeetPerSec;
		double w_ct = (windNorth*sin(headingRad) - windEast*cos(headingRad)) * KnotsToFeetPerSec;

		// compute angle between ground track and actual track, beta
		double beta = asin(-w_ct / spd);

		// compute ground speed ft/sec
		double vg = spd*cos(beta) + w_at;

		// compute time and accumulate
		double t = d / vg; // sec

		T += t;

		s += d;

	}

	return T;
}

vector<double> wind_optimal_cost_function(const double& lat1, const double& lon1, const double& alt1,
                                  const double& lat2, const double& lon2, const double& alt2,
                                  const double& spd,
                                  vector<WindGrid>* const g_wind_vec) {

	// compute the link vector, D, from p1 to p2 using gc distance
	// divide link into n segments. and compute the segment length, di.
	// for each segment di,
	//   compute the midpoint and obtain the ruc/rap wind at the midpoint
	//   resolve the wind vector along D
	//   compute track speed at midpoint as vi=vcruise+vwind
	// compute the link transit time as T=sum(ti)




	double alt = ( (alt1+alt2)/2.) ; // ft
	double D = compute_distance_gc(lat1, lon1, lat2, lon2, alt); // ft
	double dnom = 3.0 * nauticalMilesToFeet;
	double n = ceil(D/dnom);
	double d = D/n;

	double heading = compute_heading_gc(lat1, lon1, lat2, lon2);

	while(heading < 0) heading += 360.;
	while(heading > 360) heading -= 360.;

	double headingRad = heading*M_PI/180.;

	double latmid;
	double lonmid;

	vector<double> Tf;Tf.clear();
	for (unsigned int k = 0;k<g_wind_vec->size();++k){
		double Tval = 0.0; double sval = d/2;
		for(int i=0; i<(int)n; ++i) {

			compute_location_gc(lat1, lon1, sval, heading, &latmid, &lonmid, alt);

			// get windFieldComponents true for ft/sec, false for m/sec
			double windNorth=0, windEast=0;

			unsigned int inc = (unsigned int)(floor(Tval/3600.0));
			unsigned int idx = (unsigned int)(k+inc)%(g_wind_vec->size());


			g_wind_vec->at(idx).getWind(latmid, lonmid, alt, &windNorth, &windEast);



			// compute wind vectors along-track and cross-track
			//TODO: WIND SPEED IS IN KNOTS. SEE THE WIND_TOOL MAIN FILE FOR REFERENCE.
			double w_at = (windNorth*cos(headingRad) + windEast*sin(headingRad)) * KnotsToFeetPerSec;
			double w_ct = (windNorth*sin(headingRad) - windEast*cos(headingRad)) * KnotsToFeetPerSec;

			// compute angle between ground track and actual track, beta
			double beta = asin(-w_ct / spd);

			// compute ground speed ft/sec
			double vg = spd*cos(beta) + w_at;

			// compute time and accumulate
			double t = d / vg; // sec

			Tval += t;

			sval += d;

		}
		Tf.push_back(Tval);
	}

	return Tf;
}


/*
 * Wind optimal heuristic function
 */
static double wind_optimal_heuristic_function(const double& lat1, const double& lon1, const double& alt1,
        const double& lat2, const double& lon2, const double& alt2,
        const double& spd) {

	double alt = (alt1+alt2)/2.; // ft
	double D = compute_distance_gc(lat1, lon1, lat2, lon2, alt); // ft
	double v = spd + WIND_SCALE_FACTOR*g_max_wind_magnitude;
	double T = D/v;
	return T;
}

/**
 * Log a message to the error_log.  if stderrFlag == true then also
 * print the message to cerr.
 */
static void log_error(const string& msg, const bool& stderrFlag=false) {
	stringstream ss;
	time_t rawtime;
	time(&rawtime);
	char* tstr = ctime(&rawtime);

	// ctime ends with the \n character (followed by null terminator)
	// remove the newline char.
	int strlength = strlen(tstr);
	tstr[strlength-1] = '\0';

	ss << "[" << tstr << "] " << msg << endl;

	error_log.open(error_log_name.c_str(), ios::app);
	if (error_log.good()) {
		error_log << ss.str();
		error_log.close();
	}

	if (stderrFlag) {
		cerr << ss.str();
	}
}

int get_fix(const string& _name, Fix** const fix, const bool& forceNames, const bool &testPhase) {
	// if the forceNames flag is set, use the input fix pair names
	// as supplied.  otherwise, assume that the names are airport
	// names and check to see if they need to be prefixed with "K".
	string name = _name;

	int id = -1;
	map<string, int>::iterator oiter = rg_fix_ids.find(name);
	if (oiter != rg_fix_ids.end()) {
		id = rg_fix_ids.at(name);
		*fix = rg_fixes.at(name);
	} else {
		// not found in fixes, try airports...
		oiter = rg_airport_ids.find(name);
		if (oiter != rg_airport_ids.end()) {
			id = rg_fixes.size()+rg_airport_ids.at(name);
			*fix = rg_airports.at(name);
		} else {
			// if !forceNames, then try stripping the leading 'K'
			// and check again in g_fix_ids and g_airport_ids.
			// only do this for 4-letter names starting with 'K'
			// if the name is 3-letters, try prefixing with 'K'.
			if (!forceNames) {
				if (name.length() == 4 && name.at(0) == 'K') {
					name = name.substr(1);
					oiter = rg_fix_ids.find(name);
					if (oiter != rg_fix_ids.end()) {
						id = rg_fix_ids.at(name);
						*fix = rg_fixes.at(name);
					} else {
						oiter = rg_airport_ids.find(name);
						if (oiter != rg_airport_ids.end()) {
							id = rg_fixes.size()+rg_airport_ids.at(name);
							*fix = rg_airports.at(name);
						}
					}
				} else if (name.length() == 3) {
					name = "K" + name;

					oiter = rg_airport_ids.find(name);
					if(oiter != rg_airport_ids.end()) {
						id = rg_fixes.size()+rg_airport_ids.at(name);
						*fix = rg_airports.at(name);
					}
				}
			}
		}
	}

	if (id < 0 && !testPhase) {
		stringstream ss;
		ss << "ERROR: get_fix() - "
		      "could not find fix for name " << _name;
		log_error(ss.str());
	}

	return id;
}

static string get_fix(int& id, Fix** fix) {
	string retString("");

	if (id >= (int)rg_fixes.size()) {
		int airportId = id - rg_fixes.size();
		*fix = rg_airports_by_id.at(airportId);
	} else {
		*fix = rg_fixes_by_id.at(id);
	}

	if ((fix != NULL) && ((*fix) != NULL)) {
		(*fix)->getName();

		if ((*fix)->getName().length() > 0)
			retString.assign((*fix)->getName());
	}

	return retString;
}

#if LOG_GET_SHORTEST_PATHS_POLYGON
static void write_paths_gnuplot(const vector< map<FixPair, SearchPath> >& paths,
		                        const string& outfile) {
	stringstream ss;

	for(unsigned int i=0; i<paths.size(); ++i) {
		const map<FixPair, SearchPath>* pathMap = &(paths.at(i));
		map<FixPair, SearchPath>::const_iterator iter;
		for(iter=pathMap->begin(); iter!=pathMap->end(); ++iter) {
			const SearchPath* path = &(iter->second);
			deque<int> nodes = path->getNodes();
			for(unsigned int j=0; j<nodes.size(); ++j) {
				int id = nodes.at(j);
				map<int, Fix*>::iterator fiter = g_fixes_by_id.find(id);
				if(fiter != g_fixes_by_id.end()) {
					Fix* fix = fiter->second;
					ss << fix->getLongitude() << " " << fix->getLatitude() << endl;
				}
			}
			ss << endl;
		}
		ss << endl;
	}

	ofstream out;
	out.open(outfile.c_str());
	if(out.good()) {
		out << ss.str();
		out.close();
	}
}

static void write_polygons_gnuplot(const vector<Polygon>& polygons,
		                           const string& outfile) {

	stringstream ss;
	for (unsigned int i=0; i<polygons.size(); ++i) {
		const Polygon* poly = &(polygons.at(i));
		int n = poly->getNumVertices();
		const double* lons = poly->getXData();
		const double* lats = poly->getYData();
		for(int j=0; j<n; ++j) {
			ss << lons[j] << " " << lats[j] << endl;
		}
		ss << endl;
	}

	ofstream out;
	out.open(outfile.c_str());
	if (out.good()) {
		out << ss.str();
		out.close();
	}
}

static void write_polygon_sets_gnuplot(const PolygonLists& polygon_sets,
		                               const string& outfile) {
	stringstream ss;
	for (unsigned int i=0; i<polygon_sets.size(); ++i) {
		const PolygonSet* polygon_set = &(polygon_sets.at(i));
		for (unsigned int j=0; j<polygon_set->size(); ++j) {
			const Polygon* polygon = &(polygon_set->at(j));
			int n = polygon->getNumVertices();
			const double* lons = polygon->getXData();
			const double* lats = polygon->getYData();
			for(int k=0; k<n; ++k) {
				ss << lons[k] << " " << lats[k] << endl;
			}
			ss << endl;
		}
		ss << endl;
	}

	ofstream out;
	out.open(outfile.c_str());
	if(out.good()) {
		out << ss.str();
		out.close();
	}
}
#endif

static int validate_global_data() {
	// make sure the airway data has been loaded
	if(rg_airways.size() < 1) {
		string msg = "validate_global_data(): no airway data.";
		log_error(msg, true);
		return -1;
	}

	if(rg_fixes.size() < 1) {
		string msg = "validate_global_data(): no fix data.";
		log_error(msg, true);
		return -1;
	}

	return 0;
}

ResultSetKey::ResultSetKey(int scenario, double scale) :
	scenario(scenario), scale(scale){//, gnd_hold(0), cost(0) {
}

ResultSetKey::ResultSetKey(const ResultSetKey& that) :
	scenario(that.scenario), scale(that.scale){//, gnd_hold(that.gnd_hold), cost(that.cost) {
}

ResultSetKey::~ResultSetKey() {
}

bool ResultSetKey::operator<(const ResultSetKey& that) const {
	// keep keys ordered by ascending scenario number,
	// order by scale factor if scenario numbers are equal
	if(this->scenario < that.scenario) return true;
	if(this->scenario > that.scenario) return false;
	else {
		if(this->scale < that.scale) return true;
		else return false;
	}
}

bool ResultSetKey::operator==(const ResultSetKey& that) const {
	return ((this->scale == that.scale) && (this->scenario == that.scenario) );
}

ResultSetKey& ResultSetKey::operator=(const ResultSetKey& that) {
	if(this == &that) return *this;
	this->scenario = that.scenario;
	this->scale = that.scale;

	return *this;
}


class TrxAirportPairExtractor : public TrxInputStreamListener {
public:
	TrxAirportPairExtractor(set<FixPair>* const pairs=NULL) {
		if(pairs) {
			needs_free = false;
			this->pairs = pairs;
		} else {
			needs_free = true;
			this->pairs = new set<FixPair>();
		}
	}
	virtual ~TrxAirportPairExtractor() {
		if(needs_free) {
			if(this->pairs) {
				delete this->pairs;
			}
		}
	}

	virtual void onTrackTime(long timestamp) {
		// no-op
		(void)timestamp;
	}

	virtual void onTrack(const TrxRecord& trxRecord) {
		FixPair pair(trxRecord.route.origin, trxRecord.route.destination);
		pairs->insert(pair);
	}

	const set<FixPair>& getAirporPairs() const {
		return *pairs;
	}

private:
	bool needs_free;
	set<FixPair>* pairs;
};

class TrxFlightExtractor : public TrxInputStreamListener {
public:
	TrxFlightExtractor(vector<FixPair>* const flights=NULL) {
		if(flights) {
			needs_free = false;
			this->flights = flights;
		} else {
			needs_free = true;
			this->flights = new vector<FixPair>();
		}
	}
	virtual ~TrxFlightExtractor() {
		if(needs_free) {
			if(this->flights) delete this->flights;
		}
	}
	virtual void onTrackTime(long timestamp) {
		// no-op
		(void)timestamp;
	}

	virtual void onTrack(const TrxRecord& trxRecord) {
		double knotsToFps = 1.68780986;
		//string origin = (trxRecord.route.origin.length() > 3 && trxRecord.route.origin[0] == 'K' ? trxRecord.route.origin.substr(1,3) : trxRecord.route.origin);
		//string destination = (trxRecord.route.destination.length() > 3 && trxRecord.route.destination[0] == 'K' ? trxRecord.route.destination.substr(1,3) : trxRecord.route.destination);

		// the g_fixes map can contain airports as navaids or fixes.  these
		// use 3-letter codes such as SFO.  the g_airports map contains
		// airports using 4-letter codes such as KSFO. therefore,
		// in order to force the software to use airports instead of
		// fixes, we make sure that the origin and destination codes
		// are four letters starting with K.
		string origin = getCommercialAirportCode(trxRecord.route.origin, 4);
		string destination = getCommercialAirportCode(trxRecord.route.destination, 4);

		FixPair pair(origin, destination);
		// if the record airspeed is < min airspeed, replace
		// with min airspeed.
		if(trxRecord.tas < MIN_CRUISE_AIRSPEED_KNOTS) {
			pair.airspeed = MIN_CRUISE_AIRSPEED_KNOTS * knotsToFps;
		} else {
			pair.airspeed = trxRecord.tas * knotsToFps;
		}
		pair.callsign = trxRecord.acid;
		// need to set pair.altitude from mfl file.
		pair.altitude = trxRecord.cruiseAltitude;

		flights->push_back(pair);
	}

	const vector<FixPair>& getFlights() const {
		return *flights;
	}
private:
	bool needs_free;
	vector<FixPair>* flights;

	string getCommercialAirportCode(const string& code, int codeLen=4) {
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
};


#if ENABLE_COMPARE_SP_TO_WO
static double compute_flight_time(FixPair& flight, SearchPath* route, const string& debugFile="") {

	ofstream out;
	if(debugFile != "") {
		stringstream ss;
		ss << g_output_directory << "/" << flight.callsign << "_" << debugFile;
		out.open(ss.str().c_str());
		out << std::setprecision(3) << std::fixed << std::left;

		out << std::setw(10) << "# FIX1"
			<< std::setw(12) << "FIX1_LAT"
			<< std::setw(12) << "FIX1_LON"
			<< std::setw(10) << "FIX2"
			<< std::setw(12) << "FIX2_LAT"
			<< std::setw(12) << "FIX2_LON"
			<< std::setw(12) << "LAT_MID"
			<< std::setw(12) << "LON_MID"
			<< std::setw(12) << "ALT"
			<< std::setw(12) << "SPEED"
			<< std::setw(12) << "HEADING"
			<< std::setw(12) << "WIND_N"
			<< std::setw(12) << "WIND_E"
			<< std::setw(12) << "WIND_MAG"
			<< std::setw(12) << "WIND_DIR"
			<< std::setw(12) << "WIND_RES"
			<< std::setw(12) << "THETA"
			<< std::setw(12) << "SEG_LENGTH"
			<< std::setw(12) << "TOTAL_SPD"
			<< std::setw(12) << "SEG_TIME"
			<< std::setw(12) << "TOTAL_TIME"
			<< endl;
	}

	// we can use the wind-optimal cost function to compute the flight
	// time between two fixes
	double alt = flight.altitude;
	double spd = flight.airspeed;
	double T = 0;
	for(int i=1; i<route->size(); ++i) {
		int fromId = (*route)[i-1];
		int toId = (*route)[i];
		Fix* fromFix = NULL;
		Fix* toFix = NULL;
		get_fix(fromId, &fromFix);
		get_fix(toId, &toFix);
		if(!fromFix) {
			stringstream ss;
			ss << "Could not find fix for graph node id " << fromId;
			log_error(ss.str(), true);
		}
		if(!toFix) {
			stringstream ss;
			ss << "Could not find fix for graph node id " << toId;
			log_error(ss.str(), true);
		}
		if(fromFix == NULL) {
			cout << __FILE__ << " (" << __LINE__ << ")" << endl;
			cout << "fromId=" << fromId << ", toId=" << toId << ", numFixes=" << g_fixes.size() << ", numAirports=" << g_airports.size() << endl;
			exit(-1);
		}
		if(toFix == NULL) {
			cout << __FILE__ << " (" << __LINE__ << ")" << endl;
			cout << "fromId=" << fromId << ", toId=" << toId << ", numFixes=" << g_fixes.size() << ", numAirports=" << g_airports.size() << endl;
			exit(-1);
		}

		double t = wind_optimal_cost_function(fromFix->getLatitude(),
				                        fromFix->getLongitude(),
				                        alt,
				                        toFix->getLatitude(),
				                        toFix->getLongitude(),
				                        alt,
				                        spd);
		T += t;

		if(out.is_open()) {
			double twopi = 2.*M_PI;
			double d = compute_distance_gc(fromFix->getLatitude(),
					                       fromFix->getLongitude(),
					                       toFix->getLatitude(),
					                       toFix->getLongitude(),
					                       alt);
			double heading = compute_heading_gc(fromFix->getLatitude(),
                                                fromFix->getLongitude(),
                                                toFix->getLatitude(),
                                                toFix->getLongitude());
			double latMid,lonMid;
			compute_location_gc(fromFix->getLatitude(),
					            fromFix->getLongitude(),
					            d/2, heading,
					            &latMid, &lonMid,
					            alt);
			double windNorth=0,windEast=0;
			getWindFieldComponents(latMid, lonMid, alt, &windNorth, &windEast, true);
			double windMag = WIND_SCALE_FACTOR * sqrt(windNorth*windNorth + windEast*windEast);

			double windDir = atan2(windEast, windNorth);
			double headingRad = heading*M_PI/180.;
			while(headingRad < 0) headingRad += twopi;
			while(headingRad > twopi) headingRad -= twopi;
			while(windDir < 0) windDir += twopi;
			while(windDir > twopi) windDir -= twopi;

			// compute angle between d and wind vec:
			double theta = (headingRad - windDir);

			// resolve wind speed
			double vwind = windMag * cos(theta);
			double v = spd + vwind;

			out << std::setw(10) << fromFix->getName()
				<< std::setw(12) << fromFix->getLatitude()
				<< std::setw(12) << fromFix->getLongitude()
				<< std::setw(10) << toFix->getName()
				<< std::setw(12) << toFix->getLatitude()
				<< std::setw(12) << toFix->getLongitude()
				<< std::setw(12) << latMid
				<< std::setw(12) << lonMid
				<< std::setw(12) << alt
				<< std::setw(12) << spd
				<< std::setw(12) << (headingRad*180./M_PI)
				<< std::setw(12) << windNorth
				<< std::setw(12) << windEast
				<< std::setw(12) << windMag
				<< std::setw(12) << (windDir*180./M_PI)
				<< std::setw(12) << vwind
				<< std::setw(12) << (theta*180./M_PI)
				<< std::setw(12) << d
				<< std::setw(12) << v
				<< std::setw(12) << t
				<< std::setw(12) << T
				<< endl;
		}
	}

	if(out.is_open()) {
		out.close();
	}
	return T;
}
#endif

static SearchPath* find_shortest_path(SearchGraph& graph,
		                       FixPair& airportPair,
		                       map<FixPair, SearchPath>* const pathsOut,
		                       const bool forceNames=false) {

    (void)forceNames;

	string source = airportPair.origin;
	string sink = airportPair.destination;

	int sourceId = -1;
	int sinkId = -1;

#if 0
	string origin = getCommercialAirportCode(airportPair.origin);
	string destination = getCommercialAirportCode(airportPair.destination);

	bool source_is_airport = false;
	bool sink_is_airport = false;
	bool source_is_fix = false;
	bool sink_is_fix = false;

    map<string, int>::iterator ap_iter;
    map<string, int>::iterator fx_iter;

	// determine if the source and sink are airports or general fixes
    // note, airport keys in the g_airports map are prefixed with 'K',
    // such as 'KSFO' and not 'SFO'. this is true only for commercial
    // airports that are all letters. airport 94E, for example is just
    // 94E and not K94E.
	ap_iter = g_airport_ids.find(origin);
	if(ap_iter != g_airport_ids.end()) source_is_airport = true;

	ap_iter = g_airport_ids.find(destination);
	if(ap_iter != g_airport_ids.end()) sink_is_airport = true;



	if (!source_is_airport) {
	    fx_iter = g_fix_ids.find(source);
	    if(fx_iter != g_fix_ids.end()) {
	        source_is_fix = true;
	    }
	}

	if (!sink_is_airport) {
	    fx_iter = g_fix_ids.find(sink);
	    if(fx_iter != g_fix_ids.end()) {
	        sink_is_fix = true;
	    }
	}

	if (source_is_airport) {
	    source = origin;
	}
	if (sink_is_airport) {
	    sink = destination;
	}

	// if the source node is an airport then get the sourceId from
	// the global airport hash maps. otherwise, if it is not an airport
	// then get the sourceId from the global Fix hash maps.
	if (source_is_airport) {
	    sourceId = g_fixes.size() + g_airport_ids.at(source);
	} else {
	    if(source_is_fix) {
	        sourceId = g_fix_ids.at(source);
	    }
	}

    // if the sink node is an airport then get the sinkId from
    // the global airport hash maps. otherwise, if it is not an airport
    // then get the sinkId from the global Fix hash maps.
	if (sink_is_airport) {
	    sinkId = g_fixes.size() + g_airport_ids.at(sink);
	} else {
	    if(sink_is_fix) {
	        sinkId = g_fix_ids.at(sink);
	    }
	}
#endif

    int maxNodeId = rg_fixes.size()+rg_airports.size();

#if 1
	// if the forceNames flag is set, use the input fix pair names
	// as supplied.  otherwise, assume that the names are airport
	// names and check to see if they need to be prefixed with "K".
	map<string, int>::iterator oiter = rg_fix_ids.find(source);
	if (oiter != rg_fix_ids.end()) {
		sourceId = rg_fix_ids[source];
	} else {
		// not found in fixes, try airports...
		oiter = rg_airport_ids.find(source);
		if(oiter != rg_airport_ids.end()) {
			sourceId = rg_fixes.size()+rg_airport_ids[source];
		} else {
			// if !forceNames, then try stripping the leading 'K'
			// and check again in g_fix_ids and g_airport_ids.
			if(!forceNames) {
				source = source.substr(1);
				oiter = rg_fix_ids.find(source);
				if(oiter != rg_fix_ids.end()) {
					sourceId = rg_fix_ids[source];
				} else {
					oiter = rg_airport_ids.find(source);
					if(oiter != rg_airport_ids.end()) {
						sourceId = rg_fixes.size()+rg_airport_ids[source];
					} else {
						cout << "Error: could not find ID for source airport " << source << endl;
					}
				}
			}
		}
	}

	// if the forceNames flag is set, use the input fix pair names
	// as supplied.  otherwise, assume that the names are airport
	// names and check to see if they need to be prefixed with "K".
	map<string, int>::iterator diter = rg_fix_ids.find(sink);
	if (diter != rg_fix_ids.end()) {
		sinkId = rg_fix_ids[sink];
	} else {
		// not found in fixes, try airports...
		diter = rg_airport_ids.find(sink);
		if(diter != rg_airport_ids.end()) {
			sinkId = rg_fixes.size()+rg_airport_ids[sink];
		} else {
			//cout << __FILE__ << " (" << __LINE__ << ")" << endl;
			// if !forceNames, then try stripping the leading 'K'
			// and check again in g_fix_ids and g_airport_ids.
			if (!forceNames) {
				sink = sink.substr(1);
				diter = rg_fix_ids.find(sink);
				if(diter != rg_fix_ids.end()) {
					sinkId = rg_fix_ids[sink];
				} else {
					diter = rg_airport_ids.find(sink);
					if(diter != rg_airport_ids.end()) {
						sinkId = rg_fixes.size()+rg_airport_ids[sink];
					} else {
						cout << "Error: could not find ID for sink airport " << sink << endl;
					}
				}
			}
		}
	}
#endif

    // if fixes contains both origin and destination, okay to proceed.
    if (sourceId < 0 || sinkId < 0) {
        cout << "  Warning: find_shortest_paths - could not find fix id:"
             << endl
             << "           sourceId=" << sourceId
             << " (" << source << "), sinkId="
             << sinkId << " (" << sink << ")" << endl << endl;
        return NULL;
    }

    if (sourceId >= maxNodeId || sinkId >= maxNodeId) {
        cout << "  Warning: find_shortest_paths - could not find fix id:"
             << endl
             << "           sourceId=" << sourceId
             << " (" << source << "), sinkId="
             << sinkId << " (" << sink << ")" << endl << endl;
        return NULL;
    }

	// if one of the airports is in a polygon then don't find a path
	// for the pair.  the heuristic cost for an airport will have been
	// set to MIN_DOUBLE in reduce_airway_connectivity() if it was
	// found in a polygon.
	if (graph.isRemoved(sourceId)) {
#ifndef NDEBUG
		cout << "Skip pair in polygon (source): " << source
			 << " (" << sourceId << ")" << " -> " << sink
			 << " (" << sinkId << ")" << endl;
#endif
		return NULL;
	}
	if (graph.isRemoved(sinkId)) {
#ifndef NDEBUG
		cout << "Skip pair in polygon (sink): " << source
			 << " (" << sourceId << ")" << " -> " << sink
			 << " (" << sinkId << ")" << endl;
#endif
		return NULL;
	}

#ifndef NDEBUG
	cout << "Finding shortest path for pair: " << source
		 << " (" << sourceId << ")" << " -> " << sink
		 << " (" << sinkId << ")" << endl;
#endif

	// compute heuristic costs of each node in the graph
	// use great-circle distance from the goal node
	map<string, Fix*>::iterator fiter;

	Fix* goal = (sinkId >= (int)rg_fixes.size() ? rg_airports.at(sink) : rg_fixes.at(sink));

	double alt = DEFAULT_ALTITUDE_FT; // default altitude of 30000 ft.
	map<int, SearchNode*>* nodes = const_cast<map<int, SearchNode*>*>(
			                           &(graph.getNodes()));
	map<int, SearchNode*>::iterator niter;

	for (niter=nodes->begin(); niter!=nodes->end(); ++niter) {
		int id = niter->first;

		Fix* fix = NULL;
		get_fix(id, &fix);
		if(fix == NULL) {
			cout << __FILE__ << " (" << __LINE__ << ")" << endl;
			cout << "id=" << id
			     << ", numFixes=" << rg_fixes.size()
			     << ", numAirports=" << rg_airports.size()
			     << endl;
			exit(-1);
		}
		double d = compute_distance_gc(fix->getLatitude(),
											  fix->getLongitude(),
											  goal->getLatitude(),
											  goal->getLongitude(),
											  alt);
		graph.setHeuristicCost(id, d);
	}

	SearchPath path;
	AstarSearch astar;
#ifndef NDEBUG
	clock_t ast = clock();
#endif
	astar.findPath(graph, sourceId, sinkId, &path);
#ifndef NDEBUG
	cout<< "A* solution time = "<<(double)(clock() - ast)/CLOCKS_PER_SEC<<endl;
#endif
	// if the path contains only 1 node and the source and sink are
	// the same, then add the sink id to make path length 2.
	if(path.getNodes().size() == 1 && sourceId == sinkId) {
		if( path.front() == sourceId || path.front() == sinkId ) {
			path.push_front(sourceId);
		}
	}

	omp_set_lock(&paths_lock);
	pathsOut->insert(std::pair<FixPair, SearchPath>(airportPair, path));
	omp_unset_lock(&paths_lock);

	return &pathsOut->at(airportPair);
}

static SearchPath* find_lowest_cost_path(SearchGraph& graph,
                                   FixPair& flight,
                                   double (*cost_function)(const double&, const double&, const double&, const double&, const double&, const double&, const double&, WindGrid* const, vector<WindGrid>* const),
                                   double (*heuristic_function)(const double&, const double&, const double&, const double&, const double&, const double&, const double&),
                                   map<FixPair, SearchPath>* const pathsOut,
		                           //TODO:PARIKSHIT ADDER
		                           WindGrid* const g_wind = NULL,
		                           vector<WindGrid>* const g_wind_vec = NULL,
								   bool& costFlag = cost_compute_flag
		                           //TODO:PARIKSHIT ADDER ENDS
		                           ) {

	string source = flight.origin;
	string sink = flight.destination;

	int sourceId = -1;
	int sinkId = -1;

	Fix* start = NULL;
	Fix* goal = NULL;

	bool forceNames = false;

	sourceId = get_fix(flight.origin, &start, forceNames);
	sinkId = get_fix(flight.destination, &goal, forceNames);


	// if fixes contains both origin and destination, okay to proceed.
	if (sourceId < 0) {
		log_error(string("Could not find fixes " + flight.origin + " on aircraft " + flight.callsign), false);

		return NULL;
	} else if (sinkId < 0) {
		log_error(string("Could not find fixes " + flight.destination + " on aircraft " + flight.callsign), false);

		return NULL;
	}

	// if one of the airports is in a polygon then don't find a path
	// for the pair.  the heuristic cost for an airport will have been
	// set to MIN_DOUBLE in reduce_airway_connectivity() if it was
	// found in a polygon.
	if (graph.isRemoved(sourceId)) {
#ifndef NDEBUG
		cout << "Skip pair in polygon (source): " << source
			 << " (" << sourceId << ")" << " -> " << sink
			 << " (" << sinkId << ")" << endl;
#endif
		return NULL;
	}
	if (graph.isRemoved(sinkId)) {
#ifndef NDEBUG
		cout << "Skip pair in polygon (sink): " << source
			 << " (" << sourceId << ")" << " -> " << sink
			 << " (" << sinkId << ")" << endl;
#endif
		return NULL;
	}

#ifndef NDEBUG
	cout << "Finding lowest cost route: " << source
		 << " (" << sourceId << ")" << " -> " << sink
		 << " (" << sinkId << ")" << " @ " << flight.airspeed << endl;
#endif

	// recompute the path costs in the search graph using this flight's
	// cruise speed.  here, we iterate over each link in the graph
	// and compute its cost.
	int edgeCount = 0;
	double time_sc = 0.0, time_vc = 0.0;
	map< std::pair<int, int>, double >::iterator edgeIter;
	int num_max = 0;

	//HAS TO BE CREATED FOR EACH AIRPORT PAIR.
	// ALTITUDE AND AIRSPEED ARE DIFFERENT FOR EACH AIRCRAFT
	//HENCE EACH AIRPORT PAIR WILL HAVE DIFFERENT COST FUNC VALUES.
	for (edgeIter = graph.edgeCostsBegin();
		edgeIter != graph.edgeCostsEnd();
		++edgeIter) {

		if (costFlag)
			break;

		edgeCount++;

		const std::pair<int, int>* key= &(edgeIter->first);
		int fromId = key->first;
		int toId = key->second;

		if (graph.isRemoved(fromId) ||
				graph.isRemoved(toId) ||
				graph.isMultiplied(fromId,toId))
			continue;

		double p_t = graph.getEdgeCost(fromId, toId);
		vector<double> pv_t = graph.getEdgeCostVector(fromId, toId);

		Fix* fromFix = NULL;
		Fix* toFix = NULL;
		get_fix(fromId, &fromFix);
		get_fix(toId, &toFix);

		if (fromFix == NULL) { }
		if (toFix == NULL) {
			cout << __FILE__ << " (" << __LINE__ << ")" << endl;
			cout << "fromId=" << fromId << ", toId=" << toId << ", numFixes=" << rg_fixes.size() << ", numAirports=" << rg_airports.size() << endl;
			exit(-1);
		}

		double fromLat = fromFix->getLatitude();
		double fromLon = fromFix->getLongitude();
		double toLat = toFix->getLatitude();
		double toLon = toFix->getLongitude();

		vector<double> tvec;
		tvec.clear();

		clock_t tst, tmid, tend;
		tst = clock();
		double t = cost_function(fromLat,
								fromLon,
								flight.altitude,
								toLat,
								toLon,
								flight.altitude,
								flight.airspeed,
								g_wind,
								g_wind_vec);
		tmid = clock();

		if (!g_wind_vec->empty()) {
			tvec = wind_optimal_cost_function(fromLat,
												fromLon,
												flight.altitude,
												toLat,
												toLon,
												flight.altitude,
												flight.airspeed,
												g_wind_vec);
		}
		else {
			tvec = pv_t;
		}

		tend = clock();

		double dt1 = (double)(tmid - tst) / CLOCKS_PER_SEC;
		double dt2 = (double)(tend - tmid) / CLOCKS_PER_SEC;

		time_sc = time_sc + dt1;
		time_vc = time_vc + dt2;

		/*
		 * TODO:PARIKSHIT ADDER.
		 * WE WANT TO LEAVE OUT EDGES THAT HAVE MAX NUMERIC LIMIT AS THEIR COSTS.
		 * ALSO WE WANT TO LEAVE ALONE THE EDGES WITH G_MAX_COST (MEANING AIRMETS).
		 * HENCE THE THRESHOLD TO LEAVE EDGES HAS BEEN SET TO G_MAX_COST.
		 */


		double set_cost = t;
		vector<double> set_vec = tvec;

		bool is_max = false;
		for (size_t s = 0; s < pv_t.size(); ++s) {
			if (pv_t.at(s) >= 1e100) {
				is_max = true;
				break;
			}
		}

		if (p_t >= 1e100 || is_max) {
			++num_max;
			set_cost = p_t;
			set_vec = pv_t;
		}

		graph.setEdgeCostVector(fromId, toId, set_vec);
		graph.setEdgeCost(fromId, toId, set_cost);

		/*
		 * TODO:PARIKSHIT ADDER ENDS HERE
		 */
	}
	costFlag = true;

#ifndef NDEBUG
	cout << "Time needed to process scalar cost = "  << time_sc	<< " and vector cost = "
		 << time_vc	 << endl;
#endif

#ifndef NDEBUG
	cout << "edgeCount=" << edgeCount << ", numEdges=" << graph.numEdges() << ", numNodes = "<<graph.size()
		 << ", num max cost edges = " << num_max << endl;
#endif

	// recompute the heuristics in the search graph using this flight's
	// cruise speed.
	map<int, SearchNode*>* nodes = const_cast<map<int, SearchNode*>*>(
			                           &(graph.getNodes()));
	map<int, SearchNode*>::iterator niter;
	for (niter = nodes->begin(); niter != nodes->end(); ++niter) {

		int id = niter->first;

		Fix* fix = NULL;
		get_fix(id, &fix);
		if (fix == NULL) {
			cout << __FILE__ << " (" << __LINE__ << ")" << endl;
			cout << "id=" << id << ", numFixes=" << rg_fixes.size() << ", numAirports=" << rg_airports.size() << endl;
			exit(-1);
		}

		double t = heuristic_function(fix->getLatitude(),
									  fix->getLongitude(),
									  flight.altitude,
									  goal->getLatitude(),
									  goal->getLongitude(),
									  flight.altitude,
									  flight.airspeed);

		graph.setHeuristicCost(id, t);
	}

	// perform A* search
#ifndef NDEBUG
	clock_t ast = clock();
#endif

	SearchPath path;
	AstarSearch astar;
	astar.findPath(graph,
			sourceId,
			sinkId,
			&path,
			(!g_wind_vec->empty()) ? true : false);
#ifndef NDEBUG
	cout << "Num nodes in path = " << path.size() << endl;
#endif
#ifndef NDEBUG
	cout << "A* solution time = " << (double)(clock() - ast)/CLOCKS_PER_SEC << endl;
#endif

	// if the path contains only 1 node and the source and sink are
	// the same, then add the sink id to make path length 2.
	if (path.getNodes().size() == 1 && sourceId == sinkId) {
		if ( path.front() == sourceId || path.front() == sinkId ) {
			path.push_front(sourceId);
		}
	}

	omp_set_lock(&paths_lock);
	pathsOut->insert(std::pair<FixPair, SearchPath>(flight, path));
	omp_unset_lock(&paths_lock);

	return &pathsOut->at(flight);
}

/**
 * This function finds the wind optimal route for a single flight's origin
 * airport, destination airport, and cruise speed.
 * The flight origin, destination, and cruise speed are stored in a FixPair
 * object.
 */
// Deprecated: replaced by find_lowest_cost_path()
#if 0
static SearchPath* find_wind_optimal_path(SearchGraph& graph,
                                   FixPair& flight,
                                   map<FixPair, SearchPath>* const pathsOut,
                                   const bool forceNames=false) {

	string source = flight.origin;
	string sink = flight.destination;

	int sourceId = -1;
	int sinkId = -1;

	Fix* start = NULL;
	Fix* goal = NULL;

	sourceId = get_fix(flight.origin, &start, forceNames);
	sinkId = get_fix(flight.destination, &goal, forceNames);

	// if fixes contains both origin and destination, okay to proceed.
	if(sourceId < 0 || sinkId < 0) {
		return NULL;
	}

	// if one of the airports is in a polygon then don't find a path
	// for the pair.  the heuristic cost for an airport will have been
	// set to MIN_DOUBLE in reduce_airway_connectivity() if it was
	// found in a polygon.
	if(graph.isRemoved(sourceId)) {
#ifndef NDEBUG
		cout << "Skip pair in polygon (source): " << source
			 << " (" << sourceId << ")" << " -> " << sink
			 << " (" << sinkId << ")" << endl;
#endif
		return NULL;
	}
	if(graph.isRemoved(sinkId)) {
#ifndef NDEBUG
		cout << "Skip pair in polygon (sink): " << source
			 << " (" << sourceId << ")" << " -> " << sink
			 << " (" << sinkId << ")" << endl;
#endif
		return NULL;
	}

#ifndef NDEBUG
	cout << "Finding wind-optimal route: " << source
		 << " (" << sourceId << ")" << " -> " << sink
		 << " (" << sinkId << ")" << " @ " << flight.airspeed << endl;
#endif

	// recompute the path costs in the search graph using this flight's
	// cruise speed.  here, we iterate over each link in the graph
	// and compute its cost.
	int edgeCount = 0;
	map< std::pair<int,int>, double >::iterator edgeIter;
	for(edgeIter=graph.edgeCostsBegin();
		edgeIter!=graph.edgeCostsEnd();
		++edgeIter) {

		edgeCount++;

		const std::pair<int,int>* key= &(edgeIter->first);
		int fromId = key->first;
		int toId = key->second;

		Fix* fromFix = NULL;
		Fix* toFix = NULL;
		get_fix(fromId, &fromFix);
		get_fix(toId, &toFix);

		if(fromFix == NULL) {
			cout << __FILE__ << " (" << __LINE__ << ")" << endl;
			cout << "fromId=" << fromId << ", toId=" << toId << ", numFixes=" << g_fixes.size() << ", numAirports=" << g_airports.size() << endl;
			exit(-1);
		}
		if(toFix == NULL) {
			cout << __FILE__ << " (" << __LINE__ << ")" << endl;
			cout << "fromId=" << fromId << ", toId=" << toId << ", numFixes=" << g_fixes.size() << ", numAirports=" << g_airports.size() << endl;
			exit(-1);
		}

		double fromLat = fromFix->getLatitude();
		double fromLon = fromFix->getLongitude();
		double toLat = toFix->getLatitude();
		double toLon = toFix->getLongitude();
		double alt = DEFAULT_ALTITUDE_FT;
		double t = wind_optimal_cost_function(fromLat, fromLon, alt,
				                              toLat, toLon, alt,
				                              flight.airspeed);

		graph.setEdgeCost(fromId, toId, t);
	}

	// recompute the heuristics in the search graph using this flight's
	// cruise speed.
	double alt = DEFAULT_ALTITUDE_FT; // default altitude of 30000 ft.
	map<int, SearchNode*>* nodes = const_cast<map<int, SearchNode*>*>(
			                           &(graph.getNodes()));
	map<int, SearchNode*>::iterator niter;
	for(niter=nodes->begin(); niter!=nodes->end(); ++niter) {
		int id = niter->first;

		Fix* fix = NULL;
		get_fix(id, &fix);
		if(fix == NULL) {
			cout << __FILE__ << " (" << __LINE__ << ")" << endl;
			cout << "id=" << id << ", numFixes=" << g_fixes.size() << ", numAirports=" << g_airports.size() << endl;
			exit(-1);
		}

		double t = wind_optimal_heuristic_function(fix->getLatitude(),
											  fix->getLongitude(),
											  alt,
											  goal->getLatitude(),
											  goal->getLongitude(),
											  alt,
											  flight.airspeed);
		graph.setHeuristicCost(id, t);
	}

	// perform A* search
	SearchPath path;
	AstarSearch astar;
	astar.findPath(graph, sourceId, sinkId, &path);

	// if the path contains only 1 node and the source and sink are
	// the same, then add the sink id to make path length 2.
	if(path.getNodes().size() == 1 && sourceId == sinkId) {
		if( path.front() == sourceId || path.front() == sinkId ) {
			path.push_front(sourceId);
		}
	}
	pathsOut->insert(std::pair<FixPair, SearchPath>(flight, path));
	return &pathsOut->at(flight);
}
#endif /* if 0 */

/**
 * This function runs the lowest-level loop to invoke A* over the
 * graph and find the wind-optimal path for each flight.  The input
 * graph to this function should have had the fixes within polygons
 * already removed.
 */
static void find_wind_optimal_paths(SearchGraph& graph,
		                            vector<FixPair>& flights,
		                            map<FixPair, SearchPath>* const pathsOut,
		                            WindGrid* const g_wind = NULL,
		                            vector<WindGrid>* const g_wind_vec = NULL,
		                            const bool forceNames=false,
									bool& costFlag = cost_compute_flag) {
	(void)forceNames;
	int count = 0;
	int total = flights.size();


	omp_init_lock(&paths_lock);
    // we don't use omp parallel because we would have to to make copies
    // of the search graph for each thread. this kills any performance
    // benefits from multi-threading.
	//pragma omp parallel for schedule(static, 128)
	for (unsigned int i = 0; i < flights.size(); ++i) {
		// find wind optimal path
		find_lowest_cost_path(graph, flights.at(i),
				wind_optimal_cost_function,
				wind_optimal_heuristic_function,
				pathsOut,
				g_wind,
				g_wind_vec,
				costFlag);
	}

	map<FixPair,SearchPath>::iterator iter;
	for (iter = pathsOut->begin(); iter != pathsOut->end(); ++iter) {
		const FixPair *fp = &(iter->first);
		SearchPath *sp = &(iter->second);
		const deque<int> nodes = sp->getNodes();
		double c_cost = 0.0, w_cost = 0.0;
		double alt = fp->altitude;
		double spd = fp->airspeed;
		if (nodes.size() > 0) {
			for (unsigned int p = 0; p < nodes.size()-1; ++p) {
				int fID = nodes.at(p);
				int tID = nodes.at(p+1);
				Fix* f_fix = NULL; get_fix(fID,&f_fix);
				Fix* t_fix = NULL; get_fix(tID,&t_fix);
				double lat1 = f_fix->getLatitude(),lat2 = t_fix->getLatitude();
				double lon1 = f_fix->getLongitude(),lon2 = t_fix->getLongitude();
				double w_c = wind_optimal_cost_function(lat1, lon1, alt,
															lat2, lon2, alt,
															spd, g_wind, g_wind_vec);

				double c_c = 0;
				if (g_wind_vec->empty())
					c_c = graph.getEdgeCost(nodes.at(p), nodes.at(p+1));
				else
					c_c = graph.getEdgeCost(nodes.at(p), nodes.at(p+1),c_cost);
				c_cost += c_c;
				w_cost += w_c;

			}
		}
		sp->setPathCost(c_cost);
	}

	omp_destroy_lock(&paths_lock);
}




/**
 * This function runs the lowest-level loop to invoke A* over the
 * graph and find the shortest path for each fix pair.  The input
 * graph to this function should have had the fixes within polygons
 * already removed.
 */
static void find_shortest_paths(SearchGraph& graph,
		                        set<FixPair>& airportPairs,
		                        map<FixPair, SearchPath>* const pathsOut,
		                        const bool forceNames=false) {

	set<FixPair>::iterator iter;
    vector<FixPair> pairsVec(airportPairs.begin(), airportPairs.end());

    omp_init_lock(&paths_lock);
    // we don't use omp parallel because we would have to to make copies
    // of the search graph for each thread. this kills any performance
    // benefits from multi-threading.

	for (unsigned int i = 0; i < pairsVec.size(); ++i) {
		find_shortest_path(graph, pairsVec.at(i), pathsOut, forceNames);
	}

	omp_destroy_lock(&paths_lock);
}




/**
 * This function runs shortest-path A* with wind and computes the flight time.
 * It runs the wind-optimal A* and computes the flight time.
 * Differences in flight time are output to text file.
 */
#if ENABLE_COMPARE_SP_TO_WO
static void compare_wind_optimal_paths(SearchGraph& graph,
		                               vector<FixPair>& flights,
		                               map<FixPair, SearchPath>* const pathsOut) {

	stringstream ss;
	ss << std::fixed << std::left << std::setprecision(6);

	string errorsFname = g_output_directory + string("/flight_time_errors.log");
	ofstream errors;
	errors.open(errorsFname.c_str());

	string fname = g_output_directory + string("/flight_times.out");
	ofstream out;
	out.open(fname.c_str());
	out << std::fixed << std::left << std::setprecision(6);


	// compute the flight times for shortest paths (includes wind)
	// we can use the wind_optimal_cost_function to compute the
	// flight time of each route segment.
	vector<FixPair>::iterator iter;
	int count=0;
	int total= flights.size();
	for(iter=flights.begin(); iter!=flights.end(); ++iter) {

		FixPair* key = &(*iter);
		cout << "Process flight " << (++count) << " of " << total << endl;

		map<FixPair, SearchPath> shortestPaths;
		map<FixPair, SearchPath> windOptimalPaths;

		// find shortest path (ignore wind)
		SearchPath* sp = find_lowest_cost_path(graph, *key,
				                               gc_distance_cost_function,
				                               gc_distance_cost_function,
				                               &shortestPaths);

		// find wind optimal path
		SearchPath* wo = find_lowest_cost_path(graph, *key,
				                               wind_optimal_cost_function,
				                               wind_optimal_heuristic_function,
				                               &windOptimalPaths);

		// make sure that there is both a shortest path and a wind optimal
		// path for the flight.  if one is missing, then log the error
		// and skip to the next flight.
		if(sp==NULL || wo==NULL) {
			errors << key->callsign << ": missing path (sp=" << (sp!=NULL) << ", wo=" << (wo!=NULL) << ")" << endl;
			continue;
		}

		// compute the flight times of shortest path (includes wind)
		double shortestPathTime = compute_flight_time(*key, sp, "sp.out");

		// compute the flight times of wind-optimal path (includes wind)
		double windOptimalPathTime = compute_flight_time(*key, wo, "wo.out");

		// compute the difference in flight times
		double dt = windOptimalPathTime - shortestPathTime;

		pathsOut->insert(std::pair<FixPair,SearchPath>(*key, *wo));

		// log the times
		out << std::setw(10) << std::setprecision(0) << key->callsign
		   << std::setw(10) << std::setprecision(0) << key->altitude
		   << std::setw(10) << std::setprecision(3) << key->airspeed
		   << std::setw(16) << std::setprecision(3) << shortestPathTime
		   << std::setw(16) << std::setprecision(3) << windOptimalPathTime
		   << std::setw(16) << std::setprecision(3) << dt
		   << "\n";
	}

	out.close();

	errors.close();
}
#endif

#if !USE_LIBSEARCH
static int get_paths(SearchGraph& reducedGraph,
		             set<FixPair>& airportPairs,
		             vector< map<FixPair, SearchPath> >* const pathsOut) {

	// make sure we have valid output pointer
	if(!pathsOut) {
		log_error("get_paths(): invalid output pointer.", true);
		return -1;
	}

	// make sure the airway data has been loaded
	if(validate_global_data() < 0) {
		return -1;
	}

	// find the shortest paths for the input polygons and add the
	// results to the output.
	map<FixPair, SearchPath> paths;
	find_shortest_paths(reducedGraph, airportPairs, &paths);
	pathsOut->push_back(paths);

	return 0;
}

static int get_wind_optimal_paths(SearchGraph& reducedGraph,
        vector<FixPair>& flights,
        vector< map<FixPair, SearchPath> >* const pathsOut,
        WindGrid* const g_wind,
        vector<WindGrid>* const g_wind_vec = NULL,
		bool& costFlag = cost_compute_flag
        ) {
	// make sure we have valid output pointer
	if (!pathsOut) {
		log_error("get_paths(): invalid output pointer.", true);
		return -1;
	}

	// make sure the airway data has been loaded
	if (validate_global_data() < 0) {
		return -1;
	}

	// find the shortest paths for the input polygons and add the
	// results to the output.
	map<FixPair, SearchPath> paths;
	find_wind_optimal_paths(reducedGraph, flights, &paths, g_wind, g_wind_vec, costFlag);
	pathsOut->push_back(paths);

	return 0;
}

#else

#endif

static int compareRerouteWithNominal(const SearchGraph& reducedGraph,
								const FixPair* const pair,
								const SearchPath* const nominalPath,
								vector< map<FixPair,SearchPath> >* const tmpPaths,
								double& p_cost) {
	int i = 0;
	p_cost = i*3600;

	deque<int> nodes = nominalPath->getNodes();
	size_t num_hrs = 0;
	if (nodes.size() > 1)
		num_hrs = reducedGraph.getEdgeCostVectorSize(nodes.at(0), nodes.at(1));

	double n_cost = 0;
	if (nodes.size() > 1) {
		for (size_t k = 0; k < nodes.size()-1; ++k) {
			int fid = nodes.at(k);
			int tid = nodes.at(k+1);

			if (num_hrs > 1)
				n_cost += reducedGraph.getEdgeCost(fid, tid, n_cost);
			else
				n_cost += reducedGraph.getEdgeCost(fid, tid);

			if (n_cost > 1e20){
				i++;
				break;
			}
		}
	}

	if (tmpPaths->front().empty()) {
		SearchPath path;
		path.clear();

		while (true) {
			for (size_t k = 0; k < nodes.size()-1; ++k) {
				int fid = nodes.at(k); int tid = nodes.at(k+1);
				if (num_hrs > 1)
					p_cost += reducedGraph.getEdgeCost(fid, tid, p_cost);
				else
					p_cost += reducedGraph.getEdgeCost(fid, tid);
			}

			if (i >= (int)num_hrs) {
				i = 0;
				break;
			}
			if (p_cost < 1e10) {
				path = *nominalPath;
				break;
			}
			else {
				p_cost = 3600*(++i);//st_cost = p_cost;
			}
		}
		if (path.size() != 0) {
			map<FixPair,SearchPath> pathmap;
			pathmap.insert(std::pair<FixPair, SearchPath>(*pair, path));
			tmpPaths->push_back(pathmap);
		}
	}
	else {
		SearchPath init_sp = tmpPaths->front().find(*pair)->second;
		double ini_cost = 0.0;
		deque<int> nodes2 = init_sp.getNodes();
		if (nodes2.size() > 1) {
			for (size_t k = 0; k < nodes2.size()-1; ++k) {
				int fid = nodes2.at(k);
				int tid = nodes2.at(k+1);

				if (num_hrs > 1)
					ini_cost += reducedGraph.getEdgeCost(fid, tid, ini_cost);
				else
					ini_cost += reducedGraph.getEdgeCost(fid, tid);
			}
		}

		if (ini_cost < n_cost) {
			p_cost = ini_cost;
			return 0;
		}

		SearchPath path = init_sp;
		while (true) {
			if (nodes2.size() > 1) {
				for (size_t k = 0; k < nodes.size()-1; ++k) {
					if (k < nodes2.size()-1) {
						int fid = nodes2.at(k);
						int tid = nodes2.at(k+1);

						if (num_hrs > 1)
							p_cost += reducedGraph.getEdgeCost(fid, tid, p_cost);
						else
							p_cost += reducedGraph.getEdgeCost(fid, tid);
					}
				}
			}
			if (p_cost >= num_hrs*3600 || p_cost >= ini_cost) {
				path = *nominalPath;
				i = 0;
				break;
			}
			if (p_cost < ini_cost) {
				path = *nominalPath;
				i = 0;
				break;
			}
			else {
				p_cost = 3600*++i;
				ini_cost = p_cost;
			}
		}

		tmpPaths->front().at(*pair) = path;
	}

	return i;
}

/*
 * Find the entry/exit fix ids along the path that are closest to the
 * polygon boundary and lie outside of the polygon
 */
static void getEntryExitFixes(SearchPath& path,
		Polygon& poly,
		Fix** const entryFix,
		Fix** const exitFix,
		const int& offset = 0) {
	int entryId;
	if (path.getNodes().size() > 0)
		entryId = path.front();
	if (offset > 0) {
		SearchPathIterator it = path.begin() + offset;
		entryId = *it;
	}

	Fix* entry = NULL;
	if (entryId >= (int)rg_fixes.size()) {
		if (rg_airports_by_id.find(entryId-rg_fixes.size()) != rg_airports_by_id.end())
			entry = rg_airports_by_id.at(entryId-rg_fixes.size());
	} else {
		if (rg_fixes_by_id.find(entryId) != rg_fixes_by_id.end())
			entry = rg_fixes_by_id.at(entryId);
	}

	// assign exit fix
	int exitId;
	if (path.getNodes().size() > 0)
		exitId = path.back();

	Fix* exit = NULL;
	if (exitId >= (int)rg_fixes.size()) {
		if (rg_airports_by_id.find(exitId-rg_fixes.size()) != rg_airports_by_id.end())
			exit = rg_airports_by_id.at(exitId-rg_fixes.size());
	} else {
		if (rg_fixes_by_id.find(exitId) != rg_fixes_by_id.end())
			exit = rg_fixes_by_id.at(exitId);
	}

	// find the polygon entry/exit fixes by iterating over the
	// path from beginning to end and marking when the fixes
	// are found inside the polygon or outside the polygon.
	SearchPathIterator it;
	for (it = path.begin()+offset; it != path.end(); ++it) {
		int id = (*it);
		Fix* fix = NULL;
		if (id >= (int)rg_fixes.size()) {
			if (rg_airports_by_id.find(id-rg_fixes.size()) != rg_airports_by_id.end())
				fix = rg_airports_by_id.at(id-rg_fixes.size());
		} else {
			if (rg_fixes_by_id.find(id) != rg_fixes_by_id.end())
				fix = rg_fixes_by_id.at(id);
		}

		if (fix) {
			double lat = fix->getLatitude();
			double lon = fix->getLongitude();
			double x, y;
			latlon_to_xy(lat, lon, &x, &y);

			// set the found entry/exit flags
			if (poly.contains(lon, lat)) {
				break;
			} else {
				entry = fix;
			}
		}
	}

	// copy the fix ids to the output variables.
	if (entryFix) *entryFix = entry;
	if (exitFix) *exitFix = exit;
}

/**
 * Compute the convex hull of the union of input polygons
 */
void polygonUnionConvexHull(vector<Polygon>& polygons,
		                    vector<double>* const xconvex,
		                    vector<double>* const yconvex) {

	if(!xconvex) return;
	if(!yconvex) return;

	// count the number of data points
	int numPoints = 0;
	for(unsigned int i=0; i<polygons.size(); ++i) {
		const Polygon* poly = &(polygons.at(i));
		numPoints += poly->getNumVertices();
	}

	// allocate and initialize x,y data vectors
	double xdata[numPoints];
	double ydata[numPoints];
	memset(xdata, 0, numPoints*sizeof(double));
	memset(ydata, 0, numPoints*sizeof(double));

	// add data to x,y
	int offset = 0;
	for(unsigned int i=0; i<polygons.size(); ++i) {
		const Polygon* poly = &(polygons.at(i));
		int n = poly->getNumVertices();
		memcpy(xdata+offset, poly->getXData(), n*sizeof(double));
		memcpy(ydata+offset, poly->getYData(), n*sizeof(double));
		offset += n;
	}

	// compute the convex hull
	convex_hull_scan(numPoints, xdata, ydata, xconvex, yconvex);
}

/**
 * Post process the rerouted segments.
 * prefix the reroute path with the nominal path fixes up to the entry fix.
 *
 * Starting with the entry fix, perform the heading check to remove
 * fixes that create backtracking.
 */
void processReroutedPath(SearchPath& nominalPath, SearchPath& reroute) {

	// form the complete route path
	int startId = reroute.front();
	deque<int> nodes = nominalPath.getNodes();
	deque<int>::iterator it;
	for (it = find(nodes.begin(), nodes.end(), startId)-1; it >= nodes.begin(); --it) {
		reroute.push_front(*it);
	}

	// iterate over complete rerouted path and check for backtracking.
	// start at the polygon entry fix and proceed until we come to the first
	// link that is not a back tracking link.
	// NOTE: this will only remove the initial back-tracking from the entry
	// fix.  it is still possible for the route to have backtracking
	// in the beginning or remainder of the route.
	double pi = M_PI;
	double twoPi = 2.*M_PI;
	double halfPi = .5*M_PI;
	vector<int> toErase;

	it=find(reroute.begin(), reroute.end(), startId);
	int id = *it;
	int idprev = (it == reroute.begin() ? *it : *(it-1));

	if(id == idprev) return;

	Fix* fix_entry = (id>=(int)rg_fixes.size() ? rg_airports_by_id[id-rg_fixes.size()] : rg_fixes_by_id[id]);
	Fix* fix_prev = (idprev>=(int)rg_fixes.size() ? rg_airports_by_id[idprev-rg_fixes.size()] : rg_fixes_by_id[idprev]);

	double x,y,xprev,yprev;
	latlon_to_xy(fix_entry->getLatitude(), fix_entry->getLongitude(), &x, &y);
	latlon_to_xy(fix_prev->getLatitude(), fix_prev->getLongitude(), &xprev, &yprev);

	double theta0 = atan2(y-yprev, x-xprev);
	if(theta0 < 0) theta0 += twoPi;

	for(it=it+1; it!=reroute.end(); ++it) {

		Fix* fix_next = ((*it)>=(int)rg_fixes.size() ? rg_airports_by_id[(*it)-rg_fixes.size()] : rg_fixes_by_id[*it]);
		if(!fix_next) continue;

		double xnext,ynext;
		latlon_to_xy(fix_next->getLatitude(), fix_next->getLongitude(), &xnext, &ynext);

		double theta1 = atan2(ynext-y, xnext-x);
		if(theta1 < 0) theta1 += twoPi;

		double dtheta = theta1-theta0;
		if(dtheta >= pi) dtheta -= twoPi;
		if(dtheta < -pi) dtheta += twoPi;

		if((dtheta > halfPi) || (dtheta < -halfPi)) {
			toErase.push_back(it-reroute.begin());
		} else {
			// break once we find that we are not backtracking.
			break;
		}
	}

	if(toErase.size() > 0) {
		vector<int>::reverse_iterator riter;
		for(riter=toErase.rbegin(); riter!=toErase.rend(); ++riter) {
			int index = *riter;
			reroute.erase(reroute.begin()+index, reroute.begin()+index+1);
		}
	}
}

void getScaledWeatherPolygons(const PolygonLists& weatherScenarios,
		                      const double scalingFactors[],
		                      int numScalingFactors,
		                      map<double, PolygonLists>& weatherScenariosList) {

	// for each polygon in each scenario, scale the polygon and
	// add it to the mapping...
	// iterate over scaling factors




	for(int k=0; k<numScalingFactors; ++k) {
		double scalingFactor = scalingFactors[k];

		int numScenarios = weatherScenarios.size();
		PolygonLists scaledWeatherScenarios;
		// iterate over the scenarios
		for(int i=0; i<numScenarios; ++i) {
			const PolygonSet* scenario = &(weatherScenarios.at(i));
			int numPolygons = scenario->size();

			// scale each polygon in the ith scenario
			PolygonSet scaledWeatherScenario;
			for(int j=0; j<numPolygons; ++j) {

				const Polygon* polygon = &(scenario->at(j));
				if(scalingFactor != 1) {
					Polygon convex;
					Polygon scaled;
					polygon->convexHull(&convex);
					convex.scale(scalingFactor, scalingFactor, &scaled);
					scaledWeatherScenario.push_back(scaled);
				}
				else {
					scaledWeatherScenario.push_back(*polygon);
				}
			}
			scaledWeatherScenarios.push_back(scaledWeatherScenario);

		}
		weatherScenariosList.insert(std::pair<double, PolygonLists>(
				scalingFactor, scaledWeatherScenarios));
	}
}

void getMaxConvexHulls(map<double, PolygonLists>& weatherScenariosList,
		               vector<Polygon>& maxConvexHulls,
		               const double& maxScaleFactor) {

	const PolygonLists* maxScaledWeatherScenarios =
			&(weatherScenariosList.at(maxScaleFactor));

	int numPolygonsPerScenario = maxScaledWeatherScenarios->front().size();

	for(int i=0; i<numPolygonsPerScenario; ++i) {

		vector<Polygon> polys;

		// obtain polygon i from each scenario at max scale level
		// and add to vector of polys to compute union for
		PolygonLists::const_iterator scenarioIter;
		for(scenarioIter=maxScaledWeatherScenarios->begin();
			scenarioIter!=maxScaledWeatherScenarios->end();
			++scenarioIter) {
			const PolygonSet* scenario = &(*scenarioIter);
			polys.push_back(scenario->at(i));
		}

		// compute the convex hull of union of polys
		vector<double> xconvex;
		vector<double> yconvex;
		polygonUnionConvexHull(polys, &xconvex, &yconvex);

		// construct the Polygon for the convex hull
		int numVertices = xconvex.size();
		Polygon p(xconvex, yconvex, numVertices);
		Polygon unionConvexHull;
		p.scale(1.2, 1.2, &unionConvexHull);
		maxConvexHulls.push_back(unionConvexHull);
	}
}


//PARIKSHIT ADDER FOR NATS.
void getUnionConvexHull(const PolygonSet& weatherScenariosList,
		               Polygon& unionConvexHull) {
	vector<Polygon> polys;
	// obtain polygon i from each scenario at max scale level
	// and add to vector of polys to compute union for
	for (size_t k =0;k < weatherScenariosList.size(); ++k){
		polys.push_back(weatherScenariosList.at(k));
	}

	// compute the convex hull of union of polys
	vector<double> xconvex;
	vector<double> yconvex;
	polygonUnionConvexHull(polys, &xconvex, &yconvex);

	// construct the Polygon for the convex hull
	int numVertices = xconvex.size();
	Polygon p(xconvex, yconvex, numVertices);
	p.scale(1.0, 1.0, &unionConvexHull);
}


bool routeIntersectsPoly(const Polygon* const poly,
		                 const vector<double>* const lats,
						 const vector<double>* const lons,
						 int& first_idx,
						 int& last_idx
						 ) {
	// test whether the path intersects any polygon in the set.  if so,
	// return true.  if path does not intersect any polygon return false.



	if (lats->size() != lons->size()){
		printf("lat and lon sizes are not same. \n");
		return false;
	}
	if (lats->size() < 2){
		return false;
	}

	first_idx = -1;
	last_idx = -1;

	bool intersects = false;
	for (size_t k=0;k< lats->size()-1; ++k){
		double lat1 = lats->at(k);
		double lat2 = lats->at(k+1);

		double lon1 = lons->at(k);
		double lon2 = lons->at(k+1);

		if(poly->contains(lon1, lat1, true)) {
			if (intersects == false && k > 0){
				first_idx = k;
			}
			intersects = true;
			continue;
		}
		if(poly->contains(lon2, lat2, true)) {
			if (intersects == false){
				first_idx = k+1;
			}
			intersects = true;
			continue;
		}
		if(poly->intersectsSegment(lon1,lat1,lon2,lat2,NULL,NULL,true)){
			if (intersects == false){
				first_idx = k;
			}
			intersects = true;
			continue;
		}

		if (intersects == true){
			last_idx = k;
			break;
		}
	}

	// if we get here, no intersections, return false.
	return intersects;
}

void getPathAroundPolygon(const Polygon& convexHull,
		const std::pair<double,double>& entry_lat_lon,
		const std::pair<double,double>& exit_lat_lon,
		vector<std::pair<double,double> > &reroute_lat_lon){
	reroute_lat_lon.clear();

	int numVertices = convexHull.getNumVertices();

	double* lonx_data = (double *)calloc(numVertices,sizeof(double));
	double* laty_data = (double *)calloc(numVertices,sizeof(double));


	for( int i=0;i<numVertices; ++i){
		lonx_data[i]=convexHull.getXData()[i];
		laty_data[i]=convexHull.getYData()[i];
	}


	//find nearest pt to entry;
	double min_dist = numeric_limits<double>::max();
	int idx = -1;

	double entry_laty = entry_lat_lon.first;
	double entry_lonx = entry_lat_lon.second;

	for(int i = 0;i<numVertices; ++i){
		double dist_gc = compute_distance_gc(entry_laty,entry_lonx,
				laty_data[i],lonx_data[i]);

		if (dist_gc < min_dist){
			min_dist = dist_gc;
			idx = i;
		}
	}

	int entry_idx = idx;

	//find nearest pt to exit;
	min_dist = numeric_limits<double>::max();
	idx = -1;

	double exit_laty = exit_lat_lon.first;
	double exit_lonx = exit_lat_lon.second;

	for(int i = 0;i<numVertices; ++i){
		double dist_gc = compute_distance_gc(exit_laty,exit_lonx,
				laty_data[i],lonx_data[i]);

		if (dist_gc < min_dist){
			min_dist = dist_gc;
			idx = i;
		}
	}
	int exit_idx = idx;

	//get path from entry to exit
	//first get ordered list of directions. Can be only 2 direction

	vector<int> order1;order1.clear(); order1.push_back(entry_idx);
	int order_len = 1;
	int idx_cnt = entry_idx;
	while (order_len<numVertices-1){
		if (idx_cnt == numVertices - 2){
			idx_cnt = 0;
		}
		else{
			idx_cnt ++;
		}
		order1.push_back(idx_cnt);
		if (idx_cnt == exit_idx)
			break;
		order_len++;
	}

	vector<int> order2;order2.clear(); order2.push_back(exit_idx);
	order_len = 1;
	idx_cnt = exit_idx;
	while (order_len<numVertices-1){
		if (idx_cnt == numVertices - 2){
			idx_cnt = 0;
		}
		else{
			idx_cnt ++;
		}
		order2.push_back(idx_cnt);
		if (idx_cnt == entry_idx)
			break;
		order_len++;
	}

	std::reverse(order2.begin(),order2.end());

	//then get the total distance

	double dist1 = 0;
	for (size_t k=0;k<order1.size()-1; ++k){
		size_t idx1 = (size_t)order1[k],idx2 = (size_t)order1[k+1];
		double d = compute_distance_gc(laty_data[idx1],lonx_data[idx1],
				laty_data[idx2],lonx_data[idx2]);
		dist1 = dist1 +d;
	}

	double dist2 = 0;
	for (size_t k=0;k<order2.size()-1; ++k){
		size_t idx1 = (size_t)order2[k],idx2 = (size_t)order2[k+1];
		double d = compute_distance_gc(laty_data[idx1],lonx_data[idx1],
				laty_data[idx2],lonx_data[idx2]);
		dist2 = dist2 +d;
	}

	vector<int> order; order.clear();
	if(dist2>dist1)
		order = order1;
	else
		order = order2;

	for (size_t k=0;k < order.size(); ++k){
		double lat1 = laty_data[order[k]];
		double lon1 = lonx_data[order[k]];
		pair<double,double> lat_lon_pair(lat1,lon1);

		reroute_lat_lon.push_back(lat_lon_pair);
	}

	free(lonx_data);
	free(laty_data);

	//check heading difference
	//Remove wps with heading difference of more than 120

	double lat0 = entry_lat_lon.first;
	double lon0 = entry_lat_lon.second;

	while(true){
		if (reroute_lat_lon.size() == 1){
			break;
		}
		double lat1 = reroute_lat_lon.at(0).first;
		double lon1 = reroute_lat_lon.at(0).second;

		double lat2 = reroute_lat_lon.at(1).first;
		double lon2 = reroute_lat_lon.at(1).second;

		double heading1 = compute_heading_gc(lat0,lon0,lat1,lon1);
		double heading2 = compute_heading_gc(lat1,lon1,lat2,lon2);

		double diff = abs(heading1-heading2);
		//accute angle
		if (diff > 120){
			reroute_lat_lon.erase(reroute_lat_lon.begin()+0);
		}
		else{
			break;
		}

	}

	double lat2 = exit_lat_lon.first;
	double lon2 = exit_lat_lon.second;

	while(true){
		if (reroute_lat_lon.size() == 1){
			break;
		}
		size_t sz = reroute_lat_lon.size()-1;
		double lat1 = reroute_lat_lon.at(sz).first;
		double lon1 = reroute_lat_lon.at(sz).second;

		double lat0 = reroute_lat_lon.at(sz-1).first;
		double lon0 = reroute_lat_lon.at(sz-1).second;

		double heading1 = compute_heading_gc(lat0,lon0,lat1,lon1);
		double heading2 = compute_heading_gc(lat1,lon1,lat2,lon2);

		double diff = abs(heading1-heading2);
		//accute angle
		if (diff > 120){
			reroute_lat_lon.erase(reroute_lat_lon.end()-1);
		}
		else{
			break;
		}

	}
}

int weatherAvoidanceRoutesForNATS(const PolygonSet& weatherScenariosList,
		const vector<double>& lats_deg,
		const vector<double>& lons_deg,
		vector<double>& lat_reroute_deg,
		vector<double>& lon_reroute_deg,
		vector<pair<int,int> >& similar_idx_wpts){

	//get union convex hull
	Polygon unionConvexHull;
	getUnionConvexHull(weatherScenariosList,unionConvexHull);

	int first_idx = -1,last_idx = -1;
	bool intersects = routeIntersectsPoly(&unionConvexHull,
			&lats_deg, &lons_deg, first_idx,last_idx);

	if (intersects && first_idx >= 0 && last_idx > 0 && last_idx < lats_deg.size()-1){

		double entry_lat = lats_deg.at(first_idx);
		double entry_lon = lons_deg.at(first_idx);

		double exit_lat = lats_deg.at(last_idx);
		double exit_lon = lons_deg.at(last_idx);

		std::pair<double,double> entry_lat_lon(entry_lat,entry_lon);
		std::pair<double,double> exit_lat_lon(exit_lat,exit_lon);
		vector<std::pair<double,double> > reroute_lat_lon;reroute_lat_lon.clear();
		getPathAroundPolygon(unionConvexHull,
				entry_lat_lon,
				exit_lat_lon,
				reroute_lat_lon);

		if (first_idx > 1){
			for (size_t s = 1; s<first_idx; ++s){
				lat_reroute_deg.push_back(lats_deg.at(s));
				lon_reroute_deg.push_back(lons_deg.at(s));
			}
		}

		int num_wp_rem = last_idx-first_idx-1;
		int num_wp_add = reroute_lat_lon.size();
		int diff = num_wp_add-num_wp_rem;

		for (int s = 0; s<first_idx+1;++s){
			std::pair<int,int> simidx(s,s);
			similar_idx_wpts.push_back(simidx);
		}
		for (int s=last_idx; s<lats_deg.size(); ++s){
			std::pair<int,int> simidx(s,s+diff);
			similar_idx_wpts.push_back(simidx);
		}


		for(size_t s = 0;s<reroute_lat_lon.size(); ++s){
			lat_reroute_deg.push_back(reroute_lat_lon.at(s).first);
			lon_reroute_deg.push_back(reroute_lat_lon.at(s).second);
		}

		if (last_idx < lats_deg.size() - 1 ){
			for (size_t s = last_idx; s<lats_deg.size(); ++s){
				lat_reroute_deg.push_back(lats_deg.at(s));
				lon_reroute_deg.push_back(lons_deg.at(s));
			}
		}
	}
	else{
		lat_reroute_deg = lats_deg;
		lon_reroute_deg = lons_deg;

		for (size_t k = 0; k< lats_deg.size(); ++k){
			std::pair<int,int> simpair(k,k);
			similar_idx_wpts.push_back(simpair);
		}
	}

	return 1;
}



void find_intersecting_polygons(const double& lat_deg, const double& lon_deg, const double& alt_ft, const double& nmi_rad,
				const vector<double>& fp_lat_deg,
				const vector<double>& fp_lon_deg,
				const PolygonSet& weatherpolygons,
				PolygonSet& polygons_ahead){

	assert( fp_lat_deg.size() == fp_lon_deg.size() );
	if (weatherpolygons.empty()) return;
	else if (fp_lat_deg.empty() || fp_lon_deg.empty()) return;

	vector<double> fp_lat_in_range;
	vector<double> fp_lon_in_range;

	double dist_ft = 0.0;
	int i=0;
	double curr_lat_deg = lat_deg, curr_lon_deg = lon_deg;
	double leg_dist_ft = 0;
	while( i < (int)fp_lat_deg.size() ){
		double next_lat_deg = fp_lat_deg.at(i);
		double next_lon_deg = fp_lon_deg.at(i);

		leg_dist_ft = compute_distance_gc(curr_lat_deg,curr_lon_deg,next_lat_deg, next_lon_deg, alt_ft);
		dist_ft +=  leg_dist_ft;

		if ( dist_ft < (nmi_rad*nauticalMilesToFeet) ){
			fp_lat_in_range.push_back(next_lat_deg); fp_lon_in_range.push_back(next_lon_deg);
		}
		else{
			break;
		}
		curr_lat_deg = next_lat_deg; curr_lon_deg = next_lon_deg;
		i++;
	}
	//Now if the last line is not fully within we need to consider part of it.
	dist_ft -=leg_dist_ft;
	double dist_left_ft = nmi_rad*(nmi_rad*nauticalMilesToFeet) - dist_ft;
	double heading_deg_gc = compute_heading_gc(fp_lat_deg[i],fp_lon_deg[i],curr_lat_deg,curr_lon_deg);

	double to_Lat = 0,to_Lon = 0;
	compute_location_gc(fp_lat_deg[i], fp_lon_deg[i], dist_left_ft, heading_deg_gc, &to_Lat, &to_Lon);

	fp_lat_in_range.push_back(to_Lat);
	fp_lon_in_range.push_back(to_Lon);

	//now push the intersecting polygons
	for (Polygon poly:weatherpolygons) {
		int first_idx = 0,last_idx = 0;
		bool intersects = routeIntersectsPoly(&poly, &fp_lat_in_range, &fp_lon_in_range, first_idx, last_idx);
		if (intersects) polygons_ahead.push_back(poly);
	}
}

//END PARIKSHIT ADDER FOR NATS.

void getPolygonEntryExitPoints(map<FixPair, SearchPath>& nominalPaths,
		                       vector<Polygon>& maxConvexHulls,
		                       vector<FixPair>& entryExitPairs,
                               map<SearchPath, FixPair>& nominalEntryFixes,
                               const bool& replanFlag) {

	// if replanning from start point then return the first and last
	// points of the nominal path.
	if(replanFlag) {
		map<FixPair, SearchPath>::iterator nomPathIter;
		for(nomPathIter=nominalPaths.begin();
				nomPathIter!=nominalPaths.end();
				++nomPathIter) {

			FixPair entryExitPair = (nomPathIter->first);
			string callsign = entryExitPair.callsign;
			double airspeed = entryExitPair.airspeed;
			double altitude = entryExitPair.altitude;

			SearchPath nominalPath = (nomPathIter->second);
			int end = nominalPath.getNodes().size()-1;
			int entryId = nominalPath.getNodes().at(0);
			int exitId = nominalPath.getNodes().at(end);
			Fix* entryFix = (entryId>=(int)rg_fixes.size() ? rg_airports_by_id[entryId-rg_fixes.size()] : rg_fixes_by_id.at(entryId));
			Fix* exitFix = (exitId>=(int)rg_fixes.size() ? rg_airports_by_id[exitId-rg_fixes.size()] : rg_fixes_by_id.at(exitId));
			if(entryFix && exitFix) {

				FixPair pair(entryFix->getName(), exitFix->getName());
				//TODO:Parikshit adder
				pair.callsign = callsign;pair.airspeed = airspeed; pair.altitude = altitude;
				//TODO:Parikshit adder ends
				entryExitPairs.push_back(pair);
				nominalEntryFixes.insert(
						std::pair<SearchPath, FixPair>(nominalPath, pair));
			}
		}
		return;
	}

	// count total number of polygon points
	int numPoints = 0;
	vector<Polygon>::iterator maxPolyIter;
	for(maxPolyIter=maxConvexHulls.begin();
			maxPolyIter!=maxConvexHulls.end();
			++maxPolyIter) {
		int n = maxPolyIter->getNumVertices();
		numPoints += n;
	}

	// create x,y arrays of points
	double* xdata = new double[numPoints];
	double* ydata = new double[numPoints];
	int index = 0;
	for(maxPolyIter=maxConvexHulls.begin();
			maxPolyIter!=maxConvexHulls.end();
			++maxPolyIter) {

		int n = maxPolyIter->getNumVertices();
		for(int i=0; i<n; ++i) {
			xdata[index] = maxPolyIter->getXData()[i];
			ydata[index] = maxPolyIter->getYData()[i];
			++index;
		}
	}

	// convex hull to get the super poly
	vector<double> xconvex;
	vector<double> yconvex;
	convex_hull_scan(numPoints, xdata, ydata, &xconvex, &yconvex);
	Polygon maxPoly(xconvex, yconvex, xconvex.size());

	map<FixPair, SearchPath>::iterator nomPathIter;
	for(nomPathIter=nominalPaths.begin();
			nomPathIter!=nominalPaths.end();
			++nomPathIter) {

		FixPair entryExitPair = (nomPathIter->first);
		string callsign = entryExitPair.callsign;
		double airspeed = entryExitPair.airspeed;
		double altitude = entryExitPair.altitude;

		SearchPath nominalPath = (nomPathIter->second);

		Fix* entryFix = NULL;
		Fix* exitFix = NULL;
		getEntryExitFixes(nominalPath, maxPoly, &entryFix, &exitFix);

		if(entryFix && exitFix) {
			FixPair pair(entryFix->getName(), exitFix->getName());
			//TODO:Parikshit adder
			pair.callsign = callsign;pair.airspeed = airspeed; pair.altitude = altitude;
			//TODO:Parikshit adder ends
			entryExitPairs.push_back(pair);

			// map the nominal path to this entry/exit pair
			nominalEntryFixes.insert(
					std::pair<SearchPath, FixPair>(nominalPath, pair));
		}
	}

	// free arrays
	delete [] xdata;
	delete [] ydata;
}

void getPolygonEntryExitPointsForTOS(map<FixPair, vector<SearchPath> >& nominalTrajs,
		                       vector<Polygon>& maxConvexHulls,
		                       vector<FixPair>& entryExitPairs,
                               map<SearchPath, FixPair>& nominalEntryFixes,
                               const bool& replanFlag,
							   const int& offset_begin = 0) {
	// if replanning from start point then return the first and last
	// points of the nominal path.
	if (replanFlag) {
		map<FixPair, vector<SearchPath> >::iterator nomPathIter;
		for (nomPathIter = nominalTrajs.begin();
				nomPathIter != nominalTrajs.end();
				++nomPathIter) {
			//TODO:PARIKSHIT ADDER
			FixPair entryExitPair = (nomPathIter->first);
			string callsign = entryExitPair.callsign;
			double airspeed = entryExitPair.airspeed;
			double altitude = entryExitPair.altitude;
			//TODO:PARIKSHIT ADDER ENDS

			vector<SearchPath> nominalTraj = (nomPathIter->second);

			for (unsigned int nt = 0; nt < nominalTraj.size(); ++nt) {
				int end = nominalTraj.at(nt).getNodes().size()-1;
				int entryId = nominalTraj.at(nt).getNodes().at(0);
				int exitId = nominalTraj.at(nt).getNodes().at(end);
				Fix* entryFix = (entryId>=(int)rg_fixes.size() ?
						rg_airports_by_id[entryId-rg_fixes.size()] : rg_fixes_by_id.at(entryId));
				Fix* exitFix = (exitId>=(int)rg_fixes.size() ?
						rg_airports_by_id[exitId-rg_fixes.size()] : rg_fixes_by_id.at(exitId));

				if (entryFix && exitFix) {

					FixPair pair(entryFix->getName(), exitFix->getName());
					//TODO:Parikshit adder
					pair.callsign = callsign;pair.airspeed = airspeed; pair.altitude = altitude;
					//TODO:Parikshit adder ends
					entryExitPairs.push_back(pair);
					nominalEntryFixes.insert(
							std::pair<SearchPath, FixPair>(nominalTraj.at(nt), pair));
				}
			}
		}

		return;
	}

	// count total number of polygon points
	int numPoints = 0;
	vector<Polygon>::iterator maxPolyIter;
	for (maxPolyIter = maxConvexHulls.begin();
			maxPolyIter != maxConvexHulls.end();
			++maxPolyIter) {
		int n = maxPolyIter->getNumVertices();
		numPoints += n;
	}

	// create x,y arrays of points
	double* xdata = new double[numPoints];
	double* ydata = new double[numPoints];
	int index = 0;
	for (maxPolyIter = maxConvexHulls.begin();
			maxPolyIter != maxConvexHulls.end();
			++maxPolyIter) {

		int n = maxPolyIter->getNumVertices();
		for (int i = 0; i < n; ++i) {
			xdata[index] = maxPolyIter->getXData()[i];
			ydata[index] = maxPolyIter->getYData()[i];
			++index;
		}
	}

	// convex hull to get the super poly
	vector<double> xconvex;
	vector<double> yconvex;
	convex_hull_scan(numPoints, xdata, ydata, &xconvex, &yconvex);
	Polygon maxPoly(xconvex, yconvex, xconvex.size());

	map<FixPair, vector<SearchPath> >::iterator nomPathIter;
	for (nomPathIter = nominalTrajs.begin();
			nomPathIter != nominalTrajs.end();
			++nomPathIter) {

		FixPair entryExitPair = (nomPathIter->first);
		string callsign = entryExitPair.callsign;
		double airspeed = entryExitPair.airspeed;
		double altitude = entryExitPair.altitude;

		vector<SearchPath> nominalTraj = (nomPathIter->second);

		for (unsigned int nt = 0; nt < nominalTraj.size(); ++nt) {
			Fix* entryFix = NULL;
			Fix* exitFix = NULL;
			getEntryExitFixes(nominalTraj.at(nt), maxPoly, &entryFix, &exitFix, offset_begin);

			if (entryFix && exitFix) {
				FixPair pair(entryFix->getName(), exitFix->getName());

				pair.callsign = callsign;
				pair.airspeed = airspeed;
				pair.altitude = altitude;

				entryExitPairs.push_back(pair);

				// map the nominal path to this entry/exit pair
				nominalEntryFixes.insert(
						std::pair<SearchPath, FixPair>(nominalTraj.at(nt), pair));
			}
		}
	}

	// free arrays
	delete [] xdata;
	delete [] ydata;
}

void logPath(const SearchPath* const p,
		     const int& scenario, const double scale) {

#if LOG_SCENARIO_PATHS

	stringstream fname;
	if(scenario == -1) {
		fname << "log/paths_nom.log";
	} else {
		fname << "log/paths_" << scenario << "_" << fixed << setprecision(1)
			<< scale << "x.log";
	}

	ofstream log;
	log.open(fname.str().c_str(), ios::app);
	log << fixed << setprecision(8);

	string orig, dest;
	get_fix_name(p->getNodes().at(0), &orig);
	get_fix_name(p->getNodes().at(p->getNodes().size()-1), &dest);

	for(unsigned int i=0; i<p->getNodes().size(); ++i) {
		int id = p->getNodes().at(i);
		double lat, lon;
		get_fix_location(id, &lat, &lon);
		log << setw(16) << lon << setw(16) << lat << setw(8)
			<< orig << setw(8) << dest << endl;
	}
	log << endl;

	log.close();
#else
	(void)p;
	(void)scenario;
	(void)scale;
#endif

}

void logTrajs(const vector<SearchPath>* const p,
		     const int& scenario, const double scale) {

#if LOG_SCENARIO_PATHS

	stringstream fname;
	if(scenario == -1) {
		fname << "log/paths_nom.log";
	} else {
		fname << "log/paths_" << scenario << "_" << fixed << setprecision(1)
			<< scale << "x.log";
	}

	ofstream log;
	log.open(fname.str().c_str(), ios::app);
	log << fixed << setprecision(8);

	string orig, dest;
	get_fix_name(p->getNodes().at(0), &orig);
	get_fix_name(p->getNodes().at(p->getNodes().size()-1), &dest);

	for(unsigned int i=0; i<p->getNodes().size(); ++i) {
		int id = p->getNodes().at(i);
		double lat, lon;
		get_fix_location(id, &lat, &lon);
		log << setw(16) << lon << setw(16) << lat << setw(8)
			<< orig << setw(8) << dest << endl;
	}
	
	log << endl;

	log.close();
#else
	(void)p;
	(void)scenario;
	(void)scale;
#endif
}

bool pathIntersectsPoly(const PolygonSet* const polys,
		                 const SearchPath* const p,
						 const SearchGraph* const g = NULL,
						 bool useAirmetCriteria = true) {

	// test whether the path intersects any polygon in the set.  if so,
	// return true.  if path does not intersect any polygon return false.


	double cost = 0.0;
	for(unsigned int i=1; i<p->getNodes().size(); ++i) {
		int id1 = p->getNodes().at(i-1);
		int id2 = p->getNodes().at(i);
		double lat1, lon1, lat2, lon2;
		get_fix_location(id1, &lat1, &lon1);
		get_fix_location(id2, &lat2, &lon2);
		if (g != NULL)
			cost = cost + g->getEdgeCost(id1,id2,cost);
		for(unsigned int j=0; j<polys->size(); ++j) {
			const Polygon* poly = &(polys->at(j));
			const string met_type = poly->getPolyType();
			if (g != NULL){
				const int st_hr = poly->getStartHour();
				const int end_hr = poly->getEndHour();
				if (st_hr*1.0> cost/3600.00 || end_hr*1.0 < cost/3600.00) continue;
			}

			size_t found = met_type.find("AIRMET");
			if (found != string::npos && useAirmetCriteria) continue;
			if(poly->contains(lon1, lat1, true)) {
				return true;
			}
			if(poly->contains(lon2, lat2, true)) {
				return true;
			}
			if(poly->intersectsSegment(lon1,lat1,lon2,lat2,NULL,NULL,true))
				return true;
		}
	}

	// if we get here, no intersections, return false.
	return false;
}

/**
 * This function will iterate over each scenario and each scale in each
 * scenario and compute the reduced search graph (remove fixes in polygons)
 * and then call get_paths (which will call find_shortest_paths(), which
 * will invoke A*) for each start/end fix pair.  The resulting path will
 * be post processed and added to the output set.  The post processing
 * will append the beginning portion of the route starting from the origin
 * airport (in the case of re-route paths) and remove any back tracking
 * segments.
 */
#if !USE_LIBSEARCH
static void findReroutePaths(map<double, PolygonLists>& weatherScenariosList,
		              map<SearchPath, FixPair>& nominalEntryFixes,
		              SearchGraph& graph,
		              ResultSets* const results) {

	vector<double> emptyFactors;
	PolygonSet emptyPolys;
	map<double, PolygonLists>::iterator scaledScenarioIter;
	for (scaledScenarioIter = weatherScenariosList.begin();
			scaledScenarioIter != weatherScenariosList.end();
			++scaledScenarioIter) {

		double scale = scaledScenarioIter->first;
		const PolygonLists* scenarioPolys = &(scaledScenarioIter->second);

		// compute the A* shortest paths between each entry/exit node
		// pair for the given polygons.  we don't supply scaling
		// factors to the function since they have already been scaled.
		PolygonLists::const_iterator scenarioPolysIter;
		for(scenarioPolysIter=scenarioPolys->begin();
				scenarioPolysIter!=scenarioPolys->end();
				++scenarioPolysIter) {

			int scenario = scenarioPolysIter-scenarioPolys->begin();

			cout << "  Processing: scenario=" << scenario
				 << " scale-level=" << scale << endl;

			const PolygonSet* polys = &(*scenarioPolysIter);
			map<SearchPath, FixPair>::iterator npit;

			// compute the reduced graph for the current scenario
			// scale combination.
			SearchGraph reducedGraph(graph);
			reduce_airway_connectivity(reducedGraph, *polys);

			for(npit=nominalEntryFixes.begin();
					npit!=nominalEntryFixes.end();
					++npit) {

				SearchPath nominalPath = npit->first;
				FixPair entryExitPair = npit->second;
				string entry = entryExitPair.origin;
				string exit = entryExitPair.destination;
				//string origin = g_fix_names[nominalPath.getNodes().at(0)];

				int firstNodeId = nominalPath.getNodes().at(0);
				int originId = firstNodeId - rg_fix_names.size();
				string origin;

				// TODO: Cleanup this code block!!!!
				if(originId < 0) {

				    originId = firstNodeId;

				    if(originId >= (int)rg_fix_names.size()) {
				        cout << "  ERROR: invalid fixId " << originId << endl;
				    }

                    map<int,string>::iterator apit = rg_fix_names.find(originId);
                    if(apit == rg_fix_names.end()) {
                        // try the second node. perhaps the airport doesn't
                        // exist in the nfd dataset.
                        int secondNodeId = nominalPath.getNodes().at(1);
                        originId = secondNodeId - rg_fix_names.size();

                        if(originId < 0) {
                            originId = secondNodeId;
                            apit = rg_fix_names.find(originId);
                            if(apit == rg_fix_names.end()) {
                                cout << "  ERROR: could not find fix id " << originId << " of " << rg_fix_names.size() << endl;
                            }

                            origin = rg_fix_names.at(originId);
                        } else {

                            if(originId >= (int)rg_airport_names.size()) {
                                cout << "  ERROR: invalid originId " << originId << endl;
                            }

                            origin = rg_airport_names.at(originId);
                        }

                    } else {

                        origin = rg_fix_names.at(originId);
                    }

				} else {
                    origin = rg_airport_names.at(originId);
				}

				string destination = exit;

#ifndef NDEBUG
				cout << "  " << origin << " " << entry
					 << " " << destination
					 << " " << scenario << " " << scale << endl;
#endif

				// check to see if the nominal path intersects the polygons
				// at the current scenario and scale level.  if not,
				// then there is no need to do reroutes, just add the
				// nominal path to the results and continue to the next path.
				bool intersects = pathIntersectsPoly(polys, &nominalPath);
				if(!intersects) {
					FixPair newPairKey(origin, destination);
					ResultSetKey key(scenario, scale);
					ResultSets::iterator iter = results->find(key);
					if(iter == results->end()) {
						ResultSet result;
						result[newPairKey] = nominalPath;
						results->insert(std::pair<ResultSetKey, ResultSet>(
								        key, result));
					} else {
						results->at(key).insert(std::pair<FixPair,SearchPath>(newPairKey, nominalPath));
					}
					logPath(&nominalPath, scenario, scale);

					continue;
				}

				vector< map<FixPair, SearchPath> > tmpPaths;
				map< double, map<FixPair, SearchPath> > pathsOut;

				set<FixPair> pairs;
				pairs.insert(FixPair(entry, exit));
				get_paths(reducedGraph, pairs, &tmpPaths);

				for(unsigned int i=0; i<tmpPaths.size(); ++i) {
					map<FixPair, SearchPath>* m = &(tmpPaths.at(i));
					map<FixPair, SearchPath>::iterator it;

					// we need to modify the map key so that the
					// origin is the original input parameter
					// origin instead of the reroute entry point.
					// we also need to append the beginning portion
					// of the nominal route to the reroute path
					// up to the reroute entry point.
					for(it=m->begin(); it!=m->end(); ++it) {
						SearchPath* p = &(it->second);
						processReroutedPath(nominalPath, *p);

						// verify that the path ends with the correct
						// destination.  if the search failed then the path
						// might end with the polygon entry point
						int lastId = p->getNodes().at(p->getNodes().size()-1);
						//int destId = g_fix_ids[destination];
						int destId = rg_fix_ids.size()+rg_airport_ids[destination];
						if(lastId != destId) {
							string lastName = rg_fix_names[lastId];
							stringstream ss;
							ss << "ERROR: reroute path does not end with"
									" destination fix (origin=" << origin <<
									", dest=" << destination  << " (" << destId << "), last=" <<
									lastName << " (" << lastId << "), scenario=" << scenario <<
									", scale=" << scale << ").";
							log_error(ss.str());
							continue;
						}

						// verify that the path does not intersect any
						// of the polygons at the current scenario scale.
						// if it does, then the search failed and we need
						// to log an error.
						bool intersects = pathIntersectsPoly(polys, p);
						if(intersects) {
							stringstream ss;
							ss << "ERROR: reroute path intersects polygon"
									" (origin=" << origin << ", dest=" <<
									destination << ", scenario=" << scenario <<
									", scale=" << scale << ").";
							log_error(ss.str());
							continue;
						}

						FixPair newPairKey(origin, destination);
						ResultSetKey key(scenario, scale);
						ResultSets::iterator iter = results->find(key);
						if(iter == results->end()) {
							ResultSet result;
							result[newPairKey] = *p;
							results->insert(std::pair<ResultSetKey, ResultSet>(
									        key, result));
						} else {
							results->at(key).insert(std::pair<FixPair,
									SearchPath>(newPairKey, *p));
						}
						logPath(p, scenario, scale);
					}
				}
			}
		}
	}
}

/**
 * TODO: Parikshit Adder
 * This function is the same implementation as find_wind_optimal_paths() for
 * finding Trajectory Option Sets. Default num of trajectories
 * is set to 5.
 */


static void find_wind_optimal_TOS(SearchGraph& graph,
				  vector<FixPair>& flights,
				  map<FixPair, vector<SearchPath> >& Tout,
				  SearchGraph& tmpgraph,
				  WindGrid* const g_wind = NULL,
				  vector<WindGrid>* const g_wind_vec = NULL,
				  const int& num_traj = 5,
				  tos_gen_method t_method = REM_LOW_COST,
				  const FixedProc& fproc = DEFAULT_FIXED_PROC,
				  const int& offset = 1) {
	vector<map<FixPair, SearchPath>* >* TOSout = new vector<map<FixPair, SearchPath>* >();

	int offset_begin = offset, offset_end = 1;

	if (fproc.procType == "SID")
		offset_begin++;
	else if (fproc.procType == "STAR")
		offset_end++;

	bool costFlag = false;
	for (int sp = 0; sp < num_traj; ++sp) {
		map<FixPair, SearchPath>* pathsOut = new map<FixPair, SearchPath>();
		int count = 0;
		int total = flights.size();

		omp_init_lock(&paths_lock);
		// we don't use omp parallel because we would have to to make copies
		// of the search graph for each thread. this kills any performance
		// benefits from multi-threading.
		//pragma omp parallel for schedule(static, 128)

		for (unsigned int i = 0; i < flights.size(); ++i) {
			// find wind optimal path
			find_lowest_cost_path(graph,
					flights.at(i),
					wind_optimal_cost_function,
					wind_optimal_heuristic_function,
					pathsOut,
					g_wind,
					g_wind_vec,
					costFlag);
		}
		omp_destroy_lock(&paths_lock);

		/*
		 * THIS PIECE OF CODE FINDS THE MAX INITIAL COST
		 * WITHOUT ANY WEATHER CONSTRAINTS
		 * NOT PARTICULARLY NEEDED FOR ANY APPLICATION BUT JUST
		 * FOR INFORMATION CAN BE USEFUL. KEEP IN MIND THE AIRSPEED AND
		 * ALTITUDE OF LAST PAIR OF AIRPORTS USED HERE.
		 */

		if (sp == 0) {
			tmpgraph = graph;

			g_max_cost = MIN_DOUBLE;
			map< std::pair<int, int>, double >::iterator edgeIter;
			for (edgeIter = graph.edgeCostsBegin();
					edgeIter != graph.edgeCostsEnd();
					++edgeIter) {
				const std::pair<int, int>* key= &(edgeIter->first);
				int fromId = key->first;
				int toId = key->second;
				double p_t = graph.getEdgeCost(fromId, toId);
				if (p_t > g_max_cost)
					g_max_cost = p_t;
			}
		}
		/*
		 * MAX COST CALCULATION TILL HERE.
		 */

		TOSout->push_back(pathsOut);

		map<FixPair, SearchPath>::iterator pIter;

		/*TODO:PARIKSHIT ADDER COST CALCULATION. JUST FOR REPORTING NO OTHER USE.
		 * FIXME:CAN BE REMOVED IF YOU WANT*/
		for (pIter = pathsOut->begin(); pIter != pathsOut->end(); ++pIter) {
			SearchPath* p = &(pIter->second);
			const FixPair* f = &(pIter->first);
			deque<int> t_nodes = p->getNodes();
			double p_cost = 0;

			if (t_nodes.size() > 0) {
				for (unsigned int ns = 0;
						ns < t_nodes.size()-1;
						++ns) {
					double g_c = 0;
					if (!g_wind_vec->empty())
						g_c = graph.getEdgeCost(t_nodes[ns],t_nodes[ns+1],p_cost);
					else
						g_c = graph.getEdgeCost(t_nodes[ns],t_nodes[ns+1]);
					p_cost = p_cost + g_c;
				}
			}
			p->setPathCost(p_cost);
		} // end - for
		//TODO:PARIKSHIT ADDER COST CALCULATION ENDS

		for (pIter = pathsOut->begin(); pIter != pathsOut->end(); ++pIter) {
			int minfID = 0;
			int mintID = 1;
			double mincost = numeric_limits<double>::max();
			const SearchPath* p = &(pIter->second);
			const FixPair* f = &(pIter->first);
			deque<int> t_nodes = p->getNodes();

			double p_cost = 0;
			double s_cost = 0;

			// DO NOT CONSIDER SOURCE AND SINKS
			if (t_nodes.size() > 0) {
				for (unsigned int idx = offset_begin; idx < t_nodes.size()-offset_end; ++idx) {
					int fID = t_nodes.at(idx);
					int tID = t_nodes.at(idx+1);
					double t_cost = 0;
					if (!g_wind_vec->empty())
						t_cost = graph.getEdgeCost(fID,tID,p_cost);
					else
						t_cost = graph.getEdgeCost(fID,tID);
					p_cost += t_cost;

	//				ONE WAY TO REMOVE ENTIRE MIN PATH EXCEPT THE FIRST LINK.
					if (t_method == REM_ENTIRE_PATH) {
						graph.setEdgeCost(fID,tID,numeric_limits<double>::max());

						if (!g_wind_vec->empty()) {
							vector<double> eCV = graph.getEdgeCostVector(fID,tID);
							size_t idx = (size_t)( floor(p_cost/3600) )% ( eCV.size() );
							eCV.at(idx) = numeric_limits<double>::max();
							graph.setEdgeCostVector(fID,tID,eCV);
						}

						continue;
					}

// 					Remove Min cost per unit length
					if (t_method == REM_LOW_COST_PER_LENGTH) {
						Fix* f_fix = NULL; get_fix(fID,&f_fix);
						Fix* t_fix = NULL; get_fix(tID,&t_fix);

						double lat1 = f_fix->getLatitude();
						double lat2 = t_fix->getLatitude();
						double lon1 = f_fix->getLongitude();
						double lon2 = t_fix->getLongitude();

						double alt = f->altitude;
						double dval = compute_distance_gc(lat1, lon1, lat2, lon2, alt);

						t_cost = t_cost/dval;
					}

// 				ENDS HERE

					if (t_cost < mincost) {
						mincost = t_cost;
						s_cost = p_cost;
						minfID = fID;
						mintID = tID;
					}
#ifndef NDEBUG
					cout << "fid = " << fID <<" tID = " << tID << " cost = " << t_cost << endl;
#endif
				}
			}
#ifndef NDEBUG
			cout << " origin " << f->origin << " destination " << f->destination << endl;
			cout << " min from = " << minfID << " min to = " << mintID << " cost = " << mincost << endl;
#endif
			if ( t_method != REM_ENTIRE_PATH ) {

				graph.setEdgeCost(minfID, mintID, numeric_limits<double>::max());

				const unsigned int sz = graph.getEdgeCostVectorSize(minfID, mintID);
				vector<double> eCV(sz, numeric_limits<double>::max());
				if (!g_wind_vec->empty()) {
					vector<double> eCV = graph.getEdgeCostVector(minfID, mintID);
					size_t idx = (size_t)( floor(s_cost/3600) ) % ( eCV.size() );
					eCV.at(idx) = numeric_limits<double>::max();
					graph.setEdgeCostVector(minfID, mintID, eCV);
				}
			}
		} // end - for
	}

	// Now put it in vector of Results
	map<FixPair, SearchPath>::iterator pIt;

	for (unsigned int ts = 0; ts < TOSout->size(); ++ts) {
		for (pIt = TOSout->at(ts)->begin(); pIt != TOSout->at(ts)->end(); ++pIt) {
			FixPair fp = pIt->first;
			SearchPath sp = pIt->second;
			Tout[fp].push_back(sp);

			SearchPathIterator it;
			for (it = sp.begin(); it != sp.end(); ++it) {
				int id = (*it);

				Fix* fix = NULL;
				if (id >= (int)rg_fixes.size()) {
					if (rg_airports_by_id.find(id-rg_fixes.size()) != rg_airports_by_id.end())
						fix = rg_airports_by_id.at(id-rg_fixes.size());
				} else {
					if (rg_fixes_by_id.find(id) != rg_fixes_by_id.end())
						fix = rg_fixes_by_id.at(id);
				}
			}
		}
	}

	for (unsigned int ts = 0; ts < TOSout->size();++ts) {
		if (TOSout->at(ts)) delete TOSout->at(ts);
	}

	if (TOSout) delete TOSout;
}
/*
 * TODO:Parikshit Adder Ends
 */

static void findWindOptimalReroutePaths(map<double, PolygonLists>& weatherScenariosList,
        map<SearchPath, FixPair>& nominalEntryFixes,
        SearchGraph& graph,
        ResultSets* const results,
        WindGrid* const g_wind,
        vector<WindGrid>* const g_wind_vec = NULL
        ) {

	vector<double> emptyFactors;
	PolygonSet emptyPolys;
	map<double, PolygonLists>::iterator scaledScenarioIter;
	for(scaledScenarioIter=weatherScenariosList.begin();
			scaledScenarioIter!=weatherScenariosList.end();
			++scaledScenarioIter) {

		double scale = scaledScenarioIter->first;
		const PolygonLists* scenarioPolys = &(scaledScenarioIter->second);

		// compute the A* shortest paths between each entry/exit node
		// pair for the given polygons.  we don't supply scaling
		// factors to the function since they have already been scaled.
		PolygonLists::const_iterator scenarioPolysIter;
		for (scenarioPolysIter=scenarioPolys->begin();
				scenarioPolysIter!=scenarioPolys->end();
				++scenarioPolysIter) {

			int scenario = scenarioPolysIter-scenarioPolys->begin();

			const PolygonSet* polys = &(*scenarioPolysIter);
			map<SearchPath, FixPair>::iterator npit;

			// compute the reduced graph for the current scenario
			// scale combination.
			SearchGraph reducedGraph(graph);
			reduce_airway_connectivity(reducedGraph, *polys);


			for (npit=nominalEntryFixes.begin();
					npit!=nominalEntryFixes.end();
					++npit) {

				SearchPath nominalPath = npit->first;
				FixPair entryExitPair = npit->second;

				string callsign = entryExitPair.callsign;
				double airspeed = entryExitPair.airspeed;
				double altitude = entryExitPair.altitude;

				string entry = entryExitPair.origin;
				string exit = entryExitPair.destination;
				string origin = rg_fix_names[nominalPath.getNodes().at(0)];
				string destination = exit;

#ifndef NDEBUG
				cout << "RouteGenerator: Processing"
					 << "  " << origin << " " << entry
					 << " " << destination << endl;
#endif

				// check to see if the nominal path intersects the polygons
				// at the current scenario and scale level.  if not,
				// then there is no need to do reroutes, just add the
				// nominal path to the results and continue to the next path.
				bool intersects = pathIntersectsPoly(polys, &nominalPath);
				if (!intersects) {
					FixPair newPairKey(origin, destination);
					ResultSetKey key(scenario, scale);
					ResultSets::iterator iter = results->find(key);
					if(iter == results->end()) {
						ResultSet result;
						result[newPairKey] = nominalPath;
						results->insert(std::pair<ResultSetKey, ResultSet>(
								        key, result));
					} else {
						results->at(key).insert(std::pair<FixPair,SearchPath>(newPairKey, nominalPath));
					}
					logPath(&nominalPath, scenario, scale);
					continue;
				}

				vector< map<FixPair, SearchPath> > tmpPaths;
				map< double, map<FixPair, SearchPath> > pathsOut;

				vector<FixPair> pairs;

				FixPair pair(entry, exit);
				pair.callsign = callsign; pair.airspeed = airspeed; pair.altitude = altitude;

				pairs.push_back(pair);
				get_wind_optimal_paths(reducedGraph, pairs, &tmpPaths, g_wind, g_wind_vec);



				for(unsigned int i=0; i<tmpPaths.size(); ++i) {
					map<FixPair, SearchPath>* m = &(tmpPaths.at(i));
					map<FixPair, SearchPath>::iterator it;


					// we need to modify the map key so that the
					// origin is the original input parameter
					// origin instead of the reroute entry point.
					// we also need to append the beginning portion
					// of the nominal route to the reroute path
					// up to the reroute entry point.
					for(it=m->begin(); it!=m->end(); ++it) {
						SearchPath* p = &(it->second);

						processReroutedPath(nominalPath, *p);

						// verify that the path ends with the correct
						// destination.  if the search failed then the path
						// might end with the polygon entry point
						int lastId = p->getNodes().at(p->getNodes().size()-1);

						int destId = rg_fix_ids.size()+rg_airport_ids[destination];

						if(lastId != destId) {
							string lastName = rg_fix_names[lastId];
							stringstream ss;
							ss << "ERROR: reroute path does not end with"
									" destination fix (origin=" << origin <<
									", dest=" << destination << ", last=" <<
									lastName << ", scenario=" << scenario <<
									", scale=" << scale << ").";
							log_error(ss.str(),true);
							continue;
						}

						// verify that the path does not intersect any
						// of the polygons at the current scenario scale.
						// if it does, then the search failed and we need
						// to log an error.
						bool intersects = pathIntersectsPoly(polys, p);
						if(intersects) {
							stringstream ss;
							ss << "ERROR: reroute path intersects polygon"
									" (origin=" << origin << ", dest=" <<
									destination << ", scenario=" << scenario <<
									", scale=" << scale << ").";
							log_error(ss.str(),true);
							continue;
						}

						FixPair newPairKey(origin, destination);

						newPairKey.callsign = callsign; newPairKey.airspeed = airspeed;
						newPairKey.altitude = altitude;

						ResultSetKey key(scenario, scale);
						ResultSets::iterator iter = results->find(key);
						if(iter == results->end()) {
							ResultSet result;
							result[newPairKey] = *p;
							results->insert(std::pair<ResultSetKey, ResultSet>(
									        key, result));
						} else {
							results->at(key).insert(std::pair<FixPair,
									SearchPath>(newPairKey, *p));
						}
						logPath(p, scenario, scale);
					}
				}
			}
		}
	}
}

/*
 * TODO:PARIKSHIT ADDER FOR TRAJECTORY OPTION SETS
 */

static void findWindOptimalReroutePathsForTOS(map<double, PolygonLists>& weatherScenariosList,
        map<SearchPath, FixPair>& nominalEntryFixes,
        SearchGraph& graph,
        ResultTrajs* const results,
        WindGrid* const g_wind,
        vector<WindGrid>* g_wind_vec,
        tos_gen_method t_method = REM_LOW_COST,
		const FixedProc& fproc = DEFAULT_FIXED_PROC,
		bool no_ground_hold = false,
		const int& offset = 1
        ) {
	//Adder for fixing procedure
	int offset_begin = offset, offset_end = 1;

	if (fproc.procType == "SID")
		offset_begin++;
	else if (fproc.procType == "STAR")
		offset_end++;

	vector<double> emptyFactors;
	PolygonSet emptyPolys;
	map<double, PolygonLists>::iterator scaledScenarioIter;
	for (scaledScenarioIter = weatherScenariosList.begin();
			scaledScenarioIter != weatherScenariosList.end();
			++scaledScenarioIter) {
		double scale = scaledScenarioIter->first;
		const PolygonLists* scenarioPolys = &(scaledScenarioIter->second);

		// compute the A* shortest paths between each entry/exit node
		// pair for the given polygons.  we don't supply scaling
		// factors to the function since they have already been scaled.
		PolygonLists::const_iterator scenarioPolysIter;
		for (scenarioPolysIter = scenarioPolys->begin();
				scenarioPolysIter != scenarioPolys->end();
				++scenarioPolysIter) {
			int scenario = scenarioPolysIter-scenarioPolys->begin();

			const PolygonSet* polys = &(*scenarioPolysIter);
			map<SearchPath, FixPair>::iterator npit;

			// compute the reduced graph for the current scenario
			// scale combination.

			SearchGraph reducedGraph(graph);

			bool costFlag = false;
			for (npit = nominalEntryFixes.begin();
					npit != nominalEntryFixes.end();
					++npit) {
				SearchPath nominalPath = npit->first;
				FixPair entryExitPair = npit->second;

				string callsign = entryExitPair.callsign;
				double airspeed = entryExitPair.airspeed;
				double altitude = entryExitPair.altitude;

				string entry = entryExitPair.origin;
				string exit = entryExitPair.destination;
				string origin;
				if ((nominalPath.getNodes().size() > 0) && (rg_fix_names.find(nominalPath.getNodes().at(0)) != rg_fix_names.end()))
					origin = rg_fix_names[nominalPath.getNodes().at(0)];
				string destination = exit;

				FixPair newPairKey(entry, destination);
				newPairKey.callsign = callsign;
				newPairKey.airspeed = airspeed;
				newPairKey.altitude = altitude;

#ifndef NDEBUG
				cout << "  " << origin << " " << entry
					 << " " << destination
					 << " " << scenario << " " << scale << endl;
#endif

				vector< map<FixPair, SearchPath> > tmpPaths;

				vector<FixPair> pairs;

				FixPair pair(entry, exit);
				pair.callsign = callsign;
				pair.airspeed = airspeed;
				pair.altitude = altitude;

				pairs.push_back(pair);

				//THIS STAYS BECAUSE TRAJECTORY OPTIONS HAS ALREADY BEEN TAKEN CARE OF
				//IN THE nominalEntryFixes THINGY. JUST NEED TO RUN CASE BY CASE TOS FOR REDUCED
				//GRAPH.
				get_wind_optimal_paths(reducedGraph,
						pairs,
						&tmpPaths,
						g_wind,
						g_wind_vec,
						costFlag);

				double p_cost = 0;
				int gnd_hold = compareRerouteWithNominal(reducedGraph,
						&pair,
						&nominalPath,
						&tmpPaths,
						p_cost);
#ifndef NDEBUG
				cout<<"GROUND HOLD = " << gnd_hold << " cost = "
					<<floor(p_cost/3600)<<":"<<60 * ( p_cost/3600 - floor(p_cost/3600)  )<<endl;
#endif
				for (unsigned int i = 0; i < tmpPaths.size(); ++i) {
					map<FixPair, SearchPath>* m = &(tmpPaths.at(i));
					map<FixPair, SearchPath>::iterator it;

					// we need to modify the map key so that the
					// origin is the original input parameter
					// origin instead of the reroute entry point.
					// we also need to append the beginning portion
					// of the nominal route to the reroute path
					// up to the reroute entry point.
					for (it = m->begin(); it != m->end(); ++it) {
						SearchPath* p = &(it->second);
						const FixPair *fp = &(it->first);
						processReroutedPath(nominalPath, *p);
						deque<int> *nodeseq = const_cast<deque<int>* >( &(p->getNodes()) );

						// verify that the path ends with the correct
						// destination.  if the search failed then the path
						// might end with the polygon entry point
						int lastId;
						if (p->getNodes().size() > 0)
							lastId = p->getNodes().at(p->getNodes().size()-1);

						int destId = rg_fix_ids.size() + rg_airport_ids[destination];

						if (lastId != destId) {
							string lastName = rg_fix_names[lastId];
							stringstream ss;
							ss << "ERROR: reroute path does not end with"
									" destination fix (origin=" << origin <<
									", dest=" << destination << ", last=" <<
									lastName << ", scenario=" << scenario <<
									", scale=" << scale << ").";
							log_error(ss.str(),true);

							continue;
						}

						// verify that the path does not intersect any
						// of the polygons at the current scenario scale.
						// if it does, then the search failed and we need
						// to log an error.
						bool intersects = pathIntersectsPoly(polys, p);
						if (intersects && gnd_hold == 0 && p_cost > 1e10) {
							stringstream ss;
							ss << "ERROR: reroute path intersects polygon"
									" (origin=" << origin << ", dest=" <<
									destination << ", scenario=" << scenario <<
									", scale=" << scale << ").";
							log_error(ss.str(),true);

							continue;
						}

						ResultSetKey key(scenario, scale);
						ResultTrajs::iterator iter = results->find(key);
						if (iter == results->end()) {
							ResultTraj result;
							result[newPairKey].push_back(*p);

							results->insert(std::pair<ResultSetKey, ResultTraj>(
									        key, result));
						} else {
							ResultTraj::iterator itv = results->at(key).find(newPairKey);
							if (itv == results->at(key).end()){
								vector<SearchPath> sp;sp.push_back(*p);

								results->at(key).insert(std::pair<FixPair,vector<SearchPath> >(newPairKey,sp) );
							}
							else {
								results->at(key).at(newPairKey).push_back(*p);
							}
						}
						logPath(p, scenario, scale);

						//REMOVING TOS PATH
						double mincost = numeric_limits<double>::max();
						int minfID = 0, mintID = 1;
						double p_cost = 0.0;
						double s_cost = 0.0;
						for (unsigned int idx = offset_begin;idx<nodeseq->size()-offset_end;++idx) {
							int fID = nodeseq->at(idx);
							int tID = nodeseq->at(idx+1);
							double t_cost = 0.0;
							if (!g_wind_vec->empty())
								t_cost = reducedGraph.getEdgeCost(fID,tID,p_cost);
							else
								t_cost = reducedGraph.getEdgeCost(fID,tID);
							p_cost += t_cost;
			//				ONE WAY TO REMOVE ENTRY MIN PATH EXCEPT THE FIRST LINK.
							if (t_method == REM_ENTIRE_PATH) {
								reducedGraph.setEdgeCost(fID,tID,numeric_limits<double>::max());
								if (no_ground_hold) {
									const unsigned int sz = reducedGraph.getEdgeCostVectorSize(fID,tID);
									vector<double> eCV(sz,numeric_limits<double>::max());
									reducedGraph.setEdgeCostVector(fID,tID,eCV);
								}
								else if (!g_wind_vec->empty()) {
									vector<double> eCV = graph.getEdgeCostVector(fID,tID);
									size_t idx = (size_t)( floor(p_cost/3600) )% ( eCV.size() );
									eCV.at(idx) = numeric_limits<double>::max();

									reducedGraph.setEdgeCostVector(fID,tID,eCV);

								}
								continue;
							}
			// 			Remove Min cost per unit length
							if (t_method == REM_LOW_COST_PER_LENGTH) {
								Fix* f_fix = NULL; get_fix(fID,&f_fix);
								Fix* t_fix = NULL; get_fix(tID,&t_fix);
								double lat1 = f_fix->getLatitude();double lat2 = t_fix->getLatitude();
								double lon1 = f_fix->getLongitude();double lon2 = t_fix->getLongitude();
								double alt = fp->altitude;
								double dval = compute_distance_gc(lat1, lon1, lat2, lon2, alt);
								t_cost = t_cost/dval;
							}
			// 				ENDS HERE
							if (t_cost < mincost) {
								mincost = t_cost;
								s_cost = p_cost;
								minfID = fID;
								mintID = tID;
							}
						}

						if ( t_method != REM_ENTIRE_PATH ) {
							reducedGraph.setEdgeCost(minfID,mintID,numeric_limits<double>::max());
							if (no_ground_hold) {
								const unsigned int sz = reducedGraph.getEdgeCostVectorSize(minfID,mintID);
								vector<double> eCV(sz,numeric_limits<double>::max());

								reducedGraph.setEdgeCostVector(minfID,mintID,eCV);
							}
							else if (!g_wind_vec->empty()) {
								vector<double> eCV = graph.getEdgeCostVector(minfID,mintID);
								size_t idx = (size_t)( floor(s_cost/3600) )% ( eCV.size() );
								eCV.at(idx) = numeric_limits<double>::max();

								reducedGraph.setEdgeCostVector(minfID,mintID,eCV);
							}
						}
						//REMOVING TOS PATH ENDS HERE
					}
				}
			}
		}
	}
}

/*
 * END PARIKSHIT ADDER FOR TRAJECTORY OPTIONS
 */
#else

#endif



#if !USE_LIBSEARCH
int get_paths(SearchGraph& graph, const char* trxfile,
		      const PolygonLists& weatherScenarios,
		      const double scalingFactors[], const int& numFactors,
		      ResultSets* const results, PolygonSets* const polysOut,
		      const bool& replanFlag) {

	// parse the trx file for origin-destination pairs
	// make sure we have valid output pointer
	if(!results) {
		log_error("get_paths(): invalid output pointer.", true);
		return -1;
	}

	// make sure the airway data has been loaded
	if(validate_global_data() < 0) {
		return -1;
	}

	set<FixPair> airportPairs;
	parse_trx_airport_pairs(trxfile, &airportPairs);

	// compute the scaled convex polygons for each weather scenario
	map<double, PolygonLists> weatherScenariosList;
	getScaledWeatherPolygons(weatherScenarios, scalingFactors, numFactors,
			                 weatherScenariosList);

	// compute the convex hull of the union of the largest scaled polygons
	// across all weather scenarios. for each scenario PolygonSet in the scale
	// level, iterate over polygons and compute the convex hull
	vector<Polygon> maxConvexHulls;
	if(numFactors > 0) {
		double maxScaleFactor = scalingFactors[numFactors-1];
		getMaxConvexHulls(weatherScenariosList, maxConvexHulls, maxScaleFactor);
	}

	// loop over scenarios and scales and reset the log files
#if LOG_SCENARIO_PATHS
	stringstream fname;
	fname << "log/paths_nom"
			<< fixed << setprecision(1) << ".log";

	ofstream log;
	log.open(fname.str().c_str(), ios::out);
	log << endl;
	log.close();

	map<double, PolygonLists>::iterator scaledScenarioIter;
	for(scaledScenarioIter=weatherScenariosList.begin();
			scaledScenarioIter!=weatherScenariosList.end();
			++scaledScenarioIter) {
		double scale = scaledScenarioIter->first;

		const PolygonLists* scenarioPolys = &(scaledScenarioIter->second);
		PolygonLists::const_iterator scenarioPolysIter;
		for(scenarioPolysIter=scenarioPolys->begin();
				scenarioPolysIter!=scenarioPolys->end();
				++scenarioPolysIter) {

			int scenario = scenarioPolysIter-scenarioPolys->begin();

			stringstream fname;
			fname << "log/paths_" << scenario << "_"
					<< fixed << setprecision(1) << scale << "x.log";

			ofstream log;
			log.open(fname.str().c_str(), ios::out);
			log << endl;
			log.close();
		}
	}
#endif

	// find the nominal shortest paths, ignoring the polygons
	map<FixPair, SearchPath> nominalPaths;
	find_shortest_paths(graph, airportPairs, &nominalPaths);

	// add the nominal paths to the result set with key scenario=-1, scale=0.
	map<FixPair, SearchPath>::iterator nomIter;
	for(nomIter=nominalPaths.begin(); nomIter!=nominalPaths.end(); ++nomIter) {
		const FixPair* pair = &(nomIter->first);
		const SearchPath* nomPath = &(nomIter->second);
		ResultSetKey key(-1, 0);
		ResultSets::iterator iter = results->find(key);
		if(iter == results->end()) {
			ResultSet result;
			result[*pair] = *nomPath;
			results->insert(std::pair<ResultSetKey, ResultSet>(
							key, result));
		} else {
			results->at(key).insert(std::pair<FixPair,
					SearchPath>(*pair, *nomPath));
		}
		logPath(nomPath, -1, 0);
	}

	// return here if there are no polygons to route around
	if(maxConvexHulls.size() < 1) return 0;

	// for each of the nominal paths, find the entry and exit points to
	// the convex hull of the union of scaled polygons.  for each of
	// the entry/exit point pairs, compute the shortest path around
	// each of the scaled polygons in each weather scenario
	vector<FixPair> entryExitPairs;
	map<SearchPath, FixPair> nominalEntryFixes;
	getPolygonEntryExitPoints(nominalPaths, maxConvexHulls,
			                  entryExitPairs, nominalEntryFixes, replanFlag);

	// call get_trx_shortest_paths() to get the routes around
	// each polygon scaling in each scenario:
	// outer-loop: iterate over scenarios
	findReroutePaths(weatherScenariosList, nominalEntryFixes, graph, results);

	// if provided an output PolygonSets then copy the scaled polygons
	// to the output variable
	if(polysOut) {
		map<double, PolygonLists>::iterator scaledScenarioIter;
		for(scaledScenarioIter=weatherScenariosList.begin();
				scaledScenarioIter!=weatherScenariosList.end();
				++scaledScenarioIter) {

			double scale = scaledScenarioIter->first;
			const PolygonLists* scenarioPolys = &(scaledScenarioIter->second);

			PolygonLists::const_iterator scenarioPolysIter;
			for(scenarioPolysIter=scenarioPolys->begin();
					scenarioPolysIter!=scenarioPolys->end();
					++scenarioPolysIter) {

				int scenario = scenarioPolysIter-scenarioPolys->begin();

				const PolygonSet* polys = &(*scenarioPolysIter);

				PolygonSetKey key(scenario, scale);
				polysOut->insert(std::pair<PolygonSetKey, PolygonSet>(key,
						*polys));
			}
		}
	}

	return 0;
}
#else

#endif

#if !USE_LIBSEARCH
int get_paths(SearchGraph& graph,
		const set<FixPair>& airportPairs,
	    const PolygonLists& weatherScenarios,
	    const double scalingFactors[],
		const int& numFactors,
	    ResultSets* const results,
		PolygonSets* const polysOut,
	    const bool& replanFlag) {
	// make sure the airway data has been loaded
	if (validate_global_data() < 0) {
		return -1;
	}

	// parse the trx file for origin-destination pairs
	// make sure we have valid output pointer
	if (!results) {
		log_error("get_paths(): invalid output pointer.", true);
		return -1;
	}

	// convert the set<FixPair> to string[][2]
	int numPairs = airportPairs.size();
	string airportPairsArray[numPairs][2];
	set<FixPair>::iterator it;
	int i=0;
	for (it=airportPairs.begin(); it!=airportPairs.end(); ++it) {
		const FixPair* pair = &(*it);
		airportPairsArray[i][0] = pair->origin;
		airportPairsArray[i][1] = pair->destination;
		++i;
	}

	// compute the scaled convex polygons for each weather scenario
	map<double, PolygonLists> weatherScenariosList;
	getScaledWeatherPolygons(weatherScenarios, scalingFactors, numFactors,
			                 weatherScenariosList);

	// compute the convex hull of the union of the largest scaled polygons
	// across all weather scenarios. for each scenario PolygonSet in the scale
	// level, iterate over polygons and compute the convex hull
	vector<Polygon> maxConvexHulls;
	if(numFactors > 0) {
		double maxScaleFactor = scalingFactors[numFactors-1];
		getMaxConvexHulls(weatherScenariosList, maxConvexHulls, maxScaleFactor);
	}

	// loop over scenarios and scales and reset the log files
#if LOG_SCENARIO_PATHS
	stringstream fname;
	fname << "log/paths_nom"
			<< fixed << setprecision(1) << ".log";

	ofstream log;
	log.open(fname.str().c_str(), ios::out);
	log << endl;
	log.close();

	map<double, PolygonLists>::iterator scaledScenarioIter;
	for(scaledScenarioIter=weatherScenariosList.begin();
			scaledScenarioIter!=weatherScenariosList.end();
			++scaledScenarioIter) {
		double scale = scaledScenarioIter->first;

		const PolygonLists* scenarioPolys = &(scaledScenarioIter->second);
		PolygonLists::const_iterator scenarioPolysIter;
		for(scenarioPolysIter=scenarioPolys->begin();
				scenarioPolysIter!=scenarioPolys->end();
				++scenarioPolysIter) {

			int scenario = scenarioPolysIter-scenarioPolys->begin();

			stringstream fname;
			fname << "log/paths_" << scenario << "_"
					<< fixed << setprecision(1) << scale << "x.log";

			ofstream log;
			log.open(fname.str().c_str(), ios::out);
			log << endl;
			log.close();
		}
	}
#endif

	// find the nominal shortest paths, ignoring the polygons
	map<FixPair, SearchPath> nominalPaths;
	find_shortest_paths(graph, const_cast<set<FixPair>&>(airportPairs), &nominalPaths);

	// add the nominal paths to the result set with key scenario=-1, scale=0.
	map<FixPair, SearchPath>::iterator nomIter;
	for(nomIter=nominalPaths.begin(); nomIter!=nominalPaths.end(); ++nomIter) {
		const FixPair* pair = &(nomIter->first);
		const SearchPath* nomPath = &(nomIter->second);
		ResultSetKey key(-1, 0);
		ResultSets::iterator iter = results->find(key);
		if(iter == results->end()) {
			ResultSet result;
			result[*pair] = *nomPath;
			results->insert(std::pair<ResultSetKey, ResultSet>(
							key, result));
		} else {
			results->at(key).insert(std::pair<FixPair,
					SearchPath>(*pair, *nomPath));
		}
		logPath(nomPath, -1, 0);
	}

	// return here if there are no polygons to route around
	if(maxConvexHulls.size() < 1) return 0;

	// for each of the nominal paths, find the entry and exit points to
	// the convex hull of the union of scaled polygons.  for each of
	// the entry/exit point pairs, compute the shortest path around
	// each of the scaled polygons in each weather scenario
	vector<FixPair> entryExitPairs;
	map<SearchPath, FixPair> nominalEntryFixes;
	getPolygonEntryExitPoints(nominalPaths, maxConvexHulls,
			                  entryExitPairs, nominalEntryFixes, replanFlag);

	// call get_trx_shortest_paths() to get the routes around
	// each polygon scaling in each scenario:
	// outer-loop: iterate over scenarios
	findReroutePaths(weatherScenariosList, nominalEntryFixes, graph, results);

	// if provided an output PolygonSets then copy the scaled polygons
	// to the output variable
	if(polysOut) {
		map<double, PolygonLists>::iterator scaledScenarioIter;
		for(scaledScenarioIter=weatherScenariosList.begin();
				scaledScenarioIter!=weatherScenariosList.end();
				++scaledScenarioIter) {

			double scale = scaledScenarioIter->first;
			const PolygonLists* scenarioPolys = &(scaledScenarioIter->second);

			PolygonLists::const_iterator scenarioPolysIter;
			for(scenarioPolysIter=scenarioPolys->begin();
					scenarioPolysIter!=scenarioPolys->end();
					++scenarioPolysIter) {

				int scenario = scenarioPolysIter-scenarioPolys->begin();

				const PolygonSet* polys = &(*scenarioPolysIter);

				PolygonSetKey key(scenario, scale);
				polysOut->insert(std::pair<PolygonSetKey, PolygonSet>(key,
						*polys));
			}
		}
	}

	return 0;
}

int get_wind_optimal_paths(SearchGraph& graph, const vector<FixPair>& flights,
	      WindGrid* const g_wind,
	      vector<WindGrid>* const g_wind_vec,
	      const PolygonLists& weatherScenarios,
	      const double scalingFactors[], const int& numFactors,
	      ResultSets* const results, PolygonSets* const polysOut,
	      const bool& replanFlag) {
	// make sure the airway data has been loaded
	if(validate_global_data() < 0) {
		return -1;
	}

	// parse the trx file for origin-destination pairs
	// make sure we have valid output pointer
	if(!results) {
		log_error("get_wind_optimal_paths(): invalid output pointer.", true);
		return -1;
	}

	// compute the scaled convex polygons for each weather scenario
	map<double, PolygonLists> weatherScenariosList;
	getScaledWeatherPolygons(weatherScenarios, scalingFactors, numFactors,
			                 weatherScenariosList);
	// compute the convex hull of the union of the largest scaled polygons
	// across all weather scenarios. for each scenario PolygonSet in the scale
	// level, iterate over polygons and compute the convex hull
	vector<Polygon> maxConvexHulls;
	if (numFactors > 0) {
		double maxScaleFactor = scalingFactors[numFactors-1];
		getMaxConvexHulls(weatherScenariosList, maxConvexHulls, maxScaleFactor);
	}

	// loop over scenarios and scales and reset the log files
#if LOG_SCENARIO_PATHS
	stringstream fname;
	fname << "log/paths_nom"
			<< fixed << setprecision(1) << ".log";

	ofstream log;
	log.open(fname.str().c_str(), ios::out);
	log << endl;
	log.close();

	map<double, PolygonLists>::iterator scaledScenarioIter;
	for(scaledScenarioIter=weatherScenariosList.begin();
			scaledScenarioIter!=weatherScenariosList.end();
			++scaledScenarioIter) {
		double scale = scaledScenarioIter->first;

		const PolygonLists* scenarioPolys = &(scaledScenarioIter->second);
		PolygonLists::const_iterator scenarioPolysIter;
		for(scenarioPolysIter=scenarioPolys->begin();
				scenarioPolysIter!=scenarioPolys->end();
				++scenarioPolysIter) {

			int scenario = scenarioPolysIter-scenarioPolys->begin();

			stringstream fname;
			fname << "log/paths_" << scenario << "_"
					<< fixed << setprecision(1) << scale << "x.log";

			ofstream log;
			log.open(fname.str().c_str(), ios::out);
			log << endl;
			log.close();
		}
	}
#endif

	// find the maximum wind vector magnitude from rap grid
	// the following are globals in rap.h...
	// uWindArrayMeterPerSec[NumHybridLevels][NiOfWindComp*NjOfWindComp]
	// vWindArrayMeterPerSec[NumHybridLevels][NiOfWindComp*NjOfWindComp]
	// NiOfWindComp: 451
	// NjOfWindComp: 337
	// NumHybridLevels: 50
	//double metersToFt = 3.2808399;
	if (g_wind_vec){
		g_max_wind_magnitude = MIN_DOUBLE;
		for(unsigned int k = 0; k<g_wind_vec->size(); ++k){
			double max_mag = g_wind_vec->at(k).getMaxWind();
			if (max_mag> g_max_wind_magnitude)
				g_max_wind_magnitude = max_mag;
		}
	}
	else{
		g_max_wind_magnitude = g_wind->getMaxWind();
	}

	// find the nominal wind optimal paths, ignoring the polygons
	map<FixPair, SearchPath> nominalPaths;
#if ENABLE_COMPARE_SP_TO_WO
	// subset of flights for debugging
	unsigned int maxFlights = 100000;
	vector<FixPair> someFlights;
	vector<FixPair>::const_iterator startIter = flights.begin();
	vector<FixPair>::const_iterator endIter = (flights.size() < maxFlights ? flights.end() : flights.begin()+maxFlights);
	someFlights.insert(someFlights.begin(), startIter, endIter);
	compare_wind_optimal_paths(graph, someFlights, &nominalPaths);
#else
	find_wind_optimal_paths(graph, const_cast<vector<FixPair>&>(flights), &nominalPaths, g_wind,
							g_wind_vec);
#endif



	// add the nominal paths to the result set with key scenario=-1, scale=0.
	map<FixPair, SearchPath>::iterator nomIter;
	for(nomIter=nominalPaths.begin(); nomIter!=nominalPaths.end(); ++nomIter) {
		const FixPair* pair = &(nomIter->first);
		const SearchPath* nomPath = &(nomIter->second);

		ResultSetKey key(-1, 0);
		ResultSets::iterator iter = results->find(key);
		if(iter == results->end()) {
			ResultSet result;
			result[*pair] = *nomPath;
			results->insert(std::pair<ResultSetKey, ResultSet>(
							key, result));
		} else {
			results->at(key).insert(std::pair<FixPair,
					SearchPath>(*pair, *nomPath));
		}
		logPath(nomPath, -1, 0);
	}


	// return here if there are no polygons to route around
	if(maxConvexHulls.size() < 1) {
		insert_sids_stars_single(results);
		return 0;
	}

	// for each of the nominal paths, find the entry and exit points to
	// the convex hull of the union of scaled polygons.  for each of
	// the entry/exit point pairs, compute the shortest path around
	// each of the scaled polygons in each weather scenario
	vector<FixPair> entryExitPairs;
	map<SearchPath, FixPair> nominalEntryFixes;
	getPolygonEntryExitPoints(nominalPaths, maxConvexHulls,
			                  entryExitPairs, nominalEntryFixes, replanFlag);


	// call get_trx_shortest_paths() to get the routes around
	// each polygon scaling in each scenario:
	// outer-loop: iterate over scenarios
	findWindOptimalReroutePaths(weatherScenariosList, nominalEntryFixes, graph,
			results, g_wind, g_wind_vec);
	insert_sids_stars_single(results);
	// if provided an output PolygonSets then copy the scaled polygons
	// to the output variable
	if(polysOut) {
		map<double, PolygonLists>::iterator scaledScenarioIter;
		for(scaledScenarioIter=weatherScenariosList.begin();
				scaledScenarioIter!=weatherScenariosList.end();
				++scaledScenarioIter) {

			double scale = scaledScenarioIter->first;
			const PolygonLists* scenarioPolys = &(scaledScenarioIter->second);

			PolygonLists::const_iterator scenarioPolysIter;
			for(scenarioPolysIter=scenarioPolys->begin();
					scenarioPolysIter!=scenarioPolys->end();
					++scenarioPolysIter) {

				int scenario = scenarioPolysIter-scenarioPolys->begin();

				const PolygonSet* polys = &(*scenarioPolysIter);

				PolygonSetKey key(scenario, scale);
				polysOut->insert(std::pair<PolygonSetKey, PolygonSet>(key,
						*polys));
			}
		}
	}

	return 0;
}


int get_wind_optimal_TOS(SearchGraph& graph,
		  const vector<FixPair>& flights,
	      WindGrid* const g_wind,
	      vector<WindGrid>* const g_wind_vec,
		  PIREPset* const g_pirep,
	      const PolygonLists& weatherScenarios,
	      const double scalingFactors[],
		  const int& numFactors,
	      ResultTrajs* const results,
		  PolygonSets* const polysOut,
	      const int& num_trajs,
		  const bool& replanFlag,
	      const tos_gen_method& t_method,
		  const FixedProc& fproc,
		  const int& offset) {
	// make sure the airway data has been loaded
	if (validate_global_data() < 0) {
		return -1;
	}

	// parse the trx file for origin-destination pairs
	// make sure we have valid output pointer
	if (!results) {
		log_error("get_wind_optimal_paths(): invalid output pointer.", true);

		return -1;
	}

	// compute the scaled convex polygons for each weather scenario
#ifndef NDEBUG
	clock_t strt = clock();
#endif
	map<double, PolygonLists> weatherScenariosList;
	getScaledWeatherPolygons(weatherScenarios,
			scalingFactors,
			numFactors,
			weatherScenariosList);
#ifndef NDEBUG
	cout << endl << "Time taken for getScaledWeatherPolygons = " << (double)(clock()- strt)/CLOCKS_PER_SEC << endl;
#endif
	// compute the convex hull of the union of the largest scaled polygons
	// across all weather scenarios. for each scenario PolygonSet in the scale
	// level, iterate over polygons and compute the convex hull
	vector<Polygon> maxConvexHulls;
	if (numFactors > 0) {
		double maxScaleFactor = scalingFactors[numFactors-1];
		getMaxConvexHulls(weatherScenariosList, maxConvexHulls, maxScaleFactor);
	}

	//Get maximum wind magnitude for A* heuristic function.

	if (!g_wind_vec->empty()) {
		g_max_wind_magnitude = MIN_DOUBLE;
		for (unsigned int k = 0; k<g_wind_vec->size(); ++k) {
			double max_mag = g_wind_vec->at(k).getMaxWind();
			if (max_mag > g_max_wind_magnitude)
				g_max_wind_magnitude = max_mag;
		}
	}
	else {
		g_max_wind_magnitude = g_wind->getMaxWind();
	}

	//Get the trajectory option sets
	map<FixPair, vector<SearchPath> > nominalTOS;
	SearchGraph origgraph;//JUST A STORE FOR GRAPH COSTS
	
#ifndef NDEBUG
	strt = clock();
#endif

	find_wind_optimal_TOS(graph,
			const_cast<vector<FixPair>&>(flights),
			nominalTOS,
			origgraph,
			g_wind,
			g_wind_vec,
			num_trajs,
			t_method,
			fproc,
			offset);
			
#ifndef NDEBUG
	cout << endl <<"Time taken for find_wind_optimal_TOS = " << (double)(clock()- strt)/CLOCKS_PER_SEC << endl;
#endif

	// add the nominal paths to the result set with key scenario=-1, scale=0.

	// return here if there are no polygons to route around
	if (maxConvexHulls.size() < 1) {
		ResultSetKey nomkey(-1, 0);
		results->insert(make_pair(nomkey, nominalTOS));
		insert_sids_stars(results);

		return 0;
	}

	// for each of the nominal paths, find the entry and exit points to
	// the convex hull of the union of scaled polygons.  for each of
	// the entry/exit point pairs, compute the shortest path around
	// each of the scaled polygons in each weather scenario

	vector<FixPair> entryExitPairs;
	map<SearchPath, FixPair> nominalEntryFixes;
#ifndef NDEBUG
	strt = clock();
#endif
	getPolygonEntryExitPointsForTOS(nominalTOS,
			maxConvexHulls,
			entryExitPairs,
			nominalEntryFixes,
			replanFlag);
#ifndef NDEBUG
	cout << endl <<"Time taken for getPolygonEntryExitPointsForTOS = " << (double)(clock()- strt)/CLOCKS_PER_SEC << endl;
#endif

	// call get_trx_shortest_paths() to get the routes around
	// each polygon scaling in each scenario:
	// outer-loop: iterate over scenarios
#ifndef NDEBUG
	strt = clock();
#endif

	findWindOptimalReroutePathsForTOS(weatherScenariosList,
			nominalEntryFixes,
			origgraph,
			results,
			g_wind,
			g_wind_vec,
			t_method,
			fproc,
			true,
			offset);
#ifndef NDEBUG
	cout << endl << "Time taken for findWindOptimalReroutePathsForTOS = " << (double)(clock()- strt)/CLOCKS_PER_SEC << endl;
#endif

	// if provided an output PolygonSets then copy the scaled polygons
	// to the output variable
	if (polysOut) {
		map<double, PolygonLists>::iterator scaledScenarioIter;
		for (scaledScenarioIter=weatherScenariosList.begin();
				scaledScenarioIter!=weatherScenariosList.end();
				++scaledScenarioIter) {

			double scale = scaledScenarioIter->first;
			const PolygonLists* scenarioPolys = &(scaledScenarioIter->second);

			PolygonLists::const_iterator scenarioPolysIter;
			for (scenarioPolysIter=scenarioPolys->begin();
					scenarioPolysIter!=scenarioPolys->end();
					++scenarioPolysIter) {

				int scenario = scenarioPolysIter-scenarioPolys->begin();

				const PolygonSet* polys = &(*scenarioPolysIter);

				PolygonSetKey key(scenario, scale);
				polysOut->insert(std::pair<PolygonSetKey, PolygonSet>(key,
						*polys));
			}
		}
	}

	return 0;
}

#else

#endif

/*
 * Use this for standard shortest path
 */
int parse_trx_airport_pairs(const string& trxfile, set<FixPair>* const pairs) {

	// make sure we have a valid TRX file to open if the user didn't
	// specify the origin or destination
	ifstream in;
	in.open(trxfile.c_str());
	if(!in.is_open()) {
		log_error("parse_trx_airport_pairs(): invalid TRX file.", true);
		return -1;
	}
	in.close();

#ifndef NDEBUG
	cout << "Finding all TRX airport pairs..." << endl;
#endif

	// setup the trx parser and listener
	string filename = string(trxfile);
	TrxAirportPairExtractor airportPairExtractor(pairs);
	TrxInputStream trxParser(filename);
	trxParser.addTrxInputStreamListener(&airportPairExtractor);

	// parse the trx file to generate airport pairs
	trxParser.parse();

#ifndef NDEBUG
	cout << "Parsed " << pairs->size() << " airport pairs:" << endl;
#endif

	return 0;
}

/*
 * Use this for wind optimal
 */
int parse_trx_flights(const string& trxfile,
		const string& mflfile, vector<FixPair>* const flights) {
	// make sure we have a valid TRX file to open
	ifstream in;
	in.open(trxfile.c_str());
	if(!in.is_open()) {
		log_error("parse_trx_flights(): invalid TRX file.", true);
		return -1;
	}
	in.close();

	// make sure we have a valid MFL file to open
	if(!is_new_trx_format(trxfile)) {
        in.open(mflfile.c_str());
        if(!in.is_open()) {
            log_error("parse_trx_flights(): invalid MFL file.", true);
            return -1;
        }
        in.close();
	}

#ifndef NDEBUG
	cout << "Parsing TRX flights..." << endl;
#endif

	// setup the trx parser and listener
	string filename = string(trxfile);
	TrxFlightExtractor flightExtractor(flights);
	TrxInputStream trxParser(filename, mflfile);
	trxParser.addTrxInputStreamListener(&flightExtractor);

	// parse the trx file to generate airport pairs
	trxParser.parse();

	return 0;
}

/*
 * Detect new trx format which includes mfl
 */
bool is_new_trx_format(const string& trxfile) {
    // parse the first track line and count the tokens if 10 tokens
    // then its an old file. if 11 tokens then its a new file.
    ifstream in;
    in.open(trxfile.c_str());
    if(!in.is_open()) return false;

    bool is_new = false;

    while (in.good()) {
        string line = "";
        getline(in, line);

        // remove leading or trailing whitespace
        line = trim(line);

        // skip blank lines
        if (line.length() < 1) continue;

        // tokenize the line on space
        deque<string> tokens = tokenize(line, " ");
        if (tokens.at(0) == "TRACK") {
            if(tokens.size() > 10) {
                // new
                is_new = true;
            } else {
                // old
                is_new = false;
            }
            break;
        }
    }
    in.close();

    return is_new;
}

static const Airway* getAirway(const string& name,
		   const string& description="",
		   const string& altLevel="",
		   const string& routeType="",
		   const vector<string>& waypointNames=EMPTY_STRING_VECTOR,
		   const vector<double>& minAltitude1=EMPTY_DOUBLE_VECTOR,
		   const vector<double>& minAltitude2=EMPTY_DOUBLE_VECTOR,
		   const vector<double>& maxAltitude=EMPTY_DOUBLE_VECTOR) {
	// check if the global map contains key equal to name.
	// if no, construct a new instance and add it to the global map.
	// return the mapped instance.
	map<string, Airway*>::iterator iter = rg_airways.find(name);
	if (iter == rg_airways.end()) {
		Airway* airway = new Airway(name,
				description,
				altLevel,
				routeType,
				waypointNames,
				minAltitude1,
				minAltitude2,
				maxAltitude);
		rg_airways.insert(std::pair<string, Airway*>(name, airway));
	}

	return rg_airways[name];
}

static const Sid* getSid(const string& name,
		                 const string& airport,
		                 const map< string, vector<string> > waypoints,
						 const map< string, pair<string,string> > rt_to_trans_rttype= map< string, pair<string,string> >(),
						 const map<string, vector<pair<string,string> > > path_term = map<string, vector<pair<string,string> > >(),
						 const map<string, vector<pair<string,string> > > alt_desc = map<string, vector<pair<string,string> > >(),
						 const map<string, vector<pair<string,double> > > alt_1 = map<string, vector<pair<string,double> > >(),
						 const map<string, vector<pair<string,double> > > alt_2 = map<string, vector<pair<string,double> > >(),
						 const map<string, vector<pair<string,double> > > spd_limit = map<string, vector<pair<string,double> > >(),
						 const map<string, vector<pair<string,string> > > recco_nav = map<string, vector<pair<string,string> > >(),
						 const map<string, vector<pair<string,double> > > theta  = map<string, vector<pair<string,double> > >(),
						 const map<string, vector<pair<string,double> > > rho =  map<string, vector<pair<string,double> > >(),
						 const map<string, vector<pair<string,double> > > mag_course  = map<string, vector<pair<string,double> > >(),
						 const map<string, vector<pair<string,double> > > rt_dist  = map<string, vector<pair<string,double> > >()
						 ) {

	// check if the global map contains key.
	// if no, construct a new instance and add it to the global map.
	// return the mapped instance.
	ProcKey key(name, airport);
	map<ProcKey, Sid*>::iterator iter = rg_sids.find(key);
	if(iter == rg_sids.end()) {
		Sid* sid = new Sid(name, airport, waypoints,rt_to_trans_rttype);
		sid->path_term = path_term;
		sid->alt_desc = alt_desc;
		sid->alt_1 = alt_1;
		sid->alt_2 = alt_2;
		sid->spd_limit = spd_limit;
		sid->recco_nav = recco_nav;
		sid->theta = theta;
		sid->rho = rho;
		sid->mag_course = mag_course;
		sid->rt_dist = rt_dist;

#ifndef NDEBUG
		vector<string> endpoints;
		sid->getExitPoints(&endpoints);
		cout << "sid " << name << " endpoints:" << endl;
		for(unsigned int i=0; i<endpoints.size(); ++i) {
			cout << "  " << endpoints.at(i) << endl;
		}
#endif
		rg_sids.insert(std::pair<ProcKey, Sid*>(key, sid));
	}

	return rg_sids[key];
}

static const Star* getStar(const string& name,
		                  const string& airport,
		                  const map< string, vector<string> > waypoints,
						  const map< string, pair<string,string> > rt_to_trans_rttype = map< string, pair<string,string> >(),
						  const map<string, vector<pair<string,string> > > path_term = map<string, vector<pair<string,string> > >(),
						  const map<string, vector<pair<string,string> > > alt_desc = map<string, vector<pair<string,string> > >(),
						  const map<string, vector<pair<string,double> > > alt_1 = map<string, vector<pair<string,double> > >(),
						  const map<string, vector<pair<string,double> > > alt_2 = map<string, vector<pair<string,double> > >(),
						  const map<string, vector<pair<string,double> > > spd_limit = map<string, vector<pair<string,double> > >(),
						  const map<string, vector<pair<string,string> > > recco_nav = map<string, vector<pair<string,string> > >(),
						  const map<string, vector<pair<string,double> > > theta  = map<string, vector<pair<string,double> > >(),
						  const map<string, vector<pair<string,double> > > rho =  map<string, vector<pair<string,double> > >(),
						  const map<string, vector<pair<string,double> > > mag_course  = map<string, vector<pair<string,double> > >(),
						  const map<string, vector<pair<string,double> > > rt_dist  = map<string, vector<pair<string,double> > >()
						  ) {

	// check if the global map contains key.
	// if no, construct a new instance and add it to the global map.
	// return the mapped instance.
	ProcKey key(name, airport);
	map<ProcKey, Star*>::iterator iter = rg_stars.find(key);
	if (iter == rg_stars.end()) {
		Star* star = new Star(name, airport, waypoints, rt_to_trans_rttype);
		star->path_term = path_term;
		star->alt_desc = alt_desc;
		star->alt_1 = alt_1;
		star->alt_2 = alt_2;
		star->spd_limit = spd_limit;
		star->recco_nav = recco_nav;
		star->theta = theta;
		star->rho = rho;
		star->mag_course = mag_course;
		star->rt_dist = rt_dist;

		rg_stars.insert(std::pair<ProcKey, Star*>(key, star));
	}

	return rg_stars[key];
}

static const Approach* getApproach(const string& name,
		                  const string& airport,
						  const string& runway,
		                  const map< string, vector<string> > waypoints,
						  const map< string, pair<string,string> > rt_to_trans_rttype = map< string, pair<string,string> >(),
						  const map<string, vector<pair<string,string> > > path_term = map<string, vector<pair<string,string> > >(),
						  const map<string, vector<pair<string,string> > > alt_desc = map<string, vector<pair<string,string> > >(),
						  const map<string, vector<pair<string,double> > > alt_1 = map<string, vector<pair<string,double> > >(),
						  const map<string, vector<pair<string,double> > > alt_2 = map<string, vector<pair<string,double> > >(),
						  const map<string, vector<pair<string,double> > > spd_limit = map<string, vector<pair<string,double> > >(),
						  const map<string, vector<pair<string,string> > > recco_nav = map<string, vector<pair<string,string> > >(),
						  const map<string, vector<pair<string,double> > > theta  = map<string, vector<pair<string,double> > >(),
						  const map<string, vector<pair<string,double> > > rho =  map<string, vector<pair<string,double> > >(),
						  const map<string, vector<pair<string,double> > > mag_course  = map<string, vector<pair<string,double> > >(),
						  const map<string, vector<pair<string,double> > > rt_dist  = map<string, vector<pair<string,double> > >()
						  ) {

	// check if the global map contains key.
	// if no, construct a new instance and add it to the global map.
	// return the mapped instance.
	ProcKey key(name, airport);
	map<ProcKey, Approach*>::iterator iter = rg_approach.find(key);
	if (iter == rg_approach.end()) {
		Approach* approach = new Approach(name, airport, runway, waypoints, rt_to_trans_rttype);
		approach->path_term = path_term;
		approach->alt_desc = alt_desc;
		approach->alt_1 = alt_1;
		approach->alt_2 = alt_2;
		approach->spd_limit = spd_limit;
		approach->recco_nav = recco_nav;
		approach->theta = theta;
		approach->rho = rho;
		approach->mag_course = mag_course;
		approach->rt_dist = rt_dist;

		rg_approach.insert(std::pair<ProcKey, Approach*>(key, approach));
	}

	return rg_approach[key];
}

static const Airport* getAirport(const string& airportCode,
		                         const string& name,
		                         const double& latitude,
		                         const double& longitude,
		                         const double& elevation,
								 const double& magvar=0) {
	Airport* airport_ptr = new Airport(airportCode, name, latitude, longitude, elevation, magvar);

	map<string, Airport*>::iterator iter = rg_airports.find(airportCode);
	if (iter == rg_airports.end()) {
		int newId = rg_airports.size();

		rg_airports.insert(std::pair<string, Airport*>(airportCode, airport_ptr));
		rg_airports_by_id.insert(std::pair<int, Airport*>(newId, airport_ptr));

		rg_airport_ids.insert(std::pair<string, int>(airportCode, newId));
		rg_airport_names.insert(std::pair<int, string>(newId, airportCode));
	}

	return airport_ptr;
}

void create_rg_airport(const string& airportCode,
        const string& name,
        const double& latitude,
        const double& longitude,
        const double& elevation,
		const double& magvar=0) {
    getAirport(airportCode, name, latitude, longitude, elevation, magvar);
}

static const Navaid* getNavaid(const string& name=UNSET_STRING,
		   const string& description="",
		   const double& latitude=0,
		   const double& longitude=0,
		   const double& frequency=0,
		   const double& dmeLatitude=0,
		   const double& dmeLongitude=0,
		   const double& dmeElevation=0,
		   const string& region="") {
	// check if the global map contains key equal to name.
	// if no, construct a new instance and add it to the global map.
	// return the mapped instance.
	map<string, Fix*>::iterator iter = rg_fixes.find(name);
	if (iter == rg_fixes.end() || (region.compare(0, 3, "USA") == 0)) {
		int newId = rg_fixes.size();

		if (rg_fixes.find(name) != rg_fixes.end()) {
			Fix* fix_ptr = rg_fixes.at(name);

			delete fix_ptr;
		}

		Navaid* navaid = new Navaid(name,
									description,
				                    latitude,
									longitude,
									frequency,
				                    dmeLatitude,
									dmeLongitude,
									dmeElevation);
		rg_fixes[name] = navaid;
		rg_fixes_by_id[newId] = navaid;
		rg_fix_names[newId] = name;
		rg_fix_ids[name] = newId;
	}

	return (Navaid*)rg_fixes[name];
}

static const Waypoint* getWaypoint(const string& name=UNSET_STRING,
		 const string& description="",
		 const double& latitude=0,
		 const double& longitude=0,
		 const string& region="") {
	// check if the global map contains key equal to name.
	// if no, construct a new instance and add it to the global map.
	// return the mapped instance.
	map<string, Fix*>::iterator iter = rg_fixes.find(name);
	if (iter == rg_fixes.end() || (region.compare(0, 3, "USA") == 0 )) {
		int newId = rg_fixes.size();

		Waypoint* waypoint = new Waypoint(name,
										description,
				                        latitude,
										longitude);
		rg_fixes[name] = waypoint;
		rg_fixes_by_id[newId] = waypoint;
		rg_fix_names[newId] = name;
		rg_fix_ids[name] = newId;
	}

	return (Waypoint*)rg_fixes[name];
}

static bool is_found(string& str) {
	size_t found1 = str.find("-");
	size_t found2 = str.find("RW");

	if (found1 != string::npos && found2 != string::npos)
		return true;
	else
		return false;
}

static bool check_if_runway(Approach* const currapp) {
	bool retval = false;

	for (map<string,vector<string> > ::iterator it = currapp->waypoints.begin();
			it != currapp->waypoints.end();
			++it) {
		vector<string> wplist = it->second;
		vector<string>::iterator its = find_if(wplist.begin(), wplist.end(), is_found);

		if (its != wplist.end()) {
			retval = true;
			break;
		}
	}

	return retval;
}

template<typename T>
static bool check_proc_consistency(T* const proc) {
	map<string, vector<string> > wp_map = proc->waypoints;
	bool inconsistent_flag = false;

	if (wp_map.size() <= 1) {
		map<string, vector<string> >::iterator it = wp_map.begin();
		for (;it != wp_map.end(); ++it) {
			if (it->second.size() <= 1)
				inconsistent_flag = true;
		}
	}

	return inconsistent_flag;
}

/**
 * Add airway if any part of the flight plans is not in airway data
 */
void supplement_airways_by_flightPlans_logic(waypoint_node_t* waypoint_node_ptr) {
	waypoint_node_t* cur_waypoint_node_ptr = waypoint_node_ptr;
	waypoint_node_t* next_waypoint_node_ptr = NULL;

	bool tmp_flag_matchAirwayLink = false;
	waypoint_node_t* tmp_airway_entry_node_ptr = NULL;
	waypoint_node_t* tmp_airway_exit_node_ptr = NULL;

	vector<string> new_waypointNames;
	vector<double> new_minAltitude1;
	vector<double> new_minAltitude2;
	vector<double> new_maxAltitude;

	while (cur_waypoint_node_ptr != NULL) {
		next_waypoint_node_ptr = cur_waypoint_node_ptr->next_node_ptr;

		if ((cur_waypoint_node_ptr->proctype != NULL)
				&& (indexOf(cur_waypoint_node_ptr->proctype, "ENROUTE") == 0) &&
				(next_waypoint_node_ptr != NULL) && (indexOf(next_waypoint_node_ptr->proctype, "ENROUTE") == 0)) {
			map<string, Airway*>::iterator ite_airway;
			for (ite_airway = rg_airways.begin(); ite_airway != rg_airways.end(); ite_airway++) {
				Airway* tmp_Airway_ptr = ite_airway->second;
				vector<string> tmp_waypointNames = tmp_Airway_ptr->waypointNames;
				if (tmp_waypointNames.size() > 1) {
					tmp_flag_matchAirwayLink = false; // Reset

					for (unsigned int i = 0; i < (tmp_waypointNames.size()-1); i++) {
						if ((tmp_waypointNames.at(i).compare(cur_waypoint_node_ptr->wpname) == 0) && (tmp_waypointNames.at(i+1).compare(next_waypoint_node_ptr->wpname) == 0)) {
							tmp_flag_matchAirwayLink = true;

							break;
						}
					}
				}

				if (tmp_flag_matchAirwayLink)
					break;
			}

			if (!tmp_flag_matchAirwayLink) {
				if (tmp_airway_entry_node_ptr == NULL)
					tmp_airway_entry_node_ptr = cur_waypoint_node_ptr;

				tmp_airway_exit_node_ptr = next_waypoint_node_ptr;
			}
		}

		cur_waypoint_node_ptr = cur_waypoint_node_ptr->next_node_ptr;
	}

	if ((tmp_airway_entry_node_ptr != NULL) && (tmp_airway_exit_node_ptr != NULL)) {
		waypoint_node_t* tmp_waypoint_node_ptr = tmp_airway_entry_node_ptr;

		while (tmp_waypoint_node_ptr != NULL) {
			new_waypointNames.push_back(string(tmp_waypoint_node_ptr->wpname));
			if (tmp_waypoint_node_ptr->alt_1 > 0) {
				new_minAltitude1.push_back(tmp_waypoint_node_ptr->alt_1);
			} else {
				new_minAltitude1.push_back(0.0);
			}
			new_minAltitude2.push_back(0);
			if (tmp_waypoint_node_ptr->alt_2 > 0) {
				new_maxAltitude.push_back(tmp_waypoint_node_ptr->alt_2);
			} else {
				new_maxAltitude.push_back(0.0);
			}

			if (tmp_waypoint_node_ptr == tmp_airway_exit_node_ptr)
				break;

			tmp_waypoint_node_ptr = tmp_waypoint_node_ptr->next_node_ptr;
		}
	}

	if (new_waypointNames.size() > 0) {
		rg_supplemental_airway_seq++;

		stringstream ss;
		ss << "UserDefinedAirway";
		ss << rg_supplemental_airway_seq;

		getAirway(ss.str(),
					"",
					"",
					"",
					new_waypointNames,
					new_minAltitude1,
					new_minAltitude2,
					new_maxAltitude);
	}
}

void load_airways(const string& airwaysXmlFilename) {
	// load the airways xml.
	ifstream in;
	in.open(airwaysXmlFilename.c_str());
	if(!in.good()) return;
	stringstream xmlss;
	while(in.good()) {
		string line;
		getline(in, line);
		xmlss << trim(line);
	}
	in.close();

	// parse the xml string
	string xml = xmlss.str();
	xml_document<char> doc;
	doc.parse<0>(const_cast<char*>(xml.c_str()));

	// traverse the dom and build the Airway structs
	xml_node<> *root = doc.first_node("map");
	xml_node<> *entry = root->first_node("entry");
	xml_node<> *list = entry->first_node("list");
	xml_node<> *airway = list->first_node("osi.atm.datastore.beans.Airway");

	while(airway) {

		xml_node<> *airwayName = airway->first_node("name");
		xml_node<> *airwayDesc = airway->first_node("description");
		xml_node<> *airwayAltLevel = airway->first_node("altLevel");
		xml_node<> *airwayRouteType = airway->first_node("routeType");
		xml_node<> *airwayWaypoints = airway->first_node("waypoints");
		xml_node<> *fix = airwayWaypoints->first_node();

		vector<string> waypointNames;
		stringstream ss;

		while(fix) {

			string fixClass = fix->name();
			if(fixClass == "osi.atm.datastore.beans.Navaid") {
				xml_node<> *fixName = fix->first_node("name");
				xml_node<> *fixDescr = fix->first_node("description");
				xml_node<> *fixLat = fix->first_node("latitude");
				xml_node<> *fixLon = fix->first_node("longitude");
				xml_node<> *fixFreq = fix->first_node("frequency");
				xml_node<> *fixDmeLat = fix->first_node("dmeLatitude");
				xml_node<> *fixDmeLon = fix->first_node("dmeLongitude");
				xml_node<> *fixDmeElev = fix->first_node("dmeElevation");
				ss << fixName->value() << " ";
				waypointNames.push_back(fixName->value());

				// build a navaid and add to global map
				getNavaid(fixName->value(),
						fixDescr->value(),
						atof(fixLat->value()),
						atof(fixLon->value()),
						atof(fixFreq->value()),
						atof(fixDmeLat->value()),
						atof(fixDmeLon->value()),
						atof(fixDmeElev->value()));

			} else if(fixClass == "osi.atm.datastore.beans.Waypoint") {
				xml_node<> *fixName = fix->first_node("name");
				xml_node<> *fixDescr = fix->first_node("description");
				xml_node<> *fixLat = fix->first_node("latitude");
				xml_node<> *fixLon = fix->first_node("longitude");
				ss << fixName->value() << " ";
				waypointNames.push_back(fixName->value());

				// build a waypoint and add to global map
				getWaypoint(fixName->value(),
						fixDescr->value(),
						atof(fixLat->value()),
						atof(fixLon->value()));
			}

			fix = fix->next_sibling();
		}

		vector<double> minAlt1;
		vector<double> minAlt2;
		vector<double> maxAlt;

		xml_node<> *airwayMinAltitude1 = airway->first_node("minAltitude1");
		xml_node<> *minAlt1Value = airwayMinAltitude1->first_node();
		while(minAlt1Value) {
			minAlt1.push_back( atof(minAlt1Value->value()) );
			minAlt1Value = minAlt1Value->next_sibling();
		}
		xml_node<> *airwayMinAltitude2 = airway->first_node("minAltitude2");
		xml_node<> *minAlt2Value = airwayMinAltitude2->first_node();
		while(minAlt2Value) {
			minAlt2.push_back( atof(minAlt2Value->value()) );
			minAlt2Value = minAlt2Value->next_sibling();
		}
		xml_node<> *airwayMaxAltitude = airway->first_node("maxAltitude");
		xml_node<> *maxAltValue = airwayMaxAltitude->first_node();
		while(maxAltValue) {
			maxAlt.push_back( atof(maxAltValue->value()) );
			maxAltValue = maxAltValue->next_sibling();
		}

		getAirway( airwayName->value(),
				   airwayDesc->value(),
				   airwayAltLevel->value(),
				   airwayRouteType->value(),
				   waypointNames,
				   minAlt1,
				   minAlt2,
				   maxAlt);

		airway = airway->next_sibling();
	}

#ifndef NDEBUG
	cout << "Loaded " << rg_fixes.size() << " fixes." << endl;
	cout << "Loaded " << rg_airways.size() << " airways." << endl;
#endif
}


void load_airways_alt(const string& CIFPfilename,const string& routeconf) {
	/*
	 * Airport Specific stuff
	 */

	string secCode=""; string subSecCode="";
	int secCodeFld=0; int subSecCodeFld=0;
	int routenamestart=0; int routenameend=0;
	int fixnamestart=0; int fixnameend=0;
	int rtTypeFld = 0;int rtLevFld = 0;
	int minaltstart1 = 0;int minaltend1= 0;
	int minaltstart2 = 0;int minaltend2= 0;
	int maxaltstart = 0;int maxaltend= 0;

	string line;
	char *token;
	ifstream configFile(routeconf.c_str());
	if (configFile.is_open()) {
		int gcount = 0;
		while (getline(configFile,line)) {
			char *dup = strdup(line.c_str());
			token = strtok(dup, ":");
			int count = 0;
			while (token!=NULL) {
				if(count == 1 && gcount==0)
					secCode = string(token);
				if (count == 1 && gcount==1)
					subSecCode = string(token);
				if (count == 1 && gcount==2)
					secCodeFld = atoi(token);
				if (count == 1 && gcount==3)
					subSecCodeFld = atoi(token);
				if (count == 1 && gcount==4){
					char *tvalue = strtok(token,",");
					int cnt =0;
					while(tvalue !=NULL){
						if(cnt==0)
							routenamestart = atoi(tvalue);
						if(cnt == 1)
							routenameend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				if (count == 1 && gcount == 5){
					char *tvalue = strtok(token,",");
					int cnt =0;
					while(tvalue !=NULL){
						if(cnt==0)
							fixnamestart = atoi(tvalue);
						if(cnt == 1)
							fixnameend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				if (count == 1 && gcount == 6)
					rtTypeFld = atoi(token);
				if (count == 1 && gcount == 7)
					rtLevFld = atoi(token);
				if (count == 1 && gcount == 8){
					char *tvalue = strtok(token,",");
					int cnt =0;
					while(tvalue !=NULL){
						if(cnt==0)
							minaltstart1 = atoi(tvalue);
						if (cnt == 1)
							minaltend1 = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL, ",");
					}
				}
				if (count == 1 && gcount == 9){
					char *tvalue = strtok(token, ",");
					int cnt =0;
					while (tvalue !=NULL) {
						if (cnt == 0)
							minaltstart2 = atoi(tvalue);
						if (cnt == 1)
							minaltend2 = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL, ",");
					}
				}
				if (count == 1 && gcount == 10){
					char *tvalue = strtok(token, ",");
					int cnt =0;
					while (tvalue !=NULL) {
						if (cnt == 0)
							maxaltstart = atoi(tvalue);
						if (cnt == 1)
							maxaltend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL, ",");
					}
				}
				count++;
				token = strtok(NULL, ":");
			}
			free(dup);
			gcount++;
		}
		configFile.close();
	}

	set<string> routename;routename.clear();
	vector<string> wpname; wpname.clear();
	vector<double> minalt1; minalt1.clear();
	vector<double> minalt2; minalt2.clear();
	vector<double> maxalt; maxalt.clear();
	string altlevel="";
	string rttype="";
	string prevname = "None";
	ifstream myfile (CIFPfilename.c_str());

	if (myfile.is_open()) {
		while ( getline (myfile,line) ) {
			if ( (line.length() > 10)
					) {// NOT ONLY USA for Enroute

				if ( (line.compare(secCodeFld-1,1,secCode) == 0) &&
						(line.compare(subSecCodeFld-1,1,subSecCode )==0) ) {
					string name = line.substr(routenamestart-1,routenameend-routenamestart+1);
					removeSpaces(name);
					set<string>::iterator it = routename.find(name);
					if (it == routename.end() ) {
						if (!routename.empty()) {
							getAirway( prevname,
									prevname,
									altlevel,
									rttype,
									wpname,
									minalt1,
									minalt2,
									maxalt);
							wpname.clear();minalt1.clear();minalt2.clear();maxalt.clear();
							prevname = name;
						}

						string rttype = line.substr(rtTypeFld-1,1);
						string altlevel = line.substr(rtLevFld-1,1);

						string fixname = line.substr(fixnamestart-1,fixnameend-fixnamestart+1);
						removeSpaces(fixname);

						map<string,Fix*>::iterator iter = rg_fixes.find(fixname);

						wpname.push_back(fixname);

						double altv =0.0;
						string minAlt1 = line.substr(minaltstart1-1,minaltend1-minaltstart1+1);removeSpaces(minAlt1);
						try{
							altv = atof(minAlt1.c_str());
							minalt1.push_back(altv);
						}
						catch(int &doNothing){}

						string minAlt2 = line.substr(minaltstart2-1,minaltend2-minaltstart2+1);removeSpaces(minAlt2);
						try{
							altv = atof(minAlt2.c_str());
							minalt2.push_back(altv);
						}
						catch(int &doNothing){}

						string maxAlt = line.substr(maxaltstart-1,maxaltend-maxaltstart+1);removeSpaces(maxAlt);
						try{
							altv = atof(maxAlt.c_str());
							maxalt.push_back(altv);
						}
						catch(int &doNothing){}

						routename.insert(name);
					}
					else {
						string fixname = line.substr(fixnamestart-1,fixnameend-fixnamestart+1);
						removeSpaces(fixname);

						map<string,Fix*>::iterator iter = rg_fixes.find(fixname);

						wpname.push_back(fixname);

						double altv =0.0;
						string minAlt1 = line.substr(minaltstart1-1,minaltend1-minaltstart1+1);removeSpaces(minAlt1);
						try {
							altv = atof(minAlt1.c_str());
							minalt1.push_back(altv);
						}
						catch (int &doNothing) {}

						string minAlt2 = line.substr(minaltstart2-1,minaltend2-minaltstart2+1);removeSpaces(minAlt2);
						try {
							altv = atof(minAlt2.c_str());
							minalt2.push_back(altv);
						}
						catch (int &doNothing) {}

						string maxAlt = line.substr(maxaltstart-1,maxaltend-maxaltstart+1);removeSpaces(maxAlt);
						try {
							altv = atof(maxAlt.c_str());
							maxalt.push_back(altv);
						}
						catch (int &doNothing) {}
					}

				}
			}
		}

		myfile.close();
	}
#ifndef NDEBUG
	cout << "Loaded " << rg_fixes.size() << " fixes." << endl;
	cout << "Loaded " << rg_airways.size() << " airways." << endl;
#endif
}

void load_airways_data(const string& CIFPfilename) {
	/*
	 * Airport Specific stuff
	 */

	string secCode=""; string subSecCode="";
	int secCodeFld=0; int subSecCodeFld=0;
	int routenamestart=0; int routenameend=0;
	int fixnamestart=0; int fixnameend=0;
	int rtTypeFld = 0;int rtLevFld = 0;
	int minaltstart1 = 0;int minaltend1= 0;
	int minaltstart2 = 0;int minaltend2= 0;
	int maxaltstart = 0;int maxaltend= 0;



	string line;

	set<string> routename; routename.clear();
	vector<string> wpname; wpname.clear();
	vector<double> minalt1; minalt1.clear();
	vector<double> minalt2; minalt2.clear();
	vector<double> maxalt; maxalt.clear();

	string altlevel="";string rttype="";string prevname = "None";
	ifstream myfile (CIFPfilename.c_str());
	if (myfile.is_open()){
		while ( getline (myfile,line) ){
			if ( (line.length() > 10)
					){// NOT ONLY USA for Enroute
				if (
						(line.compare(0, 1, secCode) == 0)
						&&
						(line.compare(0, 1, subSecCode) == 0)
						) {

					string name = line.substr(routenamestart-1,routenameend-routenamestart+1);
					removeSpaces(name);
					set<string>::iterator it = routename.find(name);
					if(it == routename.end() ){
						if (!routename.empty()){
							getAirway( prevname,
									prevname,
									altlevel,
									rttype,
									wpname,
									minalt1,
									minalt2,
									maxalt);
							wpname.clear();minalt1.clear();minalt2.clear();maxalt.clear();
							prevname = name;
						}


						string rttype = line.substr(rtTypeFld-1,1);
						string altlevel = line.substr(rtLevFld-1,1);

						string fixname = line.substr(fixnamestart-1,fixnameend-fixnamestart+1);
						removeSpaces(fixname);

						map<string,Fix*>::iterator iter = rg_fixes.find(fixname);

						wpname.push_back(fixname);

						double altv =0.0;
						string minAlt1 = line.substr(minaltstart1-1,minaltend1-minaltstart1+1);removeSpaces(minAlt1);
						try{
							altv = atof(minAlt1.c_str());
							minalt1.push_back(altv);
						}
						catch(int &doNothing){}

						string minAlt2 = line.substr(minaltstart2-1,minaltend2-minaltstart2+1);removeSpaces(minAlt2);
						try{
							altv = atof(minAlt2.c_str());
							minalt2.push_back(altv);
						}
						catch(int &doNothing){}

						string maxAlt = line.substr(maxaltstart-1,maxaltend-maxaltstart+1);removeSpaces(maxAlt);
						try{
							altv = atof(maxAlt.c_str());
							maxalt.push_back(altv);
						}
						catch(int &doNothing){}

						routename.insert(name);

					}
					else{
						string fixname = line.substr(fixnamestart-1,fixnameend-fixnamestart+1);
						removeSpaces(fixname);

						map<string,Fix*>::iterator iter = rg_fixes.find(fixname);

						wpname.push_back(fixname);

						double altv =0.0;
						string minAlt1 = line.substr(minaltstart1-1,minaltend1-minaltstart1+1);removeSpaces(minAlt1);
						try{
							altv = atof(minAlt1.c_str());
							minalt1.push_back(altv);
						}
						catch(int &doNothing){}

						string minAlt2 = line.substr(minaltstart2-1,minaltend2-minaltstart2+1);removeSpaces(minAlt2);
						try{
							altv = atof(minAlt2.c_str());
							minalt2.push_back(altv);
						}
						catch(int &doNothing){}

						string maxAlt = line.substr(maxaltstart-1,maxaltend-maxaltstart+1);removeSpaces(maxAlt);
						try{
							altv = atof(maxAlt.c_str());
							maxalt.push_back(altv);
						}
						catch(int &doNothing){}
					}
				}
			}
		}

		myfile.close();
	}
#ifndef NDEBUG
	cout << "Loaded " << rg_fixes.size() << " fixes." << endl;
	cout << "Loaded " << rg_airways.size() << " airways." << endl;
#endif

}

int createFix(const string& fixname,const string& CIFPfilename,
		const string& fixconf,const bool termFlag,const string& airport){

	map<string, Fix*>::iterator iter = rg_fixes.find(fixname);
	if (iter != rg_fixes.end())
		return 0;

	string secCodeWp = ""; string secCodeNa = "";string secCodeTerm = "";
	int secCodeFld = 0;
	string subSecCodeWp = "";string subSecCodeTrWp = "";string subSecCodeTrNa = "";string subSecCodeTrRw = "";
	int subSecCodeFld = 0; int subSecCodeFldTermWp = 0;
	int regnamestart = 0; int regnameend = 0;
	int fixnamestart = 0; int fixnameend = 0;
	int freqstart = 0; int freqend =0;
	int latstart = 0;int latend = 0;
	int lonstart = 0;int lonend = 0;
	int elestart = 0;int eleend = 0;

	string line;
	char *token;
	ifstream configFile(fixconf.c_str());
	if(configFile.is_open()){
		int gcount = 0;
		while(getline(configFile,line)){
			char *dup = strdup(line.c_str());
			token = strtok(dup, ":");
			int count = 0;
			while(token!=NULL){
				if (count == 1 && gcount==0){
					char *tvalue = strtok(token,",");
					int cnt =0;
					while(tvalue !=NULL){
						if(cnt==0)
							secCodeWp = tvalue;
						if(cnt == 1)
							secCodeNa = tvalue;
						if(cnt == 2)
							secCodeTerm = tvalue;
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				if (count == 1 && gcount==1)
					secCodeFld = atoi(token);
				if (count == 1 && gcount==2){
					char *tvalue = strtok(token,",");
					int cnt =0;
					while(tvalue !=NULL){
						if(cnt==0)
							subSecCodeWp = tvalue;
						if(cnt == 1)
							subSecCodeTrWp = tvalue;
						if(cnt == 2)
							subSecCodeTrNa = tvalue;
						if(cnt == 3)
							subSecCodeTrRw = tvalue;
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				if (count == 1 && gcount==3){
					char *tvalue = strtok(token,",");
					int cnt =0;
					while(tvalue !=NULL){
						if(cnt==0)
							subSecCodeFld = atoi(tvalue);
						if(cnt == 1)
							subSecCodeFldTermWp = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				if (count == 1 && gcount == 4){
					char *tvalue = strtok(token,",");
					int cnt =0;
					while(tvalue !=NULL){
						if(cnt==0)
							regnamestart = atoi(tvalue);
						if(cnt == 1)
							regnameend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				if (count == 1 && gcount == 5){
					char *tvalue = strtok(token,",");
					int cnt =0;
					while(tvalue !=NULL){
						if(cnt==0)
							fixnamestart = atoi(tvalue);
						if(cnt == 1)
							fixnameend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				if (count == 1 && gcount == 6){
					char *tvalue = strtok(token,",");
					int cnt =0;
					while(tvalue !=NULL){
						if(cnt==0)
							freqstart = atoi(tvalue);
						if(cnt == 1)
							freqend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				if (count == 1 && gcount == 7){
					char *tvalue = strtok(token,",");
					int cnt =0;
					while(tvalue !=NULL){
						if(cnt==0)
							latstart = atoi(tvalue);
						if(cnt == 1)
							latend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				if (count == 1 && gcount == 8){
					char *tvalue = strtok(token,",");
					int cnt =0;
					while(tvalue !=NULL){
						if(cnt==0)
							lonstart = atoi(tvalue);
						if(cnt == 1)
							lonend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				if (count == 1 && gcount == 9){
					char *tvalue = strtok(token,",");
					int cnt =0;
					while(tvalue !=NULL){
						if(cnt==0)
							elestart = atoi(tvalue);
						if(cnt == 1)
							eleend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				count++;
				token = strtok(NULL,":");

			}
			free(dup);
			gcount++;
		}
		configFile.close();
	}

	ifstream myfile (CIFPfilename.c_str());
	if (myfile.is_open()) {
		while ( getline (myfile,line) ) {

			if ((line.length() > 10)) {//&& ( line.compare(1,3,"USA") == 0 )){ // Not ONLY USA for Enroute

				string reg = line.substr(regnamestart-1,regnameend-regnamestart+1);
				removeSpaces(reg);
				string name = line.substr(fixnamestart-1,fixnameend-fixnamestart+1);
				removeSpaces(name);
				double lat, lon;
				string lati = line.substr(latstart-1,latend-latstart+1);
				convertLatLon(lati,lat,true);
				string longi = line.substr(lonstart-1,lonend-lonstart+1);
				convertLatLon(longi,lon,false);

				if (name.compare(0,name.length(),fixname) == 0) {
					if ( (line.compare(secCodeFld-1,1,secCodeWp) == 0)
							&&	(line.compare(subSecCodeFld-1,1,subSecCodeWp )==0) )
						getWaypoint(name,reg,lat,lon);


					else if (line.compare(secCodeFld-1,1,secCodeNa) == 0) {
						double elev;
						string ele = line.substr(elestart-1,eleend-elestart+1);
						convertElev(ele,elev);
						string freqs = line.substr(freqstart-1,freqend-freqstart+1);
						double freqv = atof(freqs.c_str());

						//VOR and DME lat lon assumed to be the same
						getNavaid(name,	reg,lat,lon,freqv,lat,lon,elev);
					}
					else if (line.compare(secCodeFld-1,1,secCodeTerm) == 0 && termFlag) {
						if (reg.compare(0,reg.length(),airport) == 0) {
							if  ( line.compare(subSecCodeFldTermWp-1,1,subSecCodeTrWp )==0 )
								getWaypoint(name,reg,lat,lon);
							else if (line.compare(subSecCodeFld-1,1,subSecCodeTrRw) == 0)
								getWaypoint(name+'-'+airport,reg,lat,lon);

							else if (line.compare(subSecCodeFld-1,1,subSecCodeTrNa) == 0) {
								double elev;
								string ele = line.substr(elestart-1,eleend-elestart+1);
								convertElev(ele,elev);

								string freqs = line.substr(freqstart-1,freqend-freqstart+1);
								double freqv = atof(freqs.c_str());

								//VOR and DME lat lon assumed to be the same
								getNavaid(name,	reg, lat, lon, freqv, lat, lon, elev);
							}
						}

					}

				}
			}
		}

		myfile.close();
	}

	return 0;
}

void load_fixes(const string& CIFPfilename,
                const string& fixconf,
                const set<string>* const airportFilter,
                const set<string>* const nameFilter) {
	string secCodeWp = "";
	string secCodeNa = "";
	string secCodeTerm = "";

	int secCodeFld = 0;

	string subSecCodeWp = "";
	string subSecCodeTrWp = "";
	string subSecCodeTrNa = "";
	string subSecCodeTrRw = "";

	int subSecCodeFld = 0; int subSecCodeFldTermWp = 0;
	int regnamestart = 0; int regnameend = 0;
	int fixnamestart = 0; int fixnameend = 0;
	int freqstart = 0; int freqend =0;
	int latstart = 0;int latend = 0;
	int lonstart = 0;int lonend = 0;
	int dmelatstart = 0;int dmelatend = 0;
	int dmelonstart = 0;int dmelonend = 0;
	int elestart = 0;int eleend = 0;

	string line;
	char *token;
	ifstream configFile(fixconf.c_str());
	if (configFile.is_open()) {
		int gcount = 0;

		while (getline(configFile,line)) {
			char *dup = strdup(line.c_str());
			token = strtok(dup, ":");
			int count = 0;

			while (token!=NULL) {
				if (count == 1 && gcount==0) {
					char *tvalue = strtok(token,",");
					int cnt =0;
					while (tvalue != NULL){
						if (cnt == 0)
							secCodeWp = tvalue;
						if (cnt == 1)
							secCodeNa = tvalue;
						if (cnt == 2)
							secCodeTerm = tvalue;
						cnt++;
						tvalue = strtok(NULL, ",");
					}
				}
				if (count == 1 && gcount == 1)
					secCodeFld = atoi(token);
				if (count == 1 && gcount == 2) {
					char *tvalue = strtok(token, ",");
					int cnt =0;
					while (tvalue != NULL) {
						if (cnt == 0)
							subSecCodeWp = tvalue;
						if (cnt == 1)
							subSecCodeTrWp = tvalue;
						if (cnt == 2)
							subSecCodeTrNa = tvalue;
						if (cnt == 3)
							subSecCodeTrRw = tvalue;
						cnt++;
						tvalue = strtok(NULL, ",");
					}
				}
				if (count == 1 && gcount == 3) {
					char *tvalue = strtok(token, ",");
					int cnt =0;
					while (tvalue != NULL){
						if (cnt == 0)
							subSecCodeFld = atoi(tvalue);
						if (cnt == 1)
							subSecCodeFldTermWp = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL, ",");
					}
				}
				if (count == 1 && gcount == 4) {
					char *tvalue = strtok(token, ",");
					int cnt = 0;
					while (tvalue != NULL) {
						if (cnt == 0)
							regnamestart = atoi(tvalue);
						if (cnt == 1)
							regnameend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL, ",");
					}
				}
				if (count == 1 && gcount == 5) {
					char *tvalue = strtok(token, ",");
					int cnt =0;
					while (tvalue != NULL){
						if (cnt == 0)
							fixnamestart = atoi(tvalue);
						if (cnt == 1)
							fixnameend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL, ",");
					}
				}
				if (count == 1 && gcount == 6) {
					char *tvalue = strtok(token, ",");
					int cnt = 0;
					while (tvalue != NULL){
						if (cnt == 0)
							freqstart = atoi(tvalue);
						if (cnt == 1)
							freqend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL, ",");
					}
				}
				if (count == 1 && gcount == 7) {
					char *tvalue = strtok(token, ",");
					int cnt =0;
					while (tvalue != NULL){
						if (cnt == 0)
							latstart = atoi(tvalue);
						if (cnt == 1)
							latend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL, ",");
					}
				}
				if (count == 1 && gcount == 8) {
					char *tvalue = strtok(token, ",");
					int cnt =0;
					while (tvalue != NULL) {
						if (cnt == 0)
							lonstart = atoi(tvalue);
						if (cnt == 1)
							lonend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL, ",");
					}
				}
				if (count == 1 && gcount == 9) {
					char *tvalue = strtok(token, ",");
					int cnt =0;
					while (tvalue != NULL) {
						if (cnt == 0)
							dmelatstart = atoi(tvalue);
						if (cnt == 1)
							dmelatend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL, ",");
					}
				}
				if (count == 1 && gcount == 10) {
					char *tvalue = strtok(token, ",");
					int cnt = 0;
					while (tvalue != NULL) {
						if (cnt == 0)
							dmelonstart = atoi(tvalue);
						if (cnt == 1)
							dmelonend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL, ",");
					}
				}
				if (count == 1 && gcount == 11) {
					char *tvalue = strtok(token, ",");
					int cnt =0;
					while (tvalue != NULL) {
						if (cnt == 0)
							elestart = atoi(tvalue);
						if (cnt == 1)
							eleend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL, ",");
					}
				}
				count++;
				token = strtok(NULL, ":");

			}

			free(dup);
			gcount++;
		}
		configFile.close();
	}

	ifstream myfile (CIFPfilename.c_str());
	if (myfile.is_open()) {
		while ( getline (myfile,line) ) {
			if ( (line.length() > 10)
					   ) {
				string region = line.substr(1, 3);
				if ( (line.compare(secCodeFld-1, 1, secCodeWp) == 0)
						&&	(line.compare(subSecCodeFld-1, 1, subSecCodeWp )==0) ) {
					string reg = line.substr(regnamestart-1, regnameend-regnamestart+1);
					removeSpaces(reg);
					string name = line.substr(fixnamestart-1, fixnameend-fixnamestart+1);
					removeSpaces(name);

					double lat, lon;
					string lati = line.substr(latstart-1, latend-latstart+1);
					convertLatLon(lati, lat, true);
					string longi = line.substr(lonstart-1, lonend-lonstart+1);
					convertLatLon(longi, lon, false);
					getWaypoint(name, reg, lat, lon, region);
				}
				else if (line.compare(secCodeFld-1, 1, secCodeNa) == 0) {
					string reg = line.substr(regnamestart-1, regnameend-regnamestart+1);
					removeSpaces(reg);
					string name = line.substr(fixnamestart-1, fixnameend-fixnamestart+1);
					removeSpaces(name);

					double lat, lon;
					string lati = line.substr(latstart-1, latend-latstart+1);
					removeSpaces(lati);
					if (lati.length()==0)
						lati = line.substr(dmelatstart-1, dmelatend-dmelatstart+1);
					convertLatLon(lati, lat, true);
					string longi = line.substr(lonstart-1, lonend-lonstart+1);
					removeSpaces(longi);
					if (longi.length()==0)
						longi = line.substr(dmelonstart-1, dmelonend-dmelonstart+1);
					convertLatLon(longi, lon, false);

					double elev;
					string ele = line.substr(elestart-1, eleend-elestart+1);
					convertElev(ele, elev);
					string freqs = line.substr(freqstart-1, freqend-freqstart+1);
					double freqv = atof(freqs.c_str());
					//VOR and DME lat lon assumed to be the same
					getNavaid(name,	reg, lat, lon, freqv, lat, lon, elev, region);
				}

				else if (line.compare(secCodeFld-1, 1, secCodeTerm) == 0) {
					string reg = line.substr(regnamestart-1, regnameend-regnamestart+1);
					removeSpaces(reg);
					if (!airportFilter->empty()) {
						set<string>::iterator findResult = airportFilter->find(reg);
						if (findResult == airportFilter->end())
							continue;
					}

					string name = line.substr(fixnamestart-1, fixnameend-fixnamestart+1);
					removeSpaces(name);

					double lat, lon;
					string lati = line.substr(latstart-1, latend-latstart+1);
					convertLatLon(lati, lat, true);
					string longi = line.substr(lonstart-1, lonend-lonstart+1);
					convertLatLon(longi, lon, false);

					if (line.compare(subSecCodeFldTermWp-1, 1, subSecCodeTrWp ) == 0) {
						getWaypoint(name,reg, lat, lon, region);
					}
					else if (line.compare(subSecCodeFldTermWp-1, 1, subSecCodeTrRw) == 0) {
						getWaypoint(name+"-"+reg, reg, lat, lon, region);
					}
					else if (line.compare(subSecCodeFld-1, 1, subSecCodeTrNa) == 0) {
						double elev;
						string ele = line.substr(elestart-1, eleend-elestart+1);
						convertElev(ele, elev);

						string freqs = line.substr(freqstart-1, freqend-freqstart+1);
						double freqv = atof(freqs.c_str());

						//VOR and DME lat lon assumed to be the same

						getNavaid(name,	reg, lat, lon, freqv, lat, lon, elev, region);
					}
				}
			}
		}
		myfile.close();
	}
}

void unload_rg_airways() {
	map<string, Fix*>::iterator fiter;
	map<string, Airway*>::iterator aiter;

	for (fiter = rg_fixes.begin(); fiter != rg_fixes.end(); ++fiter) {
		Fix* f = fiter->second;

		if (f) {
			delete f;
		}
	}

	for (aiter = rg_airways.begin(); aiter != rg_airways.end(); ++aiter) {
		Airway* a = aiter->second;
		if (a) {
			delete a;
		}
	}

	rg_fixes.clear();
	rg_airways.clear();
}

static void load_procs(const string& xmlFilename, const string& classname,
		const set<string>* const airportFilter) {
	// load the xml.
	ifstream in;
	in.open(xmlFilename.c_str());
	if (!in.good()) return;

	stringstream xmlss;
	while (in.good()) {
		string line;
		getline(in, line);
		xmlss << trim(line);
	}
	in.close();

	// parse the xml string
	string xml = xmlss.str();
	xml_document<char> doc;
	doc.parse<0>(const_cast<char*>(xml.c_str()));

	// traverse the dom and build the Sid structs
	xml_node<> *root = doc.first_node("map");
	xml_node<> *entry = root->first_node("entry");

	while (entry) {

		xml_node<> *list = entry->first_node("list");
		xml_node<> *proc = list->first_node(classname.c_str());

		while (proc) {
			xml_node<> *procName = proc->first_node("name");
			xml_node<> *airport = proc->first_node("airport");
			xml_node<> *routes = proc->first_node("waypoints");
			xml_node<> *route = routes->first_node("entry");

			// if the airport name is not in the filter set then
			// continue to the next proc.
			if (airportFilter) {
				set<string>::iterator findResult = airportFilter->find(airport->value());
				if (findResult == airportFilter->end()) {
					proc = proc->next_sibling();
					continue;
				}
			}

			map< string, vector<string> > routeNames;

			while (route) {
				xml_node<> *routeName = route->first_node("string");
				xml_node<> *waypoints = route->first_node("list");
				xml_node<> *fix = waypoints->first_node();

				vector<string> waypointNames;

				while (fix) {

					string fixClass = fix->name();

					if (fixClass == "osi.atm.datastore.beans.Navaid") {
						xml_node<> *fixName = fix->first_node("name");
						xml_node<> *fixDescr = fix->first_node("description");
						xml_node<> *fixLat = fix->first_node("latitude");
						xml_node<> *fixLon = fix->first_node("longitude");
						xml_node<> *fixFreq = fix->first_node("frequency");
						xml_node<> *fixDmeLat = fix->first_node("dmeLatitude");
						xml_node<> *fixDmeLon = fix->first_node("dmeLongitude");
						xml_node<> *fixDmeElev = fix->first_node("dmeElevation");

						waypointNames.push_back(fixName->value());

						// build a navaid and add to global map
						getNavaid(fixName->value(),
								fixDescr->value(),
								atof(fixLat->value()),
								atof(fixLon->value()),
								atof(fixFreq->value()),
								atof(fixDmeLat->value()),
								atof(fixDmeLon->value()),
								atof(fixDmeElev->value()));

					} else if (fixClass == "osi.atm.datastore.beans.Waypoint") {
						xml_node<> *fixName = fix->first_node("name");
						xml_node<> *fixDescr = fix->first_node("description");
						xml_node<> *fixLat = fix->first_node("latitude");
						xml_node<> *fixLon = fix->first_node("longitude");

						waypointNames.push_back(fixName->value());

						// build a waypoint and add to global map
						getWaypoint(fixName->value(),
								fixDescr->value(),
								atof(fixLat->value()),
								atof(fixLon->value()));

					} else if (fixClass == "osi.atm.datastore.beans.Runway") {
						// instead of the runway, we'll use the airport
						waypointNames.push_back(airport->value());
					}

					fix = fix->next_sibling();

				} // while fix

				routeNames.insert(std::pair< string, vector<string> >(routeName->value(), waypointNames));



				route = route->next_sibling();
			} // while route

			if (classname == "osi.atm.datastore.beans.Sid") {
				getSid(procName->value(), airport->value(), routeNames);
			} else if(classname == "osi.atm.datastore.beans.Star") {
				getStar(procName->value(), airport->value(), routeNames);
			}

			proc = proc->next_sibling();
		} // while sid

		entry = entry->next_sibling();
	}// while entry


#ifndef NDEBUG
	if (classname == "osi.atm.datastore.beans.Sid") {
		cout << "Loaded " << rg_sids.size() << " sids." << endl;
	} else if(classname == "osi.atm.datastore.beans.Star") {
		cout << "Loaded " << rg_stars.size() << " stars." << endl;
	}
#endif
}

static double parse_str(const string& str){
	if (str == "")
		return -10000;
	else
		return atof(str.c_str());
}

static double parse_alt_string(const string& str) {
	if (str.length() == 0) {
		return -10000;
	}
	else if (str == "UNKNN") {
		return -10000;
	}
	else {
		if (str.substr(0,2) == "FL") {
			string alt = str.substr(2,str.length()-2);
			double alt_d = atof(alt.c_str()) * 100.0;
			return alt_d;
		}
		else {
			double alt_d = atof(str.c_str());
			return alt_d;
		}
	}
}


void load_procs_alt(const string& CIFPfilename,
					const string& procType,
					const FixedProc& fproc
					) {
	string line = "";

	string secCode="P"; string subSecCode="";
	int secCodeFld=5; int subSecCodeFld=13;
	int aptcodestart=7; int aptcodeend=10;
	int procnamestart=14; int procnameend=19;
	int rttypefld = 20;
	int transidstart=21;int transidend=25;
	int seqnostart=27; int seqnoend=29;
	int fixnamestart=30; int fixnameend=34;
	int pathtermstart=48; int pathtermend=49;
	int recconavstart=51; int recconavend=54;
	int thetastart=63;int thetaend=66;
	int rhostart=67;int rhoend=71;
	int magcoursestart=71; int magcourseend=74;
	int routediststart=75; int routedistend=78;
	int altdescfld = 83;
	int alt1start = 85; int alt1end = 89;
	int alt2start = 90; int alt2end = 94;
	int spdlimstart = 100; int spdlimend = 102;

	if (procType == "SID")
		subSecCode = "D";
	else if (procType == "STAR")
		subSecCode = "E";
	else
		subSecCode = "F";


	 map<string, vector<string> > wp_map;wp_map.clear();
	 map<string, pair<string,string> > rt_to_trans_rttype;rt_to_trans_rttype.clear();
	 map<string, vector<pair<string,string> > > path_term;path_term.clear();
	 map<string, vector<pair<string,string> > > alt_desc;alt_desc.clear();
	 map<string, vector<pair<string,double> > > alt_1;alt_1.clear();
	 map<string, vector<pair<string,double> > > alt_2;alt_2.clear();
	 map<string, vector<pair<string,double> > > spd_limit;spd_limit.clear();

	 map<string, vector<pair<string,string> > > recco_nav;
	 map<string, vector<pair<string,double> > > theta;
	 map<string, vector<pair<string,double> > > rho;
	 map<string, vector<pair<string,double> > > mag_course;
	 map<string, vector<pair<string,double> > > rt_dist;

	 int prevseqno = 10000;
	 string currap = "";
	 string currproc = "";
	 string currroute = "";
	 string prevwpname = "";
	 string prevtrans = "";
	 string prevrwy = "";
	 string runway = "";
	 int rtcount = 0;


	 int no_wp_no_trans_cnt = 0;
	 std::ifstream myfile (CIFPfilename.c_str());
	 if (myfile.is_open()){
		 while ( getline (myfile,line) ){
			 if ((line.length() > 10)
					 ){
				 if ( (line.compare(secCodeFld-1,1,secCode) == 0) &&
						 (line.compare(subSecCodeFld-1,1,subSecCode )==0) ){

					 string airport = line.substr(aptcodestart-1,aptcodeend-aptcodestart+1);
					 trim(airport);

					 map<string,Airport*>::iterator it = rg_airports.find(airport);
					 if (it == rg_airports.end())
						 continue;

					 string procname = line.substr(procnamestart-1,procnameend-procnamestart+1);
					 trim(procname);

					 string rttype = line.substr(rttypefld-1,1);

					 string wpname = line.substr(fixnamestart-1,fixnameend-fixnamestart+1);
					 trim(wpname);

					 string trans = line.substr(transidstart-1,transidend-transidstart+1);
					 trim(trans);

					 string seqnostr = line.substr(seqnostart-1,seqnoend-seqnostart+1);
					 trim(seqnostr);

					 string pterm =line.substr(pathtermstart-1,pathtermend-pathtermstart+1);
					 trim(pterm);

					 string recconavstr = line.substr(recconavstart-1,recconavend-recconavstart+1);
					 trim(recconavstr);

					 string thetastr = line.substr(thetastart-1,thetaend-thetastart+1);
					 trim(thetastr); double theta_val = parse_str(thetastr)/10.0;

					 string rhostr = line.substr(rhostart-1,rhoend-rhostart+1);
					 trim(rhostr); double rho_val = parse_str(rhostr)/10.0;

					 string magcoursestr =line.substr(magcoursestart-1, magcourseend-magcoursestart+1);
					 trim(magcoursestr); double magcourse = parse_str(magcoursestr)/10.0;

					 string routediststr = line.substr(routediststart-1,routedistend-routediststart+1);
					 trim(routediststr); double rtdist_nm = parse_str(routediststr)/10.0;

					 string altdescstr = line.substr(altdescfld-1,1);
					 trim(altdescstr); if (altdescstr == "") altdescstr = "NONE";

					 string alt1str = line.substr(alt1start-1,alt1end-alt1start+1);
					 trim(alt1str);double alt1 = parse_alt_string(alt1str);

					 string alt2str = line.substr(alt2start-1,alt2end-alt2start+1);
					 trim(alt2str);double alt2 = parse_alt_string(alt2str);

					 string speedlimstr = line.substr(spdlimstart-1,spdlimend-spdlimstart+1);
					 trim(speedlimstr); double spdlim = -10000;
					 if (speedlimstr.length() != 0)
						 spdlim = atof(speedlimstr.c_str());

					 //Helicoptor Routes
					 if (procType == "APPROACH" && procname.compare(0, 1, "H") == 0)
							continue;

					 if (airport == fproc.airport && procType == fproc.procType) {
						 if (procname.compare(0, strlen(fproc.procName.c_str()), fproc.procName)) {
							 continue;
						 }
					 }

					 int seqno = atoi(seqnostr.c_str());

					 if (wpname.length() == 0 && trans.length() == 0) {
/*				FIXME: WHEN THESE TWO ARE NOT GIVEN CREATES ERROR*/
 						 if (pterm.compare("VI") == 0
							|| pterm.compare("VA") == 0
							|| pterm.compare("VD") == 0
							|| pterm.compare("VR") == 0
							|| pterm.compare("CI") == 0
							|| pterm.compare("CA") == 0
							|| pterm.compare("CD") == 0
							|| pterm.compare("CR") == 0
								 ) {
							 trans = prevtrans;
							 stringstream ss; ss << no_wp_no_trans_cnt++; string str_end = ss.str();
							 wpname = pterm+"_LEG_NO_WP_NO_TRANS_ID_"+procname+"_"+str_end+"_"+airport;
						 }
						 else {
							 cout << " in " << __LINE__ << " file " << __FILE__ << endl;
							 cout << line << endl;

							 continue;
						 }
					 }


					 if (trans.length()) {
						 if (trans == "ALL")
							 trans = trans + "_RWY";
					 }


					 if (procType == "APPROACH") {
						 string rnum =  procname.substr(2-1,3-2+1);
						 string dir = procname.substr(4-1,1);
						 runway = rnum;
						 if (dir != "-")
							 runway += dir;
					 }

					 bool sidRWFlag = false;
					 if (rttype == "1" || rttype == "4" || rttype == "F" || rttype == "T")
						 sidRWFlag = true;
					 bool starRWFlag = false;
					 if (rttype == "3" || rttype == "6" || rttype == "9" || rttype == "S")
						 starRWFlag = true;

					 if (wpname.length() == 0 &&
						  ((sidRWFlag && procType == "SID") ||
								  (starRWFlag && procType == "STAR"))
								  ){
						 if ( (trans.compare(0,2,"RW") == 0)
								 && isInteger(trans.substr(2,2))){
							 string lastchar = trans.substr(trans.length()-1,1);
							 if (lastchar.compare(0,1,"B") == 0)
								 trans.replace(trans.length()-1,1,"R");
	 						 if (pterm.compare("VI") == 0
								|| pterm.compare("VA") == 0
								|| pterm.compare("VM") == 0
								|| pterm.compare("VD") == 0
								|| pterm.compare("VR") == 0
								|| pterm.compare("CI") == 0
								|| pterm.compare("CA") == 0
								|| pterm.compare("CD") == 0
								|| pterm.compare("CR") == 0
									 ){
	 							 wpname = pterm+"_LEG-NO_WP_NAME_"+trans+"_"+procname+"_"+airport;
	 						 }
							 else{
								 cout << " in " << __LINE__ <<" file " <<__FILE__ <<endl;
							 	 cout <<line <<endl;
							 }
						 }
						 else{
							 wpname = trans;
						 }
					 }




					 if (procType == "APPROACH"){
						 if (wpname.length() == 0){
							 if ( (pterm.compare("VI") == 0)
								|| (pterm.compare("CI") == 0)
								||(pterm.compare("PI") == 0)){
								 wpname = pterm+"_LEG-INTERCEPT_POINT_"+trans+"_"+procname+"_"+airport;
							 }
							 else{
								 cout << " in " << __LINE__ <<" file " <<__FILE__ <<endl;
								 cout <<line <<endl;
								 continue;
							 }
						 }
						 if (wpname.compare(0,2,"RW") == 0 &&
								 isInteger(wpname.substr(2,2)) )
							 wpname = wpname+'-'+airport;
					 }



					if (wpname.length() == 0 &&
							((!sidRWFlag && procType == "SID" )
									|| (!starRWFlag && procType == "STAR")) ){

						if (trans.compare(0,2,"RW")
								&& isInteger(trans.substr(2,2) ) ){
							cout << " ERROR in " << __LINE__ <<" file " <<__FILE__ <<endl;
							cout <<line <<endl<<endl;
							continue;
						}

						if (trans.length()){
							string lastchar = trans.substr(trans.length()-1,1);
							if ( (lastchar.compare(0,1,"B") == 0)
									&& (trans.compare(0,2,"RW") == 0)
									&& isInteger(trans.substr(2,2))){
								trans.replace(trans.length()-1,1,"R");
							}
							if (pterm.compare("VI") == 0
									|| pterm.compare("VA") == 0
									|| pterm.compare("VM") == 0
									|| pterm.compare("VD") == 0
									|| pterm.compare("VR") == 0
									|| pterm.compare("CI") == 0
									|| pterm.compare("CA") == 0
									|| pterm.compare("CD") == 0
									|| pterm.compare("CR") == 0
									){
								wpname = pterm+"_LEG-NO_WP_NAME_"+trans+"_"+procname+"_"+airport;
							}
	 						 else{
	 							 cout << " ERROR in " << __LINE__ <<" file " <<__FILE__ <<endl;
	 							 cout <<line <<endl<<endl;
	 						 }
						}
						else{
							printf(" ERROR: No transition or waypoint present for ap %s and proc %s...continuing\n",
									airport.c_str(),procname.c_str());
							continue;
						}
					}

                    if (trans.length()) {
                        if ((trans.compare(0,2,"RW") == 0)
                                && isInteger(trans.substr(2,2))) {
                            runway = trans;
                        }
                    }

					pair<string,string> trans_rttype;
					if (trans.length())
						trans_rttype = make_pair(trans,rttype);
					else
						trans_rttype = make_pair("BLANK-FIELD",rttype);

					if (currproc != procname){
						if (!wp_map.empty()){

							if (procType == "SID")
								getSid(currproc, currap, wp_map,rt_to_trans_rttype,
										path_term,alt_desc,alt_1,alt_2,spd_limit,
										recco_nav,theta,rho,mag_course,rt_dist);
							else if (procType == "STAR")
								getStar(currproc, currap, wp_map,rt_to_trans_rttype,
										path_term,alt_desc,alt_1,alt_2,spd_limit,
										recco_nav,theta,rho,mag_course,rt_dist);
							else if (procType == "APPROACH")
								getApproach(currproc, currap, prevrwy, wp_map,rt_to_trans_rttype,
										path_term,alt_desc,alt_1,alt_2,spd_limit,
										recco_nav,theta,rho,mag_course,rt_dist);
							else{
								cout << __FILE__ << " (" << __LINE__ << ")";
								printf(" Wrong procType. Exiting.\n");
								exit(EXIT_SUCCESS);
							}
						}
						currap = airport;
						currproc = procname;
						if (pterm.compare("VI") == 0
								|| pterm.compare("VA") == 0
								|| pterm.compare("VM") == 0
								|| pterm.compare("VD") == 0
								|| pterm.compare("VR") == 0
								|| pterm.compare("CI") == 0
								|| pterm.compare("CA") == 0
								|| pterm.compare("CD") == 0
								|| pterm.compare("CR") == 0
								){
							if (trans.length()){
								string lastchar = trans.substr(trans.length()-1,1);
								if (lastchar.compare(0,1,"B") == 0)
									trans.replace(trans.length()-1,1,"R");
							}
							if (procType == "SID"){
								currroute = trans+'-'+airport;
							}
						}
						else{
							currroute = wpname;
						}
						wp_map.clear();
						wp_map[currroute].push_back(wpname);

						rt_to_trans_rttype.clear();
						rt_to_trans_rttype.insert(pair<string,pair<string,string> >(currroute,trans_rttype));
						path_term.clear();
						path_term[currroute].push_back(pair<string,string>(wpname,pterm));
						alt_desc.clear();

						alt_desc[currroute].push_back(pair<string,string>(wpname,altdescstr));
						alt_1.clear();

						alt_1[currroute].push_back(pair<string,double>(wpname,alt1));
						alt_2.clear();

						alt_2[currroute].push_back(pair<string,double>(wpname,alt2));
						spd_limit.clear();

						spd_limit[currroute].push_back(pair<string,double>(wpname,spdlim));

						recco_nav.clear();
						recco_nav[currroute].push_back(pair<string,string>(wpname,recconavstr));

						theta.clear();
						theta[currroute].push_back(pair<string,double>(wpname,theta_val));

						rho.clear();
						rho[currroute].push_back(pair<string,double>(wpname,rho_val));

						mag_course.clear();
						mag_course[currroute].push_back(pair<string,double>(wpname,magcourse));

						rt_dist.clear();
						rt_dist[currroute].push_back(pair<string,double>(wpname,rtdist_nm));

						rtcount = 0;
					}
					else {
						if ((!sidRWFlag && procType == "SID")
								|| procType == "STAR"
								|| procType == "APPROACH"){
							if(prevseqno > seqno){
								if (wp_map[wpname].empty()){
									currroute = wpname;
									rtcount = 0;

								}
								else{
									rtcount++;
									std::stringstream ss;
									ss << rtcount;
									string stradd = ss.str();
									currroute = wpname+stradd;
								}

							}

						wp_map[currroute].push_back(wpname);

						}
						else{

							if (trans.length()){

								string lastchar = trans.substr(trans.length()-1,1);
								if (lastchar.compare(0,1,"B") == 0)
									trans.replace(trans.length()-1,1,"R");
							}
							else{
								cout<<" transition name  length = 0"<<endl;
							}
							if (procType == "SID"){
								currroute = trans+'-'+airport;
								vector<string>::iterator itr =
										find(wp_map[currroute].begin(),wp_map[currroute].end(),
												currroute );

								if (itr == wp_map[currroute].end() ){
									if (wpname.length() == 0){
										wp_map[currroute].push_back(currroute);
									}
								}
								if (wpname.length() && currroute != wpname){
									wp_map[currroute].push_back(wpname);
								}
								rtcount = 0;
							}
							else{
								cerr<<"Procedure not found.  Exception occurred"<<endl;
								exit(EXIT_SUCCESS);
							}
						}
						rt_to_trans_rttype.insert(pair<string,pair<string,string> >(currroute,trans_rttype));

						path_term[currroute].push_back(pair<string,string>(wpname,pterm));

						alt_desc[currroute].push_back(pair<string,string>(wpname,altdescstr));

						alt_1[currroute].push_back(pair<string,double>(wpname,alt1));

						alt_2[currroute].push_back(pair<string,double>(wpname,alt2));

						spd_limit[currroute].push_back(pair<string,double>(wpname,spdlim));

						recco_nav[currroute].push_back(pair<string,string>(wpname,recconavstr));

						theta[currroute].push_back(pair<string,double>(wpname,theta_val));

						rho[currroute].push_back(pair<string,double>(wpname,rho_val));

						mag_course[currroute].push_back(pair<string,double>(wpname,magcourse));

						rt_dist[currroute].push_back(pair<string,double>(wpname,rtdist_nm));


					}
					prevtrans = trans;
					prevrwy = runway;
					prevseqno = seqno;
					prevwpname = wpname;
				  }
			  }
		  }

		  myfile.close();
	  }

	 if (!wp_map.empty()) {

		 if (procType == "SID")
			 getSid(currproc, currap, wp_map,rt_to_trans_rttype,
					 path_term,alt_desc,alt_1,alt_2,spd_limit,
					 recco_nav,theta,rho,mag_course,rt_dist);
		 else if (procType == "STAR")
			 getStar(currproc, currap, wp_map,rt_to_trans_rttype,
					 path_term,alt_desc,alt_1,alt_2,spd_limit,
					 recco_nav,theta,rho,mag_course,rt_dist);
		 else if (procType == "APPROACH")
			 getApproach(currproc, currap, prevrwy, wp_map,rt_to_trans_rttype,
					 path_term,alt_desc,alt_1,alt_2,spd_limit,
					 recco_nav,theta,rho,mag_course,rt_dist);
		 else{
			 cout << __FILE__ << " (" << __LINE__ << ")";
			 printf(" Wrong procType. Exiting.\n");
			 exit(EXIT_SUCCESS);
		 }
	 }
}

void load_procs_alt(const string& CIFPfilename,
		const string& procType,
		const string& procconf,
		const FixedProc& fproc){



	/*
	 * Airport Specific stuff
	 */
	string secCode=""; string subSecCode="";
	int secCodeFld=0; int subSecCodeFld=0;
	int aptcodestart=0; int aptcodeend=0;
	int procnamestart=0; int procnameend=0;
	int rttypefld = 0;
	int transidstart=0;int transidend=0;
	int seqnostart=0; int seqnoend=0;
	int fixnamestart=0; int fixnameend=0;



	string line;
	char *token;
	ifstream configFile(procconf.c_str());
	if(configFile.is_open()){
		int gcount = 0;
		while(getline(configFile,line)){
			char *dup = strdup(line.c_str());
			token = strtok(dup, ":");
			int count = 0;
			while(token!=NULL){
				if(count == 1 && gcount==0)
					secCode = string(token);
				if (count == 1 && gcount==1){
					char *tvalue = strtok(token,",");
					int cnt =0;
					while(tvalue !=NULL){
						if( cnt==0 && procType == "SID" )
							subSecCode = string(tvalue);
						if( cnt == 1 && procType == "STAR")
							subSecCode = string(tvalue);
						if( cnt == 2 && procType == "APPROACH")
							subSecCode = string(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				if (count == 1 && gcount==2)
					secCodeFld = atoi(token);
				if (count == 1 && gcount==3)
					subSecCodeFld = atoi(token);
				if (count == 1 && gcount==4){
					char *tvalue = strtok(token,",");
					int cnt =0;
					while(tvalue !=NULL){
						if(cnt==0)
							aptcodestart = atoi(tvalue);
						if(cnt == 1)
							aptcodeend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				if (count == 1 && gcount==5){
					char *tvalue = strtok(token,",");
					int cnt =0;
					while(tvalue !=NULL){
						if(cnt==0)
							procnamestart = atoi(tvalue);
						if(cnt == 1)
							procnameend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				if (count == 1 && gcount==6){
					rttypefld  = atoi(token);
				}
				if (count == 1 && gcount==7){
					char *tvalue = strtok(token,",");
					int cnt =0;
					while(tvalue !=NULL){
						if(cnt==0)
							transidstart = atoi(tvalue);
						if(cnt == 1)
							transidend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				if (count == 1 && gcount==8){
					char *tvalue = strtok(token,",");
					int cnt =0;
					while(tvalue !=NULL){
						if(cnt==0)
							seqnostart = atoi(tvalue);
						if(cnt == 1)
							seqnoend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				if (count == 1 && gcount == 9){
					char *tvalue = strtok(token,",");
					int cnt =0;
					while(tvalue !=NULL){
						if(cnt==0)
							fixnamestart = atoi(tvalue);
						if(cnt == 1)
							fixnameend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				count++;
				token = strtok(NULL,":");
			}
			free(dup);
			gcount++;
		}
		configFile.close();
	}




	ifstream myfile (CIFPfilename.c_str());

	map< string,vector<string> > routeNames;
	map <string, pair<string,string> > rt_to_trans_rttype;
	set<string> procset;procset.clear();
	int prevseqno = 10000;
	string currroute = "";
	string currap = "";
	string currproc = "";
	string prevwpname ="";
	string prevtrans = "";
	string runway = "";
	string prevrwy = "";
	int rtcount = 0;
	if (myfile.is_open()){
		while ( getline (myfile,line) ){
			if ((line.length() > 10)
					){
				if ( (line.compare(secCodeFld-1,1,secCode) == 0) &&
						(line.compare(subSecCodeFld-1,1,subSecCode )==0) ){

					string airport = line.substr(aptcodestart-1,aptcodeend-aptcodestart+1);
					removeSpaces(airport);
					map<string,Airport*>::iterator it = rg_airports.find(airport);
					if (it == rg_airports.end())
						continue;
					string procname = line.substr(procnamestart-1,procnameend-procnamestart+1);
					removeSpaces(procname);

					//Helicoptor Routes
					if (procType =="APPROACH" && procname.compare(0,1,"H") == 0 )
						continue;

					if (airport == fproc.airport && procType == fproc.procType){
						if (procname.compare(0,strlen(fproc.procName.c_str()),fproc.procName)){
							continue;
						}
					}

					string rttype = line.substr(rttypefld-1,1);
					string wpname = line.substr(fixnamestart-1,fixnameend-fixnamestart+1);
					removeSpaces(wpname);

					string trans = line.substr(transidstart-1,transidend-transidstart+1);
					removeSpaces(trans);

					string seqnostr = line.substr(seqnostart-1,seqnoend-seqnostart+1);
					int seqno = atoi(seqnostr.c_str());

					if(procType == "APPROACH"){
						string rnum =  procname.substr(2-1,3-2+1);
						string dir = procname.substr(4-1,1);
						runway = rnum;
						if(dir != "-")
							runway += dir;
					}

					if (wpname.length() == 0 && trans.length() == 0)
						continue;

					bool sidRWFlag = false;
					if (rttype == "1" || rttype == "4" || rttype == "F" || rttype == "T")
						sidRWFlag = true;

					bool starRWFlag = false;
					if (rttype == "3" || rttype == "6" || rttype == "9" || rttype == "S")
						starRWFlag = true;

					if (wpname.length() == 0 &&
							((sidRWFlag && procType == "SID") ||
							 (starRWFlag && procType == "STAR"))
							 ){
						if ((seqno < prevseqno) && (wpname.compare(0,2,"RW") == 0)
								&& isInteger(wpname.substr(2,2))){
							wpname = trans+'-'+airport;
						}
						else{
							prevtrans = trans;
							continue;
						}
					}


					if (procType == "APPROACH"){
						if (wpname.length() == 0)
							continue;
						if (wpname.compare(0,2,"RW") == 0 &&
								isInteger(wpname.substr(2,2)) )

							wpname = wpname+'-'+airport;
					}

					if (prevseqno <= seqno){
						if(wpname == prevwpname){
							prevtrans = trans;
							continue;
						}
					}


					if (wpname.length() == 0 &&
							((!sidRWFlag && procType == "SID")
									|| (!starRWFlag && procType == "STAR")) ){

						if (trans.compare(0,2,"RW") )
							continue;

						if (trans.length()){
							string lastchar = trans.substr(trans.length()-1,1);
							if (lastchar.compare(0,1,"B") == 0)
								trans.replace(trans.length()-1,1,"R");
							wpname = trans+'-'+airport;
						}
						else{
							cout << __FILE__ << " (" << __LINE__ << ")";
							printf(" ERROR: No transition or waypoint present for ap %s and proc %s...continuing\n",
									airport.c_str(),procname.c_str());
							continue;
						}
					}


					if (currap.compare(0,currap.length(),airport)){
						if (!routeNames.empty()){


							if (procType == "SID")
								getSid(currproc, currap, routeNames,rt_to_trans_rttype);
							else if (procType == "STAR")
								getStar(currproc, currap, routeNames,rt_to_trans_rttype);
							else if (procType == "APPROACH")
								getApproach(currproc, currap, prevrwy, routeNames,rt_to_trans_rttype);
							else{
								cout << __FILE__ << " (" << __LINE__ << ")";
								printf(" Wrong procType =%s. Exiting.\n",procType.c_str());
								exit(EXIT_SUCCESS);
							}
						}
						currap = airport;
						currproc =  procname;
						currroute = wpname;
						procset.clear();
						routeNames.clear();
						rt_to_trans_rttype.clear();
						prevseqno = 10000;
						rtcount = 0;
					}


					set<string>::iterator itp = procset.find(procname);
					if (itp == procset.end() ){
						if (!routeNames.empty()){
							if (procType == "SID")
								getSid(currproc, currap, routeNames,rt_to_trans_rttype);
							else if (procType == "STAR")
								getStar(currproc, currap, routeNames,rt_to_trans_rttype);
							else if (procType == "APPROACH")
								getApproach(currproc, currap, prevrwy, routeNames,rt_to_trans_rttype);
							else{
								cout << __FILE__ << " (" << __LINE__ << ")";
								printf(" Wrong procType =%s. Exiting.\n",procType.c_str());
								exit(EXIT_SUCCESS);
							}
						}
						currproc = procname;
						currroute = wpname;
						routeNames.clear();
						rt_to_trans_rttype.clear();
						procset.insert(currproc);
						prevseqno = 10000;
						rtcount = 0;
					}

					pair<string,string> trans_rttype;
					if (trans.length())
						trans_rttype = make_pair(trans,rttype);
					else
						trans_rttype = make_pair("BLANK-FIELD",rttype);

					if ((!sidRWFlag && procType == "SID")
							|| procType == "STAR" || procType =="APPROACH"){
						if (prevseqno>seqno){
							if (routeNames[wpname].empty()){
								routeNames[wpname].push_back(wpname);
								currroute = wpname;
								prevseqno = seqno;
								rtcount = 0;
							}
							else{
								rtcount++;
								stringstream ss;
								ss << rtcount;
								string stradd = ss.str();
								currroute = wpname+stradd;
								routeNames[currroute].push_back(wpname);
								prevseqno = seqno;
							}
						}
						else{
							routeNames[currroute].push_back(wpname);
							prevseqno = seqno;
						}
						rt_to_trans_rttype.insert(pair<string,pair<string,string> >(currroute,trans_rttype));
					}
					else{
						if (trans.length()){
							string lastchar = trans.substr(trans.length()-1,1);
							if (lastchar.compare(0,1,"B") == 0)
								trans.replace(trans.length()-1,1,"R");
						}
						else{
							cout<<" transition name  length = 0"<<endl;
						}
						if (procType == "SID"){
							currroute = trans+'-'+airport;
							vector<string>::iterator itr =
									find(routeNames[currroute].begin(),
											routeNames[currroute].end(), currroute );
							if (itr == routeNames[currroute].end())
								routeNames[currroute].push_back(currroute);
							if (wpname.length() && currroute != wpname)
								routeNames[currroute].push_back(wpname);
							prevseqno = seqno;
							rtcount = 0;
						}
						else{
							cerr<<"ProcType = "<<procType<< ". Exception occurred"<<endl;
						}
						rt_to_trans_rttype.insert(pair<string,pair<string,string> >(currroute,trans_rttype));
					}
					prevwpname = wpname;
					prevtrans = trans;
					prevrwy = runway;
				}//if ( (line.compare(secCodeFld-1,1,secCode) == 0) && ...
			}// if ((line.length() > 10) && ( line.compare(1,3,"USA") == 0 ))
		}//while ( getline (myfile,line) )
		myfile.close();

	}

	//ADDING THE LAST PROC
	if (!routeNames.empty()){
		if (procType == "SID")
			getSid(currproc, currap, routeNames,rt_to_trans_rttype);
		else if (procType == "STAR")
			getStar(currproc, currap, routeNames,rt_to_trans_rttype);
		else if (procType == "APPROACH")
			getApproach(currproc, currap, prevrwy, routeNames,rt_to_trans_rttype);
		else{
			cout << __FILE__ << " (" << __LINE__ << ")";
			printf(" Wrong procType =%s. Exiting.\n",procType.c_str());
			exit(EXIT_SUCCESS);
		}
	}


#ifndef NDEBUG
	if (procType == "SID")
		cout << "Loaded " << rg_sids.size() << " sids." << endl;
	else if (procType == "STAR")
		cout << "Loaded " << rg_stars.size() << " stars." << endl;
    cout << "Loaded " << rg_fixes.size() << " fixes." << endl;
#endif
}

void star_approach_map(){
	g_star_app_map.clear();

	for (map<ProcKey, Star*>::iterator its = rg_stars.begin(); its != rg_stars.end(); ++its){
    	ProcKey key_star = its->first;
    	Star* currstar = its->second;
    	vector<string> exitpoints;
    	currstar->getExitPoints(&exitpoints);

    	for ( size_t s = 0; s < exitpoints.size(); ++s){
    		string curr_point = exitpoints.at(s);


    		string curr_app = "";string proc_branch = "";
    		pair<ProcKey,string> pair1(key_star,curr_point);
    		pair<ProcKey,string> pair2(ProcKey("",""),"");
    		for (map<ProcKey, Approach*>::iterator itv = rg_approach.begin();
    				itv != rg_approach.end(); ++itv) {
    			ProcKey key_app = itv->first;
    			if (key_star.airport != key_app.airport) continue;
    			Approach* currapp = itv->second;
    			//check approach if Runway is there
    			// Currently only using those approaches that have a
    			// landing runway specified. Otherwise
    			// runway assignment becomes very complex and can't be automated
    			if ( !check_if_runway(currapp) )
    				continue;
    			for (map<string,vector<string> >::iterator itr = currapp->waypoints.begin();
    					itr != currapp->waypoints.end(); ++itr){
    				vector<string> wplist = itr->second;
    				vector<string>::iterator itk = find(wplist.begin(),wplist.end(), curr_point);
    				if (itk != wplist.end()){
    					proc_branch = itr->first;
    					break;
    				}
    			}
    			if (proc_branch != ""){
    				pair2 = make_pair(itv->first, curr_point);
    				curr_app = currapp->name;
    				break;
    			}
    		}
    		if (proc_branch != ""){
    			g_star_app_map.insert(make_pair(pair1,pair2));
    		}
    		else{
    			Fix* star_fix = NULL; get_fix(curr_point,&star_fix);
    			if (star_fix == NULL) continue;
    			double lat1 = star_fix->getLatitude();double lon1 = star_fix->getLongitude();
    			double min_dist = MAX_DOUBLE;
    			string min_wp = "";
        		for (map<ProcKey, Approach*>::iterator itv = rg_approach.begin();
        				itv != rg_approach.end(); ++itv){
        			ProcKey key_app = itv->first;
        			if (key_star.airport != key_app.airport) continue;
        			Approach* currapp = itv->second;
        			//check approach if Runway is there
        			// Currently only using those approaches that have a
        			// landing runway specified. Otherwise
        			// runway assignment becomes very complex and can't be automated
        			if ( !check_if_runway(currapp) )
        				continue;

        			for (map<string,vector<string> >::iterator itr = currapp->waypoints.begin();
        					itr != currapp->waypoints.end(); ++itr){
        				vector<string> wplist = itr->second;
        				for(size_t idx = 0; idx < wplist.size(); ++idx){
        					size_t found1 = wplist.at(idx).find("RW");
        					size_t found2 = wplist.at(idx).find(key_star.airport);
        					if (found1 != string::npos && found2 != string::npos)
        						break;
        					Fix* appfix = NULL; get_fix(wplist.at(idx),&appfix);
        					if (appfix == NULL) continue;
        					double dist = compute_distance_gc( lat1, lon1,
        	                           appfix->getLatitude(), appfix->getLongitude() );
        					if (dist <min_dist){
        						min_dist = dist;
        						min_wp = wplist.at(idx);
        						pair2 = make_pair(itv->first,min_wp);
        					}
        				}
        			}
        		}

        		g_star_app_map.insert(make_pair(pair1,pair2));
    		}//else loop for if (proc_branch != ""){

    	}
	}
}

void remove_extra_fixes(){


	map<string,Fix*>::iterator itfix = rg_fixes.begin();


	while (itfix != rg_fixes.end()) {




		string fixname = itfix->first;
		int fixid = rg_fix_ids[fixname];



		bool deleteFlag = true;

		//Search Airways
		map<string,Airway*>::iterator itaw = rg_airways.begin();
		for(itaw = rg_airways.begin();itaw != rg_airways.end(); ++itaw){
			Airway* aw = itaw->second;

			vector<string>::iterator its = find(aw->waypointNames.begin(),aw->waypointNames.end(), fixname);
			if (its != aw->waypointNames.end()) {
				deleteFlag = false;
				break;


			}
		}

		if (!deleteFlag) {
			++itfix;
			continue;
		}

		//search Sid
		map<ProcKey,Sid*>::iterator itsid = rg_sids.begin();

		for (itsid = rg_sids.begin();itsid != rg_sids.end();++itsid){
			Sid* sid = itsid->second;

			map<string,vector<string> >::iterator itr = sid->waypoints.begin();
			for (itr = sid->waypoints.begin(); itr != sid->waypoints.end(); itr++){
				vector<string>::iterator its = find(itr->second.begin(),itr->second.end(),fixname);
				if(its != itr->second.end()){
					deleteFlag = false;
					break;
				}
			}
			if(!deleteFlag)
				break;

		}

		if (!deleteFlag){
			++itfix;
			continue;
		}


		//search star
		map<ProcKey,Star*>::iterator itstar = rg_stars.begin();

		for (itstar = rg_stars.begin();itstar != rg_stars.end();++itstar){
			Star* star = itstar->second;

			map<string,vector<string> >::iterator itr = star->waypoints.begin();
			for (itr = star->waypoints.begin(); itr != star->waypoints.end(); itr++){
				vector<string>::iterator its = find(itr->second.begin(),itr->second.end(),fixname);
				if(its != itr->second.end()){
					deleteFlag = false;
					break;
				}
			}
			if(!deleteFlag)
				break;

		}

		if (!deleteFlag){
			++itfix;
			continue;
		}

        //search approach
        map<ProcKey,Approach*>::iterator itapp = rg_approach.begin();

        for (itapp = rg_approach.begin(); itapp != rg_approach.end(); ++itapp){
            Approach* app = itapp->second;

            map<string,vector<string> >::iterator itr = app->waypoints.begin();
            for (itr = app->waypoints.begin(); itr != app->waypoints.end(); itr++){
                vector<string>::iterator its = find(itr->second.begin(), itr->second.end(), fixname);
                if(its != itr->second.end()){
                    deleteFlag = false;
                    break;
                }
            }
            if(!deleteFlag)
                break;

        }

        if (!deleteFlag){
            ++itfix;
            continue;
        }


		if (deleteFlag){
			rg_fixes.erase(itfix++);
			rg_fixes_by_id.erase(fixid);
			rg_fix_ids.erase(fixname);
			rg_fix_names.erase(fixid);
		}

	}

	int fixid = 0;
	for (map<string,Fix*>::iterator it = rg_fixes.begin(); it != rg_fixes.end();++it){
		string name = it->first;

		rg_fixes_by_id[fixid] = it->second;
		rg_fix_ids[name] = fixid;
		rg_fix_names[fixid] = name;
		fixid++;
	}


#ifndef NDEBUG
	cout << "Loaded " << rg_fixes.size() << " fixes." << endl;
	cout << "Loaded " << rg_airways.size() << " airways." << endl;
	cout << "Loaded " << rg_sids.size() << " sids." << endl;
	cout << "Loaded " << rg_stars.size() << " stars." << endl;
	cout << "Loaded " << rg_approach.size() << " approaches." << endl;
#endif

}
/*
 * THIS CODE IS EXECUTED WHEN THE FIX IS MISSING FROM ALL THE POSSIBLE
 * UPLOADED FIXES. SEARCH FOR INTERNATIONAL FIX NAMES. THIS IS JUST A FALLBACK
 * OPTION.
 */
bool getFixFromCIFP(const string &wp,
		const string &CIFPfilepath,
		string &reg,
		double &lat_v,
		double &lon_v,
		double &ele_v,
		double &freq,
		waypoint_type& wptype){
	bool exists = false;
	string line = "";
	ifstream myfile (CIFPfilepath.c_str());
	string act_name = "";
	if (myfile.is_open()) {
		/*THIS NUMBERS 4, 13, 6, 12 ... ETC ARE FIELD NUMBERS
		 * OF FIXES NAVAIDS, WAYPOINTS, AIRPORTS OBTAINED FROM
		 * CIFP FILE. JUST HARD CODED HERE AS THEY DO NOT CHANGE.
		 */
		while( getline(myfile,line) ){
			if( line.compare(4,1,"D") == 0){
				if (line.compare(13,wp.length(),wp) == 0){
					string check_false_wp = line.substr(13,wp.length()+1);
					removeSpaces(check_false_wp);
					if ( wp.length() != check_false_wp.length() )
						continue;
					act_name = line.substr(13,4);
					exists = true;
					wptype = NAVAID;
					break;
				}
			}
			else if( (line.compare(4,1,"E") == 0) && (line.compare(5,1,"A") == 0)){
				if (line.compare(13,wp.length(),wp) == 0){
					string check_false_wp = line.substr(13,wp.length()+1);
					removeSpaces(check_false_wp);
					if ( wp.length() != check_false_wp.length() )
						continue;
					act_name = line.substr(13,4);
					exists = true;
					wptype = ENROUTE;
					break;
				}
			}
			else if( (line.compare(4,1,"P") == 0) && (line.compare(12,1,"C") == 0)){
				if (line.compare(13,wp.length(),wp) == 0){
					string check_false_wp = line.substr(13,wp.length()+1);
					removeSpaces(check_false_wp);
					if ( wp.length() != check_false_wp.length() )
						continue;
					act_name = line.substr(13,4);
					exists = true;
					wptype = TERMINAL;
					break;
				}
			}
			else if ( (line.compare(4,1,"P") == 0) && (line.compare(12,1,"A") == 0)){
				if (line.compare(6,wp.length(),wp) == 0
						|| line.compare(6,wp.length()+1,"K"+wp) == 0
						|| line.compare(6,wp.length()+1,"C"+wp) == 0
						|| line.compare(6,wp.length()+1,"P"+wp) == 0){
					act_name = line.substr(6,4);
					exists = true;
					wptype = AIRPORT;
					break;
				}
			}

		}

		if (exists){
			reg = line.substr(7-1,10-7+1);
			string frequ = line.substr(23-1,27-23+1);
			freq = atof(frequ.c_str());
			string lat = line.substr(33-1,41-33+1);
			convertLatLon(lat,lat_v,true);
			string lon = line.substr(42-1,51-42+1);
			convertLatLon(lon,lon_v,false);
			string ele = line.substr(57-1,61-57+1);
			convertElev(ele,ele_v);

		}
		myfile.close();
	}

	return exists;

}

void add_fix(const string& fixname,const string& CIFPfilepath){
	double lat=0,lon=0,elev=0,freq = 0;waypoint_type wptype;
	string reg="";
	bool exists = getFixFromCIFP(fixname, CIFPfilepath, reg, lat,lon,elev, freq, wptype);
	if (exists){
		switch(wptype){
		case NAVAID:
			getNavaid(fixname,reg,lat, lon, freq,lat, lon, elev);
			break;
		case ENROUTE:
			getWaypoint(fixname, reg,lat,lon);
			break;
		case TERMINAL:
			getWaypoint(fixname, reg,lat,lon,"");
			break;
		case AIRPORT:
			getAirport(fixname, fixname, lat, lon, elev);
			break;
		default:
			cout<< wptype <<" "<< fixname;
			cout << __FILE__ << " (" << __LINE__ << ")" << endl;
			break;
		}
	}
}

void check_and_add_fixes(const string& CIFPfilepath){

	//First Airways

	for (map<string, Airway*>::iterator it = rg_airways.begin(); it != rg_airways.end(); ++it){
		Airway* aw = it->second;
		vector<string> wps = aw->waypointNames;
		for (size_t k = 0; k<wps.size(); ++k){
			Fix *ffix = NULL;
			int ids = get_fix(wps.at(k),&ffix, false,true);
			if (ids<0)
				add_fix(wps.at(k), CIFPfilepath);
		}
	}

	//Then Sids
	for (map<ProcKey, Sid*>::iterator it = rg_sids.begin(); it != rg_sids.end(); ++it){
		Sid* aw = it->second;
		map<string,vector<string> > wpmap = aw->waypoints;
		for (map<string,vector<string> >::iterator it = wpmap.begin();
				it != wpmap.end(); ++it){
			vector<string> wplist = it->second;
			for (size_t k=0; k< wplist.size(); ++k){
				Fix *ffix = NULL;
				int ids = get_fix(wplist.at(k),&ffix, false,true);
				if (ids<0)
					add_fix(wplist.at(k), CIFPfilepath);
			}
		}
	}

	//Then Stars
	for (map<ProcKey, Star*>::iterator it = rg_stars.begin(); it != rg_stars.end(); ++it){
		Star* aw = it->second;
		map<string,vector<string> > wpmap = aw->waypoints;
		for (map<string,vector<string> >::iterator it = wpmap.begin();
				it != wpmap.end(); ++it){
			vector<string> wplist = it->second;
			for (size_t k=0; k< wplist.size(); ++k){
				Fix *ffix = NULL;
				int ids = get_fix(wplist.at(k),&ffix, false,true);
				if (ids<0)
					add_fix(wplist.at(k), CIFPfilepath);
			}
		}
	}

	// Approach not done as we are only generating enroute network.

}

void load_sids(const string& sidsXmlFilename, const set<string>* const airportFilter) {
	load_procs(sidsXmlFilename, "osi.atm.datastore.beans.Sid", airportFilter);
}

void unload_rg_sids() {

	map<ProcKey, Sid*>::iterator siter;

	for (siter = rg_sids.begin(); siter != rg_sids.end(); ++siter) {
		Sid* s = siter->second;
		if (s) {
			delete s;
		}
	}

	rg_sids.clear();
}

void load_stars(const string& starsXmlFilename, const set<string>* const airportFilter) {
	load_procs(starsXmlFilename, "osi.atm.datastore.beans.Star", airportFilter);
}

void unload_rg_stars() {
	map<ProcKey, Star*>::iterator siter;

	for (siter = rg_stars.begin(); siter != rg_stars.end(); ++siter) {
		Star* s = siter->second;
		if (s) {
			delete s;
		}
	}

	rg_stars.clear();
}

template<typename T>
int find_min_dist_proc(Fix* const t_fix,const string& airport, string& procname){


	double min_dist = std::numeric_limits<double>::max();
	double t_lat = t_fix->getLatitude();
	double t_lon = t_fix->getLongitude();


	if (typeid(T) == typeid(Sid) ){

		for (map<ProcKey,Sid*>::iterator itv = rg_sids.begin();
				itv != rg_sids.end(); ++itv){
			const ProcKey* keyval = &(itv->first);
			if (keyval->airport !=airport ) continue;
			Sid *sid = itv->second;
			if ( check_proc_consistency<Sid>(sid) ) continue;
			vector<string> exitnames;exitnames.clear();
			sid->getExitPoints(&exitnames);
			for (size_t k=0;k<exitnames.size(); ++k){
				Fix* f = NULL;get_fix(exitnames.at(k),&f);
				if (f == NULL) continue;
				double f_lat = f->getLatitude();
				double f_lon = f->getLongitude();

				double dist = compute_distance_gc(t_lat,t_lon,f_lat,f_lon,0);

				if(min_dist > dist){
					min_dist = dist;
					procname = sid->name;
				}
			}


		}

	}

	else if (typeid(T) == typeid(Star) ){

		for (map<ProcKey,Star*>::iterator itv = rg_stars.begin();
				itv != rg_stars.end(); ++itv){
			const ProcKey* keyval = &(itv->first);
			if (keyval->airport !=airport ) continue;
			Star *star = itv->second;
			if ( check_proc_consistency<Star>(star) ) continue;
			vector<string> entrynames;entrynames.clear();
			star->getEntryPoints(&entrynames);
			for (size_t k=0;k<entrynames.size(); ++k){
				Fix* f = NULL;get_fix(entrynames.at(k),&f);
				if (f == NULL) continue;
				double f_lat = f->getLatitude();
				double f_lon = f->getLongitude();

				double dist = compute_distance_gc(t_lat,t_lon,f_lat,f_lon,0);

				if(min_dist > dist){
					min_dist = dist;
					procname = star->name;
				}
			}


		}

	}

	return 0;

}


int insert_sids_stars(ResultTrajs* const results){

	if (rg_sids.empty() || rg_stars.empty() ){
		printf(" Sids or Stars empty. Exiting now...\n");
		exit(-1);
		return 0;
	}

	ResultTrajs::const_iterator iter = results->begin();
	for(; iter != results->end(); ++iter){
		ResultSetKey key = iter->first;
		ResultTraj* trajs = const_cast<ResultTraj*>( &(iter->second) );
		for (ResultTraj::iterator it = trajs->begin(); it != trajs->end(); ++it){
			const FixPair* fp = &(it->first);
			vector<SearchPath>* vsp = &(it->second);
			for (size_t i = 0; i <vsp->size(); ++i){
				SearchPath* sp = &(vsp->at(i));

				//GET SID
				Fix* fix0 = NULL; int id1 = sp->getNodes().at(0);
				string airport = get_fix(id1,&fix0);
				Fix *fix1 = NULL; int id2 = sp->getNodes().at(1);
				string sid_exit = get_fix(id2,&fix1);
				string sidname = "";
				for (map<ProcKey,Sid*>::iterator itv = rg_sids.begin();
						itv != rg_sids.end(); ++itv){
					const ProcKey* keyval = &(itv->first);
					if (keyval->airport !=airport ) continue;
					Sid *sid = itv->second;
					if ( check_proc_consistency<Sid>(sid) ) continue;
					vector<string> exitnames;exitnames.clear();
					sid->getExitPoints(&exitnames);
					vector<string>::iterator its =
							find(exitnames.begin(),exitnames.end(),sid_exit);
					if (its != exitnames.end()){
						sidname = sid->name;
						break;
					}
				}

				if(sidname == ""){
					find_min_dist_proc<Sid>(fix1,airport,sidname);
				}

				//GET STAR
				Fix *fix2 = NULL; int id3 = sp->getNodes().at( (size_t)sp->size()-1 );
				airport = get_fix(id3,&fix2);
				Fix *fix3 = NULL; int id4 = sp->getNodes().at( (size_t)sp->size()-2 );
				string star_entry = get_fix(id4,&fix3);
				string starname = "";
				for (map<ProcKey,Star*>::iterator itv = rg_stars.begin();
						itv != rg_stars.end(); ++itv){
					const ProcKey* keyval = &(itv->first);
					if (keyval->airport !=airport ) continue;
					Star *star = itv->second;
					if ( check_proc_consistency<Star>(star) ) continue;
					vector<string> entrynames;entrynames.clear();
					star->getEntryPoints(&entrynames);
					vector<string>::iterator its =
							find(entrynames.begin(),entrynames.end(),star_entry);
					if (its != entrynames.end()){
						starname = star->name;
						break;
					}
				}


				if (starname == ""){
					find_min_dist_proc<Star>(fix3,airport,starname);
				}

				// Setting approach which is linked with STAR
				ProcKey keyval(starname,airport);
				string appname = "";
				for (STARAPPmap::iterator itapp = g_star_app_map.begin();
						itapp != g_star_app_map.end(); ++ itapp){
					std::pair<ProcKey,string> pair1 = itapp->first;
					if (pair1.first == keyval){
						appname = itapp->second.first.name;
						break;
					}
				}

				results->at(key).at(*fp).at(i).setSid(sidname);
				results->at(key).at(*fp).at(i).setStar(starname);
				results->at(key).at(*fp).at(i).setApproach(appname);
			}
		}
	}
	return 1;
}


int insert_sids_stars_single(ResultSets* const results){
		ResultSets::const_iterator iter = results->begin();
		for(; iter != results->end(); ++iter){
			ResultSetKey key = iter->first;
			ResultSet* sets = const_cast<ResultSet*>( &(iter->second) );
			for (ResultSet::iterator it = sets->begin(); it != sets->end(); ++it){
				const FixPair* fp = &(it->first);
				SearchPath* sp = &(it->second);

				//GET SID
				Fix* fix0 = NULL; int id1 = sp->getNodes().at(0);
				string airport = get_fix(id1,&fix0);
				Fix *fix1 = NULL; int id2 = sp->getNodes().at(1);
				string sid_exit = get_fix(id2,&fix1);
				string sidname = "";
				for (map<ProcKey,Sid*>::iterator itv = rg_sids.begin();
						itv != rg_sids.end(); ++itv){
					const ProcKey* keyval = &(itv->first);
					if (keyval->airport !=airport ) continue;
					Sid *sid = itv->second;
					if ( check_proc_consistency<Sid>(sid) ) continue;
					vector<string> exitnames;exitnames.clear();
					sid->getExitPoints(&exitnames);
					vector<string>::iterator its =
							find(exitnames.begin(),exitnames.end(),sid_exit);
					if (its != exitnames.end()){
						sidname = sid->name;
						break;
					}
				}

				if(sidname == ""){
					find_min_dist_proc<Sid>(fix1,airport,sidname);
				}
				//GET STAR
				Fix *fix2 = NULL; int id3 = sp->getNodes().at( (size_t)sp->size()-1 );
				airport = get_fix(id3,&fix2);
				Fix *fix3 = NULL; int id4 = sp->getNodes().at( (size_t)sp->size()-2 );
				string star_entry = get_fix(id4,&fix3);
				string starname = "";
				for (map<ProcKey,Star*>::iterator itv = rg_stars.begin();
						itv != rg_stars.end(); ++itv){
					const ProcKey* keyval = &(itv->first);
					if (keyval->airport !=airport ) continue;
					Star *star = itv->second;
					if ( check_proc_consistency<Star>(star) ) continue;
					vector<string> entrynames;entrynames.clear();
					star->getEntryPoints(&entrynames);
					vector<string>::iterator its =
							find(entrynames.begin(),entrynames.end(),star_entry);
					if (its != entrynames.end()){
						starname = star->name;
						break;
					}
				}


				if (starname == ""){
					find_min_dist_proc<Star>(fix3,airport,starname);
				}

				// Setting approach which is linked with STAR
				ProcKey keyval(starname,airport);
				string appname = "";
				for (STARAPPmap::iterator itapp = g_star_app_map.begin();
						itapp != g_star_app_map.end(); ++ itapp){
					std::pair<ProcKey,string> pair1 = itapp->first;
					if (pair1.first == keyval){
						appname = itapp->second.first.name;
						break;
					}
				}

				results->at(key).at(*fp).setSid(sidname);
				results->at(key).at(*fp).setStar(starname);
				results->at(key).at(*fp).setApproach(appname);
			}
		}

	return 1;
}

void load_airports(const string& airportsXmlFilename, const set<string>* const airportFilter) {


	// load the airports xml.
	ifstream in;
	in.open(airportsXmlFilename.c_str());
	if (!in.good()) return;
	stringstream xmlss;
	while (in.good()) {
		string line;
		getline(in, line);
		xmlss << trim(line);
	}
	in.close();

	// parse the xml string
	string xml = xmlss.str();
	xml_document<char> doc;
	doc.parse<0>(const_cast<char*>(xml.c_str()));

	// traverse the dom and build the Airport structs
	xml_node<> *root = doc.first_node("map");
	xml_node<> *entry = root->first_node("entry");

	while(entry) {
		xml_node<> *airport = entry->first_node("osi.atm.datastore.beans.Airport");
		xml_node<> *id = airport->first_node("id");
		xml_node<> *name = airport->first_node("name");
		xml_node<> *latitudeNode = airport->first_node("latitude");
		xml_node<> *longitudeNode = airport->first_node("longitude");
		xml_node<> *elevationNode = airport->first_node("elevation");

		string nameval = string(id->value());



		if(!airportFilter->empty()) {

			set<string>::iterator findResult = airportFilter->find(id->value());
			if(findResult == airportFilter->end()) {
				entry = entry->next_sibling();
				continue;
			}
		}

		double latitude = atof(latitudeNode->value());
		double longitude = atof(longitudeNode->value());
		double elevation = atof(elevationNode->value());

		getAirport(id->value(), name->value(), latitude, longitude, elevation);

		entry = entry->next_sibling();

	}
}


void load_airports_alt(const string& CIFPfilepath,
		const set<string>* const airportFilter,
		const string& apconfigfile) {
	/*
	 * 	 * Airport Specific stuff
	 * 	 	 */
	string secCode=""; string subSecCode="";
	int secCodeFld=0; int subSecCodeFld=0;
	int aptcodestart=0; int aptcodeend=0;
	int latstart=0;int latend=0;
	int lonstart=0;int lonend=0;
	int magvarstart=0;int magvarend=0;
	int elestart=0;int eleend=0;
	int aptnamestart=0; int aptnameend = 0;

	string line;
	char *token;
	ifstream configFile(apconfigfile.c_str());
	if (configFile.is_open()) {
		int gcount = 0;
		while (getline(configFile,line)) {
			char *dup = strdup(line.c_str());
			token = strtok(dup, ":");
			int count = 0;
			while (token!=NULL) {
				if (count == 1 && gcount == 0)
					secCode = string(token);
				if (count == 1 && gcount == 1)
					subSecCode = string(token);
				if (count == 1 && gcount == 2)
					secCodeFld = atoi(token);
				if (count == 1 && gcount == 3)
					subSecCodeFld = atoi(token);
				if (count == 1 && gcount == 4) {
					char *tvalue = strtok(token,",");
					int cnt =0;
					while (tvalue != NULL) {
						if (cnt == 0)
							aptcodestart = atoi(tvalue);
						if (cnt == 1)
							aptcodeend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				if (count == 1 && gcount==5) {
					char *tvalue = strtok(token,",");
					int cnt =0;
					while (tvalue != NULL) {
						if (cnt == 0)
							latstart = atoi(tvalue);
						if (cnt == 1)
							latend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				if (count == 1 && gcount == 6) {
					char *tvalue = strtok(token,",");
					int cnt =0;
					while (tvalue != NULL) {
						if (cnt == 0)
							lonstart = atoi(tvalue);
						if (cnt == 1)
							lonend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				if (count == 1 && gcount == 7) {
					char *tvalue = strtok(token,",");
					int cnt =0;
					while (tvalue != NULL) {
						if (cnt == 0)
							magvarstart = atoi(tvalue);
						if (cnt == 1)
							magvarend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				if (count == 1 && gcount == 8) {
					char *tvalue = strtok(token,",");
					int cnt =0;
					while (tvalue != NULL){
						if (cnt == 0)
							elestart = atoi(tvalue);
						if (cnt == 1)
							eleend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				if (count == 1 && gcount==9) {
					char *tvalue = strtok(token,",");
					int cnt =0;
					while (tvalue != NULL) {
						if (cnt == 0)
							aptnamestart = atoi(tvalue);
						if (cnt == 1)
							aptnameend = atoi(tvalue);
						cnt++;
						tvalue = strtok(NULL,",");
					}
				}
				count++;
				token = strtok(NULL,":");
			}
			free(dup);
			gcount++;
		}
	}

	ifstream myfile (CIFPfilepath.c_str());
	if (myfile.is_open()) {
		while ( getline (myfile,line) ) {
			if ((line.length() > 10)
					) {
				if ( (line.compare(secCodeFld-1, 1, secCode) == 0) &&
						(line.compare(subSecCodeFld-1, 1, subSecCode ) == 0) ) {
					string aptcode = line.substr(aptcodestart-1, aptcodeend-aptcodestart+1);
					removeSpaces(aptcode);
					if (!airportFilter->empty()) {
						set<string>::iterator findResult = airportFilter->find(aptcode);
						if (findResult == airportFilter->end())
							continue;
					}

					double lat, lon;
					string lati = line.substr(latstart-1, latend-latstart+1);
					convertLatLon(lati, lat, true);
					string longi = line.substr(lonstart-1, lonend-lonstart+1);
					convertLatLon(longi, lon, false);

					double elev;
					string eleve = line.substr(elestart-1, eleend-elestart+1);
					convertElev(eleve, elev);

					double mvar;
					string magvar = line.substr(magvarstart-1, magvarend-magvarstart+1);
					convertMagVarDec(magvar, mvar);

					string aptname = line.substr(aptnamestart-1, aptnameend-aptnamestart+1);

					getAirport(aptcode, aptname, lat, lon, elev, mvar);
				}
			}
		}

		myfile.close();
	}
}

void load_rg_airports() {
	string tmp_airport_code;

	if (map_ground_waypoint_connectivity.size() > 0) {
		map<string, GroundWaypointConnectivity>::iterator ite_map;
		for (ite_map = map_ground_waypoint_connectivity.begin(); ite_map != map_ground_waypoint_connectivity.end(); ite_map++) {
			tmp_airport_code = ite_map->first;

			NatsAirport tmp_NatsAirport;
			tmp_NatsAirport.code = tmp_airport_code;

			vector<NatsAirport>::iterator itap = find(g_airports.begin(), g_airports.end(), tmp_NatsAirport);
			if (itap != g_airports.end()) {
				tmp_NatsAirport = *itap;
				create_rg_airport(tmp_NatsAirport.code, tmp_NatsAirport.name, tmp_NatsAirport.latitude, tmp_NatsAirport.longitude, tmp_NatsAirport.elevation, tmp_NatsAirport.mag_variation);
			}
		}
	}
}

void unload_rg_airports() {
	map<string, Airport*>::iterator aiter;

	for (aiter = rg_airports.begin(); aiter != rg_airports.end(); ++aiter) {
		Airport* a = aiter->second;
		if (a) {
			delete a;
		}
	}

	rg_airports.clear();
	rg_airports_by_id.clear();
	rg_airport_ids.clear();
	rg_airport_names.clear();
}

void get_airway_connectivity(SearchGraph* const graph,
		const unsigned int& num_wind_comp,
		double(*cost_function)(const double&,const double&,const double&,const double&,
				const double&,const double&,const double&)
		) {

	if (graph == NULL) return;

	(void)cost_function;

	// make sure the airway data has been loaded
	if (validate_global_data() < 0) {
		return;
	}

	// number of nodes in the graph is number of fixes
	int numNodes = rg_fixes.size() + rg_airports.size();

	// allocate the graph member data
	// the connectivity matrix should have row/column 0,...,(m-1),m+0,...m+n.
	// where m is the number of fixes and n is  the number of airports.
	bool** connectivity = (bool**)calloc(numNodes, sizeof(bool*));

#pragma omp parallel for schedule(static, 128)
	for (int i = 0; i < numNodes; ++i) {
		connectivity[i] = (bool*)calloc(numNodes, sizeof(bool));
	}

	// create an array of default row costs,
	// initialized to MAX_DOUBLE ('infinity')
	double defaultPathCosts[numNodes];
#pragma omp parallel for schedule(static, 128)
	for (int i = 0; i < numNodes; ++i) {
		defaultPathCosts[i] = MAX_DOUBLE;
	}

	// create and initialize path costs to 'infinity'
	double** pathCosts = (double**)calloc(numNodes, sizeof(double*));
#pragma omp parallel for schedule(static, 128)
	for (int i = 0; i < numNodes; ++i) {
		pathCosts[i] = (double*)calloc(numNodes, sizeof(double));
		memcpy(pathCosts[i], defaultPathCosts, numNodes*sizeof(double));
	}

	// create and initialize heuristics to 0
	double* heuristicCosts = (double*)calloc(numNodes, sizeof(double));

	// locals for computing average edge cost
	double maxCost = MIN_DOUBLE;
	double sum = 0;
	double count = 0;

	// iterate over airways and add a connection between sequential
	// fixes in the airway.  only consider airways that start with origin
	// and end with destination
	map<string, Airway*>::iterator iter;
	for (iter = rg_airways.begin(); iter != rg_airways.end(); ++iter) {
		Airway* airway = iter->second;

		for (unsigned int i = 1; i < airway->waypointNames.size(); ++i) {
			string fixname1 = airway->waypointNames[i];
			int fixid1 = -1;
			Fix* fix1 = NULL;
			fixid1 = get_fix(fixname1, &fix1);

			string fixname0 = airway->waypointNames[i-1];

			int fixid0 = -1;
			Fix* fix0 = NULL;
			fixid0 = get_fix(fixname0, &fix0);

			if (fix0 == NULL) {
				continue;
			}
			if (fix1 == NULL) {
				continue;
			}

			if (fixid0 < 0) continue;
			if (fixid1 < 0) continue;

			if (connectivity[fixid0][fixid1] == 1) continue;
			if (connectivity[fixid1][fixid0] == 1) continue;

			double alt = 30000; // TODO: use actual altitude instead of default

			double cost = compute_distance_gc(fix0->getLatitude(),
					                          fix0->getLongitude(),
					                          fix1->getLatitude(),
					                          fix1->getLongitude(),
					                          alt);

			// assign array values
			heuristicCosts[fixid0] = 0; // to be assigned later
			heuristicCosts[fixid1] = 0; // to be assigned later
			connectivity[fixid0][fixid1] = 1;
			connectivity[fixid1][fixid0] = 1;
			pathCosts[fixid0][fixid1] = cost;
			pathCosts[fixid1][fixid0] = cost;

			sum += cost;
			count++;
		} // end - for
	} // end - for

	// build the lookup grid
	LookupGrid grid(rg_fixes);

#if ENABLE_SID_STAR
	// iterate over airports.  for each airport, iterate over sids
	// for that airport and obtain the list of sid endpoints.
	// for each sid endpoint, if the endpoint is in the global
	// map of waypoints then add a connection from the airport to
	// the endpoint.  if the endpoint is not in the global map then
	// skip it.  if an airport has no sids then connect the airport
	// to the nearest waypoint in the global map.
	int indexOffset = rg_fixes.size();
	map<string, Airport*>::iterator aiter;
	for (aiter = rg_airports.begin(); aiter != rg_airports.end(); ++aiter) {
		Airport* airport = aiter->second;

		if (airport == NULL) {
			continue;
		}

		string apname = airport->id;

		int apid = -1;//g_airport_ids[fixname1];
		map<string, int>::iterator fiter = rg_airport_ids.find(apname);
		if (fiter == rg_airport_ids.end()) {
			stringstream ss;
			ss << "ERROR: get_airway_connectivity() - "
				  "could not find airport for name " << apname;
			log_error(ss.str());

			continue;
		}
		
		apid = fiter->second;
		if (apid < 0) continue;

		apid += indexOffset;

		// iterate over global sids until we come to one for the
		// airport.  there may be several.
		map<ProcKey, Sid*>::iterator siter;
		int sidCount = 0;
		for (siter = rg_sids.begin(); siter != rg_sids.end(); ++siter) {
			const ProcKey* key = &(siter->first);
			if (key->airport == airport->id) {
				Sid* sid = siter->second;
				if (sid == NULL) continue;

				sidCount++;

				// get the sid exit points and iterate over them
				vector<string> endpoints;
				sid->getExitPoints(&endpoints);
				for (unsigned int i = 0; i < endpoints.size(); ++i) {
					string endpoint = endpoints.at(i);
					map<string, Fix*>::iterator fiter;
					fiter = rg_fixes.find(endpoint);
					if (fiter != rg_fixes.end()) {
						Fix* fix = fiter->second;

						string fixname = fix->getName();
						int fixid = -1;
						map<string, int>::iterator iter = rg_fix_ids.find(fixname);
						if (iter == rg_fix_ids.end()) {
							stringstream ss;
							ss << "ERROR: get_airway_connectivity() - "
								  "could not find fix for name " << fixname;
							log_error(ss.str());

							continue;
						}
						fixid = iter->second;
						if (fixid < 0) continue;

						// add a connection from airport to fix.
						// unlike the airway connections, which are
						// bi-directional, we make this connection
						// uni-directional from the airport to the fix.
						if (connectivity[apid][fixid] == 1) continue;

						double alt = DEFAULT_ALTITUDE_FT; // TODO: use actual altitude instead of default
						double cost = compute_distance_gc(airport->getLatitude(),
								                          airport->getLongitude(),
								                          fix->getLatitude(),
								                          fix->getLongitude(),
								                          alt);

#ifndef NDEBUG
						cout << "add connection from airport " << apname << " to " << fixname << " for sid " << sid->name << endl;
#endif

						// assign array values
						heuristicCosts[apid] = 0; // to be assigned later
						heuristicCosts[fixid] = 0; // to be assigned later
						connectivity[apid][fixid] = 1;
						pathCosts[apid][fixid] = cost;

						sum += cost;
						count++;
					}
				} // end - for
			}
		} // end - for

		// if there were no sids for the airport then connect
		// airport to the nearest global fix
		if (sidCount == 0) {
			vector<Fix*>* cellFixes = &(grid.getFixes(airport->latitude, airport->longitude));

			vector<Fix*>::iterator citer;

			double dmin = std::numeric_limits<double>::max();
			int fixidMin = -1;
			string fixnameMin = "";
			for (citer = cellFixes->begin(); citer != cellFixes->end(); ++citer) {
				string fixname = (*citer)->getName();

				int fixid = rg_fix_ids[fixname];
				Fix* fix = rg_fixes[fixname];
				if (fix == NULL) continue;
				map<string, int>::iterator fiter = rg_fix_ids.find(fixname);
				if (fiter == rg_fix_ids.end()) {
					stringstream ss;
					ss << "ERROR: get_airway_connectivity() - "
						  "could not find fix for name " << fixname;
					log_error(ss.str());

					continue;
				}
				fixid = fiter->second;
				if (fixid < 0) continue;
				fix = rg_fixes[fixname];

				double alt = DEFAULT_ALTITUDE_FT; // TODO: use actual altitude instead of default
				double cost = compute_distance_gc(fix->getLatitude(),
						                          fix->getLongitude(),
						                          airport->getLatitude(),
						                          airport->getLongitude(),
						                          alt);
				if (cost < dmin) {
					dmin = cost;
					//nearestFix = fix;
					fixidMin = fixid;
					fixnameMin = fixname;
				}
			}

			if (connectivity[apid][fixidMin] == 1) continue;

#ifndef NDEBUG
			cout << "no sid info, connect " << apname << " (" << apid << ") to " << fixnameMin << " (" << fixidMin << ") , dmin=" << dmin << endl;
#endif
			// assign array values
			heuristicCosts[apid] = 0; // to be assigned later
			heuristicCosts[fixidMin] = 0; // to be assigned later
			connectivity[apid][fixidMin] = 1;
			pathCosts[apid][fixidMin] = dmin;

			sum += dmin;
			count++;
		} // end - if

		// now do stars

		// iterate over global stars until we come to one for the
		// airport.  there may be several.
		map<ProcKey, Star*>::iterator stiter;
		int starCount = 0;
		for (stiter = rg_stars.begin(); stiter != rg_stars.end(); ++stiter) {
			const ProcKey* key = &(stiter->first);
			if (key->airport == airport->id) {
				Star* star = stiter->second;
				if (star == NULL) continue;

				starCount++;

				// get the sid exit points and iterate over them
				vector<string> entrypoints;
				star->getEntryPoints(&entrypoints);
				for (unsigned int i=0; i<entrypoints.size(); ++i) {
					string entrypoint = entrypoints.at(i);
					map<string, Fix*>::iterator fiter;
					fiter = rg_fixes.find(entrypoint);
					if (fiter != rg_fixes.end()) {
						Fix* fix = fiter->second;

						string fixname = fix->getName();
						int fixid = -1;
						map<string, int>::iterator iter = rg_fix_ids.find(fixname);
						if (iter == rg_fix_ids.end()) {
							stringstream ss;
							ss << "ERROR: get_airway_connectivity() - "
								  "could not find fix for name " << fixname;
							log_error(ss.str());

							continue;
						}
						fixid = iter->second;

						if (fixid < 0) continue;

						// add a connection from airport to fix.
						// unlike the airway connections, which are
						// bi-directional, we make this connection
						// uni-directional from the airport to the fix.
						if (connectivity[fixid][apid] == 1) continue;

						double alt = 30000; // TODO: use actual altitude instead of default
						double cost = compute_distance_gc(airport->getLatitude(),
								                          airport->getLongitude(),
								                          fix->getLatitude(),
								                          fix->getLongitude(),
								                          alt);

#ifndef NDEBUG
						cout << "add connection from airport " << apname << " to " << fixname << " for star " << star->name << endl;
#endif

						// assign array values
						heuristicCosts[apid] = 0; // to be assigned later
						heuristicCosts[fixid] = 0; // to be assigned later
						connectivity[fixid][apid] = 1;
						pathCosts[fixid][apid] = cost;

						sum += cost;
						count++;
					}
				} // end - for
			}
		} // end - for

		// if there were no sids for the airport then connect
		// airport to the nearest global fix
		if (starCount == 0) {
			vector<Fix*>* cellFixes = &(grid.getFixes(airport->latitude, airport->longitude));

			vector<Fix*>::iterator citer;

			double dmin = std::numeric_limits<double>::max();
			int fixidMin = -1;
			string fixnameMin = "";
			for (citer=cellFixes->begin(); citer!=cellFixes->end(); ++citer) {
				string fixname = (*citer)->getName();
				int fixid = rg_fix_ids[fixname];
				Fix* fix = rg_fixes[fixname];

				if (fix == NULL) continue;

				map<string, int>::iterator fiter = rg_fix_ids.find(fixname);

				if (fiter == rg_fix_ids.end()) {
					stringstream ss;
					ss << "ERROR: get_airway_connectivity() - "
						  "could not find fix for name " << fixname;
					log_error(ss.str());

					continue;
				}

				fixid = fiter->second;
				if (fixid < 0) continue;

				fix = rg_fixes[fixname];

				double alt = 30000; // TODO: use actual altitude instead of default
				double cost = compute_distance_gc(fix->getLatitude(),
						                          fix->getLongitude(),
						                          airport->getLatitude(),
						                          airport->getLongitude(),
						                          alt);
				if (cost < dmin) {
					dmin = cost;
					//nearestFix = fix;
					fixidMin = fixid;
					fixnameMin = fixname;
				}
			}

			if (connectivity[fixidMin][apid] == 1) continue;

#ifndef NDEBUG
			cout << "no star info, connect " << apname << " (" << apid << ") to " << fixnameMin << " (" << fixidMin << ") , dmin=" << dmin << endl;
#endif
			// assign array values
			heuristicCosts[apid] = 0; // to be assigned later
			heuristicCosts[fixidMin] = 0; // to be assigned later
			connectivity[fixidMin][apid] = 1;
			pathCosts[fixidMin][apid] = dmin;

			sum += dmin;
			count++;
		}
	}
#else
	// iterate over airports and connect each to the nearest
	// airway node
	//int indexOffset = g_fixes.size();
	map<string, Airport*>::iterator aiter;
	for (aiter=g_airports.begin(); aiter!=g_airports.end(); ++aiter) {
		Airport* airport = aiter->second;
		if(airport == NULL) continue;

		vector<Fix*>* cellFixes = &(grid.getFixes(airport->latitude, airport->longitude));

		GridKey key = grid.getKey(airport->latitude, airport->longitude);

		if (cellFixes->size() < 1) continue;

		string fixname1 = airport->id;
		int fixid1 = -1;
		Fix* fix1;
		fixid1 = get_fix(fixname1, &fix1);

		if(fix1 == NULL) {
			cout << __FILE__ << " (" << __LINE__ << ")" << endl;
			exit(-1);
			continue;
		}
		if(fixid1 < 0) continue;

		int i = fixid1;// + indexOffset;

		vector<Fix*>::iterator citer;

		double dmin = std::numeric_limits<double>::max();
		int j = -1;
		string fixnameMin = "";
		for(citer=cellFixes->begin(); citer!=cellFixes->end(); ++citer) {
			string fixname0 = (*citer)->getName();
			int fixid0 = -1;
			Fix* fix0 = NULL;
			fixid0 = get_fix(fixname0, &fix0);

			if(fix0 == NULL) {
				cout << __FILE__ << " (" << __LINE__ << ")" << endl;
				exit(-1);
				continue;
			}

			double alt = 30000; // TODO: use actual altitude instead of default
			double cost = compute_distance_gc(fix0->getLatitude(),
					                          fix0->getLongitude(),
					                          airport->getLatitude(),
					                          airport->getLongitude(),
					                          alt);
			if(cost < dmin) {
				dmin = cost;

				j = fixid0;
				fixnameMin = fixname0;
			}
		}

		if(connectivity[j][i] == 1) continue;
		if(connectivity[i][j] == 1) continue;

		// assign array values
		heuristicCosts[j] = 0; // to be assigned later
		heuristicCosts[i] = 0; // to be assigned later
		connectivity[j][i] = 1;
		connectivity[i][j] = 1;
		pathCosts[j][i] = dmin;
		pathCosts[i][j] = dmin;

		sum += dmin;
		count++;
	}
#endif

	double avgCost = sum / count;
	(void)avgCost;
	maxCost = 2. * avgCost;

#ifndef NDEBUG
	cout << "average cost: " << avgCost << endl;
	cout << "max cost: " << maxCost << endl;
	cout << "numNodes: " << numNodes << endl;
#endif

	// create the search graph
	graph->set(numNodes,
			connectivity,
			pathCosts,
			heuristicCosts,
			NULL,
			maxCost,
			num_wind_comp);

	// cleanup local mem
	for (int i = 0; i < numNodes; ++i) {
		free(connectivity[i]);
		free(pathCosts[i]);
	}

	free(connectivity);
	free(pathCosts);
	free(heuristicCosts);
}


void reduce_airway_connectivity(SearchGraph& graph,
		                        const vector<Polygon>& polygons,
		                        vector<Fix*>* const removedFixes) {

	if (polygons.size() < 1) return;

	map<int, SearchNode*>* nodes =
			const_cast<map<int, SearchNode*>*>(&(graph.getNodes()));
	map<int, SearchNode*>::iterator iter;


	// iterate over each node in the graph and see if it is in
	// a polygon.  do computations in xy space.
	// if it is in a polygon then add it to the list of nodes to remove


	for (iter = nodes->begin(); iter != nodes->end(); ++iter) {
		SearchNode* node = iter->second;

		int id = node->id;

		Fix* fix = (id >= (int)rg_fixes.size() ? rg_airports_by_id.at(id - rg_fixes.size()) : rg_fixes_by_id.at(id));

		for (unsigned int j = 0; j < polygons.size(); ++j) {

			const Polygon* poly = &(polygons.at(j));
			const string met_type = poly->getPolyType();
			const int st_hr = poly->getStartHour();
			const int end_hr =poly->getEndHour();




			if (poly->contains(fix->getLongitude(), fix->getLatitude(), true)) {
				// fix is in polygon, remove it.

				size_t found = met_type.find("AIRMET");
				if (found != string::npos)
#if USE_MULTIPLIER
					graph.removeNode(id,g_multiplier,true,st_hr,end_hr);
#else
					graph.removeNode(id,g_max_cost,false,st_hr,end_hr);
#endif
				else
					graph.removeNode(id, MAX_DOUBLE,false,st_hr,end_hr);

				if (removedFixes)
					removedFixes->push_back(fix);
			} else {

				// fix is not in polygon.  check for segment
				// intersection with the polygon
				SearchNode* node = graph.getNode(id);
				multiset<SearchNode*, SearchNodeCostComparator>::iterator citer;

				for (citer = node->children.begin();
						citer != node->children.end();
						++citer) {


					double x1 = fix->getLongitude();
					double y1 = fix->getLatitude();

					SearchNode* child = *citer;
					Fix* childFix = (child->id >= (int)rg_fixes.size() ? rg_airports_by_id.at(child->id - rg_fixes.size()) : rg_fixes_by_id.at(child->id));

					double x2 = childFix->getLongitude();
					double y2 = childFix->getLatitude();

					bool intersects =
							poly->intersectsSegment(x1,y1,x2,y2,
													NULL, NULL, true);


					if (intersects) {
						size_t found = met_type.find("AIRMET");
						if (found != string::npos) {

							if (graph.isMultiplied(node->id,child->id) )
								continue;

#if USE_MULTIPLIER
							graph.setEdgeCost(node->id,child->id
									,g_multiplier*graph.getEdgeCost(node->id,child->id));

							vector<double> eCV =
									graph.getEdgeCostVector(node->id,child->id);
							if (eCV.size() > 1) {
								for (size_t viter = st_hr;
										viter < MIN(eCV.size(),(size_t) end_hr);
										++viter)
									eCV.at(viter) = g_multiplier * eCV.at(viter);
							}

							graph.setMultiplied(node->id,child->id,true);
#else
							graph.setEdgeCost(node->id,child->id,g_max_cost);

							vector<double> eCV = graph.getEdgeCostVector(node->id,child->id);
							for (unsigned int viter = st_hr;
									viter < MIN(eCV.size(),(unsigned int)end_hr);
									++viter)
								eCV.at(viter) = g_max_cost;
#endif


							graph.setEdgeCostVector(node->id,child->id,eCV);
						}//if (found != string::npos){
						else {
							graph.setEdgeCost(node->id,child->id,MAX_DOUBLE);
							vector<double> eCV = graph.getEdgeCostVector(node->id,child->id);
							if (eCV.size() > 1) {
								for (unsigned int viter = st_hr;
										viter < MIN(eCV.size(),(unsigned int) end_hr);
										++viter)
									eCV.at(viter) = MAX_DOUBLE;
							}
							graph.setEdgeCostVector(node->id,child->id,eCV);
						}//else
						if (removedFixes) {
							removedFixes->push_back(childFix);
						}//if(removedFixes)
					}//if(intersects)
				}//for(citer=node->children.begin();
			}//else {
		}//for(unsigned int j=0; j<polygons.size(); ++j) {
	}//for(iter=nodes->begin(); iter!=nodes->end(); ++iter) {
}


void processGraphWithPirep(SearchGraph& graph,PIREPset* const g_pirep,
							const size_t& wind_vec_size){

	if (g_pirep->size() < 1) return;

	map<int, SearchNode*>* nodes =
			const_cast<map<int, SearchNode*>*>(&(graph.getNodes()));
	map<int, SearchNode*>::iterator iter = nodes->begin();
	// FIXME:HARD CODED VALUES CAN BE ENTERED THROUGH CONFIG FILE.
	size_t sz = 22;
	double RAD = 1;//in nmi unlike the other places in the code where units are in feet
	for (iter = nodes->begin(); iter != nodes->end(); ++iter) {


		SearchNode* node = iter->second;
		int id = node->id;
		Fix * fix= NULL;
		get_fix(id, &fix);
		for (size_t s = 0; s < g_pirep->size(); ++s) {

			PIREP p = g_pirep->at(s);

			//FIXME:HARD CODED.CAN BE VARIED
			if (p.getUrgentFlag())
				RAD = 20;
			else
				RAD = 10;

			int st_hr = 0, end_hr = 0;
			bool nflag = false;
			if (wind_vec_size == 0 ) {
				nflag = p.nearFlag(fix->getLatitude(),fix->getLongitude(),RAD);
				st_hr = 0;
				end_hr = 24;
			}
			else {
				for (int tm = 0; tm < (int)sz; ++tm) {
					bool flg  = p.nearFlag(fix->getLatitude(),fix->getLongitude(),RAD,tm);
					if (flg && !nflag){
						nflag = true;
						st_hr = tm;
					}
					if (nflag && !flg){
						nflag = false;
						end_hr = tm;
						break;
					}
				}
			}

			if (st_hr != end_hr && st_hr != 0) {
				if (p.getWeatherPhenom() == "LLWS")
					graph.removeNode(id,numeric_limits<double>::max(),false, st_hr ,end_hr);
				else {
					int sev = p.getSeverity();
					double mult = 1;
					//FIXME: MODIFY THESE HARD CODED MULTIPLIERS AS USER INPUTS.
					if (sev == 1)
						mult = 1.5;
					else if (sev == 2)
						mult = 2;
					else if (sev == 3)
						mult = 3;
					graph.removeNode(id,mult,true, st_hr ,end_hr);
				}
			}
			//TODO REMOVE FOR THESE TIMES ST_HR AND END_HR

		}
	}

}

int get_fix_names(vector<string>* const names) {
	if(!names) return -1;
	map<string, Fix*>::iterator iter;
	for (iter = rg_fixes.begin(); iter != rg_fixes.end(); ++iter) {
		names->push_back(iter->first);
	}
	return 0;
}

int get_fix_location(const int& id, double* const lat, double* const lon) {
	if(!lat) return -1;
	if(!lon) return -1;
	Fix* fix = NULL;
	if(id >= (int)rg_fixes.size()) {
		fix = rg_airports_by_id.at(id-rg_fixes.size());
	} else {
		fix = rg_fixes_by_id.at(id);
	}
	if(!fix) return -1;
	*lat = fix->getLatitude();
	*lon = fix->getLongitude();
	return 0;
}

int get_fix_location(const string& name, double* const lat, double* const lon) {
	if(!lat) return -1;
	if(!lon) return -1;
	Fix* fix = NULL;
	map<string, Fix*>::iterator iter;
	iter = rg_fixes.find(name);
	if(iter == rg_fixes.end()) {
		map<string, Airport*>::iterator aiter;
		aiter = rg_airports.find(name);
		if(aiter != rg_airports.end()) {
			fix = aiter->second;
		}
	} else {
		fix = iter->second;
	}

	if(!fix) return -1;
	*lat = fix->getLatitude();
	*lon = fix->getLongitude();
	return 0;
}

int get_fix_name(const int& id, string* const name) {
	if(!name) return -1;
	Fix* fix = (id>=(int)rg_fixes.size() ? rg_airports_by_id.at(id-rg_fixes.size()) : rg_fixes_by_id.at(id));
	if(!fix) return -1;
	*name = fix->getName();
	return 0;
}

int get_fix_id(const string& name, int* const id) {
	if(!id) return -1;

	int fixid = -1;
	map<string, Fix*>::iterator iter;
	iter = rg_fixes.find(name);
	if(iter == rg_fixes.end()) {
		map<string, Airport*>::iterator aiter;
		aiter = rg_airports.find(name);
		if(aiter != rg_airports.end()) {
			fixid = rg_airport_ids[name];
		}
	} else {
		fixid = rg_fix_ids[name];
	}

	if(fixid < 0) return -1;
	*id = fixid;

	return 0;
}

int get_airway_names(vector<string>* const names) {
	if(!names) return -1;
	map<string, Airway*>::iterator iter;
	for(iter=rg_airways.begin(); iter!=rg_airways.end(); ++iter) {
		names->push_back(iter->first);
	}
	return 0;
}

int get_airway_fix_names(const string& name, vector<string>* const fix_names) {
	if(!fix_names) return -1;
	Airway* airway = rg_airways.at(name);
	if(!airway) return -1;
	fix_names->insert(fix_names->begin(),
			          airway->waypointNames.begin(),
			          airway->waypointNames.end());
	return 0;
}

int get_airway_fix_altitudes(const string& name,
		                     vector<double>* const min_alt1,
		                     vector<double>* const min_alt2,
		                     vector<double>* const max_alt) {
	if(!min_alt1) return -1;
	Airway* airway = rg_airways.at(name);
	if(!airway) return -1;
	min_alt1->insert(min_alt1->begin(),
			         airway->minAltitude1.begin(),
			         airway->minAltitude1.end());
	if(min_alt2) {
		min_alt2->insert(min_alt2->begin(),
				         airway->minAltitude2.begin(),
				         airway->minAltitude2.end());
	}
	if(max_alt) {
		max_alt->insert(max_alt->begin(),
				         airway->maxAltitude.begin(),
				         airway->maxAltitude.end());
	}
	return 0;
}

int get_num_result_sets(const ResultSets& result_sets) {
	return result_sets.size();
}



const ResultSet* get_result_set(const ResultSetKey& key,
		                        const ResultSets& result_sets) {
	return &result_sets.at(key);
}


int get_result_set_size(const ResultSet* const result_set) {
	if(!result_set) return -1;
	return result_set->size();
}



int get_result_value(const ResultSet* const result_set,
		             const FixPair& key, SearchPath* const value) {

	if(!value) return -1;
	if(!result_set) return -1;
	if(result_set->find(key) == result_set->end()) return -1;
	SearchPath* path = const_cast<SearchPath*>(&(result_set->at(key)));
	value->insert(value->begin(), path->begin(), path->end());
	value->setSid( path->getSid() );value->setStar( path->getStar() );
	value->setApproach( path->getApproach() );
	return 0;
}

int get_result_value_TOS(const ResultTraj* const result_trajs,
		             const FixPair& key, vector<SearchPath>* const tvalue) {
	if(!tvalue) return -1;
	if(!result_trajs) return -1;
	if(result_trajs->find(key) == result_trajs->end()) return -1;

	vector<SearchPath>* paths = const_cast<vector<SearchPath>* >(&(result_trajs->at(key)));
	for(unsigned int s=0;s<paths->size();++s){
		SearchPath* path = &(paths->at(s));
		tvalue->push_back(*path);
	}
	return 0;
}

int get_polygon_sets_keys(const PolygonSets& polygon_sets,
		                  vector<PolygonSetKey>* const keys) {
	if(!keys) return -1;
	PolygonSets::const_iterator iter;
	for(iter=polygon_sets.begin(); iter!=polygon_sets.end(); ++iter) {
		keys->push_back(iter->first);
	}
	return 0;
}

const PolygonSet* get_polygon_set(const PolygonSetKey& key,
		                          const PolygonSets& polygon_sets) {
	return &(polygon_sets.at(key));
}

const PolygonSet* get_polygon_set(const int& scenario, const double& scale,
		                          const PolygonSets& polygon_sets) {
	return &(polygon_sets.at(PolygonSetKey(scenario, scale)));
}

} /* end namespace osi */
