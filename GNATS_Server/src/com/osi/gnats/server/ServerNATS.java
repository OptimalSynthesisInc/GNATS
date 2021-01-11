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

package com.osi.gnats.server;

import java.io.BufferedOutputStream;
import java.lang.System;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.File;

import java.lang.reflect.InvocationTargetException;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.rmi.RemoteException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.SortedSet;
import java.util.TreeSet;

import com.osi.gnats.engine.CEngine;
import com.osi.gnats.rmi.RemoteNATS;
import com.osi.gnats.rmi.ServerSession;
import com.osi.gnats.server.ServerSimulation;
import com.osi.gnats.user.User;
import com.osi.gnats.user.UserManager;
import com.osi.util.Constants;

/**
 * Actual RMI class which implements RemoteNATS
 */
public class ServerNATS extends ServerClass implements RemoteNATS {
	private static long processId = -1;
	
	private HashMap<String, Class> interfaces;
    private HashMap<String, ServerClass> serverClasses;
    
    private String ip_addr = null;
    
    private static Thread thread_ServerSocket = null;
    
    // Variable to check CIFP existence
	public static boolean cifpExists = false;
    
    private static boolean flag_standalone_mode = false;
    
    private static String share_path = "share";
    
    static {
		java.lang.management.RuntimeMXBean bean = java.lang.management.ManagementFactory.getRuntimeMXBean();

	    // Get name representing the running Java virtual machine.
	    // It returns something like 6460@AURORA. Where the value
	    // before the @ symbol is the PID.
	    String jvmName = bean.getName();

	    // Extract the PID by splitting the string returned by the bean.getName() method.
		processId = Long.valueOf(jvmName.split("@")[0]);
    }
    
    /**
     * Constructor
     * @param logLevel
     * @param ip_addr
     * @throws RemoteException
     * @throws Exception
     */
    public ServerNATS(String logLevel, String ip_addr, boolean flag_standalone_mode, boolean flag_gdb) throws RemoteException, Exception {
    	super(null);

    	this.flag_standalone_mode = flag_standalone_mode;
    	
    	String dir_share = null;
    	String dir_cifp = null;
    	
    	// Hari Iyer edit on 03/11/2020, checking existence of cifp file
    	if (flag_standalone_mode) {
    		dir_share = "../GNATS_Server/share";
    	}
    	else {
    		dir_share = "share";
    	}
    	
    	SortedSet<String> sortedSet = new TreeSet<String>();
    	
    	File fileNas = new File(dir_share + "/rg/nas");
    	if (fileNas != null) {
    		File[] tmpFileArray = fileNas.listFiles();
    		if (tmpFileArray != null) {
    			for (File tmpFile : tmpFileArray) {
    				if ((tmpFile != null) && (tmpFile.isDirectory())) {
    					sortedSet.add(tmpFile.getName());
    				}
    			}
    		}
    	}
    	
    	if (0 < sortedSet.size()) {
    		dir_cifp = sortedSet.first();

    		if (dir_cifp != null) {
    			File cifpFile = new File(dir_share + "/rg/nas/" + dir_cifp + "/FAACIFP18");
    			cifpExists = cifpFile.exists();
    		}
    	}
    	
    	// End check

        interfaces = new HashMap<String, Class>();
        serverClasses = new HashMap<String, ServerClass>();

        getInterfaces();

        cEngine = new CEngine(logLevel, flag_gdb);

        this.ip_addr = ip_addr;

        if (!flag_standalone_mode) {
        	UserManager.load_user_config(); // Load user config
        } else {
        	share_path = "../GNATS_Server/share";
        }

    	thread_ServerSocket = new Thread(new Runnable_ServerSocket());
    }
    
    public static boolean isStandalone_mode() {
    	return flag_standalone_mode;
    }
    
	public HashMap<String, ServerClass> getServerClasses() { return this.serverClasses; }
	
	public void startServerSocket() {
		thread_ServerSocket.start();
	}
	
	private ServerSimulation sim;
    public ServerSimulation getSim() { return (ServerSimulation)serverClasses.get("Sim"); }
    
	private void getInterfaces() {
        try {
            Class<?>[] classes_server = com.osi.util.Utils.getClasses("com.osi.gnats.server", this.getClass().getClassLoader());
            Class<?>[] classes_server_entity = com.osi.util.Utils.getClasses("com.osi.gnats.server.entity", this.getClass().getClassLoader());
            Class<?>[] classes_server_environment = com.osi.util.Utils.getClasses("com.osi.gnats.server.environment", this.getClass().getClassLoader());
            Class<?>[] classes_server_equipment = com.osi.util.Utils.getClasses("com.osi.gnats.server.equipment", this.getClass().getClassLoader());
            
            ArrayList<Class> listClasses = new ArrayList<Class>();
            if ((classes_server != null) && (classes_server.length > 0)) {
            	for (int i = 0; i < classes_server.length; i++) {
            		listClasses.add(classes_server[i]);
            	}
            }
            if ((classes_server_entity != null) && (classes_server_entity.length > 0)) {
            	for (int i = 0; i < classes_server_entity.length; i++) {
            		listClasses.add(classes_server_entity[i]);
            	}
            }
            if ((classes_server_environment != null) && (classes_server_environment.length > 0)) {
            	for (int i = 0; i < classes_server_environment.length; i++) {
            		listClasses.add(classes_server_environment[i]);
            	}
            }
            if ((classes_server_equipment != null) && (classes_server_equipment.length > 0)) {
            	for (int i = 0; i < classes_server_equipment.length; i++) {
            		listClasses.add(classes_server_equipment[i]);
            	}
            }
            
            Class<?>[] classes = listClasses.toArray(new Class<?>[0]);
            for (int i = 0; i < classes.length; i++) {
                if ( ServerClass.class.isAssignableFrom( classes[i] ) ) {
                    
                    String name = classes[i].getSimpleName();
                    
                    if( name.compareTo("ServerClass") == 0 || name.compareTo("ServerNATS") == 0 )
                        continue;
                    
                    // Instantiate the class and add it to the map
                    try {                        
                        ServerClass serverClass = (ServerClass)classes[i].getConstructor(ServerNATS.class).newInstance(this);

                        serverClasses.put(name.replaceAll("Server", ""), serverClass);
                    } catch (NoSuchMethodException e) {
                        e.printStackTrace();   
                    } catch (InstantiationException e) {
                        e.printStackTrace();   
                    } catch (IllegalAccessException e) {
                        e.printStackTrace();   
                    } catch (InvocationTargetException e) {
                        e.printStackTrace();
                    }
                }
            }
        } catch (ClassNotFoundException e) {
            e.printStackTrace();   
        }
    }
	
	public long getServerProcessId() throws RemoteException {
		return processId;
	}
	
	public static String getSharePath() {
		return share_path;
	}
	
	public void info() throws RemoteException {
		System.out.println("NATS System Info");
		
		cEngine.info();
	}
	
	/**
	 * Login function
	 */
	public boolean login(int sessionId, 
			String auth_id) throws RemoteException {
		return login(sessionId, 
				auth_id,
				false);
	}
	
	/**
	 * Login function
	 * @param sessionId
	 * @param auth_id
	 * @param flagLocalhost
	 * @return
	 * @throws RemoteException
	 */
	private boolean login(int sessionId, 
			String auth_id,
			boolean flagLocalhost) throws RemoteException {
		boolean retValue = false;

		User tmpUserConf = null;
		ServerSession tmpServerSession = null;
		Socket tmpSocket = null;

		if (sessionId == 0) {
			throw new RemoteException("Session is invalid.");
		} else {
			if ((flagLocalhost) && ((auth_id == null) || ("".equals(auth_id)))) {
				auth_id = UserManager.DEFAULT_LOCALHOST_ADMIN_USERNAME;
			}
			
			tmpUserConf = UserManager.getUserconfByAuthid(auth_id);
			if (tmpUserConf != null) {
				tmpServerSession = UserManager.getServerSession(Integer.valueOf(sessionId));
				if (tmpServerSession != null) {
					if ((tmpServerSession.getAuth_id() != null) && (!"".equals(tmpServerSession.getAuth_id()))
							&& (!UserManager.DEFAULT_LOCALHOST_ADMIN_USERNAME.equals(tmpServerSession.getAuth_id()))
							&& (!"admin".equals(tmpServerSession.getAuth_id()))
							) {
						throw new RemoteException("This user already logged in.");
					} else {
						tmpServerSession.setAuth_id(auth_id);
						tmpServerSession.setPermission_type(tmpUserConf.getPermission_type());

						String jsonStr = "{" + "auth_id:\"" + auth_id + "\"}";

						tmpSocket = tmpServerSession.getSocket();
						if (tmpSocket != null) {
							try {
								ServerNATS.socket_outputString(tmpSocket, jsonStr);
								
								retValue = true;
							} catch (Exception ex) {
								ex.printStackTrace();
							}
						}
					}
				} else {
					throw new RemoteException("Session with server does not exist.");
				}
			} else {
				throw new RemoteException("User does not exist.");
			}
		}
		
		return retValue;
	}

	/**
	 * Logout function
	 */
	public void logout(int sessionId) throws RemoteException, Exception {
		ServerSession tmpServerSession = null;
		Socket tmpSocket = null;
		
		tmpServerSession = UserManager.getServerSession(Integer.valueOf(sessionId));
		if (tmpServerSession != null) {
			tmpServerSession.setAuth_id(null);
			tmpServerSession.setPermission_type(-1);
			
			String jsonStr = "{" + "auth_id:\"null" + "\"}";

			tmpSocket = tmpServerSession.getSocket();
			if (tmpSocket != null) {
				try {
					ServerNATS.socket_outputString(tmpSocket, jsonStr);
				} catch (Exception ex) {
					ex.printStackTrace();
				}
			}
		}
	}
	
	synchronized public static void socket_outputString(Socket socket, String string_to_output) throws IOException {
		if (socket != null) {
			OutputStream oStream = socket.getOutputStream();
			BufferedOutputStream bos = new BufferedOutputStream(oStream);
			DataOutputStream dos = new DataOutputStream(bos);
			dos.writeUTF(string_to_output);
			dos.flush();
		}
	}

	class Runnable_ServerSocket implements Runnable {
		private boolean listening = true;
		
		public void run() {
			Socket tmpSocket = null;
			String clientIpAddr = null;
			
			try {
				InetAddress inetAddr = InetAddress.getByName(ip_addr);
				ServerSocket serverSocket = new ServerSocket(Constants.SOCKET_PORT_NUMBER);
				
	            while (listening) {
	            	// Accept socket
	            	tmpSocket = serverSocket.accept(); // The program will stay at this statement before a new socket connection is established.
	            	if (tmpSocket != null) {
	            		tmpSocket.setTcpNoDelay(true);
	            		
	            		// Get session ID
	            		int tmp_sessionId = tmpSocket.hashCode();
	            		
	            		// Prepare session ID in the returning JSON string
	        	        String jsonStr = "{" + "sessionId:\"" + tmp_sessionId + "\"}";
						socket_outputString(tmpSocket, jsonStr);
	        	        
	        			// Add server session
	        			UserManager.addServerSession(Integer.valueOf(tmp_sessionId), new ServerSession(tmp_sessionId, tmpSocket));
	        			
	        			clientIpAddr = tmpSocket.getInetAddress().getHostAddress();
	        			if ((clientIpAddr != null) && (clientIpAddr.equals(ip_addr))) {
	        				login(tmp_sessionId, null, true);
	        			}
	            	}
		        }
		    } catch (IOException e) {
	            System.err.println("Could not listen on port " + Constants.SOCKET_PORT_NUMBER);
	            System.exit(-1);
	        } catch (Exception ex) {
	        	ex.printStackTrace();
	        }
		}
	}
}
