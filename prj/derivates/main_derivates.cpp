#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>

#include "getopt.h"
#include "matrix.h"
#include "filename.h"

using namespace std;
using namespace upc;

const string DEF_INPUT_EXT = "mcp";
const string DEF_OUTPUT_EXT = "d1";
const int DEF_LEN_DERIV = 2;

int usage(const char *progname, int err);
int read_options(int ArgC, const char *ArgV[], Directory &input_dir, Directory &output_dir, 
		 Ext &input_ext, Ext&output_ext, 
		 unsigned int &length_deriv, vector<string> &filenames, 
		 unsigned int &verbose);


int derivate(const string &inputfile, const string &outputfile, int k) {

  ///Read input file
  ifstream is(inputfile.c_str());
  if (!is.good()) {
    cerr << "Error opening input file: " << inputfile << endl;
    return -1;
  }
  fmatrix parameters, deriv;
  is >> parameters;

  ///TODO: compute derivates
  deriv=parameters;

  ///Write the results in the output file
  ofstream os(outputfile.c_str());
  if (!os.good()) {
    cerr << "Error opening output file: " << outputfile << endl;
    return -2;
  }
  os << deriv;
  return 0;
} 


int main(int argc, const char * argv[]) {

  Directory input_directory, output_directory;
  Ext input_extension(DEF_INPUT_EXT),
    output_extension(DEF_OUTPUT_EXT);
  vector<string> filenames;

  unsigned int verbose = 1;

  const char *prog_name = argv[0];
  unsigned int deriv_len = DEF_LEN_DERIV;
  
  ///Read command line options
  int retv;
  retv = read_options(argc, argv, input_directory, output_directory, 
		      input_extension, output_extension, 
		      deriv_len, filenames, verbose);
  if (retv != 0)
    return usage(prog_name,retv);

  unsigned int nfile;
  for (nfile=0; nfile < filenames.size(); ++nfile) {
    Filename input_filename = input_directory + filenames[nfile] + input_extension;
    Filename output_filename = output_directory + filenames[nfile] + output_extension;
    if (verbose & 1) cout << input_filename << " => " << output_filename << endl;
    if (!input_filename.exist()) {
      cerr << "ERROR: " << input_filename << " not found" << endl;
      continue;
    }
    if (!output_filename.checkDir()) {
      cerr << "ERROR creating directory for output file " << output_filename << endl;
      continue;      
    }
    if (derivate(input_filename, output_filename, deriv_len)) {
      cerr << "ERROR processing " << input_filename 
	   << " with " << prog_name << endl;
      continue;
    }
  }

  return 0;
}


int usage(const char *progname, int err)  {
  cerr << "Usage: " << progname << " [options] list_of_files\n";
  cerr << "Usage: " << progname << " [options] -F file1 ...\n\n";

  cerr << "This program computes the temporal derivative of a temporal sequence of files.\n"
       << "For each input file and output file is created\n"
       << "The name of the file can be provided in the command line,\n"
       << "using the '-F' option, or using a list with the name of files\n"
       << "to be analyzed\n"
       << "The name of the file should not contain the root directory or extension\n"
       << "which has to be provided using the options\n\n";

  cerr << "The possible options are:\n"
       << "  -d dir\tDirectory of the input files (def. \".\")\n"
       << "  -e ext\tExtension of the input files (def. \"" << DEF_INPUT_EXT << "\")\n"
       << "  -D dir\tDirectory of the input files (def. \".\")\n"
       << "  -E ext\tExtension of the input files (def. \"" << DEF_OUTPUT_EXT << "\")\n"
       << "  -k n\t2n+1 nframes are used to compute derivatve  (def. "  << DEF_LEN_DERIV << ")\n"
       << "  -v int\tBit code to control \"verbosity\"; eg: 5 => 00000101" << ")\n";
  return err;
}

int read_options(int ArgC, const char *ArgV[], Directory &input_dir, Directory &output_dir, 
		 Ext &input_ext, Ext&output_ext, unsigned int &deriv_len, 
		 vector<string> &filenames, unsigned int &verbose) {

  char option;
  bool use_list = true;
  filenames.clear();

  //optarg and optind are global variables declared and set by the getopt() function

  while ((option = getopt(ArgC, (char **)ArgV, "d:e:D:E:k:v:F")) != -1) {
    switch (option) {
    case 'd': input_dir = optarg; break;
    case 'D': output_dir = optarg; break;
    case 'e': input_ext = optarg; break;
    case 'E': output_ext = optarg; break;
    case 'k': deriv_len = atoi(optarg); break;
    case 'v': verbose = atoi(optarg); break;
    case 'F': use_list=false; break;
    case '?': return -1;
    }
  }
  if (!input_dir.empty() && *input_dir.rbegin() != '/') input_dir += '/';
  if (!output_dir.empty() && *output_dir.rbegin() != '/') output_dir += '/';
  if (input_ext[0] != '.') input_ext = '.' + input_ext;
  if (output_ext[0] != '.') output_ext = '.' + output_ext;

  //advance argc and argv to skip read options
  ArgC -= optind;
  ArgV += optind;

  if (use_list) {
    if (ArgC != 1) {
      cerr << "ERROR no list of files provided" << endl;
      return -2;
    }
    ifstream is(ArgV[0]);
    if (!is.good()) {
      cerr << "ERROR opening list of files: " << ArgV[0] << endl;
      return -3;
    }
    string s;
    while (is >> s)
      filenames.push_back(s);
  } else {
    for (int i=0; i<ArgC; ++i)
      filenames.push_back(ArgV[i]);
  }
  return 0;      
}
