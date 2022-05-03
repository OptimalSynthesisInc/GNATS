# GNATS sample
#
# Optimal Synthesis Inc.
#
# Oliver Chen
# 03.12.2020
#
# Demo of flight plan validator

from GNATS_Python_Header_standalone import *

if aircraftInterface is None :
    print("Can't get AircraftInterface")
else :
    result = aircraftInterface.validate_flight_plan_record('TRACK SQ12 B773 12127.2484 1035936.2868 150 0.22 28', 'FP_ROUTE {"ap_code": "WSSS", "ap_name": "Singapore Changi Airport", "lat": "11945.696", "lon": "1035906.813", "alt": 22.0}.<{"type": "gate", "lat": "12127.2484", "lon": "1035936.2868"}, {"type": "ramp", "lat": "12129.6922", "lon": "1035934.695"}, {"type": "taxiway", "lat": "12131.5966", "lon": "1035933.6984"}, {"type": "taxiway", "lat": "12145.4782", "lon": "1035939.4728"}, {"type": "taxiway", "lat": "12148.3984", "lon": "1035943.0434"}, {"type": "runway", "lat": "12142.7752", "lon": "1035956.2122"}>.RW<{"lat": "12142.7752", "lon": "1035956.2122"}, {"lat": "11944.223", "lon": "1035906.2082"}>.<{"wp_name": "Waypoint 1", "lat": "11945.696", "lon": "1035906.813", "alt": 22, "phase": "TAKEOFF"}, {"wp_name": "Waypoint 2", "lat": "11608.0646", "lon": "1035734.0662", "alt": 1000, "phase": "CLIMBOUT"}, {"wp_name": "Waypoint 3", "lat": "11420.241", "lon": "1035907.3782", "alt": 2500, "phase": "CLIMBOUT"}, {"wp_name": "Waypoint 4", "lat": "11617.508", "lon": "1041035.4138", "alt": 5500, "phase": "CLIMBOUT"}, {"wp_name": "Waypoint 5", "lat": "12627.945", "lon": "1044456.2194", "alt": 8500, "phase": "CLIMBOUT"}, {"wp_name": "Waypoint 6", "lat": "30319.317", "lon": "1072333.252", "alt": 23000, "phase": "CLIMB_TO_CRUISE_ALTITUDE"}, {"wp_name": "Waypoint 7", "lat": "63527.4014", "lon": "1122013.1094", "alt": 33000, "phase": "CRUISE"}, {"wp_name": "Waypoint 8", "lat": "132407.3758", "lon": "1200532.2182", "alt": 33000, "phase": "CRUISE"}, {"wp_name": "Waypoint 9", "lat": "225233.9378", "lon": "1291324.8586", "alt": 33000, "phase": "CRUISE"}, {"wp_name": "Waypoint 10", "lat": "331540.1538", "lon": "1365831.8828", "alt": 20000, "phase": "INITIAL_DESCENT"}, {"wp_name": "Waypoint 11", "lat": "333245.2724", "lon": "1381556.8476", "alt": 12000, "phase": "APPROACH"}, {"wp_name": "Waypoint 12", "lat": "350743.3596", "lon": "1402830.8094", "alt": 8500, "phase": "APPROACH"}, {"wp_name": "Waypoint 13", "lat": "352622.3974", "lon": "1403704.1154", "alt": 4500, "phase": "FINAL_APPROACH"}, {"wp_name": "Waypoint 14", "lat": "353622.1796", "lon": "1402930.5628", "alt": 2500, "phase": "FINAL_APPROACH"}, {"wp_name": "Waypoint 15", "lat": "354203.7116", "lon": "1402516.7448", "alt": 1000, "phase": "FINAL_APPROACH"}, {"wp_name": "Waypoint 16", "lat": "354438.9652", "lon": "1402324.471", "alt": 135, "phase": "FINAL_APPROACH"}>.RW<{"lat": "354436.4878", "lon": "1402326.25"}, {"lat": "354627.1596", "lon": "1402206.2724"}>.<{"type": "runway", "lat": "354618.1662", "lon": "1402212.81"}, {"type": "taxiway", "lat": "354623.1348", "lon": "1402212.882"}, {"type": "taxiway", "lat": "354624.2184", "lon": "1402215.1458"}, {"type": "taxiway", "lat": "354623.325", "lon": "1402218.2958"}, {"type": "taxiway", "lat": "354555.7028", "lon": "1402238.2548"}, {"type": "ramp", "lat": "354600.9654", "lon": "1402248.8748"}, {"type": "ramp", "lat": "354605.9628", "lon": "1402259.6274"}, {"type": "gate", "lat": "354603.0648", "lon": "1402302.0178"}>.{"ap_code": "RJAA", "ap_name": "Narita International Airport", "lat": "354438.9652", "lon": "1402324.471", "alt": 135}', 37000)
    print("Result of validation of flight plan = ", result)

# Stop GNATS Standalone environment
gnatsStandalone.stop()

shutdownJVM()
