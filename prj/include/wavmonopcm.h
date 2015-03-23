/* Copyright (C) Antonio Bonafonte, 2012
 *               Universitat Politecnica de Catalunya, Barcelona, Spain.
 *
 * Permission to copy, use, modify, sell and distribute this software
 * is granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied
 * warranty, and with no claim as to its suitability for any purpose.
 */

#ifndef UPC_WAVMONOPCM
#define UPC_WAVMONOPCM

#include <vector>
#include <string>

/** 
    Read wav files which are mono and pcm with 16 bits. 
    The results are return in a vector of floats (values are not change, maxvalue = MAX_SHORT)
    The sampling rate is also returned.    

    The function returns 0 if there is not error.
**/

int wavmonopcm_read(const std::string& inputFileName, std::vector<float> &x, float &sampling_rate);

/** 
    Write mono wav files. The samles are written as pcm with 16 bits
    The sampling rate needs to be provided.

    If the outputFileName include a directory which does not exists, it is created.
**/
int wavmonopcm_write(const std::string& outputFileName, std::vector<float> const &x, float sampling_rate);

#endif
