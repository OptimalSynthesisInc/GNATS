% GNATS sample
%
% Optimal Synthesis Inc.
%
% Oliver Chen
% 03.12.2020
%
% Demo of aircraft-related functions.
%
% Users can learn how to obtain aircraft instance, show related aircraft information, start/pause/resume simulation and output result trajectory file.

% Run header module
run('GNATS_MATLAB_Header_standalone.m');

if not(isempty(simulationInterface))
    simulationInterface.clear_trajectory();

    environmentInterface.load_rap(strcat(DIR_share, '/tg/rap'));
    
    aircraftInterface.load_aircraft(strcat(DIR_share, '/tg/trx/TRX_DEMO_WSSS_RJAA_geo.trx'), strcat(DIR_share, '/tg/trx/TRX_DEMO_WSSS_RJAA_mfl.trx'));
    
    aircraftIdArray_withinZone = aircraftInterface.getAircraftIds(30.0, 35.0, -115.0, -90.0, -1.0, -1.0);
    if ((~isempty(aircraftIdArray_withinZone)) & (size(aircraftIdArray_withinZone) > 0))
        for i = 1: size(aircraftIdArray_withinZone)
            curAcId = aircraftIdArray_withinZone(i);

            fprintf('Aircraft id in selected zone = %s, time = %ld\n', char(curAcId), round(tic * 1000));
        end
    end
    
    disp('****************************************');

    curAircraft = aircraftInterface.select_aircraft('SQ12');
    if ~isempty(curAircraft)
        airborne_flight_plan_waypoint_name_array = curAircraft.getFlight_plan_waypoint_name_array();

        for i = 1: (size(airborne_flight_plan_waypoint_name_array))
            fprintf('SQ12 airborne flight plan waypoint name = %s\n', char(airborne_flight_plan_waypoint_name_array(i, 1)));
        end
        
        curAircraft.delay_departure(100); % Delay the departure time for 100 sec
    else
        fprintf('Aircraft SQ12 not found');
    end
    
    simulationInterface.setupSimulation(10000, 10);

    simulationInterface.start(1000);

    while true
        runtime_sim_status = simulationInterface.get_runtime_sim_status();
        if (runtime_sim_status == GNATS_SIMULATION_STATUS_PAUSE)
            break;
        else
            pause(1);
        end
    end
    
    curAircraft = aircraftInterface.select_aircraft('SQ12');
    if not(isempty(curAircraft))
        disp('****************************************');
        fprintf('SQ12 (pausing at %f', simulationInterface.get_curr_sim_time());
        fprintf(' sec, latitude = %f', curAircraft.getLatitude_deg());
        fprintf(', longitude = %f', curAircraft.getLongitude_deg());
        fprintf(', altitude = %f\n', curAircraft.getAltitude_ft());
        disp('****************************************');
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

% Stop GNATS Standalone environment
gnatsStandalone.stop();

