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
 * rg_api.h
 *
 *  Created on: May 5, 2012
 *      Author: jason
 */

#ifndef RG_API_H_
#define RG_API_H_

#include "NatsAirport.h"
#include "NatsDataLoader.h"

#include "SearchGraph.h"
#include "SearchPath.h"
#include "FixPair.h"
#include "Polygon.h"
#include "PIREP.h"
#include "path_find.h"
#include "pub_WaypointNode.h"

#include "Fix.h"

#include "wind_api.h"

#include <string>
#include <set>
#include <vector>
#include <map>

using std::set;
using std::vector;
using std::map;
using std::string;


namespace osi {

extern string rg_output_directory;
extern WindGrid rg_wind;
extern vector<WindGrid> rg_wind_vec;

extern int rg_supplemental_airway_seq;

#define FeetToMeters 0.3048
#define MetersToFeet 3.2808399
#define KnotsToMetersPerSec 0.514444444
#define KnotsToFeetPerSec 1.68780986
#define MilesToFeet 5280
#define nauticalMilesToFeet 6076.11549

typedef enum _tos_gen_method {
  REM_LOW_COST=0,
  REM_LOW_COST_PER_LENGTH=1,
  REM_ENTIRE_PATH=2
} tos_gen_method;

typedef enum _waypoint_type {
	NAVAID = 'n',
	ENROUTE = 'e',
	TERMINAL = 't',
	AIRPORT = 'a'
}waypoint_type;

// new addition to fix procedure type in all TOS
typedef struct FixedProc{
	string airport;
	string procType;
	string procName;
} FixedProc;

static const FixedProc DEFAULT_FIXED_PROC = {"","",""};

double gc_distance_cost_function(const double& lat1, const double& lon1, const double& alt1,
			                     const double& lat2, const double& lon2, const double& alt2,
			                     const double& spd=0);

double wind_optimal_cost_function(const double& lat1, const double& lon1, const double& alt1,
                                  const double& lat2, const double& lon2, const double& alt2,
                                  const double& spd,
                                  WindGrid* const g_wind = NULL,
                                  vector<WindGrid>* const g_wind_vec = NULL
                                  );
vector<double> wind_optimal_cost_function(const double& lat1, const double& lon1, const double& alt1,
                                  const double& lat2, const double& lon2, const double& alt2,
                                  const double& spd,
                                  vector<WindGrid>* const g_wind_vec);

class ResultSetKey {
public:
	ResultSetKey(int scenario, double scale);
	ResultSetKey(const ResultSetKey& that);
	virtual ~ResultSetKey();
	bool operator<(const ResultSetKey& that) const;
	bool operator==(const ResultSetKey& that) const;
	ResultSetKey& operator=(const ResultSetKey& that);
	int scenario;
	double scale;
};
typedef ResultSetKey PolygonSetKey;

/**
 * A vector of Polygons
 */
typedef vector< Polygon> PolygonSet;

/**
 * A vector of vector of Polygons
 */
typedef vector< PolygonSet > PolygonLists;
typedef map<PolygonSetKey, PolygonSet> PolygonSets;

/**
 * A vector of mappings from FixPair to SearchPaths.
 * The ResultSet contains the path results for a particular weather
 * scenario and is keyed by scale fPolygonSetsactor.
 */
typedef map<FixPair, SearchPath> ResultSet;

/**
 * A vector of vector of mappings from FixPair to SearchPaths
 */
typedef map<ResultSetKey, ResultSet> ResultSets;


/**
 * TODO: Parikshit adder
 * A mapping of FixPairs to Trajectory options.
 * ResultTraj contains trajectory options keyed by fix pairs and
 */
typedef map<FixPair, vector<SearchPath> > ResultTraj;

/*
 *A map of Result Set Keys to ResultTrajs
 */
typedef map<ResultSetKey,ResultTraj> ResultTrajs;

/*
 * vector of PIREPS
 */
typedef vector<PIREP> PIREPset;

/*
 * TODO: PARIKSHIT ADDER
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *	 ConvertLatLon()
 *
 * Description:
 * 	 Converts lat lon in CIFP file to lat lon in decimal form.
 *	Inputs:
 *	   Inpstr: 		Input String
 *	   val:  		Lat/Lon value
 *	   latFlag:		Latitude or Longitude
 *
 *	Outputs:
 *		None
 */
void convertLatLon(const string& inpstr, double& val,bool latFlag);

/*
 * TODO: PARIKSHIT ADDER
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *	 ConvertElev()
 *
 * Description:
 * 	 Converts elevation in CIFP file to elevation in decimal form.
 *	Inputs:
 *	   Inpstr: 		Input String
 *	   val:  		Lat/Lon value
 *
 *	Outputs:
 *		None
 */
void convertElev(const string& inpstr, double& val);
/*
 * TODO: PARIKSHIT ADDER
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *	 convertMagVarDec()
 *
 * Description:
 * 	 Converts magnetic variation in CIFP file to magnetic variation in decimal form.
 *	Inputs:
 *	   Inpstr: 		Input String
 *	   val:  		Lat/Lon value
 *
 *	Outputs:
 *		None
 */
void convertMagVarDec(const string &inp_str,double &val);
/*
 * TODO: PARIKSHIT ADDER
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *	 removeSpaces()
 *
 * Description:
 * 	 Removes blank spaces from the string
 *	Inputs:
 *	   Inpstr: 		Input String
 *
 *
 *	Outputs:
 *		None
 */
void removeSpaces(string& input);

/**
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   get_paths()
 *
 * Description:
 *   Find paths from origin fix/airport to destination fix/airport
 *   for each of the specified origin-destination pairs
 *
 *   This function can be used to compute the shortest paths for m
 *   weather scenarios each consisting of n weather polygons.
 *
 * Inputs:
 *   origDestPairs     an nx2 matrix of strings that specify the airport
 *                     pairs to compute routes for.  for example:
 *                     string pairs[4][2] = {{"KSFO","KATL"},
 *                                           {"KATL","KSFO"},
 *                                           {"KSFO","KJFK"},
 *                                           {"KJFK","KSFO"}};
 *   numPairs          the number of pairs of orig_dest_pairs.  in the
 *                     above example, num_pairs=4.
 *   graph             nodes that define the network search graph
 *   weatherScenarios  sets of weather scenario polygons.
 *                     points in a polygon will be removed from the search
 *                     space.  Polygon vertices are expected to be defined
 *                     as lat/lon pairs where the longitude is the xdata and
 *                     latitude is the ydata.
 *   scalingFactors    vector of scaling factors by which each polygon will
 *                     be scaled.  this vector should NOT include the
 *                     scale factor of 1.0.
 *   replanFlag        true: compute route alternatives starting from origin
 *                     false: compute route alternatives starting from
 *                     polygon entry point
 *
 * In/Out:
 *   results         pointer to a ResultSets object.
 *   polysOut        pointer to a PolygonSets object
 *
 * Returns:
 *   0 on success, -1 on error.
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
// deprecated: should be private.
int get_paths(SearchGraph& graph,
		      const string origDestPairs[][2],
		      const int& numPairs,
		      const PolygonLists& weatherScenarios=PolygonLists(),
		      const double scalingFactors[]=NULL,
		      const int& numFactors=0,
		      ResultSets* const results=NULL,
		      PolygonSets* const polysOut=NULL,
		      const bool& replanFlag=true);

/**
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   get_paths()
 *
 * Description:
 *   Find all paths from origin fix/airport to destination fix/airport
 *   in the specified TRX file given the graph specified by the set of nodes.
 *   Paths will be found which avoid the specified polygons.
 *
 *   This function can be used to compute the shortest paths for m
 *   weather scenarios each consisting of n weather polygons.
 *
 * Inputs:
 *   origin            origin fix/airport.  example: "KSFO"
 *   destination       destination fix/airport.  example: "KDFW"
 *   trxfile           path to a TRX file
 *   graph             nodes that define the network search graph
 *   weatherScenarios  sets of weather scenario polygons.
 *                     points in a polygon will be removed from the search
 *                     space.  Polygon vertices are expected to be defined
 *                     as lat/lon pairs where the longitude is the xdata and
 *                     latitude is the ydata.
 *   scalingFactors    vector of scaling factors by which each polygon will
 *                     be scaled.  this vector should NOT include the
 *                     scale factor of 1.0.
 *
 * In/Out:
 *   paths         pointer to vector of paths defined as deque<int>.
 *                 each path is defined by a sequence of search node ids
 *
 * Returns:
 *   0 on success, -1 on error.
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
int get_paths(SearchGraph& graph,
		      const char* trxfile,
		      const PolygonLists& weatherScenarios=PolygonLists(),
		      const double scalingFactors[]=NULL,
		      const int& numFactors=0,
		      ResultSets* const results=NULL,
		      PolygonSets* const polysOut=NULL,
		      const bool& replanFlag=true);

/**
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   get_paths()
 *
 * Description:
 *   Find all paths from origin fix/airport to destination fix/airport
 *   in the specified TRX file given the graph specified by the set of nodes.
 *   Paths will be found which avoid the specified polygons.
 *
 *   This function can be used to compute the shortest paths for m
 *   weather scenarios each consisting of n weather polygons.
 *
 * Inputs:
 *   airportPairs      set of airport pairs
 *   trxfile           path to a TRX file
 *   graph             nodes that define the network search graph
 *   weatherScenarios  sets of weather scenario polygons.
 *                     points in a polygon will be removed from the search
 *                     space.  Polygon vertices are expected to be defined
 *                     as lat/lon pairs where the longitude is the xdata and
 *                     latitude is the ydata.
 *   scalingFactors    vector of scaling factors by which each polygon will
 *                     be scaled.  this vector should NOT include the
 *                     scale factor of 1.0.
 *
 * In/Out:
 *   paths         pointer to vector of paths defined as deque<int>.
 *                 each path is defined by a sequence of search node ids
 *
 * Returns:
 *   0 on success, -1 on error.
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
int get_paths(SearchGraph& graph,
	          const set<FixPair>& airportPairs,
	          const PolygonLists& weatherScenarios=PolygonLists(),
	          const double scalingFactors[]=NULL,
	          const int& numFactors=0,
	          ResultSets* const results=NULL,
	          PolygonSets* const polysOut=NULL,
	          const bool& replanFlag=true);

int get_wind_optimal_paths(SearchGraph& graph,
		  const vector<FixPair>& flights,
	      WindGrid* const g_wind,
	      vector<WindGrid>* const g_wind_vec,
	      const PolygonLists& weatherScenarios=PolygonLists(),
	      const double scalingFactors[]=NULL,
	      const int& numFactors=0,
	      ResultSets* const results=NULL,
	      PolygonSets* const polysOut=NULL,
	      const bool& replanFlag=true);

int get_wind_optimal_TOS(SearchGraph& graph,
		  const vector<FixPair>& flights,
	      WindGrid* const g_wind,
	      vector<WindGrid>* const g_wind_vec,
		  PIREPset* const g_pirep,
	      const PolygonLists& weatherScenarios=PolygonLists(),
	      const double scalingFactors[]=NULL,
	      const int& numFactors=0,
	      ResultTrajs* const results=NULL,
	      PolygonSets* const polysOut=NULL,
	      const int& num_trajs = 5,
	      const bool& replanFlag=true,
	      const tos_gen_method& t_method = REM_LOW_COST,
		  const FixedProc& fproc = DEFAULT_FIXED_PROC,
		  const int& offset = 1);


//THIS IS ONLY FOR NATS REROUTING
int weatherAvoidanceRoutesForNATS(const PolygonSet& weatherScenariosList,
		const vector<double>& lats_deg,
		const vector<double>& lons_deg,
		vector<double>& lat_reroute_deg,
		vector<double>& lon_reroute_deg,
		vector<pair<int,int> >& similar_idx_wpts);

//Returns intersecting polygons within nmi_rad on the way
void find_intersecting_polygons(const double& lat_deg, const double& lon_deg, const double& alt_ft, const double& nmi_rad,
				const vector<double>& fp_lat_deg,
				const vector<double>& fp_lon_deg,
				const PolygonSet& weatherpolygons,
				PolygonSet& polygons_ahead);

/**
 * Parse airport pairs from the trx file
 */
int parse_trx_airport_pairs(const string& trxfile, set<FixPair>* const pairs);

/**
 * Parse flights from trx and mfl files for wind-optimal routing
 */
int parse_trx_flights(const string& trxfile, const string& mflfile, vector<FixPair>* const flights);

/**
 * Detect new trx format which includes mfl
 */
bool is_new_trx_format(const string& trxfile);

void supplement_airways_by_flightPlans_logic(waypoint_node_t* waypoint_node_ptr);

/**
 * Load airways from the specified xml file.  This will populate the
 * global maps of airways and fixes.
 */
void load_airways(const string& airwaysXmlFilename);

/**
 * Load airways from the specified CIFP file.  This will populate the
 * global maps of airways and fixes.
 */
void load_airways_alt(const string& CIFPfilename, const string& routeconf);

void load_airways_data(const string& CIFPfilename);

/**
 * Load Fixes from the specified CIFP file.  This will populate the
 * global maps of fixes.
 */
int createFix(const string& fixname,const string& CIFPfilename,
		const string& fixconf = "../CIFPParser/config/enrtfix.config",
		const bool termFlag = false,
		const string& airport="");

/**
 * Load Fixes from the specified CIFP file.  This will populate the
 * global maps of fixes before loading airways.
 */
void load_fixes(const string& CIFPfilename,
		const string& fixconf = "../CIFPParser/config/enrtfix.config",
		const set<string>* const airportFilter=NULL,
		const set<string>* const nameFilter=NULL);


/**USE THIS ONE THIS IS THE LATEST ONE
 * Load Procs from the specified CIFP file.  This will populate the
 * global maps of procs.
 */
void load_procs_alt(const string& CIFPfilename,
		const string& procType = "SID",
		const FixedProc& fproc = DEFAULT_FIXED_PROC);

/**DEPRECATED
 * Load Procs from the specified CIFP file.  This will populate the
 * global maps of procs.
 */
void load_procs_alt(const string& CIFPfilename,
		const string& procType = "SID",
		const string& procconf= "../CIFPParser/config/Proc.config",
		const FixedProc& fproc = DEFAULT_FIXED_PROC);

/**
 * Create Star to approach map
 */

void star_approach_map();

/**
 * Remove extra nodes that are not needed
 */

void remove_extra_fixes();

/**
 * Seaches the CIFP file for FIX not present in initial load. (FALL BACK OPTION)
 */
bool getFixFromCIFP(const string &wp, const string &CIFPfilepath, string &reg,
		double &lat_v,	double &lon_v,	double &ele_v,	double &freq,	waypoint_type& wptype);
/**
 * Add just one fix not present
 */

void add_fix(const string& fixname,const string& CIFPfilepath);
/**
 * Add fixes not present
 */

void check_and_add_fixes(const string &CIFPfilepath);


/**
 * Free memory that was dynamically allocated using load_airways().
 */
void unload_rg_airways();

/**
 * Load sids from the specified xml file.  This will populate the
 * global maps of sids and fixes.
 */
void load_sids(const string& sidsXmlFilename, const set<string>* const airportFilter=NULL);

/**
 * Free memory that was dynamically allocated using load_sids().
 */
void unload_rg_sids();

/**
 * Load stars from the specified xml file.  This will populate the
 * global maps of stars and fixes.
 */
void load_stars(const string& starsXmlFilename, const set<string>* const airportFilter=NULL);

/**
 * Free memory that was dynamically allocated using load_stars().
 */
void unload_rg_stars();

/**
 * If Sid or star not found find min dist sid star
 */

template<typename T>
int find_min_dist_proc(const string& trans_wp,const string& airport, string& procname);

/**
 * Insert Sid and Stars corresponding to the first and last waypoints, respectively
 * in a result trajectory.
 */
int insert_sids_stars(ResultTrajs* const results);

/**
 * Insert Sid and Stars corresponding to the first and last waypoints, respectively
 * in resultSets.
 */
int insert_sids_stars_single(ResultSets* const results);

/**
 * Load airports from the specified xml file.  This will populate the
 * global maps of airports.
 */
void load_airports(const string& airportsXmlFilename, const set<string>* const airportFilter=NULL);

/**
 * TODO:PARIKSHIT ADDER
 * Load airports from the specified CIFP file.  This will populate the
 * global maps of airports.
 */
void load_airports_alt(const string& CIFPfilename, const set<string>* const airportFilter=NULL,
		const string& configfile = "../config/airport.config");

void load_rg_airports();

void create_rg_airport(const string& airportCode,
        const string& name,
        const double& latitude,
        const double& longitude,
        const double& elevation,
		const double& magvar);

/**
 * Free memory that was dynamically allocated using load_airports().
 */
void unload_rg_airports();

/**
 * Get the SearchGraph corresponding to the airway connectivity.
 * Input args are the SearchGraph to store connectivity and the
 * function pointer that implements the link cost function.  the default
 * cost function is the great circle distance.  The cost function must
 * take as arguments the lat/lon/alt of 2 nodes
 */
void get_airway_connectivity(SearchGraph* const graph=NULL,
		const unsigned int& num_wind_comp = 22,
		double(*cost_function)(const double&,const double&,const double&,const double&,
				const double&,const double&,const double&)=gc_distance_cost_function);

/**
 * Remove search nodes that lie within a polygon from the search graph
 */
void reduce_airway_connectivity(SearchGraph& graph,
		                      const vector<Polygon>& polygons=vector<Polygon>(),
		                      vector<Fix*>* const removedFixes=NULL);


//Incorporating PIREPs into the graph
void processGraphWithPirep(SearchGraph& graph,PIREPset* const g_pirep,
							const size_t& wind_vec_size);

////////////////////////////////////////////////////////////////////////////////
// Global Fix Functions

/**
 * Convenience functions to get the location, name, or id of a fix
 * from the global maps of fixes
 */

/**
 * Get the list of all fix names
 */
int get_fix_names(vector<string>* const names);

/**
 * Get the location of the fix with the specified id
 */
int get_fix_location(const int& id, double* const lat, double* const lon);

/**
 * Get the location of the fix with the specified name
 */
int get_fix_location(const string& name, double* const lat, double* const lon);

/**
 * Get the name of the fix with the specified id
 */
int get_fix_name(const int& id, string* const name);

/**
 * Get the id of the fix with the specified name
 */
int get_fix_id(const string& name, int* const id);

/**
 * Get the fix pointer
 */
int get_fix(const string& name, Fix** const fix, const bool& forceNames=false,
		const bool &testPhase = false);

////////////////////////////////////////////////////////////////////////////////
// Global Airway Functions

/**
 * Convenience functions to get the name, fixes and fix altitudes
 */

/**
 * Get the list of all airway names
 */
int get_airway_names(vector<string>* const names);

/**
 * Get the list of fix names for the airway with the specified name
 */
int get_airway_fix_names(const string& name, vector<string>* const fix_names);

/**
 * Get the lists of fix altitudes for the airway with the specified name
 */
int get_airway_fix_altitudes(const string& name,
		                     vector<double>* const min_alt1,
		                     vector<double>* const min_alt2=NULL,
		                     vector<double>* const max_alt=NULL);

////////////////////////////////////////////////////////////////////////////////
// Global ResultSet processing functions

/**
 * Convenience functions to process a ResultSets object
 */

/**
 * Get the number of ResultSet objects in the specified set of ResultSets
 */
int get_num_result_sets(const ResultSets& result_sets);

/**
 * Get the vector of keys for the ResultSets
 * Changed to a templated function
 */
template<class setType>
int get_result_sets_keys(const setType& result_sets,
		                 vector<ResultSetKey>* const keys) {
	if(!keys) return -1;
	typename setType::const_iterator it;
	for(it=result_sets.begin(); it!=result_sets.end(); ++it) {
		keys->push_back(it->first);
	}
	return 0;
}
/**
 * Get the ith ResultSet from the specified set of ResultSets
 */
const ResultSet* get_result_set(const ResultSetKey& key,
		                        const ResultSets& result_sets);
template<class setType, class retType>
const retType* get_result_set(const int& scenario, const double& scale,
		                        const setType& result_sets) {
	typename setType::const_iterator it;
	it = result_sets.find(ResultSetKey(scenario, scale));
	if(it == result_sets.end()) return NULL;
	return &result_sets.at(ResultSetKey(scenario, scale));
}

/**
 * Get the number of hashmaps of FixPair to SearchPath in the
 * specified ResultSet
 */
int get_result_set_size(const ResultSet* const result_set);

/**
 * Get the list of keys to the mappings for the specified ResultSet.
 * This returns the vector of scale factors
 */
template<class setType>
int get_result_keys(const setType* const result_set,
		            vector<FixPair>* const keys) {
	if(!keys) return -1;
	if(!result_set) return -1;
	typename setType::const_iterator iter;
	for(iter=result_set->begin(); iter!=result_set->end(); ++iter) {
		keys->push_back(iter->first);
	}
	return 0;
}

/**
 * Get the value of the mapping for the specified ResultSet's key
 */
int get_result_value(const ResultSet* const result_set,
		             const FixPair& key, SearchPath* const value);

/**
 * Get the value of the mapping for the specified ResultTraj's key
 */
int get_result_value_TOS(const ResultTraj* const result_trajs,
		             const FixPair& key, vector<SearchPath>* const value);

/**
 * Get the list of polygon set keys
 */
int get_polygon_sets_keys(const PolygonSets& polygon_sets,
		                  vector<PolygonSetKey>* const keys);
/**
 * Get the polygon set at the specified scenario and scale
 */
const PolygonSet* get_polygon_set(const PolygonSetKey& key,
		                          const PolygonSets& polygon_sets);
const PolygonSet* get_polygon_set(const int& scenario, const double& scale,
		                          const PolygonSets& polygon_sets);

}

#endif /* RG_API_H_ */
