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
********************************************************************************
 COMPUTATIONAL APPLIANCE FOR RAPID PREDICTION OF AIRCRAFT TRAJECTORIES (CARPAT)
          Copyright 2010 by Optimal Synthesis Inc. All rights reserved



SBIR Rights (FAR 52.227-20) Notice: Contract No. NNX11CA08C, Dated June 1, 2011.
                       Contract End Date May 31, 2013
                   Software Release Date August 16, 2011

For a period of 4 years after acceptance of all items to be delivered under this
contract, the Government agrees to use these data for Government purposes only,
and they shall not be disclosed outside the Government (including disclosure for
procurement purposes) during such period without permission of the Contractor,
except that, subject to the foregoing use and disclosure prohibitions, such data
may be disclosed for use by support Contractors.  After the aforesaid 4-year
period the Government has a royalty-free license to use, and to authorize others
to use on its behalf, these data for Government purposes, but is relieved of all
disclosure prohibitions and assumes no liability for unauthorized use of these
data by third parties.  This Notice shall be affixed to any reproductions of
these data, in whole or in part.
********************************************************************************
*/

#include <math.h>

#ifndef _CONVERSIONFACTORS_H
#define	_CONVERSIONFACTORS_H

#ifdef	__cplusplus
extern "C" {
#endif

    #define FeetToMeters 0.3048 
    #define MetersToFeet 3.2808399
    #define FeetToMiles 0.000189393939
    #define MilesToFeet 5280
    
    #define KnotsToFeetPerSec 1.68780986
    #define KnotsToMetersPerSec 0.514444444
    #define MetersPerSecToKnots 1.94384449
    #define TonnesToKg 1000
    
    #define DegToRad M_PI/180
    #define RadToDeg 180/M_PI

#ifdef	__cplusplus
}
#endif

#endif	/* _CONVERSIONFACTORS_H */

/*
********************************************************************************
 COMPUTATIONAL APPLIANCE FOR RAPID PREDICTION OF AIRCRAFT TRAJECTORIES (CARPAT)
          Copyright 2010 by Optimal Synthesis Inc. All rights reserved



SBIR Rights (FAR 52.227-20) Notice: Contract No. NNX11CA08C, Dated June 1, 2011.
                       Contract End Date May 31, 2013
                   Software Release Date August 16, 2011

For a period of 4 years after acceptance of all items to be delivered under this
contract, the Government agrees to use these data for Government purposes only,
and they shall not be disclosed outside the Government (including disclosure for
procurement purposes) during such period without permission of the Contractor,
except that, subject to the foregoing use and disclosure prohibitions, such data
may be disclosed for use by support Contractors.  After the aforesaid 4-year
period the Government has a royalty-free license to use, and to authorize others
to use on its behalf, these data for Government purposes, but is relieved of all
disclosure prohibitions and assumes no liability for unauthorized use of these
data by third parties.  This Notice shall be affixed to any reproductions of
these data, in whole or in part.
********************************************************************************
*/
