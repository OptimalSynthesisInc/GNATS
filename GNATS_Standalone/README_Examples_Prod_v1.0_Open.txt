Generalized National Airspace Trajectory-Prediction System(GNATS)

Version Production 1.0 Linux Distribution

Following are the description of the examples:

1. sample/DEMO_MC_Simulation/Example_MC_code.py: This is the Monte Carlo simulation frontend. You can create samples and insert it in GNATS for simulation.
   sample/DEMO_MC_Simulation/NATS_header.py: This module contains all the frontmatter that has to be inserted in any given GNATS Client program.
   sample/DEMO_MC_Simulation/NATS_MonteCarlo_Interface.py: This is the Monte Carlo simulation backend. It takes in all the inputs from the Monte Carlo frontend, processes it, run the simulation and produces outputs in form of csv files.
   sample/DEMO_MC_Simulation/PostProcessor.py: It is the visualization and post processing module for GNATS simulations. It takes in MC simulation results and produces histograms and plots for state variables.

2. sample/DEMO_Aircraft_Functions_beta1.9.py: Basic aircraft function examples.
   sample/DEMO_Aircraft_Functions_beta1_9.m: MATLAB sample.

3. sample/DEMO_Aircraft_State_Change_beta1.9.py: Sample program demonstrating how to change aircraft state.
   sample/DEMO_Aircraft_State_Change_beta1_9.m: MATLAB sample.

4. sample/DEMO_ControllerInterface_Prod1.0.py: Demo of Controller module functionality.

5. sample/DEMO_Gate_To_Gate_Simulation_SFO_PHX_beta1.9.py: Gate-to-gate simulation from SFO to PHX.
   sample/DEMO_Gate_To_Gate_Simulation_SFO_PHX_beta1_9.m: MATLAB sample.

6. sample/DEMO_RiskMeasuresInterface_Prod1.0.py: Demo of RiskMeasures module functionality.

7. sample/DEMO_WSSS_RJAA_Hold_Pattern_beta1.9.py: Demo of several hold patterns of a aircraft.

8. sample/Octave_SampleMonteCarloController_Beta_1.9.m: GNU Octave program of Monte Carlo simulation by changing Controller behavior. It plots out the graph showing crucial flight parameters after simulation runs through.

9. sample/Octave_SampleMonteCarloPilot_Beta_1.9.m: GNU Octave program of Monte Carlo simulation by changing Pilot behavior. It plots out the graph showing crucial flight parameters after simulation runs through.

10. sample/Octave_SampleMonteCarloRiskMeasures_Prod_1.0.m: GNU Octave program of Monte Carlo simulation by changing risk measures parameters. It plots out the graph showing crucial flight parameters after simulation runs through.

11. sample/Octave_SampleMonteCarloGroudParameters_Beta_1.9.m: GNU Octave program of Monte Carlo simulation by changing aircraft ground parameters. It plots out the graph showing crucial flight parameters after simulation runs through.

12. sample/PlotGraph.m: This is a supplementary function to SampleMonteCarlo modules. It is used to plot out graphs for desired flight parameters.

13. sample/SampleMonteCarloController_Beta_1_9.m: MATLAB program of Monte Carlo simulation by changing Controller behavior. It plots out the graph showing crucial flight parameters after simulation runs through.

14. sample/SampleMonteCarloPilot_Beta_1_9.m: MATLAB program of Monte Carlo simulation by changing Pilot behavior. It plots out the graph showing crucial flight parameters after simulation runs through.

15. sample/SampleMonteCarloRiskMeasures_Prod_1_0.m: MATLAB program of Monte Carlo simulation by changing risk measures parameters. It plots out the graph showing crucial flight parameters after simulation runs through.

16. sample/SampleMonteCarloGroundParameters_Beta_1_9.m: MATLAB program of Monte Carlo simulation by changing aircraft ground parameters. It plots out the graph showing crucial flight parameters after simulation runs through.

17. sample/PathVisualizer.py: This python module is a helper function to plot trajectories on Google Map after the simulation goes through. Pre-requisite for this is Google Chrome or Mozilla Firefox browser.

18. sample/Scilab_SampleMonteCarloController_Beta_1.9.sce: Scilab program of Monte Carlo simulation by changing Controller behavior. It plots out the graph showing crucial flight parameters after simulation runs through.

19. sample/Scilab_SampleMonteCarloPilot_Beta_1.9.sce: Scilab program of Monte Carlo simulation by changing Pilot behavior. It plots out the graph showing crucial flight parameters after simulation runs through.

20. sample/Scilab_SampleMonteCarloRiskMeasures_Prod_1.0.sce: Scilab program of Monte Carlo simulation by changing risk measures. It plots out the graph showing crucial flight parameters after simulation runs through.

21. sample/Scilab_SampleMonteCarloGroudParameters_Beta_1.9.sce: Scilab program of Monte Carlo simulation by changing aircraft ground parameters. It plots out the graph showing crucial flight parameters after simulation runs through.

22. sample/DEMO_AircraftLoadAndCostFunctions_Prod1.0.py: Python module to demonstrate use of the aircraft and cargo load and cost functions. The API documentation has the function usage information.

23. sample/DEMO_WakeVortexModel_beta1.9.py: This example demonstrates the function to model a wake vortex scenario, it yields the aircraft that would be within hazard range.

24. sample/DEMO_SimulationSubscription_beta1.9.py: This example demonstrates a simulation situation with subscription to flight state data at every time step. The code contains documentation to explain the concept in further detail.

25. sample/DEMO_ARTCC_Functions_beta1.9.py: Example usage of ARTCC functions to get center data including fixes contained in a given center.

26. sample/DEMO_CNSInterface_beta1.9.py: Demonstrates examples of functions within the CNS Interface.

27. sample/DEMO_Aircraft_Validator_Flight_Plan_Record_beta1.9.py: Validator of aircraft flight plan record.

28. sample/DEMO_Unittests_noCIFP_beta1.9.py: Unit test case suite for GNATS functions.
    Notice. This program is in continuous revision and will provide comprehensive test cases.  If you experience problems, please contact GNATS development team.

29. sample/DEMO_R_Monte_Carlo_Example_Beta_1.9.r: This is a sample R program to interface with GNATS Server to run Monte Carlo Simulation. Please see What_is_New.txt for further details.

30. sample/DEMO_Gate_To_Gate_Simulation_WSSS_RJAA_geo_beta1.9.py: Sample simulation of geo-location TRX record from Singapore Changyi international airport to Japan Narita international airport.

31. sample/DEMO_R_Monte_Carlo_Example_Beta_1.9.r: Monte Carlo R Programming language example of gate to gate simulation using GNATS.

31. sample/DEMO_TerrainInterface_beta1.10.py: Demonstration of terrain interface functions
