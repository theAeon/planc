#pragma once

#ifdef _OPENMP
#include <omp.h>
#endif
#ifdef USING_R
#include <Progress.hpp>
#else
#include "progressWrapper.h"
#endif
#include "bppnnls.hpp"
#include "inmf.hpp"

namespace planc {

template <typename T>
class BPPINMF : public INMF<T> {
private:
    arma::mat giventGiven;

    void solveH(const int& ncores) {
        tic();
#ifdef _VERBOSE
        Rcpp::Rcout << "--Solving H--  ";
#endif
        arma::mat* Wptr = this->W.get();
        arma::mat given(this->m, this->k);
        for (arma::uword i=0; i<this->nDatasets; ++i) {
            arma::mat* Vptr = this->Vi[i].get();
            arma::mat* Hptr = this->Hi[i].get();
            T* Eptr = this->Ei[i].get();
            given = *Wptr + *Vptr;
            giventGiven = given.t() * given;
            giventGiven += (*Vptr).t() * (*Vptr) * this->lambda;
            // giventInput = given.t() * (*Eptr);
            int dataSize = this->ncol_E[i];
            int numChunks = dataSize / this->INMF_CHUNK_SIZE;
            if (numChunks * this->INMF_CHUNK_SIZE < dataSize) numChunks++;
#pragma omp parallel for schedule(dynamic) default(none) shared(dataSize, Hptr, Eptr, given, numChunks) num_threads(ncores)
            for (int j = 0; j < numChunks; ++j) {
                int spanStart = j * this->INMF_CHUNK_SIZE;
                int spanEnd = (j + 1) * this->INMF_CHUNK_SIZE - 1;
                if (spanEnd > dataSize - 1) spanEnd = dataSize - 1;
                arma::mat giventInput = given.t() * (*Eptr).cols(spanStart, spanEnd);
                BPPNNLS<arma::mat, arma::vec> subProbH(giventGiven, giventInput, true);
                subProbH.solveNNLS();
                (*Hptr).rows(spanStart, spanEnd) = subProbH.getSolutionMatrix().t();
                giventInput.clear();
            }
        }
        giventGiven.clear();
#ifdef _VERBOSE
        Rcpp::Rcout << toc() << " sec" << std::endl;
#endif
    }

    void solveV(const int& ncores) {
        tic();
#ifdef _VERBOSE
        Rcpp::Rcout << "--Solving V--  ";
#endif
        arma::mat* WTptr = this->WT.get();
        arma::mat giventInput(this->k, this->INMF_CHUNK_SIZE);
        for (arma::uword i=0; i<this->nDatasets; ++i) {
            arma::mat* Hptr = this->Hi[i].get();\
            giventGiven = (*Hptr).t() * (*Hptr);
            giventGiven *= 1 + this->lambda;
            arma::mat* Vptr = this->Vi[i].get();
            arma::mat* VTptr = this->ViT[i].get();
            T* ETptr = this->EiT[i].get();
            int numChunks = this->m / this->INMF_CHUNK_SIZE;
            if (numChunks * this->INMF_CHUNK_SIZE < this->m) numChunks++;
#pragma omp parallel for schedule(dynamic) default(none) shared(numChunks, WTptr, Hptr, Vptr, ETptr, VTptr, giventInput) num_threads(ncores)
            for (int j = 0; j < numChunks; ++j) {

                int spanStart = j * this->INMF_CHUNK_SIZE;
                int spanEnd = (j + 1) * this->INMF_CHUNK_SIZE - 1;
                if (spanEnd > this->m - 1) spanEnd = this->m - 1;
                arma::mat giventInputTLS;
                giventInputTLS = (*Hptr).t() * (*ETptr).cols(spanStart, spanEnd);
                giventInputTLS -= (*Hptr).t() * *Hptr * (*WTptr).cols(spanStart, spanEnd);
                BPPNNLS<arma::mat, arma::vec> subProbV(giventGiven, giventInputTLS, true);
                subProbV.solveNNLS();
                (*Vptr).rows(spanStart, spanEnd) = subProbV.getSolutionMatrix().t();
                (*VTptr).cols(spanStart, spanEnd) = subProbV.getSolutionMatrix();
            }
        }
        giventGiven.clear();
        giventInput.clear();
#ifdef _VERBOSE
        Rcpp::Rcout << toc() << " sec" << std::endl;
#endif
    }

    void solveW(const int& ncores) {
        tic();
#ifdef _VERBOSE
        Rcpp::Rcout << "--Solving W--  ";
#endif
        arma::mat* Wptr = this->W.get();
        arma::mat* WTptr = this->WT.get();
        giventGiven = arma::zeros<arma::mat>(this->k, this->k); ///
        arma::mat giventInput;
        for (unsigned int j = 0; j < this->nDatasets; ++j) {
            arma::mat* Hptr = this->Hi[j].get();
            giventGiven += (*Hptr).t() * (*Hptr);
        }

        int numChunks = this->m / this->INMF_CHUNK_SIZE;
        if (numChunks * this->INMF_CHUNK_SIZE < this->m) numChunks++;
        for (int i = 0; i < numChunks; ++i) {
            int spanStart = i * this->INMF_CHUNK_SIZE;
            int spanEnd = (i + 1) * this->INMF_CHUNK_SIZE - 1;
            if (spanEnd > this->m - 1) spanEnd = this->m - 1;
            giventInput = arma::zeros<arma::mat>(this->k, spanEnd - spanStart + 1); ///
            #pragma omp parallel for ordered schedule(dynamic) default(none) shared(spanStart, spanEnd, giventInput) num_threads(ncores)
            for (int j = 0; j < this->nDatasets; ++j) {
                T* ETptr = this->EiT[j].get();
                arma::mat* Hptr = this->Hi[j].get();
                arma::mat* VTptr = this->ViT[j].get();
                arma::mat gtIpos = (*Hptr).t() * (*ETptr).cols(spanStart, spanEnd);
                arma::mat gtIneg = (*Hptr).t() * *Hptr * (*VTptr).cols(spanStart, spanEnd);
                #pragma omp ordered
                {
                giventInput += gtIpos;
                giventInput -= gtIneg;
                }
            }
            BPPNNLS<arma::mat, arma::vec> subProbW(giventGiven, giventInput, true); ///
            subProbW.solveNNLS();
            (*Wptr).rows(spanStart, spanEnd) = subProbW.getSolutionMatrix().t();
            (*WTptr).cols(spanStart, spanEnd) = subProbW.getSolutionMatrix();
        }
        giventGiven.clear(); ///
        giventInput.clear(); ///
        *WTptr = (*Wptr).t();
#ifdef _VERBOSE
        Rcpp::Rcout << toc() << " sec" << std::endl;
#endif
    }

public:
    BPPINMF(std::vector<std::shared_ptr<T>>& Ei, arma::uword k, double lambda) : INMF<T>(Ei, k, lambda) {
    }
    BPPINMF(std::vector<std::shared_ptr<T>>& Ei, arma::uword k, double lambda, std::vector<arma::mat> HinitList, std::vector<arma::mat> VinitList, arma::mat Winit) : INMF<T>(Ei, k, lambda, HinitList, VinitList, Winit) {
    }

    void optimizeALS(unsigned int niter, bool verbose = true, const int& ncores = 0) {
        // execute private functions here
        if (verbose) {
#ifdef USING_R
            Rcpp::Rcerr
#else
            std::cerr
#endif
            << "INMF started, niter=" << niter << std::endl;
        }
        this->objective_err = this->computeObjectiveError();
        auto start = std::chrono::high_resolution_clock::now();
        unsigned int iter = 0;
        Progress p(niter, verbose);
        while (iter < niter ) {
#ifdef USING_R
            Rcpp::checkUserInterrupt();
#endif
            solveH(ncores);
            solveV(ncores);
            solveW(ncores);
            iter++;
#ifdef USING_R
            if ( ! p.is_aborted() )
#endif
                p.increment();
#ifdef USING_R
            else break;
#endif
        }
        this->objective_err = this->computeObjectiveError();
        auto end = std::chrono::high_resolution_clock::now();
        if (verbose) {
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
#ifdef USING_R
            Rcpp::Rcerr
#else
            std::cerr
#endif
            << "Total time:      " << duration.count() << " sec" << std::endl;
#ifdef USING_R
            Rcpp::Rcerr
#else
            std::cerr
#endif
            << "Objective error: " << this->objective_err << std::endl;
        }
    }
};

}
