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

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;

import java.lang.reflect.Method;
import java.net.Socket;

import com.osi.gnats.api.EntityInterface;
import com.osi.gnats.api.EnvironmentInterface;
import com.osi.gnats.api.EquipmentInterface;
import com.osi.gnats.api.RiskMeasuresInterface;
import com.osi.gnats.api.SimulationInterface;

import com.osi.util.Constants;
import com.osi.util.Utils;

public class GNATSStandalone {
	private static GNATSStandalone instance;

	private static Class clsNATSClient;
	
	private static Object objNATSClient;
	
	private ProcessBuilder processBuilder;
	private Process processServer;

	private Thread_InputStream thread_Stdout;
	
	private static final int timeout_millisec = 60 * 1000;

	static {
		try {
			clsNATSClient = Class.forName("GNATSClient");
		} catch (Exception ex) {
			//ex.printStackTrace();
		}
	}
	
	public static GNATSStandalone start() {
		String[] args = null;
		
		return start(args);
	}
	
	public static GNATSStandalone start(String[] args) {
		if (args != null) {
			for (int i = 0; i < args.length; i++) {
				if ("-version".equals(args[i])) {
					GNATSServer.displayVersion();
					
					return null;
				}
			}
		}

		try {
			instance = new GNATSStandalone();
			
			instance.start_post();
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return instance;
	}

	private void start_post() throws Exception {
		processBuilder = new ProcessBuilder();

		if (Utils.isWindowsPlatform()) {
			processBuilder.command("./run.bat");
		} else {
			processBuilder.inheritIO();
			
			processBuilder.command("./run");
		}
		
		processBuilder.inheritIO();

		processServer = processBuilder.start();

		if (Utils.isWindowsPlatform()) {
			thread_Stdout = new Thread_InputStream(processServer.getInputStream());
			thread_Stdout.start();
		}

		try {
			Thread.sleep(1000);
		} catch (Exception ex) {}
		
		int tmpDurationMillisec = 0;
		int tmpTimeinterval = 500;
		
		while (true) {
			try {
    			Socket socket = new Socket("localhost", Constants.SOCKET_PORT_NUMBER);

    			socket.close();
    			
    			break;
			} catch (Exception ex) {
				//ex.printStackTrace();
			}
			
			if (timeout_millisec < tmpDurationMillisec) {
				throw new Exception("Timeout.  Can't start GNATSStandalone");
			}
			
			try {
				Thread.sleep(tmpTimeinterval);
			} catch (Exception ex) {}
			
			tmpDurationMillisec += tmpTimeinterval;
		}
		
		if (clsNATSClient != null) {
			Method method_getInstance = clsNATSClient.getMethod("getInstance");
			objNATSClient = method_getInstance.invoke(null);
		}
	}
	
	public void stop() throws Exception {
		if (processServer != null) {
	    	try {
		    	if ((clsNATSClient != null) && (objNATSClient != null)) {
		    		Method method = clsNATSClient.getMethod("stopStandaloneServer");
					method.invoke(objNATSClient);
		    	}
	    	} catch (Exception ex) {
				throw ex;
			}
		}
	}
	
	// ========================================================================

    public EntityInterface getEntityInterface() {
    	EntityInterface retObject = null;
    	
    	try {
	    	if ((clsNATSClient != null) && (objNATSClient != null)) {
	    		Method method = clsNATSClient.getMethod("getEntityInterface");
				retObject = (EntityInterface)method.invoke(objNATSClient);
	    	}
    	} catch (Exception ex) {
			ex.printStackTrace();
		}
    	
    	return retObject;
    }

    public SimulationInterface getSimulationInterface() {
    	SimulationInterface retObject = null;
    	
    	try {
	    	if ((clsNATSClient != null) && (objNATSClient != null)) {
	    		Method method = clsNATSClient.getMethod("getSimulationInterface");
				retObject = (SimulationInterface)method.invoke(objNATSClient);
	    	}
    	} catch (Exception ex) {
			ex.printStackTrace();
		}
    	
    	return retObject;
    }

    public EquipmentInterface getEquipmentInterface() {
    	EquipmentInterface retObject = null;
    	
    	try {
	    	if ((clsNATSClient != null) && (objNATSClient != null)) {
	    		Method method = clsNATSClient.getMethod("getEquipmentInterface");
				retObject = (EquipmentInterface)method.invoke(objNATSClient);
	    	}
    	} catch (Exception ex) {
			ex.printStackTrace();
		}
    	
    	return retObject;
    }

    public EnvironmentInterface getEnvironmentInterface() {
    	EnvironmentInterface retObject = null;
    	
    	try {
	    	if ((clsNATSClient != null) && (objNATSClient != null)) {
	    		Method method = clsNATSClient.getMethod("getEnvironmentInterface");
				retObject = (EnvironmentInterface)method.invoke(objNATSClient);
	    	}
    	} catch (Exception ex) {
			ex.printStackTrace();
		}
    	
    	return retObject;
    }
    
    public RiskMeasuresInterface getRiskMeasuresInterface() {
    	RiskMeasuresInterface retObject = null;
    	
    	try {
	    	if ((clsNATSClient != null) && (objNATSClient != null)) {
	    		Method method = clsNATSClient.getMethod("getRiskMeasuresInterface");
				retObject = (RiskMeasuresInterface)method.invoke(objNATSClient);
	    	}
    	} catch (Exception ex) {
			ex.printStackTrace();
		}
    	
    	return retObject;
    }

    public RiskMeasuresInterface getRiskMInterface() {
    	return getRiskMeasuresInterface();
    }

    public void info() {
    	try {
	    	if ((clsNATSClient != null) && (objNATSClient != null)) {
	    		Method method = clsNATSClient.getMethod("info");
				method.invoke(objNATSClient);
	    	}
    	} catch (Exception ex) {
			ex.printStackTrace();
		}
    }
    
    // Class to handle input stream
    // NATS Server is actually started as an independent process
    // To receive the stdout from the process, this class reads its input stream and output in the current process
    private class Thread_InputStream extends Thread {
    	private InputStream inputStream = null;
    	
    	public Thread_InputStream(InputStream inputStream) {
    		this.inputStream = inputStream;
    	}
    	
    	public void run() {
    		InputStreamReader inputSR = new InputStreamReader(inputStream);
    		BufferedReader br = new BufferedReader(inputSR, 1);
    		
    	    String line;
    	    try {
    	    	while ((line=br.readLine()) != null) {
    	    		System.out.println(line);
    	    		System.out.flush();
    	        }
    	    } catch (Exception ex) {
    	    	ex.printStackTrace();
    	    	
    	    	return;
    	    }
    	}
    }
}
