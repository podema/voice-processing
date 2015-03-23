
/*------------------------------------------------------------------------------* 
  | 
  | This material contains unpublished, proprietary software of 
  | the Universitat Politecnica de Catalunya. 
  | 
  | Permission to copy, use, modify, sell and distribute this software
  | is granted provided this copyright notice appears in all copies.
  | This software is provided "as is" without express or implied
  | warranty, and with no claim as to its suitability for any purpose.
  | 
  | Copyright (c) Antonio Bonafonte (antonio.bonafonte@upc.edu), Albert Febrer
  |               Universitat Politecnica de Catalunya, Barcelona, Spain. 
  | 
  *------------------------------------------------------------------------------*/



#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#ifndef _WIN32
#include <stdint.h>
#else
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
#endif

#ifndef  WAVE_FORMAT_PCM
#define  WAVE_FORMAT_PCM       0x0001
#endif
#ifndef  WAVE_FORMAT_ULAW
#define  WAVE_FORMAT_ULAW       0x0007
#endif
#ifndef  WAVE_FORMAT_ALAW
#define  WAVE_FORMAT_ALAW       0x0006
#endif

typedef struct {
	FILE *f;
	uint32_t  Fs;
	uint16_t bytesPerSample;
	uint32_t  nsamples;
	uint16_t channels;
  	uint16_t wFormatTag;
} WavFile;


    /* Ooops!!!
       
    When reading ALAW the stream is decoded inside fwavRead, so the
    contents of 'data' are 16bits PCM samples.  When reading ULAW the
    stream is not (yet) decoded, so the contents of 'data' are 8bits
    ULAW samples.

    When writing either ULAW or ALAW, the origin buffer must contain
    8bits samples in ULAW or ALAW respectively.

    This needs some fixing...
    
    */


/* Read WAV file */
WavFile *fwavOpenRead(const char *filename);
uint32_t fwavRead(int16_t *data, uint32_t nsamples, WavFile *fwav);
    void fwavCloseRead(WavFile *fwav);

/* Write WAV file */
/* WavFile *fwavOpenWrite(const char *filename, uint32_t Fs, */
/* 					   uint16_t bytesPerSample, uint16_t channels); */
WavFile *fwavOpenWrite(const char *filename, uint32_t Fs,
		       uint16_t bytesPerSample, uint16_t channels,
		       uint16_t format);
uint32_t fwavWrite(const int16_t *data, uint32_t nsamples, WavFile *fwav);
void fwavCloseWrite(WavFile *fwav);


/* Multiplex/Demultiplex buffer 
   wave_demultiplex(...) demultiplex the samples of data_ori: first, nsamples of channel 0, then
   nsamples of channel 1 ... and finally nsamples of last channel.
   The size of data_ori and data_dst should be: nsamples * nchannels
*/

int wave_demultiplex(const int16_t *data_ori, int16_t *data_dst, uint32_t nsamples, uint16_t nchannels);
int wave_multiplex(const int16_t *data_ori, int16_t *data_dst, uint32_t nsamples, uint16_t nchannels);



#ifdef __cplusplus
}
#endif
