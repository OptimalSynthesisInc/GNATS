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

#include "NatsDataLoader.h"
#include "NatsSector.h"
#include "NatsWaypoint.h"
#include "NatsAirport.h"

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

using std::cout;
using std::endl;
using std::vector;
using std::string;

using std::lower_bound;

void load_sectors(NatsDataLoader& loader) {
  string fname = "/home/jason/projects/fastsim/data/nats/usa_high.pol.crypt";
  vector<NatsSector> sectors;
  int numSectors = loader.loadSectors(fname, &sectors);
  cout << "Loaded " << numSectors << " sectors." << endl;
  cout << "First sector: " << sectors.at(0).name << endl;
  cout << "Last sector: " << sectors.at(sectors.size()-1).name << endl;
}

void load_waypoints(NatsDataLoader& loader) {
  string fname = "/home/jason/projects/fastsim/data/nats/Waypoints.crypt";
  vector<NatsWaypoint> waypoints;
  int numWaypoints = loader.loadWaypoints(fname, &waypoints);
  cout << "Loaded " << numWaypoints << " waypoints." << endl;
  cout << "First waypoint: " << waypoints.at(0).name << endl;
  cout << "Last waypoint: " << waypoints.at(waypoints.size()-1).name << endl;
}

void load_airports(NatsDataLoader& loader) {
  string fname = "/home/jason/projects/fastsim/data/nats/Airports.crypt";
  vector<NatsAirport> airports;
  int numAirports = loader.loadAirports(fname, &airports);
  cout << "Loaded " << numAirports << " airports." << endl;
  cout << "First airport: " << airports.at(0).code << endl;
  cout << "Last airport: " << airports.at(airports.size()-1).code << endl;

  NatsAirport ksfo;
  ksfo.code = "KSFO";
  vector<NatsAirport>::iterator it = lower_bound(airports.begin(), airports.end(), ksfo);
  cout << "airport=" << it->code << ", lat=" << it->latitude << ", lon=" << it->longitude << endl;
}

void load_airways(NatsDataLoader& loader) {
  string fname = "/home/jason/projects/fastsim/data/nats/Airways.crypt";
  vector<NatsAirway> airways;
  int numAirways = loader.loadAirways(fname, &airways);
  cout << "Loaded " << numAirways << " airways." << endl;
  cout << "First airway: " << airways.at(0).name << endl;
  cout << "Last airway: " << airways.at(airways.size()-1).name << endl;
}

void load_sid_stars(NatsDataLoader& loader) {
  string fname = "/home/jason/projects/fastsim/data/nats/sid_star.crypt";
  vector<NatsSid> sids;
  vector<NatsStar> stars;
  int numSidStars = loader.loadSidStars(fname, &sids, &stars);
  cout << "Loaded " << numSidStars << " sids and stars." << endl;
  cout << "Loaded " << sids.size() << " sids." << endl;
  cout << "Loaded " << stars.size() << " stars." << endl;
  if(sids.size() > 0) {
    cout << "First sid: " << sids.at(0).name << endl;
    cout << "Last sid: " << sids.at(sids.size()-1).name << endl;
  }
  if(stars.size() > 0) {
    cout << "First star: " << stars.at(0).name << endl;
    cout << "Last star: " << stars.at(stars.size()-1).name << endl;
  }

  NatsSid key;
  key.name = "PORTE3.WAGES";
  vector<NatsSid>::iterator iter = lower_bound(sids.begin(),
						sids.end(), key);
  if(iter != sids.end()) {
    NatsSid* sid = const_cast<NatsSid*>(&(*iter));
    cout << "SID: " << sid->name << endl;
    for(unsigned int i=0; i<sid->waypoints.size(); ++i) {
      string wp = sid->waypoints.at(i);
      double lat = sid->latitudes.at(i);
      double lon = sid->longitudes.at(i);
      cout << "   " << wp << "[" << lat << "," << lon << "]" << endl;
    }
  }

  NatsStar starkey;
  starkey.name = "FMG.GOLDN4";
  vector<NatsStar>::iterator jter = lower_bound(stars.begin(),
						 stars.end(), starkey);
  if(jter != stars.end()) {
    NatsStar* star = const_cast<NatsStar*>(&(*jter));
    cout << "STAR: " << star->name << endl;
    for(unsigned int i=0; i<star->waypoints.size(); ++i) {
      string wp = star->waypoints.at(i);
      double lat = star->latitudes.at(i);
      double lon = star->longitudes.at(i);
      cout << "   " << wp << "[" << lat << "," << lon << "]" << endl;
    }
  }
}

void load_pars(NatsDataLoader& loader) {
  string fname = "/home/jason/projects/fastsim/data/nats/PAR/zoa_par.crypt";
  vector<NatsPar> pars;
  int numPars = loader.loadPars(fname, &pars);
  cout << "Loaded " << numPars << " pars." << endl;
  cout << "First par: " << pars.at(0).name << ", " << pars.at(0).identifier << endl;
  cout << "Last par: " << pars.at(pars.size()-1).name << ", " << pars.at(pars.size()-1).identifier << endl;
}

int main(int argc, char* argv[]) {

  (void)argc;
  (void)argv;

  NatsDataLoader loader;

  load_airports(loader);
  
  cout << "Good bye." << endl;
  return 0;
}
