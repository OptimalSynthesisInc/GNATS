% GNATS sample
%
% Optimal Synthesis Inc.
%
% Oliver Chen
% 03.12.2020
%
% Demo of changing aircraft state
%
% This program starts the simulation for a period of time then GNATS pause automatically.
% When the simulation is at pause status, we change the aircraft state.
% When the simulation resumes, it continues to run the rest of the simulation until it finishes.
%
% Users can compare the trajectory data to see the change of aircraft state.

% Run header module
run('GNATS_MATLAB_Header_standalone.m');

if not(isempty(simulationInterface))
    simulationInterface.clear_trajectory();

    environmentInterface.load_rap(strcat(DIR_share, '/tg/rap'));
    
    aircraftInterface.load_aircraft(strcat(DIR_share, '/tg/trx/TRX_DEMO_WSSS_RJAA_geo.trx'), strcat(DIR_share, '/tg/trx/TRX_DEMO_WSSS_RJAA_mfl.trx'));

    simulationInterface.setupSimulation(27000, 30);
    
    simulationInterface.start(1020);

    % Check simulation status.  Continue to next code when it is at PAUSE status
    while true
        runtime_sim_status = simulationInterface.get_runtime_sim_status();
        if (runtime_sim_status == GNATS_SIMULATION_STATUS_PAUSE)
            break;
        else
            pause(1);
        end
    end
    
    curAircraft = aircraftInterface.select_aircraft('SQ12');
    if ~isempty(curAircraft)
        % Set new state value
        curAircraft.setLatitude_deg(1.0);
        curAircraft.setLongitude_deg(105.0);
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