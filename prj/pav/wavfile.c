
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


/* NOTE: this functions are only prepared for little-endian arquitectures */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "wavfile.h"

static int read_ckinfo(FILE *f, char *id, uint32_t *size, int sflg);
static int write_ckinfo(FILE *f, char *id, uint32_t size, int sflg);
static int find_cktype(FILE *f, const char *type, uint32_t *size, int sflg);

#define LittleEndian 1
#define BigEndian    2

static int TestEndian() {
    char b[2] = {0x00, 0x0F};
    int16_t  *p = (int16_t *) b;

    if (*p == 0x0F00) return LittleEndian;
    if (*p == 0x000F) return BigEndian;

    return 0;
}

static int16_t ADecTbl[256] = {
    -688,  -656,  -752,  -720,  -560,  -528,  -624,  -592,
    -944,  -912, -1008,  -976,  -816,  -784,  -880,  -848,
    -344,  -328,  -376,  -360,  -280,  -264,  -312,  -296,
    -472,  -456,  -504,  -488,  -408,  -392,  -440,  -424,
    -2752, -2624, -3008, -2880, -2240, -2112, -2496, -2368,
    -3776, -3648, -4032, -3904, -3264, -3136, -3520, -3392,
    -1376, -1312, -1504, -1440, -1120, -1056, -1248, -1184,
    -1888, -1824, -2016, -1952, -1632, -1568, -1760, -1696,
    -43,   -41,   -47,   -45,   -35,   -33,   -39,   -37,
    -59,   -57,   -63,   -61,   -51,   -49,   -55,   -53,
    -11,    -9,   -15,   -13,    -3,    -1,    -7,    -5,
    -27,   -25,   -31,   -29,   -19,   -17,   -23,   -21,
    -172,  -164,  -188,  -180,  -140,  -132,  -156,  -148,
    -236,  -228,  -252,  -244,  -204,  -196,  -220,  -212,
    -86,   -82,   -94,   -90,   -70,   -66,   -78,   -74,
    -118,  -114,  -126,  -122,  -102,   -98,  -110,  -106,
    688,   656,   752,   720,   560,   528,   624,   592,
    944,   912,  1008,   976,   816,   784,   880,   848,
    344,   328,   376,   360,   280,   264,   312,   296,
    472,   456,   504,   488,   408,   392,   440,   424,
    2752,  2624,  3008,  2880,  2240,  2112,  2496,  2368,
    3776,  3648,  4032,  3904,  3264,  3136,  3520,  3392,
    1376,  1312,  1504,  1440,  1120,  1056,  1248,  1184,
    1888,  1824,  2016,  1952,  1632,  1568,  1760,  1696,
    43,    41,    47,    45,    35,    33,    39,    37,
    59,    57,    63,    61,    51,    49,    55,    53,
    11,     9,    15,    13,     3,     1,     7,     5,
    27,    25,    31,    29,    19,    17,    23,    21,
    172,   164,   188,   180,   140,   132,   156,   148,
    236,   228,   252,   244,   204,   196,   220,   212,
    86,    82,    94,    90,    70,    66,    78,    74,
    118,   114,   126,   122,   102,    98,   110,   106,
};



static int swap = 0;

#define FREAD_SWAP(a, b, c, d) (swap ? fread_swap(a,b,c,d)  : fread(a,b,c,d))
#define FWRITE_SWAP(a, b, c, d) (swap ? fwrite_swap(a,b,c,d) : fwrite(a,b,c,d))

static size_t fread_swap(void *buffer, size_t size, size_t count, FILE *stream);
static size_t fwrite_swap(const void *_buffer, size_t size, size_t count, FILE *stream);
static int fswap(void *_buffer, size_t size, size_t count);


int wave_demultiplex(const int16_t *_ori, int16_t *_dst, uint32_t nsamples, uint16_t nchannels) {
    uint32_t channel, sample;
    const int16_t *ori;
    int16_t *dst;
    dst = _dst;
    for (channel = 0; channel < nchannels; ++channel) {
	ori = _ori + channel;
	for (sample = 0; sample < nsamples; ++sample) {
	    *dst = *ori;
	    ++dst;
	    ori += nchannels;
	}
    }
    return 0;
}

int wave_multiplex(const int16_t *_ori, int16_t *_dst, uint32_t nsamples, uint16_t nchannels) {
    uint32_t channel, sample;
    const int16_t *ori;
    int16_t *dst;
    ori = _ori;
    for (channel = 0; channel < nchannels; ++channel) {
	dst = _dst + channel;
	for (sample = 0; sample < nsamples; ++sample) {
	    *dst = *ori;
	    ++ori;
	    dst += nchannels;
	}
    }
    return 0;
}


WavFile *fwavOpenRead(const char *filename)
{
    WavFile *fwav;
    uint32_t riffck;
    uint32_t waveck;
    uint32_t total_bytes;
    uint32_t  nAvgBytesPerSec;
    uint16_t nBlockAlign;
    uint16_t nBitsPerSample;
    uint32_t nbytes;

    if (TestEndian() != LittleEndian) /* This version only works in little-endian architectures */
	swap = 1;

    if(filename == NULL)
	return NULL;    
    
    if ((fwav = (WavFile *)malloc(sizeof(WavFile))) == NULL)
	return NULL;

    if ((fwav->f = fopen(filename,"rb")) == NULL) {
	free(fwav);
	return NULL;
    }

    /* Find the first RIFF chunk: */
    if (find_cktype(fwav->f, "RIFF", &riffck, 0)) {
	fwavCloseRead(fwav);
	return NULL;
    }

    /* The subchunk better be WAVE: */
    if (find_cktype(fwav->f, "WAVE", &waveck, 1)) {
	fwavCloseRead(fwav);
	return NULL;
    }
    /* Find <fmt-ck> chunk: */
    if (find_cktype(fwav->f, "fmt ", &total_bytes, 0)) {
	fwavCloseRead(fwav);
	return NULL;
    }

    /* Read standard <wave-format> data: */
    FREAD_SWAP(&(fwav->wFormatTag), sizeof(uint16_t), 1, fwav->f);    /* Data encoding format */
    /* Read standard <PCM-format-specific> info: */
    switch(fwav->wFormatTag){
    case WAVE_FORMAT_PCM:
    case WAVE_FORMAT_ALAW:
    case WAVE_FORMAT_ULAW:
	break;
    default:
	/* Unknown wave-format for data. */
	fwavCloseRead(fwav);
	return NULL;
    }
     
    FREAD_SWAP(&(fwav->channels), sizeof(uint16_t), 1, fwav->f);  /* Number of channels */
    FREAD_SWAP(&(fwav->Fs), sizeof(uint32_t), 1, fwav->f);         /* Samples per second */
    FREAD_SWAP(&nAvgBytesPerSec, sizeof(uint32_t), 1, fwav->f);    /* Avg transfer rate */
    FREAD_SWAP(&nBlockAlign, sizeof(uint16_t), 1, fwav->f);       /* Block alignment */
    nbytes = 14;  /* # of bytes read so far */
	
    /* There had better be a bits/sample field: */
    if (total_bytes < nbytes+2) {
	fwavCloseRead(fwav);
	return NULL;
    }
    if (FREAD_SWAP(&nBitsPerSample, sizeof(uint16_t), 1, fwav->f) != 1) {
	fwavCloseRead(fwav);
	return NULL;
    }
    nbytes += 2;
	
    /* Are there any additional fields present? */
    if (total_bytes > nbytes) {
	/* See if the "cbSize" field is present.  If so, grab the data:*/
	if (total_bytes >= nbytes + 2) {
	    /* we have the cbSize uint16_t in the file:*/
	    uint16_t cbSize;
	    if (FREAD_SWAP(&cbSize, sizeof(uint16_t), 1, fwav->f) != 1) {
		fwavCloseRead(fwav);
		return NULL;
	    }
	    nbytes += 2;
	}
		
	/* Check for anything else:*/
	if (total_bytes > nbytes) {
	    /* Simply skip remaining stuff - we don't know what it is:*/
	    if (fseek(fwav->f,total_bytes - nbytes, SEEK_CUR) == -1) {
		fwavCloseRead(fwav);
		return NULL;
	    }
	}
    }

    /* Find <data-ck> chunk:*/
    if (find_cktype(fwav->f, "data", &total_bytes, 0)) {
	fwavCloseRead(fwav);
	return NULL;
    }

    /* PCM Format:
     * Determine # bytes/sample - format requires rounding
     * to next integer number of bytes: */
    fwav->bytesPerSample = (uint16_t)ceil((double)nBitsPerSample / 8);
    fwav->nsamples = (total_bytes / fwav->bytesPerSample) / fwav->channels;

    return fwav;
}

uint32_t fwavRead(int16_t *data, uint32_t nsamples, WavFile *fwav)
{
    uint32_t samples;

    if(data == NULL || fwav == NULL)
	return -1;
	
    if (fwav->wFormatTag == WAVE_FORMAT_ALAW) {
	unsigned char *data_A = (unsigned char *) data;
	data_A += nsamples * fwav->channels;
	samples = FREAD_SWAP(data_A, fwav->bytesPerSample, fwav->channels * nsamples, fwav->f);
	nsamples = samples;
	for (nsamples = 0; nsamples < samples; ++nsamples)
	    data[nsamples] = ADecTbl[data_A[nsamples]] << 3;
    } else
	samples = FREAD_SWAP(data, fwav->bytesPerSample, fwav->channels * nsamples, fwav->f);

   
    samples /= fwav->channels;

    return samples;
}

void fwavCloseRead(WavFile *fwav)
{

    fclose(fwav->f);
    free(fwav);
}

WavFile *fwavOpenWrite(const char *filename, uint32_t Fs,
		       uint16_t bytesPerSample, uint16_t channels,
		       uint16_t format)
{

    WavFile *fwav;
    /* Toni: fill the information we already know */

    /*    uint16_t szero[2] = {0,0};
	  uint32_t  lzero[2] = {0,0}; */

    uint32_t  nAvgBytesPerSec;
    uint16_t  nBlockAlign;
    uint16_t  nBitsPerSample;

    char msg[5];

/*     if (bytesPerSample != 2 * channels) */
/* 	return NULL; */

    if (TestEndian() != LittleEndian) 
	swap = 1;

    if (filename == NULL)
	return NULL;   

    if ((fwav = (WavFile *)malloc(sizeof(WavFile))) == NULL)
	return NULL;

    if ((fwav->f = fopen(filename,"wb")) == NULL) {
	free(fwav);
	return NULL;
    }

    fwav->Fs = Fs;
    fwav->bytesPerSample = bytesPerSample;
    fwav->nsamples = 0;
    fwav->channels = channels;
    fwav->wFormatTag = format;

    /* Determine number of bytes in RIFF chunk
     * (not including pad bytes, if needed):
     * ----------------------------------
     *  'RIFF'           4 bytes
     *  size             4 bytes (ulong)
     *  'WAVE'           4 bytes
     *	'fmt '            4 bytes
     *  size             4 bytes (ulong)
     * <wave-format>    14 bytes
     * <format_specific> 2 bytes (PCM)
     *  'data'           4 bytes
     *  size             4 bytes (ulong)
     * <wave-data>       N bytes
     * ----------------------------------*/

    /* Write RIFF chunk:*/    
    strcpy(msg, "RIFF"); 
    if (write_ckinfo(fwav->f, msg, 0, 0)) {
	fclose(fwav->f);
	free(fwav);      
	return NULL;
    }

    /* Write WAVE:*/
    strcpy(msg, "WAVE"); 
    if (write_ckinfo(fwav->f, msg, 0, 1)) {
	fclose(fwav->f);
	free(fwav);      
	return NULL;
    }

    /* Write <fmt-ck>:*/
    strcpy(msg, "fmt "); 
    if (write_ckinfo(fwav->f, msg, 16, 0)) {
	fclose(fwav->f);
	free(fwav);      
	return NULL;
    }


    /* Write <wave-format>: */
    nAvgBytesPerSec = fwav->channels * fwav->bytesPerSample * fwav->Fs;
    nBlockAlign = (uint16_t)(fwav->channels * fwav->bytesPerSample);
    nBitsPerSample = (uint16_t)(fwav->bytesPerSample * 8);

    /* Write <wave-format>:*/

    if (FWRITE_SWAP(&(fwav->wFormatTag), sizeof(uint16_t), 1, fwav->f) != 1) {
	fclose(fwav->f);
	free(fwav);      
	return NULL;
    }

    if (FWRITE_SWAP(&(fwav->channels), sizeof(uint16_t), 1, fwav->f) != 1) {
	fclose(fwav->f);
	free(fwav);      
    }


    if (FWRITE_SWAP(&(fwav->Fs), sizeof(uint32_t), 1, fwav->f) != 1) {
	fclose(fwav->f);
	free(fwav);      
	return NULL;
    }

    if (FWRITE_SWAP(&nAvgBytesPerSec, sizeof(uint32_t), 1, fwav->f) != 1){
	fclose(fwav->f);
	free(fwav);      
	return NULL;
    }

    if (FWRITE_SWAP(&nBlockAlign, sizeof(uint16_t), 1, fwav->f) != 1){
	fclose(fwav->f);
	free(fwav);      
	return NULL;
    }

    if (FWRITE_SWAP(&nBitsPerSample, sizeof(uint16_t), 1, fwav->f) != 1){
	fclose(fwav->f);
	free(fwav);      
	return NULL;
    }




    /* Write <data-ck>:*/
    strcpy(msg, "data"); 
    if (write_ckinfo(fwav->f, msg, 0, 0)) {
	fclose(fwav->f);
	free(fwav);      
	return NULL;
    }

    return fwav;
}

uint32_t fwavWrite(const int16_t *data, uint32_t nsamples, WavFile *fwav)
{
    
    /* PCM Format:
     * Determine # bytes/sample - format requires rounding
     *  to next integer number of bytes: */
    uint32_t i, size;
    unsigned char c;

    if(data == NULL || fwav == NULL)
	return -1;

    size  = nsamples * fwav->channels;
    

    if (fwav->bytesPerSample == 1) {
	if(fwav->wFormatTag == WAVE_FORMAT_PCM){
	    /* Scale data according to bits/samples: [-1,+1] -> [0,255] */
	    int16_t max = 0;
	    /* Normalized range [-1,+1] */
	    for (i=0; i<size; i++) {
		if (abs(data[i]) > max)
		    max = abs(data[i]);
	    }
	    for (i=0; i<size; i++) {
		c = (unsigned char)floor(((double)data[i]/max + 1) * 255/2 + 0.5);
		if (FWRITE_SWAP(&c, sizeof(unsigned char), 1, fwav->f) != 1)
		    return 0;
	    }
	}
	else {
	    /* WAVE_FORMAT_ALAW or WAVE_FORMAT_ULAW */ 
	    if (FWRITE_SWAP(data, sizeof(unsigned char), size, fwav->f) != size)
		return 0;
	}
    }else if (fwav->bytesPerSample == 2) {
	if (FWRITE_SWAP(data, sizeof(int16_t), size, fwav->f) != size)
	    return 0;
    }
    else{
	/* Cannot write WAVE files with more than 16 bits/sample */
	return 0; 
    }
    fwav->nsamples += nsamples;
    return nsamples;
}

void fwavCloseWrite(WavFile *fwav)
{
    uint32_t  total_bytes;
    uint32_t  riff_cksize;
    uint32_t  nAvgBytesPerSec;
    uint16_t nBlockAlign;
    uint16_t nBitsPerSample;
    char msg[5];

    if (fwav==NULL)
	return;

    /* Determine if a pad-byte is appended to data chunk: */
    if (fmod((double)(fwav->nsamples * fwav->channels * fwav->bytesPerSample), 2)) {
	char c = 0;
	if (FWRITE_SWAP(&c, sizeof(unsigned char), 1, fwav->f) != 1)
	    return;
    }
    rewind(fwav->f);

    /* Determine number of bytes in RIFF chunk 
     * (not including pad bytes, if needed): 
     * ---------------------------------- 
     *  'RIFF'           4 bytes 
     *  size             4 bytes (ulong) 
     *  'WAVE'           4 bytes
     *	'fmt '            4 bytes
     *  size             4 bytes (ulong)
     * <wave-format>    14 bytes
     * <format_specific> 2 bytes (PCM)
     *  'data'           4 bytes
     *  size             4 bytes (ulong)
     * <wave-data>       N bytes
     * ---------------------------------- */
    total_bytes = fwav->nsamples * fwav->channels * fwav->bytesPerSample;

    /* Write RIFF chunk: */
    riff_cksize = 36 + total_bytes; /* Don't include 'RIFF' or its size field */
    riff_cksize += (uint32_t)fmod((double)total_bytes, (double)2);
    strcpy(msg, "RIFF");
    if (write_ckinfo(fwav->f, msg, riff_cksize, 0))
	return;
	
    /* Write WAVE: */
    strcpy(msg, "WAVE");
    if (write_ckinfo(fwav->f, msg, 0, 1))
	return;

    /* Write <fmt-ck>: */
    strcpy(msg, "fmt ");
    if (write_ckinfo(fwav->f, msg, 16, 0))
	return;

    /* Write <wave-format>: */
    nAvgBytesPerSec = fwav->channels * fwav->bytesPerSample * fwav->Fs;
    nBlockAlign = (uint16_t)(fwav->channels * fwav->bytesPerSample);
    nBitsPerSample = (uint16_t)(fwav->bytesPerSample * 8);

    if (FWRITE_SWAP(&(fwav->wFormatTag), sizeof(uint16_t), 1, fwav->f) != 1)
	return;
    if (FWRITE_SWAP(&(fwav->channels), sizeof(uint16_t), 1, fwav->f) != 1)
	return;
    if (FWRITE_SWAP(&(fwav->Fs), sizeof(uint32_t), 1, fwav->f) != 1)
	return;
    if (FWRITE_SWAP(&nAvgBytesPerSec, sizeof(uint32_t), 1, fwav->f) != 1)
	return;
    if (FWRITE_SWAP(&nBlockAlign, sizeof(uint16_t), 1, fwav->f) != 1)
	return;
    if (FWRITE_SWAP(&nBitsPerSample, sizeof(uint16_t), 1, fwav->f) != 1)
	return;

    /* Write <data-ck>: */
    strcpy(msg, "data");
    if (write_ckinfo(fwav->f, msg, total_bytes, 0))
	return;

    fclose(fwav->f);
    free(fwav);
}

/* ------------------------------------------------------------------------
 * Private functions:
 * ------------------------------------------------------------------------

 * READ_CKINFO: Reads next RIFF chunk, but not the chunk data.
 * If optional sflg is set to nonzero, reads SUBchunk info instead.
 * Expects an open FID pointing to first byte of chunk header.
 * Returns a new chunk structure. */

static int read_ckinfo(FILE *f, char *id, uint32_t *size, int sflg)
{
    if (FREAD_SWAP(id, sizeof(char), 4, f) != 4)
	return 1;
    id[4] = '\0';

    if (!sflg) {
	/* Read chunk size (skip if subchunk): */
	if (FREAD_SWAP(size, sizeof(uint32_t), 1, f) != 1)
	    return 1;
    }

    return 0;
}

/* WRITE_CKINFO: Writes next RIFF chunk, but not the chunk data.
 *   If optional sflg is set to nonzero, write SUBchunk info instead.
 *   Expects an open FID pointing to first byte of chunk header,
 *   and a chunk structure.
 *   ck.fid, ck.ID, ck.Size, ck.Data */

static int write_ckinfo(FILE *f, char *id, uint32_t size, int sflg)
{
    if (FWRITE_SWAP(id, sizeof(char), 4, f) != 4)
	return 1;
		
    if (!sflg) {
	/* Write chunk size (skip if subchunk): */
	if (FWRITE_SWAP(&size, sizeof(uint32_t), 1, f) != 1)
	    return 1;
    }

    return 0;
}

/* FIND_CKTYPE: Finds a chunk with appropriate type.
 *   Searches from current file position specified by fid.
 *   Leaves file positions to data of desired chunk.
 *   If optional sflg is set to nonzero, finds a SUBchunk instead. */
static int find_cktype(FILE *f, const char *type, uint32_t *size, int sflg)
{
    char id[5];
    while (1) {
	if (read_ckinfo(f, id, size, sflg))
	    return 1;

	if (!strcmp(id, type))
	    return 0;

	/* Return error flag if this is not the desired subchunk type: */
	if (sflg)
	    return 1;
		
	/* Skip over data in chunk: */
	if (fseek(f, *size, SEEK_CUR)==-1)
	    return 1;
    }
    return 0;
}


/* Swaping bytes for BigEndian architectures */

int fswap(void *_buffer, size_t size, size_t count)  {
    unsigned int i,j;
    char tmp;
    char *h, *l, *buffer = (char *) _buffer;

    if (size <= 1)
	return 0;

    for (i = 0; i < count ; ++i) {
	l = buffer;
	h = l + size -1;
	for (j = 0; j < size/2; ++j) {
	    tmp = *l;
	    *l++ = *h;
	    *h-- = tmp;
	}
	buffer += size;
    }
    return 0;
}

size_t fread_swap(void *buffer, size_t size, size_t count, FILE *stream) {
    size_t retv = fread(buffer, size, count, stream);
    if (size > 1 && retv > 0) 
	fswap(buffer, size, retv);
    return retv;
}

size_t fwrite_swap(const void *_buffer, size_t size, size_t count, FILE *stream) {
    size_t retv;
    void *buffer = (void *) _buffer;
    if (size > 1) {
	fswap(buffer, size, count); 
	retv = fwrite(buffer, size, count, stream);
	/* not change original buffer */
	fswap(buffer, size, count); 
    } else
	retv = fwrite(buffer, size, count, stream);

    return retv;
}

