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


import java.net.DatagramSocket;
import java.net.InetAddress;
import java.rmi.*;
import java.rmi.server.*;
import java.rmi.registry.*;
import java.util.HashMap;
import java.util.Iterator;

import com.osi.gnats.server.ServerClass;
import com.osi.gnats.server.ServerNATS;
import com.osi.util.Constants;
import com.osi.util.Utils;

public class GNATSServer {
	private static final String GNATS_SERVER_NAME = "Generalized National Airspace Trajectory-Prediction System(GNATS) Server";
	private static final String GNATS_STANDALONE_NAME = "Generalized National Airspace Trajectory-Prediction System(GNATS) Standalone";
	private static final String OSI_COMPANY_NAME = "Optimal Synthesis Inc.";
	private static final String NATS_SERVER_VER = "Release 2.0";
	private static final String NATS_SERVER_WIN_VER = "Release 1.0 Windows 10 64bit Distribution";
	
	private static boolean flag_standalone_mode = false;
	
	ServerNATS serverNATS = null;
	
	private static String logLevel = null;
	
	private static boolean flag_gdb = false;
	
	private String ip_addr = null;

    static {
		try {
			Class clsNATSStandalone = Class.forName("GNATSStandalone");
			
			flag_standalone_mode = true;
		} catch (Exception ex) {
			//ex.printStackTrace();
		}
    }

    /**
     * Default constructor
     */
	public GNATSServer(String[] args) {
		this((flag_standalone_mode) ? Integer.parseInt(Constants.DEFAULT_STANDALONE_RMI_PORT_NUMBER) : Integer.parseInt(Constants.DEFAULT_SERVER_RMI_PORT_NUMBER), args);
	}
	
	/**
	 * Constructor
	 * @param port Port number used for RMI listening
	 */
	public GNATSServer(int port, String[] args) {
		if (args != null) {
			for (int i = 0; i < args.length; i++) {
				if (args[i].startsWith("log=")) {
					logLevel = args[i].substring(args[i].indexOf("log=")+4);
				}
				
				if (args[i].startsWith("-gdb")) {
					flag_gdb = true;
				}
			}
		}

		try {
			if (flag_standalone_mode) {
				System.out.println("================================================================================");
				System.out.println("  " + GNATS_STANDALONE_NAME);
			} else {
				System.out.println("============================================================================");
				System.out.println("  " + GNATS_SERVER_NAME);
			}
			
			if (Utils.isWindowsPlatform()) {
				System.out.println("  Version: " + NATS_SERVER_WIN_VER);
			} else {
				System.out.println("  Version: " + NATS_SERVER_VER);
			}
			
			System.out.println();
			System.out.println("  " + OSI_COMPANY_NAME);
			
			if (flag_standalone_mode) {
				System.out.println("================================================================================");
			} else {
				System.out.println("============================================================================");
			}
			
			try (final DatagramSocket socket = new DatagramSocket()) {
				 socket.connect(InetAddress.getByName("8.8.8.8"), 10002);
				 ip_addr = socket.getLocalAddress().getHostAddress();
			}
			
			System.setProperty("java.rmi.server.hostname", ip_addr);
			
			System.setProperty("org.apache.commons.logging.Log",
	                "org.apache.commons.logging.impl.NoOpLog");

			serverNATS = new ServerNATS(logLevel, ip_addr, flag_standalone_mode, flag_gdb);
			
			Registry rmiRegistry = LocateRegistry.createRegistry(port);
	
	        if ( exportObjects(rmiRegistry, port) ) {
	            if (flag_standalone_mode) {
	            	System.out.println("\nGNATS Standalone ready");
	            } else {
	            	System.out.println("\nGNATS Server started on port " + port + ".");
	            }
	            	
	            serverNATS.startServerSocket();
			} else {
				if (flag_standalone_mode) {
					System.out.println("GNATS Standalone: Error binding remote objects.");
				} else {
					System.out.println("GNATS Server: Error binding remote objects.");
				}
			}
		} catch (Exception e) {
			if (flag_standalone_mode) {
				System.out.println("error loading GNATS Standalone");
			} else {
				System.out.println("error loading GNATS Server");
			}
			
			e.printStackTrace();
		}
	}
	
    private boolean exportObjects(Registry registry, int port) {
        try {
            String serverAndPort = ip_addr + ":" + port;
            String namePrefix = "//" + serverAndPort + "/";
            
            Remote remoteStub;
            remoteStub = UnicastRemoteObject.exportObject(serverNATS, port);
            Naming.rebind(namePrefix+"RemoteNATS", remoteStub);

            HashMap<String, ServerClass> serverClasses = serverNATS.getServerClasses();
            
			Iterator<String> iter = serverClasses.keySet().iterator();
            // export and bind the remote objects to the port
            while ( iter.hasNext() ) {
                String name = iter.next();
                remoteStub = UnicastRemoteObject.exportObject( (Remote)serverClasses.get(name), port );
                Naming.rebind(namePrefix+"Remote"+name, remoteStub);
            }
            
            return true;
        } catch(Exception e) {
            e.printStackTrace();
            return false;
        }
    }
	
    /**
     * Display version information
     */
    public static void displayVersion() {
    	System.out.println(GNATS_SERVER_NAME);
    	System.out.println("Version: " + (Utils.isWindowsPlatform() ? NATS_SERVER_WIN_VER : NATS_SERVER_VER));
    	System.out.println();
    	System.out.println(OSI_COMPANY_NAME);
    }
    
    public static boolean isStandalone_mode() {
    	return flag_standalone_mode;
    }
    
	public static void main(String[] args) {
		if (args.length > 0) {
			try {
				for (int i = 0; i < args.length; i++) {
					if ("-version".equals(args[i])) {
						displayVersion();
						return;
					} else if (args[i].startsWith("log=")) {
						logLevel = args[i].substring(args[i].indexOf("log=")+4);
					}
				}
				
				int portNum = Integer.parseInt(args[0]);
				
				GNATSServer natsServer = null;
				if (portNum >= 0) {
					natsServer = new GNATSServer(portNum, args);
				} else {
					natsServer = new GNATSServer(args);
				}
			} catch (Exception ex) {
				ex.printStackTrace();
			}
		}
	}

}
