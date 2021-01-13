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
 * AdbOPFModel.h
 *
 *  Created on: Nov 27, 2012
 *      Author: jason
 *
 *  Update: 02.03, 2020
 *          Oliver Chen
 */

#ifndef ADBOPFMODEL_H_
#define ADBOPFMODEL_H_

#include <string>
#include <map>
#include <iostream>
#include "adb.h"

using std::string;
using std::map;
using std::ostream;

namespace osi {

class AdbOPFModel {
public:
    AdbOPFModel();
    virtual ~AdbOPFModel();

    // Adb Operational Performance Data (OPF)

    // Aircraft Type Block
    // data line 1:
    string type;
    int numEngines;
    adb_engine_type_e engineType;
    adb_wake_category_e wakeCategory;

    // Mass Block
    // data line 2:
    double mref;      // reference mass [pounds] (converted from us tons in file)
    double mmin;      // minimum mass [pounds] (converted from us tons in file)
    double mmax;      // maximum mass [pounds] (converted from us tons in file)
    double mpyld;     // maximum payload [pounds] (converted from us tons in file)
    double gw;        // mass gradient on hw [ft/kg]

    // Flight Envelope Block (altitudes in ft)
    // data line 3:
    double vmo;       // vmo max operating speed (cas knots)
    double mmo;       // mmo max operating mach [dimensionless]
    double hmo;       // max operating altitude (ft)
    double hmax;      // hmax (ft) max altitude at MTOW and ISA
    double gt;        // temperateure gradient on max alt [ft/Kelvin]

    // Aerodynamics Block
    // data line 4: Wing area and Buffet Coefficients (SIM)
    int ndrst;        // num drag settings, unused
    double s;         // Surface area [ft^2] (converted from m^2 in file)
    double clbo;      // Clbo [dimensionless]
    double k;         // k [dimensionless] buffeting gradient (jets only)
    double cm16;      // CM16 ???

    // data lines 5-9: Configuration Characteristics
    map<adb_flight_phase_e, string> configName;
    map<adb_flight_phase_e, double> vstall;
    map<adb_flight_phase_e, double> cd0;
    map<adb_flight_phase_e, double> cd2;

    // data lines 10-11: Spoiler states, unused by BADA 3.9
    double cdSpoilerRet[2];// = {0,0};    // spoiler retracted drag increment
    double cdSpoilerExt[2];// = {0,0};    // spoiler extended drag increment

    // data lines 12-13: Gear states
    double cdGearUp[2];// = {0,0};        // unused by BADA 3.9
    double cdGearDown[2];// = {0,0};      // gear down drag increment

    // data lines 14-15: Brakes on/off, unused by BADA 3.9
    double cdBrakesOff[2];// = {0,0};
    double cdBrakesOn[2];// = {0,0};

    // Engine Thrust Block
    // data line 16: max climb thrust coefficients (sim)
    double ctc1;  // 1st climb thrust coef [Newton] for jet/piston, [knot-Newton] for turboprop
    double ctc2;  // 2nd climb thrust coef [ft]
    double ctc3;  // 3rd climb thrust coef [1/ft^2] for jet, [Newton] for turboprop, [knot-Newton] for piston
    double ctc4;  // 1st thrust tem coef [K]
    double ctc5;  // 2nd thrust temp coef [1/K]

    // data line 17: cruise/descent thrust
    double ctdesLow;
    double ctdesHigh;
    double hpdes;
    double ctdesApp;
    double ctdesLd;

    // data line 18: ref speeds during descent
    double vdesRef;  // descent cas
    double mdesRef;  // descent mach

    // Fuel Consumption Block
    // data line 19: fuel consumption coefficients
    double cf1;
    double cf2;

    // data line 20: descent fuel flow
    double cf3;
    double cf4;

    // data line 21: cruise fuel flow correction factor
    double cfcr;

    // Ground Movement Block
    // data line 22: ground movements
    double tol;     // takeof length [ft] converted from [m] in file
    double ldl;     // landing length [ft] converted from [m] in file
    double span;    // wingspan [ft] converted from [m] in file
    double length;  // length [ft] converted from [m] in file

    friend ostream& operator<<(ostream& out, const AdbOPFModel& o);
};



}

#endif /* ADBOPFMODEL_H_ */
