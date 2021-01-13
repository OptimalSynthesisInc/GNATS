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

% GNATS sample
%
% Optimal Synthesis Inc.
%
% Oliver Chen
% 03.12.2020
%
% Demo of Conflict Detection and Resolution during trajectory simulation

% Run header module
run('GNATS_MATLAB_Header_Client.m');

NauticalMilestoFeet = 6076.12;

if not(isempty(simulationInterface))
    simulationInterface.clear_trajectory();
    
    environmentInterface.load_rap(strcat(DIR_share, '/tg/rap'));
    
    aircraftInterface.load_aircraft(strcat(DIR_share, '/tg/trx/TRX_DEMO_CDNR_v1.5.trx'), strcat(DIR_share, '/tg/trx/TRX_DEMO_CDNR_mfl_v1.5.trx'));
    
    simulationInterface.setupSimulation(36000, 30);
    
    % Set distance parameters of CDNR
    % These functions are optional.  The following values are default in GNATS.
    % If users don't call these functions, GNATS engine uses default values.
    % controllerInterface.setCDR_initiation_distance_surface(600);
    % controllerInterface.setCDR_initiation_distance_terminal(20 * NauticalMilestoFeet);
    % controllerInterface.setCDR_initiation_distance_enroute(20 * NauticalMilestoFeet);
    % controllerInterface.setCDR_separation_distance_surface(300);
    % controllerInterface.setCDR_separation_distance_terminal(7 * NauticalMilestoFeet);
    % controllerInterface.setCDR_separation_distance_enroute(10 * NauticalMilestoFeet);

    % Enable conflict detection and resolution
    controllerInterface.enableConflictDetectionAndResolution(true);

    % Start simulation for 3180 seconds
    simulationInterface.start(3180);

    while true
        runtime_sim_status = simulationInterface.get_runtime_sim_status();
        if (runtime_sim_status == GNATS_SIMULATION_STATUS_PAUSE)
            break;
        else
            pause(1);
        end
    end
    
    % Result of CDR status is a 2-dimentional array
    % Array elements are: aircraft ID of the held aircraft
    %                     aircraft ID of the conflicting aircraft
    %                     seconds of holding of the held aircraft
    % Format type: [[String, String, float]]
    % Example: [["AC1", "AC_conflicting_with_AC1", heldSeconds_AC1], ["AC2", "AC_conflicting_with_AC2", heldSeconds_AC2]]
    array_cdrStatus = controllerInterface.getCDR_status();
    if not(isempty(array_cdrStatus))
        fprintf('Show CD&R Status:');
        for i = 1: size(array_cdrStatus)
            fprintf('    AC1_held = %s\n', array_cdrStatus(i, 1));
            fprintf('    AC2_conflicting_with_AC1 = %s\n', array_cdrStatus(i, 2));
            fprintf('    Seconds held of AC1 = %f\n', array_cdrStatus(i, 3));
        end
    end

    simulationInterface.resume();
    
    % Use a while loop to constantly check simulation status.  When the simulation finishes, continue to output the trajectory data
    while true
        runtime_sim_status = simulationInterface.get_runtime_sim_status();
        if (runtime_sim_status == GNATS_SIMULATION_STATUS_ENDED)
            break;
        else
            pause(1);
        end
    end
    
    % Format epoch time string
    millis = datestr(now, 'yyyymmdd HHMMSS');
    InputDate = datenum(millis, 'yyyymmdd HHMMSS');
    UnixOrigin = datenum('19700101 000000', 'yyyymmdd HHMMSS');
    EpochSecond = round((InputDate-UnixOrigin)*86400000);

	S = dbstack();
    cur_filename = char(S(1).file);
    strIndexArray = strfind(cur_filename, '.m');

    disp('Outputting trajectory data.  Please wait....');
    fileName = sprintf('%s_%s.csv', cur_filename(1: strIndexArray(1)-1), num2str(EpochSecond));
    % Output the trajectory result file
    simulationInterface.write_trajectories(fileName);
    
    aircraftInterface.release_aircraft();
    environmentInterface.release_rap();
end
gnatsClient.disConnect()
