%{
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
%}

%{
          GENERALIZED NATIONAL AIRSPACE TRAJECTORY-PREDICTION SYSTEM (GNATS)
          Copyright 2018 by Optimal Synthesis Inc. All rights reserved
          
Author: Hari Iyer
Date: 08/20/2018
%}

function plotRouteineCalled = PlotGraph(callsign, userTrajectoryCount, platform, GNATS_HOME_SHARE)

    % Flight for which trajectories need to be plotted
    aircraftID = callsign;
    trajectoryCount = userTrajectoryCount;

    % Iterate through the number of trajectories to be plotted
    for currentTrajectory = 1 : trajectoryCount
        % Read trajectory files in  to a table and convert it to an array
        if (platform == 'MATLAB')
            CSVcontents = readtable([GNATS_HOME_SHARE '/mcSimulation/' aircraftID '-Monte-Carlo-Sim-Trajectory_' num2str(currentTrajectory) '.csv'],'Delimiter',';','ReadVariableNames',false);
            FlightParameters = table2array(CSVcontents);
        else
            % Graph plotting for visualizing vital flight parameter data 
            S = fileread([GNATS_HOME_SHARE '/mcSimulation/' aircraftID '-Octave-Monte-Carlo-Sim-Trajectory_' num2str(currentTrajectory) '.csv']);
            FlightParameters = strsplit(S, '\n');            
        end

        if (platform == 'Octave')
            FlightParameters(end) = [];
        end
        
        % Initialize flight parameter variables
        trueAirSpeedReadings = cell(length(FlightParameters) - 10, 1);
        altitudeReadings = cell(length(FlightParameters) - 10, 1);
        timestampReadings = cell(length(FlightParameters) - 10, 1);
        latitudeReadings = cell(length(FlightParameters) - 10, 1);
        longitudeReadings = cell(length(FlightParameters) - 10, 1);
        rocdReadings = cell(length(FlightParameters) - 10, 1);
        courseReadings = cell(length(FlightParameters) - 10, 1);

        startIndex = 1;
        % Get relevant flight data starting line number from simulation CSV output
        for count = 1 : length(FlightParameters)
            trajectoryPoint = strsplit(char(FlightParameters(count)), ',');
            if (length(trajectoryPoint) > 3 && strcmp(char(trajectoryPoint(3)), aircraftID))
                    startIndex = count + 1;
            end
        end
        trajectoryPoint = strsplit(char(FlightParameters(startIndex)), ',');

        % Traverse through trajectory points and read flight data
        for count = startIndex : length(FlightParameters)
            trajectoryPoint = strsplit(char(FlightParameters(count)), ',');
            if (strcmp(char(trajectoryPoint(1)), ''))
                break;
            end
            trueAirSpeedReadings{count - 8} = str2num(char(trajectoryPoint(6)));
            altitudeReadings{count - 8} = str2num(char(trajectoryPoint(4)));
            timestampReadings{count - 8} = str2num(char(trajectoryPoint(1)));
            latitudeReadings{count - 8} = str2num(char(trajectoryPoint(2)));
            longitudeReadings{count - 8} = str2num(char(trajectoryPoint(3)));
            rocdReadings{count - 8} = str2num(char(trajectoryPoint(5)));
            courseReadings{count - 8} = str2num(char(trajectoryPoint(8)));
        end

        % Convert cells to matrix in order to plot graphs
        trueAirSpeedMatrix = cell2mat(trueAirSpeedReadings);
        altitudeMatrix = cell2mat(altitudeReadings);
        timestampMatrix = cell2mat(timestampReadings);
        latitudeMatrix = cell2mat(latitudeReadings);
        longitudeMatrix = cell2mat(longitudeReadings);
        rocdMatrix = cell2mat(rocdReadings);
        courseMatrix = cell2mat(courseReadings);

        % Graph visualization begins, this can be used as template for custom plots

        % Plot altitude vs timestamp
        altitudeSubplot = subplot(3, 2, 1);
        title(altitudeSubplot, 'Altitude vs. Time');
        plot(timestampMatrix, altitudeMatrix);
        xlabel('Time (s)');
        ylabel('Altitude (feet)');
        %Optionally limit the x or y axis limits to focus on part of graph
        %xlim([9000 11000])
        hold on;
        grid on;

        %Plot true arispeed vs timestamp
        tasSubplot = subplot(3, 2, 2);
        title(tasSubplot, 'True Airspeed vs. Time');
        plot(timestampMatrix, trueAirSpeedMatrix);
        xlabel('Time (s)');
        ylabel('True Airspeed (knots)');
        %xlim([9000 11000])
        hold on;
        grid on;

        %Plot latitude vs longitude
        latLonSubplot = subplot(3, 2, 3);
        title(latLonSubplot, 'Latitude vs. Longitude');
        plot(longitudeMatrix, latitudeMatrix);
        xlabel('Longitude (degree)');
        ylabel('Latitude (degree)');
        %xlim([-114 -110])
        hold on;
        grid on;

        %Plot rate of climb/descent vs timestamp
        rocdSubplot = subplot(3, 2, 4);
        title(rocdSubplot, 'Rate of Climb/Descent vs. Time');
        plot(timestampMatrix, rocdMatrix);
        xlabel('Time (s)');
        ylabel('Rate of Climb/Descent (feet/minute)');
        %xlim([9000 11000])
        hold on;
        grid on;

        %Plot course angle vs timestamp
        courseSubplot = subplot(3, 2, 5);
        title(courseSubplot, 'Course vs. Time');
        plot(timestampMatrix, courseMatrix);
        xlabel('Time (s)');
        ylabel('Course (degree)');
        %xlim([9000 11000])
        hold on;
        grid on;

    end
end

