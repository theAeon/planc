#pragma once
/* Copyright Ramakrishnan Kannan 2018 */

#include "distntf/distauntf.hpp"
#include "nnls/bppnnls.hpp"

namespace planc {

#define ONE_THREAD_MATRIX_SIZE 2000

class DistNTFANLSBPP : public DistAUNTF {
 protected:
  /**
   * ANLS/BPP update function.
   * Given the MTTKRP and the hadamard of all the grams, we
   * determine the factor matrix to be updated.
   * @param[in] Mode of the factor to be updated
   * @returns The new updated factor
   */
  MAT update(const int mode) {
    MAT othermat(this->m_local_ncp_factors_t.factor(mode));
    if (m_nls_sizes[mode] > 0) {
      unsigned int nrhs = this->ncp_local_mttkrp_t[mode].n_cols;
      unsigned int numChunks = nrhs / ONE_THREAD_MATRIX_SIZE;
      if (numChunks * ONE_THREAD_MATRIX_SIZE < nrhs) numChunks++;
      // #pragma omp parallel for schedule(dynamic)
      for (unsigned int i = 0; i < numChunks; i++) {
        unsigned int spanStart = i * ONE_THREAD_MATRIX_SIZE;
        unsigned int spanEnd = (i + 1) * ONE_THREAD_MATRIX_SIZE - 1;
        if (spanEnd > nrhs - 1) {
          spanEnd = nrhs - 1;
        }
        BPPNNLS<MAT, arma::vec> subProblem(this->global_gram,
                  (MAT)this->ncp_local_mttkrp_t[mode].cols(spanStart, spanEnd),
                  true);
#ifdef _VERBOSE
      // #pragma omp critical
        {
          INFO << "Scheduling " << worh << " start=" << spanStart
             << ", end=" << spanEnd
             // << ", tid=" << omp_get_thread_num()
             << std::endl
             << "LHS ::" << std::endl
             << giventGiven << std::endl
             << "RHS ::" << std::endl
             << this->ncp_local_mttkrp_t[mode].cols(spanStart, spanEnd)
             << std::endl;
        }
#endif

        subProblem.solveNNLS();

#ifdef _VERBOSE
        INFO << "completed " << worh << " start=" << spanStart
           << ", end=" << spanEnd
           // << ", tid=" << omp_get_thread_num() << " cpu=" << sched_getcpu()
           << " time taken=" << t2 << std::endl;
#endif
        othermat.cols(spanStart, spanEnd) = subProblem.getSolutionMatrix();
      }
    } else {
      othermat.zeros();
    }
    return othermat;
  }

 public:
  DistNTFANLSBPP(const Tensor &i_tensor, const int i_k, algotype i_algo,
                 const arma::uvec &i_global_dims, const arma::uvec &i_local_dims,
                 const arma::uvec &i_nls_sizes, const arma::uvec &i_nls_idxs,
                 const NTFMPICommunicator &i_mpicomm)
      : DistAUNTF(i_tensor, i_k, i_algo, i_global_dims, i_local_dims,
                  i_nls_sizes, i_nls_idxs, i_mpicomm) {}
};  // class DistNTFANLSBPP

}  // namespace planc
