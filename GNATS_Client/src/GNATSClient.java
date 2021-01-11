/*
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
*/

import java.util.ArrayList;
import java.util.HashMap;

import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.FileReader;
import java.lang.reflect.InvocationTargetException;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.MalformedURLException;
import java.net.Socket;
import java.rmi.*;

import com.osi.gnats.api.BaseInterface;
import com.osi.gnats.api.EntityInterface;
import com.osi.gnats.api.EnvironmentInterface;
import com.osi.gnats.api.EquipmentInterface;
import com.osi.gnats.api.NATSInterface;
import com.osi.gnats.api.RiskMeasuresInterface;
import com.osi.gnats.api.SimulationInterface;

import com.osi.gnats.client.BaseClass;
import com.osi.gnats.client.ClientEntity;
import com.osi.gnats.client.ClientEnvironment;
import com.osi.gnats.client.ClientEquipment;
import com.osi.gnats.client.ClientNATS;
import com.osi.gnats.client.ClientSimulation;

import com.osi.gnats.rmi.RemoteEntity;
import com.osi.gnats.rmi.RemoteEnvironment;
import com.osi.gnats.rmi.RemoteEquipment;
import com.osi.gnats.rmi.RemoteNATS;
import com.osi.gnats.rmi.RemoteRiskMeasures;
import com.osi.gnats.rmi.RemoteSimulation;
import com.osi.util.Constants;
import com.osi.util.Utils;

public class GNATSClient extends BaseClass implements NATSInterface {
	private static final String GNATS_CLIENT_NAME = "Generalized National Airspace Trajectory-Prediction System(GNATS) Client";
	private static final String OSI_COMPANY_NAME = "Optimal Synthesis Inc.";
	private static final String NATS_CLIENT_VER = "Production 1.0 Linux Distribution";
	private static final String NATS_CLIENT_WIN_VER = "Production 1.0 Windows 10 64bit Distribution";

	private static boolean flag_standalone_mode = false;
	
	private long serverProcessId = -1;
	
	private static Thread_ClientSocket thread_ClientSocket = null;
	
	protected static NATSInterface natsClient = null;

	private String serverAddress;
	private String portNum;

	private static RemoteNATS remoteNATS;
    private ClientNATS clientNATS;
	
    private RemoteEntity remoteEntity;
    private EntityInterface entity;
    public EntityInterface getEntityInterface() { return (EntityInterface)getInterface("Entity"); }
    
    private RemoteSimulation remoteSimulation;
    private SimulationInterface simulation;
    public SimulationInterface getSimulationInterface() { return (SimulationInterface)getInterface("Simulation"); }

    private RemoteEquipment remoteEquipment;
    private EquipmentInterface equipment;
    public EquipmentInterface getEquipmentInterface() { return (EquipmentInterface)getInterface("Equipment"); }

    private RemoteEnvironment remoteEnvironment;
    private EnvironmentInterface environment;
    public EnvironmentInterface getEnvironmentInterface() { return (EnvironmentInterface)getInterface("Environment"); }
    
    private RemoteRiskMeasures remoteRiskMeasures;
    private RiskMeasuresInterface riskMeasures;
    public RiskMeasuresInterface getRiskMeasuresInterface() { return (RiskMeasuresInterface)getInterface("RiskMeasures"); }
    
    public RiskMeasuresInterface getRiskMInterface() { return (RiskMeasuresInterface)getInterface("RiskMeasures"); }

    static {
		try {
			Class clsNATSStandalone = Class.forName("GNATSStandalone");
			
			flag_standalone_mode = true;
		} catch (Exception ex) {
			//ex.printStackTrace();
		}
    }
    
    public boolean isStandaloneMode() {
    	return flag_standalone_mode;
    }
    
    public String getVersion() {
    	String retString = GNATS_CLIENT_NAME + "\n"
    			+ "Version: " + (Utils.isWindowsPlatform() ? NATS_CLIENT_WIN_VER : NATS_CLIENT_VER) + "\n"
    			+ "\n"
    			+ OSI_COMPANY_NAME;
    	
    	return retString;
    }
    
    public void info() throws RemoteException {
    	if (remoteNATS != null) {
    		remoteNATS.info();
    	}
    }
    
    /**
     * Default constructor
     * 
     * @throws IOException
     */
	private GNATSClient() throws IOException {
		this("data");

		init();
	}
	
	/**
	 * Constructor
	 * 
	 * @param configFileDir Directory of NATS config file
	 * @throws IOException
	 */
	private GNATSClient(String configFileDir) throws IOException {
		super(null, "GNATS", "GNATS Root Interface", "GNATS Root Interface");
        
        interfacesMap = new HashMap<String, BaseInterface>();
        
        if (flag_standalone_mode) {
        	serverAddress = Constants.DEFAULT_SERVER_RMI_IP_ADDRESS;
        	portNum = Constants.DEFAULT_STANDALONE_RMI_PORT_NUMBER;
        } else {
        	readNATSConfig(configFileDir); // Read server address and port number
        }
	}
	
	private void init() throws IOException {
		if (!flag_standalone_mode) {
			System.out.println("============================================================================");
			System.out.println("  " + GNATS_CLIENT_NAME);
			if (Utils.isWindowsPlatform()) {
				System.out.println("  Version: " + NATS_CLIENT_WIN_VER);
			} else {
				System.out.println("  Version: " + NATS_CLIENT_VER);
			}
			System.out.println();
			System.out.println("  " + OSI_COMPANY_NAME);
			System.out.println("============================================================================\n");
			
			String strIpAddr = null;
	
			try (final DatagramSocket socket = new DatagramSocket()) {
				 socket.connect(InetAddress.getByName("8.8.8.8"), 10002);
				 strIpAddr = socket.getLocalAddress().getHostAddress();
			}
		}
		
		// Try to connect to the NATS server
		if (loadRemoteObjects(serverAddress, portNum)) {
	        loadInterfaceObjects(serverAddress, portNum);

	        GNATSClient.natsClient = this;
	        
	        if (!flag_standalone_mode) {
	        	System.out.println("Connected to GNATS Server ("+serverAddress+":"+portNum+")");
	        }
		} else {
			System.out.println("Unable to connect to GNATS Server ("+serverAddress+":"+portNum+")");
		}
	}
	
	public static GNATSClient getInstance() throws IOException, Exception {
		boolean flag_calledFrom_NATSStandalone = false;
		
		StackTraceElement[] arrayStackTrace = Thread.currentThread().getStackTrace();
		for (int i = 0; i < arrayStackTrace.length; i++) {
			if ("GNATSStandalone".equals(arrayStackTrace[i].getClassName())) {
				flag_calledFrom_NATSStandalone = true;
				
				break;
			}
		}

		if (flag_standalone_mode) {
			if (!flag_calledFrom_NATSStandalone)
				throw new Exception(Constants.MSG_FUNC_INVALID_STANDALONE_MODE);
			
			natsClient = null;
		}
		
		if (natsClient == null) {
			new GNATSClient();
		}
		
		if (natsClient != null) {
			return (GNATSClient)natsClient;
		} else {
			return null;
		}
	}
	
	/*
	 * Read NATS configuration file
	 */
	private void readNATSConfig(String configFileDir) {
		FileReader fReader = null;
		BufferedReader bReader = null;
		
		try {
			fReader = new FileReader(configFileDir + "/gnats.config");
			bReader = new BufferedReader(fReader);
			
			this.serverAddress = Utils.getNextNonEmptyLine(bReader);
			this.portNum = Utils.getNextNonEmptyLine(bReader);
		} catch (Exception e) {
			// Try to get GNATS_CLIENT_HOME environment variable to determine config file path
			try {
				String natsHome = System.getenv().get("GNATS_CLIENT_HOME");
				if ((natsHome == null) || ("".equals(natsHome))) {
					System.out.println("Error reading gnats.config file.  Environment variable GNATS_CLIENT_HOME is not found or not valid.");
					e.printStackTrace();
				} else {
					fReader = new FileReader(natsHome + "/" + configFileDir + "/gnats.config");
					bReader = new BufferedReader(fReader);
					this.serverAddress = Utils.getNextNonEmptyLine(bReader);
					this.portNum = Utils.getNextNonEmptyLine(bReader);
				}
			} catch (Exception ex) {
				System.out.println("Error reading gnats.config file");
				ex.printStackTrace();
			}
		}
	}
	
	/*
	 * Load RMI remote objects
	 * 
	 * Remote interfaces map to the actual RMI server objects
	 * When we use remoteXXX interface to execute methods, it actually triggers the server-side function.
	 */
	private boolean loadRemoteObjects(String serverAddress, String portNum) {
		try {
			String serverAndPort = serverAddress + ":" + portNum;
			String lookupPrefix = "//" + serverAndPort + "/";
			
			this.remoteNATS = (RemoteNATS)Naming.lookup(lookupPrefix+"RemoteNATS");

			this.clientNATS = new ClientNATS(remoteNATS);

			this.remoteEntity = (RemoteEntity)Naming.lookup(lookupPrefix+"RemoteEntity");
			this.entity = new ClientEntity(this, remoteEntity);

			this.remoteEnvironment = (RemoteEnvironment)Naming.lookup(lookupPrefix+"RemoteEnvironment");
			this.environment = new ClientEnvironment(this, remoteEnvironment);

			this.remoteEquipment = (RemoteEquipment)Naming.lookup(lookupPrefix+"RemoteEquipment");
			this.equipment = new ClientEquipment(this, remoteEquipment);
			
			this.remoteSimulation = (RemoteSimulation)Naming.lookup(lookupPrefix+"RemoteSimulation");
			this.simulation = new ClientSimulation(this, remoteSimulation);

			if (!flag_standalone_mode) {
				thread_ClientSocket = new Thread_ClientSocket(serverAddress, Constants.SOCKET_PORT_NUMBER);
				thread_ClientSocket.start();
			}
			
			return true;
		} catch (Exception e) {
			System.out.println("No GNATS Server available.");
			e.printStackTrace();
			return false;
		}
	}
	
	/*
	 * Load interface objects
	 * 
	 * This method is to collect Client objects and put them in a map structure.
	 * The logic is to get class names under com.osi.gnats.client.  For each name, find the same-name Client class and use its constructor to create a new instance.
	 */
    private void loadInterfaceObjects(String serverAddress, String portNum) {
        String serverAndPort = serverAddress + ":" + portNum;
        String lookupPrefix = "//" + serverAndPort + "/";

        try {
            Class<?>[] classes_client = com.osi.util.Utils.getClasses("com.osi.gnats.client", this.getClass().getClassLoader());
            Class<?>[] classes_client_environment = com.osi.util.Utils.getClasses("com.osi.gnats.client.environment", this.getClass().getClassLoader());
            Class<?>[] classes_client_equipment = com.osi.util.Utils.getClasses("com.osi.gnats.client.equipment", this.getClass().getClassLoader());
            
            ArrayList<Class> listClasses = new ArrayList<Class>();
            if ((classes_client != null) && (classes_client.length > 0)) {
            	for (int i = 0; i < classes_client.length; i++) {
            		listClasses.add(classes_client[i]);
            	}
            }
            if ((classes_client_environment != null) && (classes_client_environment.length > 0)) {
            	for (int i = 0; i < classes_client_environment.length; i++) {
            		listClasses.add(classes_client_environment[i]);
            	}
            }
            if ((classes_client_equipment != null) && (classes_client_equipment.length > 0)) {
            	for (int i = 0; i < classes_client_equipment.length; i++) {
            		listClasses.add(classes_client_equipment[i]);
            	}
            }
            
            Class<?>[] classes = listClasses.toArray(new Class<?>[0]);
            for (int i = 0; i < classes.length; i++) {
                if ( BaseClass.class.isAssignableFrom( classes[i] ) ) {
                    String name = classes[i].getSimpleName();
                    if (name.compareTo("BaseClass") == 0 ||
                        name.compareTo("ClientNATS") == 0 || 
                        name.compareTo("ClientSecurityManager") == 0 ||
                        name.compareTo("ClientInterfaceManager") == 0)
                        continue;
                    // instantiate the class and add it to the map
                    try {
                        String interfacename = name.replaceAll("Client", "Remote");

                        Remote remote = Naming.lookup(lookupPrefix + interfacename);
                        BaseClass clientClass = (BaseClass)classes[i].getConstructor(NATSInterface.class, Remote.class)
                        		.newInstance(this, remote);

                        // load the remote object
                        interfacesMap.put(interfacename.replaceAll("Remote", ""), clientClass);
                    } catch (NoSuchMethodException e) {
                        e.printStackTrace();   
                    } catch (InstantiationException e) {
                        e.printStackTrace();   
                    } catch (IllegalAccessException e) {
                        e.printStackTrace();   
                    } catch (InvocationTargetException e) {
                        e.printStackTrace();
                    } catch (NotBoundException e) {
                        e.printStackTrace();   
                    } catch (MalformedURLException e) {
                        e.printStackTrace();   
                    } catch (RemoteException e) {
                        e.printStackTrace();   
                    }
                }
            }
        } catch (ClassNotFoundException e) {
            e.printStackTrace();   
        }
    }

    private BaseInterface getInterface(String name) { 
        return interfacesMap.get(name);
    }
    
    /*! \cond PRIVATE */ // Tell Doxygen that this is a private function
    /**
     * Terminate the connection with NATS Server
     * @throws RemoteException
     * @throws Exception
     */
    public void disConnect() throws RemoteException, Exception {
		if (flag_standalone_mode) {
			throw new Exception(Constants.MSG_FUNC_INVALID_STANDALONE_MODE);
		}
		
    	logout();

    	if (thread_ClientSocket != null) {
			thread_ClientSocket.shutdown();
		}
    	
        try {
    		System.out.println("GNATSClient closed connection from server.");
    	} catch (Exception ex) {
    		//ex.printStackTrace();
    	}
    }
    /*! \endcond */

    /**
     * Log in with user credential
     * @param auth_id
     * @throws RemoteException
     */
    public void login(String auth_id) throws RemoteException, Exception {
		if (flag_standalone_mode) {
			throw new Exception(Constants.MSG_FUNC_INVALID_STANDALONE_MODE);
		}
		
    	if (remoteNATS != null) {
    		remoteNATS.login(sessionId, auth_id);
    	}
    }
    
    /**
     * Log out
     * @throws RemoteException
     * @throws Exception
     */
    public void logout() throws RemoteException, Exception {
		if (flag_standalone_mode) {
			throw new Exception(Constants.MSG_FUNC_INVALID_STANDALONE_MODE);
		}
		
    	if (remoteNATS != null) {
    		remoteNATS.logout(sessionId);
    	}
    }

    public void stopStandaloneServer() throws Exception {
    	if (!flag_standalone_mode) {
    		throw new Exception(Constants.MSG_FUNC_INVALID_STANDALONE_MODE);
    	} else {
        	if (remoteNATS != null) {
        		serverProcessId = remoteNATS.getServerProcessId();
        	}
        	
    		if (serverProcessId == -1) {
    			throw new Exception("Can't stop GNATS Standalone.  Server process not found.");
    		} else {
    			if (Utils.isWindowsPlatform()) {
    				Runtime.getRuntime().exec(new String[] {"utility/stopStandalone.bat", "" + serverProcessId});
    			} else {
    				Runtime.getRuntime().exec(new String[] {"utility/stopStandalone.sh", "" + serverProcessId});
    			}
    		}
    	}
    }
    
    /**
     * Thread to handle data from NATS Server
     * 
     * This class starts a new socket to the NATS Server and routinely check the incoming data and process it accordingly.
     */
    class Thread_ClientSocket extends Thread {
    	private String dest_ip_addr = null;
    	private int dest_port = 0;
    	
    	private volatile boolean flagLooping = true;
    	private volatile boolean flagLoopEnded = false;
    	
    	private volatile boolean flagBusy = false;
    	
    	public Thread_ClientSocket(String dest_ip_addr, int dest_port) {
    		this.dest_ip_addr = dest_ip_addr;
    		this.dest_port = dest_port;
    	}
    	
    	public boolean isBusy() {
    		return flagBusy;
    	}
    	
    	public void shutdown() {
    		try {
	    		while (flagBusy) {
	   				Thread.sleep(500);
	    		}
	    		
	    		flagLooping = false;
	    		
	    		while (!flagLoopEnded) {
		   			Thread.sleep(500);
		    	}
	    		
	    		socket.close();
    		} catch (Exception ex) {}
    	}
    	
    	public void run() {
    		String strMsg = null;
    		flagLoopEnded = false;
    		
    		try {
    			socket = new Socket(dest_ip_addr, dest_port);
    			InputStream iStream = socket.getInputStream();

    			BufferedInputStream bis = new BufferedInputStream(iStream);
    			DataInputStream dis = new DataInputStream(bis);

				while (flagLooping) {
					if (dis.available() != 0) {
						strMsg = dis.readUTF();

						JSONObject jsonObj = new JSONObject(strMsg);
						
						if ((sessionId == 0) && (strMsg.toString().contains("sessionId"))) {
							sessionId = Integer.parseInt(jsonObj.getString("sessionId"));
						}
						
						if (jsonObj.has("auth_id")) {
							String json_auth_id = jsonObj.getString("auth_id");
						
							if (json_auth_id != null) {
								if (("".equals(json_auth_id)) || ("null".equalsIgnoreCase(json_auth_id))) {
									auth_id = null;
								} else {
									auth_id = json_auth_id;
								}
							}
						} else if (jsonObj.has("fileTransfer")) {
							String json_fileTransfer = jsonObj.getString("fileTransfer");
							if ((json_fileTransfer != null) && (!"".equals(json_fileTransfer))) {
								long fileLength = jsonObj.getLong("fileLength");
								
								FileOutputStream fos = new FileOutputStream(new File(json_fileTransfer));
					            BufferedOutputStream bos = new BufferedOutputStream(fos);
					            
					            for (int j = 0; j < fileLength; j++) bos.write(bis.read());

				                bos.flush();
				                bos.close();
							}
						} else if (jsonObj.has("socketBusy")) {
							String json_socketBusy = jsonObj.getString("socketBusy");
							if (json_socketBusy != null) {
								if ("true".equals(json_socketBusy)) {
									flagBusy = true;
								} else if ("false".equals(json_socketBusy)) {
									flagBusy = false;
								}
							}
						}
					}
    			} // end - while
				
				flagLoopEnded = true;
    		} catch (Exception ex) {
    			ex.printStackTrace();
    		}
    	} // end - run
    }
}
