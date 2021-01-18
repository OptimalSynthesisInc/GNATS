"""
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
"""


#!/usr/bin/env python
"""
Plots generated flight plan using Google Maps API.

Author: Hari Iyer
Date: 01/12/2019
"""

import math
import os
import csv
from glob import glob

from distutils.spawn import find_executable

def plotOnGoogleMap(FLIGHT_CALLSIGN, csvFile):
    '''
    Plotting flight plan on Google Map.
    '''

    #Get CSV with Geo-cordinates, parsing them into Google Maps format
    gnatsServerDirName = glob("../*GNATS_Server*/")[0]

    latLonFile = list(csv.reader(open(gnatsServerDirName + "/" + csvFile), delimiter=','))
        
    latLonData = []
    
    startIndex = 0

    for flight in FLIGHT_CALLSIGN:
        latLonSubData = []
        for row in latLonFile:
            if (len(row) > 3):
                time, latitude, longitude= row[0], row[1], row[2]
                if (row[0] == "AC" and row[2] == flight):
                    startIndex = latLonFile.index(row) + 1
                    
        for index in range(startIndex, len(latLonFile)):
            if (latLonFile[index] == []):
                break
            time, latitude, longitude= latLonFile[index][0], latLonFile[index][1], latLonFile[index][2]
            if [float(latitude), float(longitude)] != [0.0,0.0]:
                latLonSubData.append({'lat': float(latitude), 'lng': float(longitude), 'time': str(time)})
        
        latLonData.append(latLonSubData)


    #HTML content for flight plan visualization
    mapHTML = """
    <!DOCTYPE html>
    <html>
       <head>
          <meta name="viewport" content="initial-scale=1.0, user-scalable=no">
          <meta charset="utf-8">
          <title>Flight Plan</title>
          <style>
         html, body, #gMap {
         height: 100%;
         }
          </style>
       </head>
       <body>
          <div id="gMap"></div>
          <script>
         function setPlanToMap() {
         var gMap = new google.maps.Map(document.getElementById('gMap'), {
         zoom: 5,
         center: {lat: 38.04, lng: -99.17},
         mapTypeId: 'satellite'
                 });

                 var flightPlanPoints = """ + repr(latLonData) + """;
                 for(flight = 0; flight < flightPlanPoints.length; flight++) {
                     for(waypoint = 0; waypoint < flight.length; waypoint++) {
                     alert("d");
                     latitude = parseFloat(flightPlanPoints[flight][waypoint]['lat']);
                     longitude = parseFloat(flightPlanPoints[flight][waypoint]['lng']);
                     var marker = new google.maps.Marker({position:new google.maps.LatLng(latitude, longitude)});
                     marker.setMap(gMap);
                     label = flightPlanPoints[flight][waypoint]['time'].toString()
                     var infowindow = new google.maps.InfoWindow({
               content: label
             });
                      infowindow.open(gMap, marker);
                     }
        
         var flightRoute = new google.maps.Polyline({
         path: flightPlanPoints[flight],
         geodesic: true,
         strokeColor: "#"+((1<<24)*Math.random()|0).toString(16),
         strokeWeight: 3
         });
         flightRoute.setMap(gMap);
         }
         }
          </script>
          <script async defer
         src="https://maps.googleapis.com/maps/api/js?key=AIzaSyA0uZLor9GF8qdQS1EHBtE0xN0UZCtJJB4&callback=setPlanToMap"></script>
       </body>
    </html>
        """

    #Write HTML to file, open using system command on browser
    f = open("map.html","w")
    f.write(mapHTML)
    f.close()
    
    if (find_executable("google-chrome") is not None) :
        os.system("google-chrome map.html")
    elif (find_executable("firefox") is not None) :
        os.system("firefox map.html")
