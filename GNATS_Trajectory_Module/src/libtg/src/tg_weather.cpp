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


#include "tg_weather.h"

#include "rap.h"
#include "wind_api.h"

#include "libxml/HTMLparser.h"
#include "libxml/tree.h"

#include "nodes/element.h"
#include "nodes/node.h"

#include "util_string.h"
#include "util_time.h"

#include <ctime>
#include <dirent.h>
#include <fstream>

#include <memory>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <curl/curl.h>

using xmlpp::Node;

struct MemoryStruct {
    char *memory;
    size_t size;
};

struct FtpFile {
	const char *filename;
	FILE *stream;
};

static size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
	struct FtpFile *out = (struct FtpFile *)stream;

	if (!out->stream) {
		/* open file for writing */
		out->stream = fopen(out->filename, "wb");
		if (!out->stream)
			return -1; /* failure, can't open file to write */
	}

	return fwrite(buffer, size, nmemb, out->stream);
}

static int build_wind_grid(const GribData& grib, WindGrid* const grid) {
    if(!grid) return -1;
    int index = 0;
    for(double lat=grid->lat_min; lat<=grid->lat_max; lat+=grid->lat_step) {
      for(double lon=grid->lon_min; lon<=grid->lon_max; lon+=grid->lon_step) {
        for(double alt=grid->alt_min; alt<=grid->alt_max; alt+=grid->alt_step) {
          get_wind_components(grib, lat, lon, alt,
                      &grid->n_data[index], &grid->e_data[index],
                      KNOTS);
          ++index;
        }
      }
    }
    return 0;
}

static int parse_config_line(const string& line,
			      int* const hour,
			      string* const fname) {
  if(!hour) return -1;
  if(!fname) return -1;
  if(line.length() < 1) return -1;

  size_t comma_pos = line.find_first_of(",");

  string hour_str = line.substr(0, comma_pos);
  string fname_str = line.substr(comma_pos+1);

  trim(hour_str);
  trim(fname_str);

  *hour = atoi(hour_str.c_str());
  *fname = fname_str;

  return 0;
}

static int parse_grid_dim_line(const string& line,
			       double* const value) {
  if(!value) return -1;

  size_t equals_pos = line.find_first_of("=");
  string value_str = line.substr(equals_pos+1);
  trim(value_str);

  if(value_str.length() > 0) {
    *value = atof(value_str.c_str());
  } else {
    return -1;
  }

  return 0;
}

static int parse_grid_definition_name_line(const string& line,
					   string* const value) {
  if(!value) return -1;
  size_t equals_pos = line.find_first_of("=");
  string value_str = line.substr(equals_pos+1);
  trim(value_str);

  if(value_str.length() > 0) {
    *value = value_str;
  } else {
    return -1;
  }

  return 0;
}

int write_grid_hdf5(const string& hdf5_file,
		   const double& lat_min,
		   const double& lat_max,
		   const double& lat_step,
		   const double& lon_min,
		   const double& lon_max,
		   const double& lon_step,
		   const double& alt_min,
		   const double& alt_max,
		   const double& alt_step) {
    WindGrid grid(lat_min, lat_max, lat_step,
                  lon_min, lon_max, lon_step,
                  alt_min, alt_max, alt_step);

    return write_wind_grid_hdf5(hdf5_file, grid);
}

int readWindConfigFile(const string& config_file,
		       double* const lat_min,
		       double* const lat_max,
		       double* const lat_step,
		       double* const lon_min,
		       double* const lon_max,
		       double* const lon_step,
		       double* const alt_min,
		       double* const alt_max,
		       double* const alt_step,
		       string* const grid_def_file,
		       map<int, string>* const files) {
	if (!lat_min) return -1;
	if (!lat_max) return -1;
	if (!lat_step) return -1;
	if (!lon_min) return -1;
	if (!lon_max) return -1;
	if (!lon_step) return -1;
	if (!alt_min) return -1;
	if (!alt_max) return -1;
	if (!alt_step) return -1;
	if (!files) return -1;

	// open the config file for reading
	ifstream in;
	in.open(config_file.c_str());

	if (!in.is_open()) {
		fprintf(stderr, "ERROR: could not open configuration file %s\n", config_file.c_str());
		exit(-1);
	}

    // read the file
    string line = "";
    while (in.good()) {
		getline(in, line);

		// skip blank lines
		if (line.length() < 1) continue;

		// remove any leading or trailing whitespace
		trim(line);

		// skip comment lines starting with '#'
		if (line.at(0) == '#') continue;

		// determine if we are parsing the grid dimensions section
		int pos = line.find_first_of(" =");
		string first_word = line.substr(0, pos);
		trim(first_word);

		if (first_word == "LAT_MIN") {
			parse_grid_dim_line(line, lat_min);
		} else if (first_word == "LAT_MAX") {
			parse_grid_dim_line(line, lat_max);
		} else if (first_word == "LAT_STEP") {
			parse_grid_dim_line(line, lat_step);
		} else if (first_word == "LON_MIN") {
			parse_grid_dim_line(line, lon_min);
		} else if (first_word == "LON_MAX") {
			parse_grid_dim_line(line, lon_max);
		} else if (first_word == "LON_STEP") {
			parse_grid_dim_line(line, lon_step);
		} else if (first_word == "ALT_MIN") {
			parse_grid_dim_line(line, alt_min);
		} else if (first_word == "ALT_MAX") {
			parse_grid_dim_line(line, alt_max);
		} else if (first_word == "ALT_STEP") {
			parse_grid_dim_line(line, alt_step);
		} else if (first_word == "GRID_DEFINITION_FILE") {
			parse_grid_definition_name_line(line, grid_def_file);
		} else {
			// parse the current line
			int hour;
			string fname;
			parse_config_line(line, &hour, &fname);
			files->insert(pair<int,string>(hour, fname));
		}
    }

    // close the file
    in.close();

	return 0;
}

/**
 * Write the wind config file for US NOAA data
 */
void writeWindConfigFile_us(const string dirtodnld, const string& conf_filename, const vector<string>& filenames) {
	string str_LAT_MIN = "20";
	string str_LAT_MAX = "50";
	string str_LAT_STEP = "0.5";
	string str_LON_MIN = "-130";
	string str_LON_MAX = "-60";
	string str_LON_STEP = "0.5";
	string str_ALT_MIN = "1000";
	string str_ALT_MAX = "45000";
	string str_ALT_STEP = "500";

	string GRID_DEFINITION_FILE(dirtodnld);

	if (endsWith(dirtodnld.c_str(), "/")) {
		GRID_DEFINITION_FILE = dirtodnld.substr(0, dirtodnld.length()-1);
	}

	string endingDirName;
	if (GRID_DEFINITION_FILE.find("/") != string::npos) {
		endingDirName = GRID_DEFINITION_FILE.substr(GRID_DEFINITION_FILE.find_last_of("/", GRID_DEFINITION_FILE.length())+1, GRID_DEFINITION_FILE.length()-GRID_DEFINITION_FILE.find_last_of("/", GRID_DEFINITION_FILE.length())-1);
	} else {
		endingDirName.assign(GRID_DEFINITION_FILE);
	}

	GRID_DEFINITION_FILE.append("/");
	GRID_DEFINITION_FILE.append(endingDirName);
	GRID_DEFINITION_FILE.append("_grid.h5");

	string filepathnameToDownload;

    ofstream out;
    out.open(conf_filename.c_str());

    out << "LAT_MIN = " << str_LAT_MIN << "\n";
    out << "LAT_MAX = " << str_LAT_MAX << "\n";
    out << "LAT_STEP = " << str_LAT_STEP << "\n";
	out << "LON_MIN = " << str_LON_MIN << "\n";
	out << "LON_MAX = " << str_LON_MAX << "\n";
	out << "LON_STEP = " << str_LON_STEP << "\n";
	out << "ALT_MIN = " << str_ALT_MIN << "\n";
	out << "ALT_MAX = " << str_ALT_MAX << "\n";
	out << "ALT_STEP = " << str_ALT_STEP << "\n";
	out << "GRID_DEFINITION_FILE = " << GRID_DEFINITION_FILE << "\n";

	if (0 < filenames.size()) {
		for (unsigned int index = 0; index < filenames.size(); index++) {
			filepathnameToDownload.assign(dirtodnld);
			filepathnameToDownload.append("/");
			filepathnameToDownload.append(filenames.at(index));

			out << index << ", " << filepathnameToDownload << "\n";
		}
	}

    out.close();
}

int writeWindGridDefFile(const string grid_outfile,
		const double& lat_min,
		const double& lat_max,
		const double& lat_step,
		const double& lon_min,
		const double& lon_max,
		const double& lon_step,
		const double& alt_min,
		const double& alt_max,
		const double& alt_step) {
	printf("  Grid definition file: %s\n", grid_outfile.c_str());

	// Write the grid definition file
	write_grid_hdf5(grid_outfile,
		  lat_min, lat_max, lat_step,
		  lon_min, lon_max, lon_step,
		  alt_min, alt_max, alt_step);

	return 0;
}

int writeWindH5Files(const double& lat_min,
		const double& lat_max,
		const double& lat_step,
		const double& lon_min,
		const double& lon_max,
		const double& lon_step,
		const double& alt_min,
		const double& alt_max,
		const double& alt_step,
		map<int, string>* const grib_files) {
	// Compute table sizes
	size_t lat_size = (int)1 + ceil((lat_max - lat_min) / lat_step);
	size_t lon_size = (int)1 + ceil((lon_max - lon_min) / lon_step);
	size_t alt_size = (int)1 + ceil((alt_max - alt_min) / alt_step);

	map<int, string>::iterator iter;

	unsigned int num_time_steps = grib_files->size();

	for (iter = grib_files->begin(); iter != grib_files->end(); ++iter) {
		printf("  hour: %d,  file: %s\n", iter->first, iter->second.c_str());

		GribData grib;

#ifndef _INC__MINGW_H
		load_grib(iter->second, &grib);
#else
		load_grib_v2_win(iter->second, &grib);
#endif

		int num_levels = grib.h_data.size();

		printf("\n    GRIB edition: %d\n", grib.grib_edition);
		printf("\n    Num levels: %d\n", num_levels);
		printf("    Grid size: %d, %d\n", grib.grid_ni, grib.grid_nj);
		printf("\n");
		printf("    Output Table Bounds:\n");
		printf("      Latitude -  min: %f, max: %f, step: %f\n", lat_min, lat_max, lat_step);
		printf("      Longitude - min: %f, max: %f, step: %f\n", lon_min, lon_max, lon_step);
		printf("      Altitude -  min: %f, max: %f, step: %f\n", alt_min, alt_max, alt_step);
		printf("      Latitude cells:  %ld\n", lat_size);
		printf("      Longitude cells: %ld\n", lon_size);
		printf("      Altitude cells:  %ld\n\n", alt_size);

		// Declare grid
		WindGrid grid(lat_min, lat_max, lat_step,
				  lon_min, lon_max, lon_step,
				  alt_min, alt_max, alt_step);

		// Build grid with actual data from grib
		build_wind_grid(grib, &grid);

		int slash = iter->second.find_last_of("/");
		string basename = iter->second.substr(slash+1);
		string out_dir = iter->second.substr(0, slash);

		// Write the output for the current hour
		int dot = basename.find_last_of(".");
		string ofname = basename.substr(0, dot)+".h5";
		string ofpath = out_dir + "/" + ofname;

		write_wind_grid_hdf5(ofpath, grid);
	}
}

// Actual logic to download files from NOAA FTP site
// This function uses libcurl library to access FTP and download files.
void proc_downloadWindFilesIntoFolder(const string& dirtodnld,
		const string& ftp_rapdir,
		vector<string>* filenames) {
	CURL *curl;
	CURLcode res;

	string strURL;

	string filenameToDownload;
	string filepathnameToDownload;

	curl_global_init(CURL_GLOBAL_DEFAULT);
	if (0 < filenames->size()) {
		vector<string>::iterator iterator_filename;
		iterator_filename = filenames->begin();
		//for (iterator_filename = filenames->begin(); iterator_filename != filenames->end(); iterator_filename++) {
		while (iterator_filename != filenames->end()) {
			curl = curl_easy_init();
			if (curl) {
				struct FtpFile ftpfile = {
					"", /* name to store the file as if successful */
				    NULL
				};

				filenameToDownload.assign(*iterator_filename);

				strURL.assign("ftp://ftpprd.ncep.noaa.gov/" + ftp_rapdir + "/" + filenameToDownload);

				printf("  Saving file to %s\n", (dirtodnld + "/" + filenameToDownload).c_str());

				res = curl_easy_setopt(curl, CURLOPT_URL,
						strURL.c_str());
				if (CURLE_OK != res) {
					fprintf(stderr, "proc_downloadWindFilesIntoFolder -- curl_easy_setopt() CURLOPT_URL failed: %s\n",
										  curl_easy_strerror(res));
				}

				filepathnameToDownload.assign(dirtodnld);
				filepathnameToDownload.append("/");
				filepathnameToDownload.append(filenameToDownload);
				ftpfile.filename = filepathnameToDownload.c_str();

#ifdef SKIP_PEER_VERIFICATION
				/*
				 * If you want to connect to a site who isn't using a certificate that is
				 * signed by one of the certs in the CA bundle you have, you can skip the
				 * verification of the server's certificate. This makes the connection
				 * A LOT LESS SECURE.
				 *
				 * If you have a CA cert for the server stored someplace else than in the
				 * default bundle, then the CURLOPT_CAPATH option might come handy for
				 * you.
				 */
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
				/*
				 * If the site you're connecting to uses a different host name that what
				 * they have mentioned in their server certificate's commonName (or
				 * subjectAltName) fields, libcurl will refuse to connect. You can skip
				 * this check, but this will make the connection less secure.
				 */
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

				/* Define our callback to get called when there's data to be written */
				res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);
				if (CURLE_OK != res) {
					fprintf(stderr, "proc_downloadWindFilesIntoFolder -- curl_easy_setopt() CURLOPT_WRITEFUNCTION failed: %s\n",
										  curl_easy_strerror(res));
				}

				/* Set a pointer to our struct to pass to the callback */
				res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);
				if (CURLE_OK != res) {
					fprintf(stderr, "proc_downloadWindFilesIntoFolder -- curl_easy_setopt() CURLOPT_WRITEDATA failed: %s\n",
										  curl_easy_strerror(res));
				}

				// Switch on/off full protocol/debug output
				// Value 0L: Off
				//       1L: On
				res = curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
				if (CURLE_OK != res) {
					fprintf(stderr, "proc_downloadWindFilesIntoFolder -- curl_easy_setopt() CURLOPT_VERBOSE failed: %s\n",
										  curl_easy_strerror(res));
				}

				res = curl_easy_perform(curl);
				if (CURLE_OK != res) {
					fprintf(stderr, "proc_downloadWindFilesIntoFolder -- curl_easy_perform() failed: %s\n",
										  curl_easy_strerror(res));

					filenames->erase(iterator_filename);
				} else {
					iterator_filename++;
				}

				/* always cleanup */
				curl_easy_cleanup(curl);

				if (ftpfile.stream)
				    fclose(ftpfile.stream); /* close the local file */
			}
		} // end - for
	}

	curl_global_cleanup();
}

// Download wind files from NOAA
//
// Filename convention of wind file
//     rap.tccz.awp130pgrbfxx.grib2
//     cc: Value of the current hour.  Range from 00 to 23.  Example. 00, 13, 23
//     xx: Forcasting hour.  Range from 00 to 23.
int tg_download_wind_files(const string dirtodnld,
		const int hour1,
		const int day1,
		const int month1,
		const int year1,
		const int hour2,
		const int day2,
		const int month2,
		const int year2) {
	int max_hr_in_forecast = 24;

	DIR *dir = opendir(dirtodnld.c_str());
	if (dir == NULL) {
		printf("Creating directory %s\n", dirtodnld.c_str());
#ifdef _INC__MINGW_H
		mkdir(dirtodnld.c_str());
#else
		mkdir(dirtodnld.c_str(), 0700);
#endif
	}

	ostringstream oSS;

	int tmpHour;
	int tmpYear;
	int tmpMonth;
	int tmpDay;

	string hourstring;
	string ftp_rapdir;

	vector<string> filenames;

	if (hour1 < 0) {
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);

	    tmpHour = tm.tm_hour - 1;
	    tmpYear = tm.tm_year + 1900;
	    tmpMonth = tm.tm_mon + 1;
	    tmpDay = tm.tm_mday;

	    oSS.str(""); // Reset

		if (tmpHour < 10) {
			oSS << "t0";
			oSS << tmpHour;
		} else {
			oSS << "t";
			oSS << tmpHour;
		}
		oSS << "z";
		hourstring = oSS.str();

		filenames.clear();

	    for (int k = 0; k < max_hr_in_forecast; k++) {
	    	oSS.str(""); // Reset

	    	oSS << "rap.";
	    	oSS << hourstring;

	        if (k >= 10) {
	        	oSS << ".awp130pgrbf";
	        } else {
	        	oSS << ".awp130pgrbf0";
	        }

        	oSS << k;
        	oSS << ".grib2";

        	filenames.push_back(oSS.str());
	    }

	    oSS.str(""); // Reset

	    oSS << "pub/data/nccf/com/rap/prod/rap.";
	    oSS << tmpYear;

	    if ((tmpMonth < 10) && (tmpDay < 10)) {
	        oSS << "0";
	        oSS << tmpMonth;
	        oSS << "0";
	        oSS << tmpDay;
	    } else if (tmpMonth < 10) {
	    	oSS << "0";
	    	oSS << tmpMonth;
	    	oSS << tmpDay;
	    } else if (tmpDay < 10) {
	    	oSS << tmpMonth;
	    	oSS << "0" << tmpDay;
	    } else {
	    	oSS << tmpMonth;
	    	oSS << tmpDay;
	    }

	    ftp_rapdir.assign(oSS.str());

	    proc_downloadWindFilesIntoFolder(dirtodnld, ftp_rapdir, &filenames);
	} else if ((0 <= hour1) && (0 <= day1) && (0 <= month1) && (0 <= year1) && (hour2 < 0)) {
		time_t t = time(NULL);
		struct tm tm_now = *localtime(&t);
		struct tm tm_start;
		tm_start.tm_sec = 0;			 // Seconds.	[0-60] (1 leap second)
		tm_start.tm_min = 0;			 // Minutes.	[0-59]
		tm_start.tm_hour = hour1;		 // Hours.	    [0-23]
		tm_start.tm_mday = day1;		 // Day.		[1-31]
		tm_start.tm_mon = month1;		 // Month.	    [0-11]
		tm_start.tm_year = year1 - 1900; // Year.  1900 and later

		// If start time is later than current time
		if (difftime(mktime(&tm_start), mktime(&tm_now)) > 0) {
			tm_start = tm_now;
		}

		if ((tm_start.tm_mday < tm_now.tm_mday) || (tm_start.tm_mon < tm_now.tm_mon) || (tm_start.tm_year < tm_now.tm_year)) {
			for (time_t tmpTime = mktime(&tm_start); tmpTime < mktime(&tm_now); tmpTime+=86400) {
				struct tm tmp_tm = *localtime(&tmpTime);

				oSS.str(""); // Reset

				oSS << "pub/data/nccf/com/rap/prod/rap.";
				oSS << (tmp_tm.tm_year + 1900);

				if ((tmp_tm.tm_mon+1 < 10) && (tmp_tm.tm_mday < 10)) {
					oSS << "0" << (tmp_tm.tm_mon + 1);
					oSS << "0" << tmp_tm.tm_mday;
				} else if (tmp_tm.tm_mon+1 < 10) {
					oSS << "0" << tmp_tm.tm_mon;
					oSS << tmp_tm.tm_mday;
				} else if (tmp_tm.tm_mday < 10) {
					oSS << (tmp_tm.tm_mon + 1);
					oSS << "0" << tmp_tm.tm_mday;
				} else {
					oSS << (tmp_tm.tm_mon + 1);
					oSS << tmp_tm.tm_mday;
				}

				ftp_rapdir.assign(oSS.str());

				filenames.clear();

				if (tmp_tm.tm_mday == tm_start.tm_mday) {
					for (int k = 0; k < max_hr_in_forecast; k++) {
						oSS.str(""); // Reset

						oSS << "rap.";

						if (k >= 10) {
							oSS << "t" << k;
						} else {
							oSS << "t0" << k;
						}

						oSS << "z";
						oSS << ".awp130pgrbf00";
						oSS << ".grib2";

						filenames.push_back(oSS.str());
					}
				} else if (tmp_tm.tm_mday == tm_now.tm_mday) {
					for (int k = 0; k < tm_now.tm_hour; k++) {
						oSS.str(""); // Reset

						oSS << "rap.";

						if (k >= 10) {
							oSS << "t" << k;
						} else {
							oSS << "t0" << k;
						}

						oSS << "z";
						oSS << ".awp130pgrbf00";
						oSS << ".grib2";

						filenames.push_back(oSS.str());
					}

					oSS.str(""); // Reset

					if (tm_now.tm_hour > 10) {
						oSS << "t";
					} else {
						oSS << "t0";
					}
					oSS << (tm_now.tm_hour-1) << "z";

					hourstring = oSS.str();

					for (int k = 0; k < max_hr_in_forecast; k++) {
						oSS.str(""); // Reset

						oSS << "rap.";
						oSS << hourstring;
						oSS << ".awp130pgrbf";

						if (k >= 10) {
							oSS << k;
						} else {
							oSS << "0" << k;
						}

						oSS << ".grib2";

						filenames.push_back(oSS.str());
					}
				} else {
					for (int k = 0; k < max_hr_in_forecast; k++) {
						oSS.str(""); // Reset

						oSS << "rap.";

						if (k >= 10) {
							oSS << "t";
						} else {
							oSS << "t0";
						}

						oSS << k << "z";
						oSS << ".awp130pgrbf00";
						oSS << ".grib2";

						filenames.push_back(oSS.str());
					}
				}

				proc_downloadWindFilesIntoFolder(dirtodnld, ftp_rapdir, &filenames);
			} // end - for
		} else {
			if (tm_start.tm_hour < tm_now.tm_hour) {
				oSS.str(""); // Reset

				oSS << "pub/data/nccf/com/rap/prod/rap.";
				oSS << (tm_now.tm_year + 1900);

				if ((tm_now.tm_mon+1 < 10) && (tm_now.tm_mday < 10)) {
					oSS << "0" << (tm_now.tm_mon + 1);
					oSS << "0" << tm_now.tm_mday;
				} else if (tm_now.tm_mon+1 < 10) {
					oSS << "0" << (tm_now.tm_mon + 1);
					oSS << tm_now.tm_mday;
				} else if (tm_now.tm_mday < 10) {
					oSS << (tm_now.tm_mon + 1);
					oSS << "0" << tm_now.tm_mday;
				} else {
					oSS << (tm_now.tm_mon + 1);
					oSS << tm_now.tm_mday;
				}

				ftp_rapdir.assign(oSS.str());

				filenames.clear();

				for (int k = tm_start.tm_hour; k < tm_now.tm_hour; k++) {
					oSS.str(""); // Reset

					oSS << "rap.";

					if (k >= 10) {
						oSS << "t";
					} else {
						oSS << "t0";
					}

					oSS << k << "z";
					oSS << ".awp130pgrbf00";
					oSS << ".grib2";

					filenames.push_back(oSS.str());
				}

				proc_downloadWindFilesIntoFolder(dirtodnld, ftp_rapdir, &filenames);
			} else if (tm_start.tm_hour == tm_now.tm_hour) {
				oSS.str(""); // Reset

				if (tm_now.tm_hour > 10) {
					oSS << "t";
				} else {
					oSS << "t0";
				}
				oSS << (tm_now.tm_hour-1) << "z";

				hourstring = oSS.str();

				filenames.clear();

				for (int k = 0; k < max_hr_in_forecast; k++) {
					oSS.str(""); // Reset

					oSS << "rap.";
					oSS << hourstring;
					oSS << ".awp130pgrbf";

					if (k >= 10) {
						oSS << k;
					} else {
						oSS << "0" << k;
					}

					oSS << ".grib2";

					filenames.push_back(oSS.str());
				}

				oSS.str(""); // Reset

				oSS << "pub/data/nccf/com/rap/prod/rap.";
				oSS << (tm_now.tm_year + 1900);

				if ((tm_now.tm_mon+1 < 10) && (tm_now.tm_mday < 10)) {
					oSS << "0" << (tm_now.tm_mon + 1);
					oSS << "0" << tm_now.tm_mday;
				} else if (tm_now.tm_mon+1 < 10) {
					oSS << "0" << (tm_now.tm_mon + 1);
					oSS << tm_now.tm_mday;
				} else if (tm_now.tm_mday < 10) {
					oSS << (tm_now.tm_mon + 1);
					oSS << "0" << tm_now.tm_mday;
				} else {
					oSS << (tm_now.tm_mon + 1);
					oSS << tm_now.tm_mday;
				}

				ftp_rapdir.assign(oSS.str());

				proc_downloadWindFilesIntoFolder(dirtodnld, ftp_rapdir, &filenames);
			}
		}
	}

	// Write config file
	writeWindConfigFile_us(dirtodnld, "wind_tool.conf", filenames);

	return 0;
}

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
    	/* out of memory! */
    	printf("not enough memory (realloc returned NULL)\n");

    	return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

/**
 * Download weather files
 *
 * Three URLs will be used to download the weather data.
 *
 * Notice!
 * The logic to extract weather data must be compliant with the page content.
 * Once the page tag structure changes, the logic may not be able to retrieve the data.  As a result, NATS developers must update the logic.
 */
int tg_download_weather_files() {
	int retValue = 1;

	bool flag_success_sigmet = false;
	bool flag_success_metar = false;
	bool flag_success_airep = false;

	string filename_SIGMET;
	string filename_METAR;
	string filename_AIREP;

	vector<string> vector_target_paths;
	vector_target_paths.push_back("https://www.aviationweather.gov/sigmet/data?hazard=all&loc=all");
	vector_target_paths.push_back("https://www.aviationweather.gov/metar/data?ids=&format=raw&date=0&hours=0");
	vector_target_paths.push_back("https://www.aviationweather.gov/airep/data?id=&distance=200&format=raw&type=&age=15&layout=on&date=");

	CURL *curl;
	CURLcode res;
	int err;

	struct MemoryStruct chunk;

	char* tmpCharArray = NULL;

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();
	if (curl) {
		for (int m = 0; m < vector_target_paths.size(); m++) {
		    curl_easy_setopt(curl, CURLOPT_URL, vector_target_paths.at(m).c_str());

		    chunk.memory = (char*)malloc(1);
		    chunk.size = 0;    /* no data at this point */

#ifdef SKIP_PEER_VERIFICATION
	    /*
	     * If you want to connect to a site who isn't using a certificate that is
	     * signed by one of the certs in the CA bundle you have, you can skip the
	     * verification of the server's certificate. This makes the connection
	     * A LOT LESS SECURE.
	     *
	     * If you have a CA cert for the server stored someplace else than in the
	     * default bundle, then the CURLOPT_CAPATH option might come handy for
	     * you.
	     */
		    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
	    /*
	     * If the site you're connecting to uses a different host name that what
	     * they have mentioned in their server certificate's commonName (or
	     * subjectAltName) fields, libcurl will refuse to connect. You can skip
	     * this check, but this will make the connection less secure.
	     */
	    	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

			/* send all data to this function  */
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

			/* we pass our 'chunk' struct to the callback function */
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

			/* Perform the request, res will get the return code */
			res = curl_easy_perform(curl);

			/* Check for errors */
			if (res != CURLE_OK)
				fprintf(stderr, "tg_download_weather_files -- curl_easy_perform() failed: %s\n",
					  curl_easy_strerror(res));

			xmlDoc* doc = htmlReadDoc((xmlChar*)chunk.memory, NULL, NULL, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);

			// Encapsulate raw libxml document in a libxml++ wrapper
			xmlNode* r = xmlDocGetRootElement(doc);
			xmlpp::Element* root = new xmlpp::Element(r);

			ofstream out;

			if (root != NULL) {
				string filename("share/tg/weather/");

				tm* tm_utc = getUTCTime();

				char buffer[80];
				strftime (buffer, 80, "%Y%m%d_%H%M%S", tm_utc);
				filename.append(buffer);

				// Extract page content from different pages of Sigmet, Airep and Metar
				// Every page shows the weather data in different HTML structures.
				//
				// Notice
				// We need to regularly check the page content and see if the HTML structure changes.
				if (vector_target_paths.at(m).find("sigmet") != string::npos) {
					filename.append(".sigmet");

					out.open(filename.c_str());

					std::string xpath = "//div[@id=\"awc_main_content\"]/div[@id=\"title\"]/text()";

					vector<Node*> elements = root->find(xpath);
					for (int i = 0; i < elements.size(); i++) {
						xmlpp::ContentNode* contentNode = new xmlpp::ContentNode(elements.at(i)->cobj());
						string tmpString(contentNode->get_content().c_str());

						tmpCharArray = (char*)calloc(strlen(contentNode->get_content().c_str())+1, sizeof(char));
						trimwhitespace(tmpCharArray, strlen(contentNode->get_content().c_str())+1, contentNode->get_content().c_str());
						if (strlen(tmpCharArray) > 0)
							out << tmpCharArray << endl << endl;
						free(tmpCharArray);
					}

					xpath = "//div[@id=\"awc_main_content\"]/div[@id=\"content\"]/*";
					elements = root->find(xpath);
					for (int i = 0; i < elements.size(); i++) {
						Node* node_layer_1 = elements.at(i);

						if (strcmp(node_layer_1->get_name().c_str(), "p") == 0) {
							list<Node*> elements_span = node_layer_1->get_children("span");

							if (elements_span.size() > 0) {
								Node* node_span = elements_span.front();
								if (node_span != NULL) {
									vector<Node*> elements_span_text = node_span->find("text()");
									if (elements_span_text.size() > 0) {
										xmlpp::ContentNode* contentNode_span = new xmlpp::ContentNode(elements_span_text.at(0)->cobj());

										tmpCharArray = (char*)calloc(strlen(contentNode_span->get_content().c_str())+1, sizeof(char));
										trimwhitespace(tmpCharArray, strlen(contentNode_span->get_content().c_str())+1, contentNode_span->get_content().c_str());
										if (strlen(tmpCharArray) > 0)
											out << endl << tmpCharArray << endl;
										free(tmpCharArray);
									}
								}
							}
						} else if (strcmp(node_layer_1->get_name().c_str(), "span") == 0) {
							vector<Node*> elements_span_text = node_layer_1->find("text()");
							if (elements_span_text.size() > 0) {
								xmlpp::ContentNode* contentNode_span = new xmlpp::ContentNode(elements_span_text.at(0)->cobj());

								tmpCharArray = (char*)calloc(strlen(contentNode_span->get_content().c_str())+1, sizeof(char));
								trimwhitespace(tmpCharArray, strlen(contentNode_span->get_content().c_str())+1, contentNode_span->get_content().c_str());
								if (strlen(tmpCharArray) > 0)
									out << endl << tmpCharArray << endl;
								free(tmpCharArray);
							}
						} else if (strcmp(node_layer_1->get_name().c_str(), "pre") == 0) {
							vector<Node*> elements_pre_text = node_layer_1->find("text()");
							if (elements_pre_text.size() > 0) {
								xmlpp::ContentNode* contentNode_pre = new xmlpp::ContentNode(elements_pre_text.at(0)->cobj());

								tmpCharArray = (char*)calloc(strlen(contentNode_pre->get_content().c_str())+1, sizeof(char));
								trimwhitespace(tmpCharArray, strlen(contentNode_pre->get_content().c_str())+1, contentNode_pre->get_content().c_str());
								if (strlen(tmpCharArray) > 0)
									out << endl << tmpCharArray << endl;
								free(tmpCharArray);
							}
						}
					}

					flag_success_sigmet = true;
				} else if (vector_target_paths.at(m).find("airep") != string::npos) {
					filename.append(".airep");

					out.open(filename.c_str());

					std::string xpath = "//div[@id=\"awc_main_content\"]/div[@id=\"title\"]/text()";

					vector<Node*> elements = root->find(xpath);
					for (int i = 0; i < elements.size(); i++) {
						xmlpp::ContentNode* contentNode = new xmlpp::ContentNode(elements.at(i)->cobj());
						string tmpString(contentNode->get_content().c_str());

						tmpCharArray = (char*)calloc(strlen(contentNode->get_content().c_str())+1, sizeof(char));
						trimwhitespace(tmpCharArray, strlen(contentNode->get_content().c_str())+1, contentNode->get_content().c_str());
						if (strlen(tmpCharArray) > 0)
							out << tmpCharArray << endl << endl;
						free(tmpCharArray);
					}

					xpath = "//div[@id=\"awc_main_content\"]/div/*";
					elements = root->find(xpath);
					for (int i = 0; i < elements.size(); i++) {
						Node* node_layer_1 = elements.at(i);
						if (strcmp(node_layer_1->get_name().c_str(), "code") == 0) {
							vector<Node*> elements_code_text = node_layer_1->find("text()");
							if (elements_code_text.size() > 0) {
								xmlpp::ContentNode* contentNode_code = new xmlpp::ContentNode(elements_code_text.at(0)->cobj());

								tmpCharArray = (char*)calloc(strlen(contentNode_code->get_content().c_str())+1, sizeof(char));
								trimwhitespace(tmpCharArray, strlen(contentNode_code->get_content().c_str())+1, contentNode_code->get_content().c_str());
								if (strlen(tmpCharArray) > 0)
									out << endl << tmpCharArray << endl;
								free(tmpCharArray);
							}
						}
					}

					flag_success_airep = true;
				} else if (vector_target_paths.at(m).find("metar") != string::npos) {
					filename.append(".metar");

					out.open(filename.c_str());

					std::string xpath = "//div[@id=\"awc_main_content\"]/div[@id=\"title\"]/text()";

					vector<Node*> elements = root->find(xpath);
					for (int i = 0; i < elements.size(); i++) {
						xmlpp::ContentNode* contentNode = new xmlpp::ContentNode(elements.at(i)->cobj());
						string tmpString(contentNode->get_content().c_str());

						tmpCharArray = (char*)calloc(strlen(contentNode->get_content().c_str())+1, sizeof(char));
						trimwhitespace(tmpCharArray, strlen(contentNode->get_content().c_str())+1, contentNode->get_content().c_str());
						if (strlen(tmpCharArray) > 0)
							out << tmpCharArray << endl << endl;
						free(tmpCharArray);
					}

					xpath = "//div[@id=\"awc_main_content_wrap\"]/*";
					elements = root->find(xpath);
					for (int i = 0; i < elements.size(); i++) {
						Node* node_layer_1 = elements.at(i);
						if (strcmp(node_layer_1->get_name().c_str(), "code") == 0) {
							vector<Node*> elements_code_text = node_layer_1->find("text()");
							if (elements_code_text.size() > 0) {
								xmlpp::ContentNode* contentNode_code = new xmlpp::ContentNode(elements_code_text.at(0)->cobj());

								tmpCharArray = (char*)calloc(strlen(contentNode_code->get_content().c_str())+1, sizeof(char));
								trimwhitespace(tmpCharArray, strlen(contentNode_code->get_content().c_str())+1, contentNode_code->get_content().c_str());
								if (strlen(tmpCharArray) > 0)
									out << endl << tmpCharArray << endl;
								free(tmpCharArray);
							}
						}
					}

					flag_success_metar = true;
				}

				printf("Saving weather file: %s\n", filename.c_str());

				out.close();
			}

			free(chunk.memory);
		} // end - for

	    curl_easy_cleanup(curl);

	    if (flag_success_sigmet && flag_success_metar && flag_success_airep) {
	    	retValue = 0;
	    }
	}

	curl_global_cleanup();
}
