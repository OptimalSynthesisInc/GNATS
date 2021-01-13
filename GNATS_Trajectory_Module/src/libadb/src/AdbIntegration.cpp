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
 * AdbIntegration.cpp
 *
 * Integration utils.
 *
 *  Created on: Jan 24, 2013
 *      Author: jason
 *
 *  Update: 02.03, 2020
 *          Oliver Chen
 */

#include "AdbIntegration.h"

#include <algorithm>
#include <vector>
#include <cmath>
#include "AdbPTFModel.h"

using std::vector;
using std::lower_bound;


namespace osi {

  AdbIntegration::AdbIntegration() {
  }

  AdbIntegration::~AdbIntegration() {
  }

  double AdbIntegration::simpsons(const double& a,
				   const double& b,
				   const double& f_a,
				   const double& f_mid,
				   const double& f_b) {
    return ((b-a)/6.) * (f_a + 4.*f_mid + f_b);
  }

  double AdbIntegration::rodVsXdot(const double& x,
				    const double& h,
				    const AdbPTFModel& ac) {

    // obtain non-const iterators to the altitude vector
    // because std::lower_bound won't work with const_iterator
    AdbPTFModel* ptr = const_cast<AdbPTFModel*>(&ac);
    vector<double>::iterator begin = ptr->altitudes.begin();
    vector<double>::iterator end = ptr->altitudes.end();

    // std::lower_bound returns an iterator that points to the first
    // element in the sorted range [first,last) which does not
    // compare LESS THAN target value using the operator<.
    // this will point to first occurrance of the target value, 
    // or the position it would be inserted.
    int rowUpper, rowLower;
    vector<double>::iterator iter;
    iter = lower_bound(begin, end, h);
    rowUpper = iter - begin;
    if(rowUpper >= (int)ac.altitudes.size()) rowUpper = ac.altitudes.size()-1;
    rowLower = (rowUpper == 0 ? 0 : rowUpper-1);

    double altLower = ac.altitudes.at(rowLower);
    double altUpper = ac.altitudes.at(rowUpper);
    double vLower = ac.getDescentTas(altLower);
    double vUpper = ac.getDescentTas(altUpper);
    double dx = ac.getDescentDistance(altUpper);
    double dv = vUpper-vLower;
    double v = (vLower + x*dv/dx) * AdbPTFModel::KNOTS_TO_FEET_PER_MIN;
    double hdot = ac.getDescentRate(h);
    double xdot = sqrt(v*v - hdot*hdot);

    return (hdot / xdot);
  }

  double AdbIntegration::rocVsXdot(const double& x,
				    const double& h,
				    const AdbPTFModel& ac) {

    // obtain non-const iterators to the altitude vector
    // because std::lower_bound won't work with const_iterator
    AdbPTFModel* ptr = const_cast<AdbPTFModel*>(&ac);
    vector<double>::iterator begin = ptr->altitudes.begin();
    vector<double>::iterator end = ptr->altitudes.end();

    // std::lower_bound returns an iterator that points to the first
    // element in the sorted range [first,last) which does not
    // compare LESS THAN target value using the operator<.
    // this will point to first occurrance of the target value, 
    // or the position it would be inserted.
    int rowUpper, rowLower;
    vector<double>::iterator iter;
    iter = lower_bound(begin, end, h);
    rowUpper = iter - begin;
    if(rowUpper >= (int)ac.altitudes.size()) rowUpper = ac.altitudes.size()-1;
    rowLower = (rowUpper == 0 ? 0 : rowUpper-1);

    double altLower = ac.altitudes.at(rowLower);
    double altUpper = ac.altitudes.at(rowUpper);
    double vLower = ac.getClimbTas(altLower);
    double vUpper = ac.getClimbTas(altUpper);
    double dx = ac.getClimbDistance(altUpper);
    double dv = vUpper - vLower;
    double v = (vLower + x*dv/dx) * AdbPTFModel::KNOTS_TO_FEET_PER_MIN;
    double hdot = ac.getClimbRate(h);
    double xdot = sqrt(v*v - hdot*hdot);
    
    return (hdot / xdot);
  }

  double AdbIntegration::xdotVsRod(const double& h,
				    const AdbPTFModel& ac) {
    double tas = ac.getDescentTas(h);
    double v = tas * AdbPTFModel::KNOTS_TO_FEET_PER_MIN;
    double hdot = ac.getDescentRate(h);
    double xdot = sqrt(v*v - hdot*hdot);
    
    return (xdot / hdot); // units:
  }

  double AdbIntegration::xdotVsRoc(const double& h,
				    const AdbPTFModel& ac) {
    double tas = ac.getClimbTas(h);
    double v = tas * AdbPTFModel::KNOTS_TO_FEET_PER_MIN;
    double hdot = ac.getClimbRate(h);
    double xdot = sqrt(v*v - hdot*hdot);
    
    return (xdot / hdot);
  }

  double AdbIntegration::invRod(const double& h,
				 const AdbPTFModel& ac) {
    return 1./ac.getDescentRate(h);
  }

  double AdbIntegration::invRoc(const double& h,
				 const AdbPTFModel& ac) {
    return 1./ac.getClimbRate(h);
  }
}
