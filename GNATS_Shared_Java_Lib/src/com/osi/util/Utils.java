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

package com.osi.util;

import java.io.*;
import java.util.*;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.net.URL;

/**
 * Utility functions
 */
public class Utils {
	private static String os_name = null;
	
	static {
		os_name = System.getProperty("os.name");
	}

	// Detect if current OS is Windows
	public static boolean isWindowsPlatform() {
		return ((os_name != null) && (os_name.startsWith("Windows")));
	}
	
	// used for reading simple text files.  ignores # and // comments
	public static String getNextNonEmptyLine(BufferedReader bReader) {
		try {
			String s;
			while ((s = bReader.readLine()) != null) {
				s = s.trim();
				if (s.startsWith("//") || s.startsWith("#") ||
					s.equals("") || s.equals("\n") || s.equals("\015\012"))
					continue;
				else
					return s;								
			}
		} catch (Exception e) {
			return "";
		}
		return "";
	}	

	public static double toFeet(double meters) {
		return meters * Constants.METERS_TO_FEET;
	}
	
	public static double toMeters(double feet) {
		return feet * Constants.FEET_TO_METERS;
	}
	
	public static double[] toFeet(double[] meters) {
		if (meters == null)
			return null;
		for (int i = 0; i < meters.length; i++)
			meters[i] = toFeet(meters[i]);
		return meters;
	}
	
	public static double[] toMeters(double[] feet) {
		if (feet == null)
			return null;
		for (int i = 0; i < feet.length; i++)
			feet[i] = toMeters(feet[i]);
		return feet;
	}
	
	public static double normDeg(double a) {
		while (a < 0)
			a += 360;
		return a;
	}
	public static double normRad(double a) {
		while (a < 0)
			a += Constants.TWO_PI;
		return a;
	}
	
	public static double[] normDeg(double[] d) {
		if (d == null)
			return null;
		for (int i = 0; i < d.length; i++)
			d[i] = normDeg(d[i]);
		return d;
	}

	public static double[][] normDeg(double[][] d) {
		if (d == null)
			return null;
		for (int i = 0; i < d.length; i++)
			for (int j = 0; j < d[i].length; j++)
				d[i][j] = normDeg(d[i][j]);
		return d;
	}	

	public static double[] toRadians(double[] d) {
		if (d == null)
			return null;
		for (int i = 0; i < d.length; i++)
			d[i] = Math.toRadians(d[i]);
		return d;
	}
	
	public static double[][] toRadians(double[][] d) {
		if (d == null)
			return null;
		for (int i = 0; i < d.length; i++) {
			for (int j = 0; j < d[i].length; j++)
				d[i][j] = Math.toRadians(d[i][j]);
		}
		return d;
	}	

	public static double clamp(double val, double lower, double upper) {
		if (val < lower)
			val = lower;
		if (val > upper)
			val = upper;
		return val;
	}

	public static Class<?>[] getClasses(String pckgname, ClassLoader cld)
			throws ClassNotFoundException {
		ArrayList<Class> classes = new ArrayList<Class>();
		// Get a File object for the package
		File directory = null;
		String resourceFile = null;
		String jarFilename = null;
		
		try {
			if (cld == null) {
				throw new ClassNotFoundException("Can't get class loader.");
			}
			String path = pckgname.replace('.', '/');

			URL resource = cld.getResource(path);
			if (resource == null) {
				throw new ClassNotFoundException("No resource for " + path);
			}
			
			resourceFile = resource.getFile();

			if (resourceFile.contains(".jar")) {
				jarFilename = resourceFile.substring("file:".length(), resourceFile.indexOf(".jar")+4);

				try {
					ZipInputStream zip = new ZipInputStream(new FileInputStream(jarFilename));
					for (ZipEntry entry = zip.getNextEntry(); entry != null; entry = zip.getNextEntry()) {
					    if (!entry.isDirectory() && entry.getName().endsWith(".class")) {
					        // This ZipEntry represents a class. Now, what class does it represent?
					        String className = entry.getName().replace('/', '.'); // including ".class"
					        if (className.startsWith(pckgname)) {
					        	classes.add(Class.forName(className.substring(0, className.length() - ".class".length())));
					        }
					    }
					}
					zip.close();
				} catch (Exception ex) {
					ex.printStackTrace();
				} finally {

				}
			} else {
	            directory = new File(resourceFile);
				if (directory.exists()) {
					// Get the list of the files contained in the package
					String[] files = directory.list();
					for (int i = 0; i < files.length; i++) {
						// we are only interested in .class files
						if (files[i].endsWith(".class")) {
							// removes the .class extension
							classes.add(Class.forName(pckgname + '.'
									+ files[i].substring(0, files[i].length() - 6)));
						}
					}
				} else {
					throw new ClassNotFoundException(pckgname
							+ " does not appear to be a valid package");
				}
			}
		} catch (NullPointerException x) {
			throw new ClassNotFoundException(pckgname + " (" + directory
					+ ") does not appear to be a valid package");
		}

        return classes.toArray(new Class<?>[0]);
	}    

}
