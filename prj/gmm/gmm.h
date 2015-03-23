/* Copyright (C) Universitat Politècnica de Catalunya, Barcelona, Spain.
 *
 * Permission to copy, use, modify, sell and distribute this software
 * is granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied
 * warranty, and with no claim as to its suitability for any purpose.
 * antonio.bonafonte@upc.edu
 * Barcelona, November 2011
 */


/*
  This library implements EM to estimate the parameters of a GMM distribution
  from data.

  The algorithm starts with 1 mixture.
  Then:
  * Iterate till required number of mixtures it iterates:
  -split mixtures
  - Iterate til convergence (EM):
  + Expectation
  + Maximization

  There are functions to
  - write GMM to disk
  - read GMM from disk
  - compute prob. of data given the GMM
  ...
*/

#ifndef _GMM_H
#define _GMM_H

#include <vector>
#include <string>
#include <iostream>
#include "matrix.h"

namespace upc {
  class GMM {
    size_t vector_size; ///size of each input data (num. parameters)
    size_t nmix; ///number of mixtures in the GMM
    upc::fvector w; ///w[i]: weight of Mixture i
    upc::fmatrix mu, ///mu[i]: mean of Mixture i (vector)
      inv_sigma; ///inv_sigma[i]: inverse of sigma of mixture i (vector, assuming diagonal covariance)
    
    upc::fvector global_inv_sigma; ///used in vq, to define weighted euclidean distance

    /* Compute prob of an input vector */
    //    float gmm_prob(const float *x) const;
    /// Compute log(prob) of an input vector
    float gmm_logprob(const float *x) const;

  public:

    ///Define num. of mixtures and size of input samples (num. param.)
    void resize(size_t _nmix, size_t dim) {
      vector_size=dim; nmix=_nmix;
      w.resize(nmix);
      mu.resize(nmix, vector_size);
      inv_sigma.resize(nmix, vector_size);
    }

    /* Delete mixture k */
    void delete_mixture(unsigned int k);


    /* Estimate GMM with nmix mixtures from data. Apply splitting (centroid, 2, 4, ... nmix)
       Iterate till prob. does not improve more than prob_threshold or  max_it.
       verbose=1 => show iteration probaility in stdout
    */

    /* Create GMM with nmix mixture assigning randomly vectors to each mixture */
    int random_init(const upc::fmatrix &data, unsigned int nmix);

    /* Create GMM with nmix using vector quantizer dessign (Splitting + Lloyd) */
    int vq_lbg(const upc::fmatrix &data, unsigned int final_nmix, unsigned int max_it, float inc_threshold, int verbose);

    /* EM: reestimate GMM from data (given initial value of gmm)*/
    int em(const upc::fmatrix &data, unsigned int max_it, float inc_prob_threshold, int verbose);


    /* Starts with gmm size 1, then split gmm size 2 and apply em, then gmm size 4 and EM, etc. till arrive to gmm of size nmix */
    int em_split(const upc::fmatrix &data, unsigned int nmix, unsigned int max_it, float inc_prob_threshold, int verbose);

    /* Compute log10(prob)/n of the sequence of data; n: data.size() */
    float logprob(const upc::fmatrix &data) const;


    std::istream& read(std::istream &); ///read gmm from binary file
    std::ostream& write(std::ostream &) const; ///write gmm into binary file
    std::ostream& print(std::ostream &) const; //show gmm (text format)
    
  private:

    /* Intermediate steps in EM */
    float em_expectation(const upc::fmatrix &data, upc::fmatrix &weights) const;
    int em_maximization(const upc::fmatrix &data, const upc::fmatrix &weights);

    /* Create GMM with one mixture, centroid of data */
    int centroid(const upc::fmatrix &data);

    /* Split each mixture in two to go towards the target_size */
    int split(unsigned int target_size);

    /* Split a mixture into two close mixtures */
    void split_mixture(unsigned int src, unsigned int dest);

    /* Design vector quantizer from data */
    int vq_lloyd(const fmatrix &data, unsigned int max_it, float inc_threshold, int verbose);

    /* Intermediate steps: vq_expectation and vq_maxization */
    int vq_maximization(const fmatrix &data, const ivector &indexes, int update_variances);
    float vq_expectation(const fmatrix &data, ivector &indexes) const;

    /* Quantize input data: find the closest codevector (mixture) to input vector x and save in index */
    float vq(const float *x, int &index) const;

    /* Distance to compare input data and codevectors */
    float vq_distance(const float *x, const float *y) const;

    /* Split each codevector in two to go towards the target_size */
    int vq_split(unsigned int target_size);

    /* Split one codevector in two close ones */
    void vq_split_centroid(unsigned int n_src, unsigned int n_dest);

  };
}

///Define operator << and >> to write/read gmm to/from binary files
inline std::ostream& operator<<(std::ostream& os, const upc::GMM &gmm) {return gmm.write(os);} 
inline std::istream& operator>>(std::istream& is, upc::GMM &gmm) {return gmm.read(is);}

#endif
