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

#ifndef _CRYPTO_H
#define _CRYPTO_H

#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

using namespace std;

/**
 * Get vector of data by decrypting the file
 */
  vector<string> getVector_decrypted(const char* filename);

  /**
   * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   * Function:
   *   fclose_decrypted()
   * 
   * Description:
   *   Close an encrypted file previously opened with fopen_decrypted()
   *
   * Inputs:
   *   stream   the FILE pointer to be closed
   *
   * In/Out:
   *   none
   *
   * Returns:
   *   error code. same as standard fclose().
   * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   */
  int fclose_decrypted(FILE* file);

  /**
   * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   * Function:
   *   fclose_decrypted_r()
   *
   * Description:
   *   Close an encrypted file previously opened with fopen_decrypted()
   *   Thread-safe version of fclose_decrypted().
   *
   * Inputs:
   *   file   the FILE pointer to be closed
   *   buffer the buffer passed to fopen_decrypted_r
   *
   * In/Out:
   *   none
   *
   * Returns:
   *   error code. same as standard fclose().
   * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   */
  int fclose_decrypted_r(FILE* file, char*& buffer);

  /**
   * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   * Function:
   *   fopen_encrypted()
   * 
   * Description:
   *   Open an encrypted file as without decrypting it.  This simply
   *   wraps a call to fopen().
   *
   * Inputs:
   *   filename   name of the encrypted file to be opened
   *   mode       file access mode, same as for standard fopen().
   *              for example, "r" for read-only, "w" for write
   *
   * In/Out:
   *   none
   *
   * Returns:
   *   FILE pointer to the opened file. 
   * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   */
  FILE* fopen_encrypted(const char* filename, const char* mode);

  /**
   * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   * Function:
   *   fclose_encrypted()
   * 
   * Description:
   *   Close an encrypted file previously opened with fopen_encrypted()
   *
   * Inputs:
   *   stream   the FILE pointer to be closed
   *
   * In/Out:
   *   none
   *
   * Returns:
   *   error code. same as standard fclose().
   * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   */
  int fclose_encrypted(FILE* stream);



  /**
   * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   * Function:
   *   encryptFile()
   * 
   * Description:
   *   Encrypt a file
   *
   * Inputs:
   *   infilename   name of the input clear file to be encrypted
   *   outfilename  name of the encrypted output file
   *
   * In/Out:
   *   none
   *
   * Returns:
   *   none
   * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   */
  void encryptFile(const char* infilename, const char* outfilename);

  /**
   * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   * Function:
   *   decryptFile()
   * 
   * Description:
   *   Decrypt a file
   *
   * Inputs:
   *   infilename   name of the input encrypted file to be decrypted
   *   outfilename  name of the clear output file
   *
   * In/Out:
   *   none
   *
   * Returns:
   *   none
   * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   */
  void decryptFile(const char* infilename, const char* outfilename);

  /**
   * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   * Function:
   *   encryptFileFile()
   * 
   * Description:
   *   Encrypt a file.  Same as encryptFile, except it takes FILE pointers
   *   instead of file names as inputs
   *
   * Inputs:
   *   infilename   FILE* of clear file to be encrypted
   *   outfilename  FILE* of the encrypted output file
   *
   * In/Out:
   *   none
   *
   * Returns:
   *   none
   * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   */
  void encryptFileFile(FILE* infilename, FILE* outfilename);

  /**
   * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   * Function:
   *   decryptFileFile()
   * 
   * Description:
   *   Decrypt a file.  Same as decryptFile, except it takes FILE pointers
   *   instead of file names as inputs
   *
   * Inputs:
   *   infilename   FILE* of encrypted file to be decrypted
   *   outfilename  FILE* of the clear output file
   *
   * In/Out:
   *   none
   *
   * Returns:
   *   none
   * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   */
  void decryptFileFile(FILE* infilename, FILE* outfilename);
  
#endif /* _CRYPTO_H */
