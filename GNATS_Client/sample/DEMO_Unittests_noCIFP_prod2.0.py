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


'''
GNATS API Function unit test suite

This function can be run to ensure appropriate installation of GNATS and to verify functions are working.

Date: 15th July, 2019

Author: Vedik Jayaraj

'''


# encoding:utf-8

from GNATS_Python_Header_client import *

gnatsClient.login("admin")

simulationInterface = gnatsClient.getSimulationInterface()
assert simulationInterface is not None, "getSimulationInterface"

entityInterface = gnatsClient.getEntityInterface()
assert entityInterface is not None, "getEntityInterface"

controllerInterface = entityInterface.getControllerInterface()
pilotInterface = entityInterface.getPilotInterface()

environmentInterface = gnatsClient.getEnvironmentInterface()
assert environmentInterface is not None, "getEnvironmentInterface"

airportInterface    =    environmentInterface.getAirportInterface()
assert airportInterface != None, "getAirportInterface"

terminalAreaInterface    =     environmentInterface.getTerminalAreaInterface()
assert terminalAreaInterface is not None, "getTerminalAreaInterface"

terrainInterface    =    environmentInterface.getTerrainInterface()
assert terrainInterface != None, "getTerrainInterface"

weatherInterface = environmentInterface.getWeatherInterface()
assert weatherInterface is not None, "getWeatherInterface"

equipmentInterface = gnatsClient.getEquipmentInterface()
assert equipmentInterface is not None, "getEquipmentInterface"

aircraftInterface = equipmentInterface.getAircraftInterface()
assert aircraftInterface is not None, "getAircraftInterface"

safetyMetricsInterface    =    gnatsClient.getSafetyMetricsInterface()
assert safetyMetricsInterface is not None, "getSafetyMetricsInterface"

safetyMInterface    =    gnatsClient.getSafetyMInterface()
assert safetyMInterface is not None, "getSafetyMInterface"

# result variable stores flag on successful unit tests
result = 0

try :
	simulationInterface.clear_trajectory()
    
	environmentInterface.load_rap("share/tg/rap")
    
	aircraftInterface.load_aircraft("share/tg/trx/TRX_DEMO_SFO_PHX_GateToGate_geo.trx", "share/tg/trx/TRX_DEMO_SFO_PHX_mfl.trx")
    
	currentTime    =    simulationInterface.get_curr_sim_time()
	assert currentTime == 0.0, "get_curr_sim_time"

	simulation_id    =    simulationInterface.get_sim_id()
	assert simulation_id == 0, "get_sim_id"

	assert simulationInterface.setupSimulation(10000,    5) == 0, "setupSimulation"
	
	currentRuntimeStatus    =    simulationInterface.get_runtime_sim_status()
	assert currentRuntimeStatus == 0, "get_runtime_sim_status"

	simulationInterface.start()
	currentRuntimeStatus    =    simulationInterface.get_runtime_sim_status()
	assert currentRuntimeStatus == 1, "start"
    
	simulationInterface.pause()
	currentRuntimeStatus    =    simulationInterface.get_runtime_sim_status()
	assert currentRuntimeStatus == 2, "pause function"

	simulationInterface.resume()
	currentRuntimeStatus    =    simulationInterface.get_runtime_sim_status()
	assert currentRuntimeStatus == 3, "resume function"

	simulationInterface.resume(100)
	currentRuntimeStatus    =    simulationInterface.get_runtime_sim_status()
	assert currentRuntimeStatus == 3, "resume(100) function"

	simulationInterface.resume(100.5)
	currentRuntimeStatus    =    simulationInterface.get_runtime_sim_status()
	assert currentRuntimeStatus == 3, "resume(100.5) function"

	simulationInterface.stop()
	currentRuntimeStatus    =    simulationInterface.get_runtime_sim_status()
	assert currentRuntimeStatus == 4, "stop"

	while True:
		runtime_sim_status = simulationInterface.get_runtime_sim_status()
		if ((runtime_sim_status == GNATS_SIMULATION_STATUS_STOP) or (runtime_sim_status == GNATS_SIMULATION_STATUS_ENDED)) :
			break
		else:
			time.sleep(1)
	
	millis = int(round(time.time() * 1000))
	print("Outputting trajectory data.  Please wait....")
	simulationInterface.write_trajectories(os.path.splitext(os.path.basename(__file__))[0] + "_" + str(millis) + ".csv")

	aircraftInterface.release_aircraft()
	
	print("")
	print("************************************************************")
	print("")
	
	# ==========================================================================

	simulationInterface.clear_trajectory()

	groundVehicleInterface = equipmentInterface.getGroundVehicleInterface()
	assert groundVehicleInterface is not None, "getGroundVehicleInterface"
	
	aircraftInterface.load_aircraft("share/tg/trx/TRX_DEMO_SFO_PHX_GateToGate_geo.trx", "share/tg/trx/TRX_DEMO_SFO_PHX_mfl.trx")

	groundVehicleInterface.load_groundVehicle('share/tg/trx/TRX_DEMO_11GroundVehicles.trx')

	simulationInterface.request_aircraft('SWA1897')

	simulationInterface.request_groundVehicle('BUS123')

	simulationInterface.externalAircraft_create_trajectory_profile(    
	'ABC173',"B733",    "KPHX",
	'KSFO',    33000.0,    430.0,    37.2,    -122.4,    2500.0,    215.0,    240.0,    318.2,    
	'FLIGHT_PHASE_CRUISE')
	
	simulationInterface.externalAircraft_inject_trajectory_state_data("ABC173",    32.61,    -122.39,    3200,
	30,    250,    50,    'FLIGHT_PHASE_CRUISE',    1541784961725)

	cnsInterface    =    equipmentInterface.getCNSInterface()
	assert cnsInterface is not None, "getCNSInterface"

	badaDataInterface    =    equipmentInterface.getADBDataInterface()
	assert badaDataInterface is not None, "getADBDataInterface"

	aircraftIds    =    aircraftInterface.getAircraftIds(28.5,    30.7,    72.8,    74.9,    15000.0,    20000.9)
	assert aircraftIds == None, "getAircraftId"


	aircraftIds    =    list(aircraftInterface.getAllAircraftId())
	assert aircraftIds == ['ABC173', 'SWA1897'], "getAircraftId"

	aircraft    =    aircraftInterface.select_aircraft('SWA1897')
	assert aircraft != None, "select_aircraft"

	assert aircraftInterface.synchronize_aircraft_to_server(aircraft) == 0, "synchronize_aircraft_to_server"

	aircraftId    =    aircraft.getAcid()
	assert aircraftId == 'SWA1897', "getAcid"

	aircraftAltitude    =    aircraft.getAltitude_ft()
	assert aircraftAltitude == 13, "getAltitude_ft"

	aircraftCruiseAltitude    =    aircraft.getCruise_alt_ft()
	assert aircraftCruiseAltitude == 33000.0, "getCruise_alt_ft"

	aircraftCruiseAirspeed    =    aircraft.getCruise_tas_knots()
	assert aircraftCruiseAirspeed == 445.0, "getCruise_tas_knots"

	flightDepartureTime    =    aircraft.getDeparture_time_sec()
	assert flightDepartureTime == 0.0, "getDeparture_time_sec"

	destinationAirportElevation    =    aircraft.getDestination_airport_elevation_ft()
	assert destinationAirportElevation == 1168.0, "getDestination_airport_elevation_ft"

	flightPhase    =    aircraft.getFlight_phase()
	assert flightPhase == 3, "getFlight_phase"

	flightLatitudeArray    =    list(aircraft.getFlight_plan_latitude_array())
	assert flightLatitudeArray == [37.628299713134766, 37.63613510131836, 37.489784240722656, 37.19537353515625, 37.04014587402344, 36.487483978271484, 35.84760665893555, 34.42243957519531, 33.596065521240234, 33.511531829833984, 33.50998306274414, 33.46272277832031, 33.43552780151367, 33.337242126464844, 33.297386169433594, 33.27430725097656, 33.28049850463867, 33.329689025878906, 33.401893615722656, 33.42845153808594, 33.428619384765625, 33.42874526977539, 33.431034088134766, 33.428829193115234, 33.428855895996094, 33.42885971069336]

	flightPlanLength    =    aircraft.getFlight_plan_length()
	assert flightPlanLength == 26, "getFlight_plan_length"

	flightLongitudeArray    =    aircraft.getFlight_plan_longitude_array()
	assert list(flightLongitudeArray) == [-122.39230346679688, -122.4165267944336, -122.47457885742188, -122.01880645751953, -121.78099822998047, -120.94786071777344, -120.00047302246094, -118.0258560180664, -114.76126861572266, -114.32807922363281, -114.32022094726562, -114.08177947998047, -113.94625091552734, -113.46075439453125, -113.23029327392578, -113.06967163085938, -112.82159423828125, -112.69026947021484, -112.50096130371094, -112.4253921508789, -112.35429382324219, -112.28280639648438, -112.2069320678711, -112.21131134033203, -112.14168548583984, -112.027099609375]

	flightAltitudeDescriptionArray    =    aircraft.getFlight_plan_alt_desc_array()
	assert list(flightAltitudeDescriptionArray) == [None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None]

	flightPlanAltitude1Array    =    aircraft.getFlight_plan_alt_1_array()
	assert list(flightPlanAltitude1Array) == [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]

	flightPlanAltitude2Array    =    aircraft.getFlight_plan_alt_2_array()
	assert list(flightPlanAltitude2Array) == [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]

	flightPlanSpeedLimitArray    =    aircraft.getFlight_plan_speed_limit_array()
	assert list(flightPlanSpeedLimitArray) == [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]

	flightSpeedLimitDescriptionArray    =    aircraft.getFlight_plan_speed_limit_desc_array()
	assert list(flightSpeedLimitDescriptionArray) == [None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None, None]

	flightPathAngle    =    aircraft.getFpa_rad()
	assert flightPathAngle == 0.0, "getFpa_rad"

	courseAngle    =    aircraft.getCourse_rad()
	assert courseAngle == 0.48869219422340393, "getCourse_rad()"

	flightLandedFlag    =    aircraft.getLanded_flag()
	assert flightLandedFlag == 0, "getLanded_flag"

	flightCurrentLatitude    =    aircraft.getLatitude_deg()
	assert flightCurrentLatitude == 37.621368408203125, "getLatitude_deg"

	flightCurrentLongitude=    aircraft.getLongitude_deg()
	assert flightCurrentLongitude == -122.39006042480469, "getLongitude_deg"

	originAirportElevation    =    aircraft.getOrigin_airport_elevation_ft()
	assert originAirportElevation == 13, "getOrigin_airport_elevation_ft"

	rateOfClimbOrDescent    =    aircraft.getRocd_fps()
	assert rateOfClimbOrDescent == 0.0, "getRocd_fps"

	#sectorIndex    =    aircraft.getSector_index()
	#assert sectorIndex == -1, "getSector_index"

	targetWaypointIndex    =    aircraft.getTarget_waypoint_index()
	assert targetWaypointIndex == 1, "getTarget_waypoint_index"

	#targetWaypointName    =    aircraft.getTarget_waypoint_name()
	#assert targetWaypointName == "Ramp_06_011", "getTarget_waypoint_name"

	currentAirspeed    =    aircraft.getTas_knots()
	assert currentAirspeed == 0.0, "getTas_knots"

	topOfClimbIndex    =    aircraft.getToc_index()
	assert topOfClimbIndex == 4, "getToc_index"

	topOfDescentIndex    =    aircraft.getTod_index()
	assert topOfDescentIndex == 10, "getTod_index"

	aircraft.setAltitude_ft(27500)
	assert aircraft.getAltitude_ft() == 27500, "setAltitude_ft"
   
	aircraft.setCruise_alt_ft(35000)
	assert aircraft.getCruise_alt_ft() == 35000, "setCruise_alt_ft"

	aircraft.setCruise_tas_knots(455)
	assert aircraft.getCruise_tas_knots() == 455, "setCruise_tas_knots"
 
	aircraft.setFlight_plan_latitude_deg(5,    34.50)

	aircraft.setFlight_plan_longitude_deg(5,    -122.63)

	aircraft.setLatitude_deg(26.58)
	assert aircraft.getLatitude_deg() == 26.579999923706055, "setLatitude_deg"

	aircraft.setLongitude_deg(-122.36)
	assert aircraft.getLongitude_deg() == -122.36000061035156, "setLongitude_deg"

	aircraft.setRocd_fps(-50)
	assert aircraft.getRocd_fps() == -50, "setRocd_fps"

	aircraft.setTas_knots(400)
	assert aircraft.getTas_knots() == 400, "setTas_knots"

	assert groundVehicleInterface.release_groundVehicle() == 1, "release_groundVehicle"

	aircraftInterface.release_aircraft()
	
	print("")
	print("************************************************************")
	print("")

	# ==========================================================================

	simulationInterface.clear_trajectory()

	aircraftInterface.load_aircraft("share/tg/trx/TRX_DEMO_SFO_PHX_GateToGate_geo.trx", "share/tg/trx/TRX_DEMO_SFO_PHX_mfl.trx")

	groundVehicleInterface.load_groundVehicle('share/tg/trx/TRX_DEMO_11GroundVehicles.trx')

	assignedGroundVehicles = groundVehicleInterface.getAssignedGroundVehicleIds()
	assert assignedGroundVehicles == None, "getAssignedGroundVehicleIds"
	
	assignedGroundVehicles = groundVehicleInterface.getAssignedGroundVehicleIds("admin")
	assert assignedGroundVehicles == None, "getAssignedGroundVehicleIds"

	assert groundVehicleInterface.externalGroundVehicle_create_trajectory_profile('NEW123', 'SWA1897', 'KSFO', 37, -122, 15, 28) == 1, "externalGroundVehicle_create_trajectory_profile"

	assert groundVehicleInterface.externalGroundVehicle_inject_trajectory_state_data('NEW123', 'SWA1897', 37, -122, 15, 28) == 1, "externalGroundVehicle_inject_trajectory_state_data"

	groundVehicle = groundVehicleInterface.select_groundVehicle('BUS123')
	
	if groundVehicle:
		groundVehicleId = groundVehicle.getGvid()
		assert groundVehicleId == 'BUS123', "getGvid"
	
		groundVehicleAirportId = groundVehicle.getAirportId()
		assert groundVehicleAirportId == "KPHL", "getAirportId"

		aircraftInService = groundVehicle.getAircraftInService()
		assert aircraftInService == "SWA1898", "aircraftInService"

		isExternalGroundVehicle = groundVehicle.getFlag_external_groundvehicle()
		assert isExternalGroundVehicle == False, "getFlag_external_groundvehicle"

		user = groundVehicle.getAssigned_user()
		assert user == None, "getAssigned_user"

		latitude = groundVehicle.getLatitude()
		assert latitude == 39.8608512878418, "getLatitude"

		groundVehicle.setLatitude(37.8959)
		assert groundVehicle.getLatitude() == 37.89590072631836, "setLatitude" 

		longitude = groundVehicle.getLongitude()
		assert longitude == -75.2750015258789, "getLongitude"

		groundVehicle.setLongitude(-112.8594)
		assert groundVehicle.getLongitude() == -112.8593978881836

		altitude = groundVehicle.getAltitude()
		assert altitude == 36, "getAltitude"

		groundVehicleSpeed = groundVehicle.getSpeed()
		assert groundVehicleSpeed == 15, "getSpeed"

		groundVehicle.setSpeed(25)
		assert groundVehicle.getSpeed() == 25, "setSpeed"

		groundVehicleCourse = groundVehicle.getCourse()
		assert groundVehicle.getSpeed() == 25, "setSpeed"

		groundVehicle.setCourse(1.5)
		assert groundVehicle.getCourse() == 1.5, "getCourse"

		groundVehicleDepartureTime = groundVehicle.getDeparture_time()
		assert groundVehicleDepartureTime == 1121238016.0, "getDeparture_time"

		groundVehicleDrivePlanLatitudeArray = groundVehicle.getDrive_plan_latitude_array()
		assert list(groundVehicleDrivePlanLatitudeArray) == [39.86085891723633, 39.8615608215332, 39.86195373535156, 39.86222839355469, 39.86335372924805, 39.86442947387695, 39.86484146118164, 39.86528778076172, 39.86579132080078, 39.86635971069336, 39.86701965332031, 39.867218017578125, 39.867286682128906, 39.867347717285156, 39.86766815185547, 39.86940002441406, 39.86978530883789, 39.8698844909668, 39.869991302490234, 39.87047576904297, 39.87074661254883, 39.870784759521484, 39.870994567871094, 39.8713493347168, 39.87214660644531, 39.87303161621094, 39.87363815307617, 39.87407302856445, 39.87416076660156], "getDrive_plan_latitude_array"

		groundVehicleDrivePlanLongitudeArray = groundVehicle.getDrive_plan_longitude_array()
		assert list(groundVehicleDrivePlanLongitudeArray) == [-75.27500915527344, -75.275390625, -75.27523040771484, -75.27379608154297, -75.26821899414062, -75.26856231689453, -75.2686996459961, -75.26885223388672, -75.26901245117188, -75.26919555664062, -75.26912689208984, -75.26807403564453, -75.26701354980469, -75.26642608642578, -75.26558685302734, -75.25774383544922, -75.25584411621094, -75.25531005859375, -75.2547836303711, -75.25492095947266, -75.25504302978516, -75.25445556640625, -75.25346374511719, -75.25358581542969, -75.25385284423828, -75.25373840332031, -75.2535171508789, -75.25334930419922, -75.25212860107422], "getDrive_plan_longitude_array"

		groundVehicleDrivePlanLength = groundVehicle.getDrive_plan_length()
		assert groundVehicleDrivePlanLength == 29, "getDrive_plan_length"

		groundVehicleDrivePlanWaypointNames = groundVehicle.getDrive_plan_waypoint_name_array()
		assert list(groundVehicleDrivePlanWaypointNames) == ['Rwy_01_001', 'Txy_S_001', 'Txy_S_102', 'Txy_S_002', 'Txy_S_003', 'Txy_Z_009', 'Txy_Z_010', 'Txy_Z_011', 'Txy_Z_012', 'Txy_Z_013', 'Txy_Z_014', 'Parking_deice1_002', 'Txy_Z_003', 'Txy_Z_016', 'Txy_Z_017', 'Txy_K_001', 'Txy_K_002', 'Txy_K_003', 'Txy_K_004', 'spot_02S', 'Ramp_13_001', 'spot_02E', 'Txy_J_004', 'Ramp_01_001', 'Ramp_01_002', 'Ramp_01_003', 'Ramp_01_004', 'Ramp_01_005', 'Gate_AW_018'], "groundVehicleDrivePlanWaypointNames"

		groundVehicleTargetWaypointIndex = groundVehicle.getTarget_waypoint_index()
		assert groundVehicleTargetWaypointIndex == -1, "getTarget_waypoint_index"

		groundVehicleTargetWaypointName = groundVehicle.getTarget_waypoint_name()
		assert groundVehicleTargetWaypointName == "", "getTarget_waypoint_name"

		groundVehicle.setDrive_plan_latitude(2, 37.2518)

		groundVehicle.setDrive_plan_longitude(2, -112.8155)

	assert cnsInterface.setNavigationLocationError('SWA1897', 'LATITUDE',
	0.00005, 0.00000001, 0.9, 0.2, 1) == 0, "setNavigationLocationError"
	assert cnsInterface.setNavigationLocationError('SWA1897', 'LONGITUDE',
	0.00005, 0.00000001, 0.9, 0.2, 1) == 0, "setNavigationLocationError"

	assert cnsInterface.setNavigationAltitudeError('SWA1897', .00005, 0.2, 0) == 0, "setNavigationAltitudeError"

	assert cnsInterface.setRadarError('KSFO', 'RANGE', 25, 0.0000005, 0.2, 1) == 0, "setRadarError"
	assert cnsInterface.setRadarError('KSFO', 'AZIMUTH', 30, 0.0000005, 0.2, 1) == 0, "setRadarError"
	assert cnsInterface.setRadarError('KSFO', 'ELEVATION', 2500,0.0000005,0.2,1) == 0, "setRadarError"

	assert badaDataInterface.getADB_cruiseTas('SA',    15000) == 343.0, "getBADA_cruiseTas"

	assert badaDataInterface.getADB_climbRate_fpm('SA',    150,    'NOMINAL') == 2375.8, "getBADA_climbRate_fpm"

	assert badaDataInterface.getADB_climbTas('SA',    15000) == 357.5, "getBADA_climbTas"

	assert badaDataInterface.getADB_descentRate_fpm('SA',    150,    'NOMINAL') == 754.1, "getBADA_descentRate_fpm"

	assert badaDataInterface.getADB_descentTas('SA',    15000) == 357.5, "getBADA_descentTas"

	assert list(environmentInterface.getCenterCodes()) == ['KZAU', 'KZBW', 'KZDC', 'KZDV', 'KZFW', 'KZHU', 'KZID', 'KZJX', 'KZKC', 'KZLA', 'KZLC', 'KZMA', 'KZME', 'KZMP', 'KZNY', 'KZOA', 'KZOB', 'KZSE', 'KZTL', 'PZAN', 'PZHN', 'KZAB']

	assert len(list(environmentInterface.getFixesInCenter('KZOA'))) == 2399, "getFixesInCenter"

	airport    =    airportInterface.select_airport('KPHX')

	arrivalAirport    =    airportInterface.getArrivalAirport('SWA1897')
	assert arrivalAirport == "", "getArrivalAirport"

	departureAirport    =    airportInterface.getDepartureAirport('SWA1897')
	assert departureAirport == "", "getDepartureAirport"

# 	groundVehicleInterface.release_groundVehicle()
# 	
# 	aircraftInterface.release_aircraft()

	airportLocation    =    airportInterface.getLocation('KLAX')
	assert airportLocation == None, "getLocation"

	closestAirport    =    airportInterface.getClosestAirport(35.2,    -118.6)
	assert closestAirport == "", "getClosestAirport"

	airports    =    airportInterface.getAirportsWithinMiles(35.2, -118.6, 22.5)
	assert airports == None, "getAirportsWithinMiles"

	airportFullName    =    airportInterface.getFullName('KJFK')
	assert airportFullName == "", "getFullName"

	airportRunways    =    airportInterface.getAllRunways('PANC')
	assert airportRunways == None, "getAllRunways"

	runwayExits    =    airportInterface.getRunwayExits('KSFO',    'RW28R')
	assert runwayExits == None , "getRunwayExits"

	airportLayoutNodeMap    =    airportInterface.getLayout_node_map('PHNL')
	assert airportLayoutNodeMap == None, "getLayout_node_map"

	airportLayoutNodeData    =    airportInterface.getLayout_node_data('PHNL')
	assert airportLayoutNodeData == None, "getLayout_node_data"

	airportLayoutLinks    =    airportInterface.getLayout_links('PHNL')
	assert airportLayoutLinks == None, "getLayout_links"

	#surfaceTaxiPlan    =    airportInterface.getSurface_taxi_plan('SWA1897',    'KSFO')
	#assert list(surfaceTaxiPlan) == ['Gate_F_086', 'Ramp_06_011', 'Txy_A_B1', 'Txy_A_016', 'Txy_A_015', 'Txy_A_012', 'Txy_A_011', 'Txy_A_D', 'Txy_A_010', 'Txy_A_E', 'Txy_A_009', 'Txy_A_F1', 'Txy_A_008', 'Txy_A_G', 'Txy_A_007', 'Txy_A_006', 'Txy_A_005', 'Txy_A_H', 'Txy_A_004', 'Txy_A_M', 'Txy_A_003', 'Txy_A_002', 'Txy_A_A1', 'Txy_A_001', 'Rwy_02_001', 'Rwy_02_002'], "getSurface_taxi_plan"

	#assert airportInterface.setUser_defined_surface_taxi_plan('SWA1898',    
	#'KSFO',
	#['Gate_01_001',    'Ramp_01_001',    'Txy_01_001',    'Txy_01_002',    
	#'Rwy_02_001']) == 1, "setUser_defined_surface_taxi_plan"

	#departureRunway    =    airportInterface.getDepartureRunway('SWA1897')
	#assert departureRunway == "RW01R",  "getDepartureRunway"

	#arrivalRunway    =    airportInterface.getArrivalRunway('SWA1897')
	#assert arrivalRunway == "RW07R", "getArrivalRunway"

	airportList    =    airportInterface.getAllAirportCodesInGNATS()
	assert list(airportList) == ['KABQ', 'KATL', 'KBDL', 'KBHM', 'KBNA', 'KBOI', 'KBOS', 'KBTV', 'KBUR', 'KBWI', 'KBZN', 'KCHS', 'KCLE', 'KCLT', 'KCRW', 'KCVG', 'KDAL', 'KDCA', 'KDEN', 'KDFW', 'KDSM', 'KDTW', 'KEWR', 'KFAR', 'KFLL', 'KFSD', 'KGYY', 'KHPN', 'KIAD', 'KIAH', 'KICT', 'KILG', 'KIND', 'KISP', 'KJAC', 'KJAN', 'KJAX', 'KJFK', 'KLAS', 'KLAX', 'KLEX', 'KLGA', 'KLGB', 'KLIT', 'KMCO', 'KMDW', 'KMEM', 'KMHT', 'KMIA', 'KMKE', 'KMSP', 'KMSY', 'KOAK', 'KOKC', 'KOMA', 'KONT', 'KORD', 'KPBI', 'KPDX', 'KPHL', 'KPHX', 'KPIT', 'KPVD', 'KPWM', 'KSAN', 'KSAT', 'KSDF', 'KSEA', 'KSFO', 'KSJC', 'KSLC', 'KSNA', 'KSTL', 'KSWF', 'KTEB', 'KTPA', 'KVGT', 'PANC', 'PHNL'], "getAllAirportCodesInGNATS"

	runwayEnds    =    airportInterface.getRunwayEnds('KSFO',    'RW28R')
	assert runwayEnds == None, "getRunwayEnds"

	airport    =    airportInterface.select_airport('KORD')
	assert airport == None, "select_airport"

	if airport:
		airport    =    airportInterface.select_airport('KSFO')
		airportElevation    =    airport.getElevation()
		assert airportElevation == -1, "getElevation"

		airport    =    airportInterface.select_airport('KORD')
		airportLatitude    =    airport.getLatitude()
		assert airportLatitude == -1, "getLatitude"

		airportLongitude    =    airport.getLongitude()
		assert airportLongitude == -1, "getLongitude"

		airportName    =    airport.getName()
		assert airportName == "", "getName"

	approaches    =    terminalAreaInterface.getAllApproaches('KORD')
	assert approaches == None, "getAllApproaches"

	sids    =    terminalAreaInterface.getAllSids('KORD')
	assert sids == None, "getAllSids"

	stars    =    terminalAreaInterface.getAllStars('KORD')
	assert stars == None, "getAllStars"

	currentApproach    =    terminalAreaInterface.getCurrentApproach('SWA1897')
	assert currentApproach == None, "getCurrentApproach"

	currentSid    =    terminalAreaInterface.getCurrentSid('SWA1897')
	assert currentSid == None, "getCurrentSid"

	currentStar    =    terminalAreaInterface.getCurrentStar('SWA1897')
	assert currentStar == None, "getCurrentStar"

	sidLegNames    =    terminalAreaInterface.getProcedure_leg_names('SID',    
	'SSTIK4',    'KSFO')
	assert sidLegNames == None, "getProcedure_leg_names"

	waypointNames    =    terminalAreaInterface.getWaypoints_in_procedure_leg('SID',    
	'SSTIK4',    'KSFO',
	'PORTE')
	assert waypointNames == None, "getWaypoints_in_procedure_leg"

	waypointLocation    =    terminalAreaInterface.getWaypoint_Latitude_Longitude_deg('BOILE')
	assert waypointLocation == None, "getWaypoint_Latitude_Longitude_deg"

	procedureAlt1    =    terminalAreaInterface.getProcedure_alt_1('SID',    
	'SSTIK4',    'KSFO',    'PORTE',
	'KAYEX')
	assert procedureAlt1 == -1, "getProcedure_alt_1"

	procedureAlt2    =    terminalAreaInterface.getProcedure_alt_2('SID',    
	'SSTIK4',    'KSFO',    'PORTE',    'KAYEX')
	assert procedureAlt2 == -1, "getProcedure_alt_2"

	procedureSpeedLimit    =    terminalAreaInterface.getProcedure_speed_limit('SID',    'SSTIK3',    
	'KSFO',    'PORTE',    'KAYEX')
	assert procedureSpeedLimit == -1, "getProcedure_speed_limit"

	procedureAltitudeDesc    =    terminalAreaInterface.getProcedure_alt_desc('SID',    'SSTIK3',    'KSFO',
	'PORTE',    'KAYEX')
	assert procedureAltitudeDesc == None, "getProcedure_alt_desc"

	procedureSpeedLimitDesc    =    terminalAreaInterface.getProcedure_speed_limit_desc('SID',    'SSTIK3',
	'KSFO',    'PORTE',    'KAYEX')
	assert procedureSpeedLimitDesc == None, "getProcedure_speed_limit_desc"

	elevation    =    terrainInterface.getElevation(34.5, -122.23)
	assert elevation == 0.0, "getElevation"

	elevationMapBounds    =    terrainInterface.getElevationMapBounds()
	assert elevationMapBounds != None, "getElevationMapBounds"

	flightsInRange    =    safetyMetricsInterface.getFlightsInRange('SWA1897')
	assert list(flightsInRange) == [], "getFlightsInRange"
	
	#distance    =    safetyMetricsInterface.getDistanceToRunwayThreshold('SWA1897')
	#assert "{0:.8f}".format(distance) == "650.23049098", "getDistanceToRunwayThreshold"
	
	#distance    =    safetyMetricsInterface.getDistanceToRunwayEnd('SWA1897')
	#assert "{0:.12f}".format(distance) == "0.989098215393", "getDistanceToRunwayEnd"
	
	#alignmentAngle    =    safetyMetricsInterface.getVelocityAlignmentWithRunway('SWA1897',    'DEPARTURE')
	#assert "{0:.12f}".format(alignmentAngle) == "0.331237974306", "getVelocityAlignmentWithRunway"

	assert safetyMetricsInterface.getFlightsInWakeVortexRange('SWA1897',    200,    
	150,    400,    350,    2,    50) != None, "getFlightsInWakeVortexRange"

	assert safetyMetricsInterface.setAircraftBookValue('SWA1897',    5.6) == 0, "setAircraftBookValue"

	assert safetyMetricsInterface.setTouchdownPointOnRunway('SWA1897',    32.423,    
	-123.123) == 1, "setTouchdownPointOnRunway"
	
	assert list(safetyMetricsInterface.getTouchdownPointOnRunway('SWA1897')) == [32.423, -123.123], "getTouchdownPointOnRunway"

	assert safetyMetricsInterface.setTakeOffPointOnRunway('SWA1897',    37.625735, -122.368191) == 1, "setTakeOffPointOnRunway"

	assert list(safetyMetricsInterface.getTakeOffPointOnRunway('SWA1897')) == [37.625735, -122.368191], "getTakeOffPointOnRunway"

	assert safetyMetricsInterface.getL1Distance('KSFO', 'SWA1897', 'SWA1898') == -1, "getL1Distance"

	#assert safetyMetricsInterface.getDistanceToPavementEdge('KSFO', 'SWA1897') == -1, "getDistanceToPavementEdge"


	#pilotInterface    =    entityInterface.getPilotInterface()
	#assert pilotInterface.setActionRepeat('SWA1897',    'COURSE') == 1,  "setActionRepeat"

	pilotInterface    =    entityInterface.getPilotInterface()
	assert pilotInterface.skipFlightPhase('SWA1897', 'FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE') == 0, "skipFlightPhase"

	pilotInterface    =    entityInterface.getPilotInterface()
	assert pilotInterface.setWrongAction('SWA1897', 'COURSE', 'AIRSPEED') == 0, "setWrongAction"


	#pilotInterface    =    entityInterface.getPilotInterface()
	#assert pilotInterface.setActionReversal('SWA1897',    'COURSE') == 0, "setActionReversal"

	pilotInterface    =    entityInterface.getPilotInterface()
	assert pilotInterface.setPartialAction('SWA1897', 'VERTICAL_SPEED',    200,    25) == 0, "setPartialAction"


	#pilotInterface    =    entityInterface.getPilotInterface()
	#assert pilotInterface.skipChangeAction('SWA1897',    'COURSE') == 0, "skipChangeAction"


	pilotInterface    =    entityInterface.getPilotInterface()
	assert pilotInterface.setActionLag('SWA1897',    'COURSE',    10,    0.05,    30) == 0, "setActionLag"


	pilotInterface    =    entityInterface.getPilotInterface()
	assert pilotInterface.setFlightPlanReadError('SWA1897',    'VERTICAL_SPEED', 398.0) == 0, "setFlightPlanReadError"

	groundVehicle = groundVehicleInterface.select_groundVehicle("BUS123")

	if groundVehicle:
		groundOperatorInterface = entityInterface.getGroundOperatorInterface()
		assert groundOperatorInterface.setGroundOperatorAbsence('BUS123', 4) == 0, "setGroundOperatorAbsence"


		#groundOperatorInterface = entityInterface.getGroundOperatorInterface()
		#assert groundOperatorInterface.setActionRepeat('BUS123', 'SPEED') == 1, "setActionRepeat"


		#groundOperatorInterface = entityInterface.getGroundOperatorInterface()
		#assert groundOperatorInterface.setVehicleContact('BUS123') == 0, "setVehicleContact"


		#groundOperatorInterface = entityInterface.getGroundOperatorInterface()
		#assert groundOperatorInterface.setActionReversal('BUS123', 'COURSE') == 0, "setActionReversal"

		#groundOperatorInterface = entityInterface.getGroundOperatorInterface()
		#assert groundOperatorInterface.setPartialAction('BUS123', 'SPEED', 8, 50) == 0, "setPartialAction"


		#groundOperatorInterface = entityInterface.getGroundOperatorInterface()
		#assert groundOperatorInterface.setActionLag('BUS123', 'SPEED', 10, 0.5, 30) == 0, "setActionLag"

	
	#groundVehicleInterface.release_groundVehicle()
	
	aircraftInterface.release_aircraft()
	
	print()
	
except Exception as e:
	print(("Exception: ", e))
	result = 1
	pass

if result == 0:
    print("All GNATS API Functions have been tested successfully.")
    
gnatsClient.disConnect()
