/**
 * main.cpp
 * 
 * Application entry.
 * lektor
 * Use this program to write encrypted data files to clear files
 * or to write clear files to encrypted files.
 * Uses libcrypto
 *
 * Author: jason
 * Date: January 19, 2013
 */

#include "lektor.h"

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <cstdlib>

#include <getopt.h>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ifstream;
using std::ofstream;
using std::stringstream;

static bool   g_encrypt = true;
static string g_infile;
static string g_outfile = "";

static void print_usage(const string& progname) {
  cout << endl;
  cout << progname << endl;
  cout << "Description:" << endl;
  cout << "  Encrypt or decrypt the specified file using the AES 256-bit " << endl;
  cout << "  encryption standard.";
  cout << endl;
  cout << "Usage: " << progname << " [options]" << endl;
  cout << "  Options:" << endl;
  cout << "  --encrypt           -e             encrypt the input file [DEFAULT]" << endl;
  cout << "  --decrypt           -d             decrypt the input file" << endl;
  cout << "  --in=<file>         -i <file>      input file" << endl;
  cout << "  --out=<file>        -o <file>      output file" << endl;
  cout << "  --help              -h             Print this message" << endl;
  cout << endl;
}

static void parse_args(int argc, char* argv[]) {
  int c;
  struct option opts[] = {
    {"encrypt", 0, 0, 'e'},
    {"decrypt", 0, 0, 'd'},
    {"in", 1, 0, 'i'},
    {"out", 1, 0, 'o'},
    {"help", 0, 0, 'h'},
    {0, 0, 0, 0}
  };
  const char* optstr = "edi:o:h";
  
  while(true) {
    
    int optind = 0;
    c = getopt_long(argc, argv, optstr, opts, &optind);
    
    if(c==-1) {
      break;
    }
    switch(c) {
    case 'e':
      g_encrypt = true;
      break;
    case 'd':
      g_encrypt = false;
      break;
    case 'i':
      g_infile = string(optarg);
      break;
    case 'o':
      g_outfile = string(optarg);
      break;
    case 'h':
    case 0:
    default:
      print_usage(argv[0]);
      exit(0);
    }
  }
}

static bool can_open_file(const string& fname) {
  ifstream in;
  in.open(fname.c_str());
  if(in.is_open()) {
    in.close();
    return true;
  }
  return false;
}

int main(int argc, char* argv[]) {

  cout << "GNATS Data Encrypter/Decrypter" << endl;

  // parse options
  parse_args(argc, argv);

  // validate the input file
  if(!can_open_file(g_infile)) {
    cerr << "ERROR: invalid input file." << endl;
    exit(-1);
  }
  
  // validate the output file name and construct
  // a default name if one wasn't supplied.
  if(g_outfile == "") {
    stringstream ss;
    ss << g_infile << (g_encrypt ? ".crypt" : ".clear");
    g_outfile = ss.str();
  }

  // encrypt/decrypt
  if(g_encrypt) {
    encryptFile(g_infile.c_str(), g_outfile.c_str());
  } else {
    decryptFile(g_infile.c_str(), g_outfile.c_str());
  }

  cout << "Good bye." << endl;
  return 0;
}
