/*
 * tg_api_wrapper.h
 *
 * SWIG friendly API for tg
 */

#ifndef __TG_API_WRAPPER_H__
#define __TG_API_WRAPPER_H__

#include "tg_trajectory.h"
#include "FlightPlan.h"

#include <string>
#include <map>
#include <vector>

using namespace std;


/*
 * Print tg version info.
 */
int info();

/*
 * Initialize the tg module
 */
int initialize(const std::string& data_dir,
	       const real_t& cruise_spd_perturbation=0,
	       const int& device_id=0);

/*
 * Load wind from the specified directory
 */
int load_wind(const std::string& wind_dir);

/*
 * Load flights from the specified TRX and MFL file
 */
int load_flights(const std::string& trx_file,
		 const std::string& mfl_file);

/*
 * Run the TG to generate output trajectories
 */
int generate(const long& t_horizon_sec,
	     const long& t_step_sec,
	     std::vector<Trajectory>* const OUTPUT);

/*
 * Reset flight states to their initial states
 */
int reset_flights();

/*
 * Reset the simulation to initial conditions (t=0)
 */
int reset();

/*
 * Free system resources
 */
int shutdown();

/*
 * Write the trajectories to HDF5
 */
int write_trajectories(const std::string& fname,
		       const std::vector<Trajectory>& trajectories);

/*
 * Load the trajectories from HDF5
 */
int read_trajectories(const std::string& fname,
		      std::vector<Trajectory>* const OUTPUT);

/*
 * Return a copy of the trajectories in a target-language array
 */
int get_trajectories(std::vector<Trajectory>* const OUTPUT);

/*
 * Return the number of flights
 */
int get_num_flights();

/*
 * Return a copy of the flight plans in a target-language array
 */
int get_flightplans(std::map<int, FlightPlan>* const OUTPUT);

/*
 * Return a copy of the flight plan object for the requested flight
 */
FlightPlan get_flightplan(const int& flight_index);

/*
 * Return the cruise airspeed of the requested flight
 */
real_t get_cruise_tas(const int& flight_index);

/*
 * Return the cruise altitude of the requested flight
 */
real_t get_cruise_altitude(const int& flight_index);

/*
 * Return the origin airport of the requested flight
 */
string get_origin_airport(const int& flight_index);

/*
 * Return the origin elevation of the requested flight
 */
real_t get_origin_elevation(const int& flight_index);

/*
 * Return the destination airport of the requested flight
 */
string get_destination_airport(const int& flight_index);

/*
 * Return the destination elevation of the requested flight
 */
real_t get_destination_elevation(const int& flight_index);

/*
 * Return the aircraft type of the requested flight
 */
string get_aircraft_type(const int& flight_index);


#endif  /* __TG_API_WRAPPER_H__ */
