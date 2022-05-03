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

/**
 * This module encrypts and decrypts data using the 256-bit AES algorithm
 */

#include "aestable.h"
#include "lektor.h"
#include "ght_hash_table.h"

#include <sstream>

/*
 * DO NOT DISTRIBUTE THIS SOURCE CODE!
 * THE DECRYPTION KEY WILL BE REVEALED
 */
static unsigned char* KEY = (unsigned char*)"03152010_ENgiNEer@OPtiSyN.COm";

static int LINE_MAX_LENGTH = 128;

// a hash table for keeping track of memory file buffers that need to be freed
// when the memory file stream is closed
static ght_hash_table_t* OPEN_MEMORY_FILES = NULL;


int fclose_decrypted(FILE* file) {

	// build the hash key
	char mapKey[20];
	sprintf(mapKey, "FILE@%p", file);

	// remove from the map
	char* buffer = (char*)ght_remove(OPEN_MEMORY_FILES, strlen(mapKey)*sizeof(char), mapKey);
	if(!buffer) {
	    printf("Could not find buffer in the map for key %s.\n", mapKey);
	    return EOF;
	}

	// free the buffer and close the file
	free(buffer);
	buffer = NULL;

	int result = fclose(file);

	// if the map is empty, delete it since all mem files are closed
	int num = ght_size(OPEN_MEMORY_FILES);
	if(num == 0) {
		ght_finalize(OPEN_MEMORY_FILES);
		OPEN_MEMORY_FILES = NULL;
	}

	return result;
}

int fclose_decrypted_r(FILE* file, char*& buffer) {

	// free the buffer and close the file
	if(buffer) {
		free(buffer);
		buffer = NULL;
	}

	int result = fclose(file);

	return result;
}


vector<string> getVector_decrypted(const char* filename) {
	vector<string> retVector;
	retVector.clear();

	// open the encrypted file for reading
	FILE* encryptedFile = fopen(filename, "rb");

	// determine the file size
	fseek(encryptedFile, 0, SEEK_END);
	int file_length = ftell(encryptedFile);
	fseek(encryptedFile, 0, SEEK_SET);

	int buffer_index = 0;

	// decrypt
	uchar in[16];
	uchar out[16];
	uchar key[16];
	int ch, idx = 0;
	uchar expkey[4 * Nb * (Nr + 1)];
	strncpy ((char*)key, (char*)KEY, sizeof(key));
	ExpandKey (key, expkey);

	stringstream ss;
	ss.str(""); // Clean up

	int cnt_line_chars; // Count of characters in a line
	cnt_line_chars = 0; // Reset

	while ( ch = getc(encryptedFile), ch != EOF ) {
		in[idx++] = ch;
		if ( idx % 16 )
			continue;

		memset(out, 0, 16*sizeof(uchar)); // Clean up
		
		Decrypt (in, expkey, out);

		for ( idx = 0; idx < 16; idx++ ) {
			if ((char)out[idx] != '\n') {
				ss << (char)out[idx];

				cnt_line_chars++;
			} else {
				retVector.push_back(ss.str());

				ss.str(""); // Clean up
				cnt_line_chars = 0; // Reset
			}

			if (cnt_line_chars == LINE_MAX_LENGTH) {
				retVector.push_back(ss.str());

				ss.str(""); // Clean up
				cnt_line_chars = 0; // Reset
			}
		}

		idx = 0;
	}

	if (ss.str().size() > 0) {
		retVector.push_back(ss.str());
	}

	fclose(encryptedFile);
	encryptedFile = NULL;

	return retVector;
}

int fclose_encrypted(FILE* stream) {
	// simply call fclose on the stream
	return fclose(stream);
}

FILE* fopen_encrypted(const char* filename, const char* mode) {
	// simply call fopen on the file
	return fopen(filename, mode);
}

void decryptFileFile(FILE* fd, FILE* outfile) {

	uchar expkey[4 * Nb * (Nr + 1)];

	uchar in[16];
	uchar out[16];
	uchar key[16];
	int ch, idx = 0;
	char buffer[17];

	printf("\nDECRYPTING...\n");

		strncpy ((char*)key, (char*)KEY, sizeof(key));
		ExpandKey (key, expkey);

		while( ch = getc(fd), ch != EOF ) {
			in[idx++] = ch;
			if( idx % 16 )
				continue;

			Decrypt (in, expkey, out);

			for( idx = 0; idx < 16; idx++ ) {
				buffer[idx] = out[idx];
			}
			buffer[16] = '\0';

			fputs(buffer, outfile);
			idx = 0;
		}
}

void decryptFile(const char* infilename, const char* outfilename) {

	uchar expkey[4 * Nb * (Nr + 1)];
	FILE *fd = fopen (infilename, "rb");
	FILE* outfile = fopen(outfilename, "wb");
	uchar in[16];
	uchar out[16];
	uchar key[16];
	int ch, idx = 0;
	char buffer[17];

	printf("\nDECRYPTING...\n");

		strncpy ((char*)key, (char*)KEY, sizeof(key));
		ExpandKey (key, expkey);

		while( ch = getc(fd), ch != EOF ) {
			in[idx++] = ch;
			if( idx % 16 )
				continue;

			Decrypt (in, expkey, out);

			for( idx = 0; idx < 16; idx++ ) {
				buffer[idx] = out[idx];
			}
			buffer[16] = '\0';

			fputs(buffer, outfile);
			idx = 0;
		}

		fclose(fd);
		fclose(outfile);
}

void encryptFileFile(FILE* fd, FILE* outfile) {
	uchar expkey[4 * Nb * (Nr + 1)];
	uchar in[16];
	uchar out[16];
	uchar key[16];
	int ch, idx = 0;

	printf("\nENCRYPTING...\n");

	strncpy ((char*)key, (char*)KEY, sizeof(key));
	ExpandKey (key, expkey);

	while( ch = getc(fd), ch != EOF ) {
		in[idx++] = ch;
		if( idx % 16 )
			continue;

		Encrypt (in, expkey, out);

		for( idx = 0; idx < 16; idx++ ) {
			//putchar (out[idx]);
			putc(out[idx], outfile);
		}
		idx = 0;
	}

	if( idx )
	    while( idx % 16 ) {
            // jason: this is producing a -Warray-bounds warning in
            // gcc 4.8.2. TODO: investigate why.
	        in[idx++] = '\0';
	    }
	else
	  return;

	Encrypt (in, expkey, out);

	for( idx = 0; idx < 16; idx++ ) {
		putc(out[idx], outfile);
	}
}

void encryptFile(const char* infilename, const char* outfilename) {
	uchar expkey[4 * Nb * (Nr + 1)];
	FILE *fd = fopen (infilename, "rb");
	FILE *outfile = fopen(outfilename, "wb");
	uchar in[16];
	uchar out[16];
	uchar key[16];
	int ch, idx = 0;

	printf("\nENCRYPTING...\n");

	strncpy ((char*)key, (char*)KEY, sizeof(key));
	ExpandKey (key, expkey);

	while( ch = getc(fd), ch != EOF ) {
		in[idx++] = ch;
		if( idx % 16 )
			continue;

		Encrypt (in, expkey, out);

		for( idx = 0; idx < 16; idx++ ) {
			putc(out[idx], outfile);
		}
		idx = 0;
	}

	if( idx )
	  while( idx % 16 )
		in[idx++] = '\0';
	else
	  return;

	Encrypt (in, expkey, out);

	for( idx = 0; idx < 16; idx++ ) {
		putc(out[idx], outfile);
	}

	fclose(fd);
	fclose(outfile);
}

#ifdef TEST_AESTABLE

int main(void) {

	FILE* decryptedFile = fopen_decrypted("/home/jason/projects/NASA-ARC-1004/carpat_predictor/data/NAS/Airports.crypt", "rb");

	int length = 1000;
	char line[length];

	while( fgets(line, length, decryptedFile) ) {
		printf("%s", line);
	}
	fclose_decrypted(decryptedFile);

	return 0;
}

#endif
