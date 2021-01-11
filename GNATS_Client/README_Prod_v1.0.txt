Generalized National Airspace Trajectory-Prediction System(GNATS)

Version Production 1.0

Client Program Module

This program module allows the users to access the GNATS functions implemented on the server through the Java Remote Method Invocation (RMI) technology. Users can create customized programs by calling server-side RMI functions to create and load gate-to-gate flight plans, weather, Center/Sector definition databases, propagate aircraft trajectories in the National Airspace Ssystem (NAS,output trajectory data and create plots for visualization and analysis.  Please refer to dist/javadoc_client/com/osi/gnats/api/package-frame.html for additional information on available functions (API).


Contents of the zip file GNATS-Client.zip
===================================
<data>               Dir    Runtime data directory
<dist>               Dir    GNATS binary files
<sample>             Dir    GNATS sample codes
README.txt
README_Examples.txt
What_is_New.txt


Hardware Requirements
=====================
1. Intel/AMD 64bit CPU 1Ghz and up
2. RAM capacity at least 1GB


Software Requirements
=====================
1. Linux 64bit Operating System
   GNATS software has been tested on:
   # Ubuntu v12.04, v16.04 with gcc 4.8
   # CentOS 6.9 with gcc 4.4
   # CentOS 7 with gcc 4.8
2. Java 1.7 and later
3. Jpype(For Python codes)
   Tested on v0.6.3
   
   Please don't use newer version of Jpype.  Currently GNATS is developed in Java 1.7 and it only works with Jpype 0.6.3.
   
   To install Jpype 0.6.3
   Go to https://github.com/originell/jpype
   Download jpype-0.6.3.tar.gz
   Unzip it, enter the directory and execute commands.
       sudo python setup.py install
4. MATLAB(if applicable)
   Tested on MATLAB R2014b, R2015b, R2019a
5. Python 2.7


GNATS client config file
=======================
The config file is located at data/gnats.config
Two required required parameters are:
1. Hostname/IP address of the target GNATS server to connect to.
   If GNATS client is to connect to different machine, please set correct host name or IP address.
2. Port number to connect to(default: 2017)


File permission change
======================
chmod +x gnatshelp


Running sample programs
===========================
GNATS interface functions are written in Java and provided through the server-client mechanism to the user for their software development.  Python and MATLAB interfaces are provided for interactive or batch computations by non-Java users.  Certain bridge plugins/wrapers are required to enable this functionality.
===========================
1. Usage with Python Jpype
   (Please install Jpype plugin ready first.)
   1.1 Set the environment variable JAVA_HOME
   1.2 Edit the sample/basic_python_example_beta1.5.py and specify the correct path of gnats-client.jar, gnats-shared.jar and json.jar
   1.3 Execute
           python sample/basic_python_example_beta1.5.py

Note: If you are running in a Python IDE, please make sure that you set JAVA_HOME and set the working directory to GNATS_Client in the runtime configuration.

2. Usage with MATLAB
   2.1 Edit sample/DEMO_Gate_To_Gate_Simulation_SFO_PHX_beta1_5.m and specify the correct path of gnats-client.jar, gnats-shared.jar and json.jar
   2.2 Run sample/DEMO_Gate_To_Gate_Simulation_SFO_PHX_beta1_5.m.

3. Usage with Octave
   Please refer to "Scilab and Octave Documentation" file for installation and sample file.

4. Usage with SciLab
   Please refer to "Scilab and Octave Documentation" file for installation and sample file.


Documentation
=============
HTML documentation can be viewed in GNATS_Client/dist/doc/html/index.html


Q & A
=====
1. The client program throws out an error about not finding the gnats.config file.
   Ans. Client program will look for data/gnats.config file in runtime.  Please make sure you execute your program under GNATS_Client directory.  If you have to execute the client program in different directory, please set the OS environment variable GNATS_CLIENT_HOME to the path of GNATS_Client folder.
        Example. In Linux command-line terminal, type
            export GNATS_CLIENT_HOME=/your_path/GNATS_Client
