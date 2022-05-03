#include "tg_api_wrapper.h"
#include "tg_trajectory.h"
#include "tg_api.h"
#include "FlightPlan.h"

#include <string>
#include <map>
#include <vector>

using namespace std;

/*
 * Print tg version info.
 */
int info() {
  return tg_info();
}

/*
 * Initialize the tg module
 */
int initialize(const std::string& data_dir,
	       const real_t& cruise_spd_perturbation,
	       const int& device_id) {
  return tg_init(data_dir, cruise_spd_perturbation, device_id);
}

/*
 * Load wind from the specified directory
 */
int load_wind(const std::string& wind_dir) {
  return tg_load_rap(wind_dir);
}

/*
 * Load flights from the specified TRX and MFL file
 */
int load_flights(const std::string& trx_file,
		 const std::string& mfl_file) {
  return tg_load_trx(trx_file, mfl_file);
}

/*
 * Run the TG to generate output trajectories
 */
int generate(const long& t_horizon_sec,
	     const long& t_step_sec,
	     std::vector<Trajectory>* const OUTPUT) {
  if(!OUTPUT) return -1;
  int err = tg_generate(t_horizon_sec, t_step_sec);
  OUTPUT->clear();
  OUTPUT->insert(OUTPUT->begin(),
		 g_trajectories.begin(),
		 g_trajectories.end());
  return err;
}

/*
 * Reset flight states to their initial states
 */
int reset_flights() {
  return tg_reset_aircraft();
}

/*
 * Reset the simulation to initial conditions (t=0)
 */
int reset() {
  return tg_reset();
}

/*
 * Free system resources
 */
int shutdown() {
  return tg_shutdown();
}

/*
 * Write the trajectories to HDF5
 */
int write_trajectories(const std::string& fname,
		       const std::vector<Trajectory>& trajectories) {
  return tg_write_trajectories(fname, trajectories);
}

/*
 * Load the trajectories from HDF5
 */
int read_trajectories(const std::string& fname,
		      std::vector<Trajectory>* const OUTPUT) {
  return tg_read_trajectories(fname, OUTPUT);
}

/*
 * Get a target-language wrapped list of trajectories
 */
int get_trajectories(std::vector<Trajectory>* const OUTPUT) {
  return tg_get_trajectories(OUTPUT);
}

/*
 * Return the number of flights
 */
int get_num_flights() {
  int num_flights;
  tg_get_num_flights(&num_flights);
  return num_flights;
}

/*
 * Get a target-language wrapped list of flightplans
 */
int get_flightplans(std::map<int, FlightPlan>* const OUTPUT) {
  return tg_get_flightplans(OUTPUT);
}

/*
 * Get a copy of the specified flight plan
 */
FlightPlan get_flightplan(const int& flight_index) {
  FlightPlan fp;
  tg_get_flightplan(flight_index, &fp);
  return fp;
}

/*
 * Return the cruise airspeed of the requested flight
 */
real_t get_cruise_tas(const int& flight_index) {
  real_t tas;
  tg_get_cruise_tas(flight_index, &tas);
  return tas;
}

/*
 * Return the cruise altitude of the requested flight
 */
real_t get_cruise_altitude(const int& flight_index) {
  real_t alt;
  tg_get_cruise_altitude(flight_index, &alt);
  return alt;
}

/*
 * Return the origin airport of the requested flight
 */
string get_origin_airport(const int& flight_index) {
  string ap;
  tg_get_origin_airport(flight_index, &ap);
  return ap;
}

/*
 * Return the origin elevation of the requested flight
 */
real_t get_origin_elevation(const int& flight_index) {
  real_t alt;
  tg_get_origin_elevation(flight_index, &alt);
  return alt;
}

/*
 * Return the destination airport of the requested flight
 */
string get_destination_airport(const int& flight_index) {
  string ap;
  tg_get_destination_airport(flight_index, &ap);
  return ap;
}

/*
 * Return the destination elevation of the requested flight
 */
real_t get_destination_elevation(const int& flight_index) {
  real_t alt;
  tg_get_destination_elevation(flight_index, &alt);
  return alt;
}

/*
 * Return the aircraft type of the requested flight
 */
string get_aircraft_type(const int& flight_index) {
  string actype;
  tg_get_aircraft_type(flight_index, &actype);
  return actype;
}
