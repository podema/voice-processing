#include <iostream>
#include <algorithm>
#include "wavmonopcm.h"
#include "wavfile.h"
#include "filename.h"

using namespace std;
using namespace upc;

int wavmonopcm_read(const string& inputFileName, vector<float> &x, float &sampling_rate) {

  if (inputFileName.empty())
    return -1;

  WavFile *iwav = fwavOpenRead(inputFileName.c_str());
  if (iwav == 0)
    return -2;

  /* Only mono files, and till 12kHz supported */
  if (iwav->wFormatTag != WAVE_FORMAT_PCM || iwav->channels != 1 || iwav->Fs > 16000 || iwav->nsamples == 0) {
    cerr << "Error: only mono, pcm files, with sampling freq <= 16k are supported: " << inputFileName << endl;
    return -3;
  }

  sampling_rate = iwav->Fs;
  vector<short> xs(iwav->nsamples);
  if (fwavRead(&(*xs.begin()), iwav->nsamples, iwav) != iwav->nsamples) {
    cerr << "Error reading input file" << inputFileName << endl;
    return -4;
  }
  fwavCloseRead(iwav);

  //Convert from short to float
  x.resize(xs.size());
  copy(xs.begin(), xs.end(), x.begin());
  return 0;
}

int wavmonopcm_write(const string& _outputFileName, vector<float> const &y, float rate) {
  if (_outputFileName.empty())
    return -1;

  if (y.empty())
    return -4;

  /* Create directory for output file, just in case it is not created*/
  Filename outputFileName(_outputFileName);
  outputFileName.checkDir();

  /* Open the output file, with header information */
  WavFile *owav = fwavOpenWrite(outputFileName.c_str(), (float) rate, 2, 1, WAVE_FORMAT_PCM);
  if (owav == 0) {
    cerr << "Error opening output file" << outputFileName << endl;
    return -2;
  }

  //Convert from float to short
  vector<short> ys(y.size());
  copy(y.begin(), y.end(), ys.begin());

  if (fwavWrite(&(*ys.begin()), ys.size(), owav) != ys.size()) {
    cerr << "Error writting output file" << outputFileName << endl;
    return -3;
  }
  fwavCloseWrite(owav);
  return 0;  
}

