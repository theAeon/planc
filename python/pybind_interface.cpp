//
// Created by andrew on 6/19/2024.
//

#include <carma>
#include "config.h"
#include <nmf_lib.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/pytypes.h>

namespace py = pybind11;

//Rcpp::List nmf(const SEXP& x, const arma::uword &k, const arma::uword &niter = 30,
//               const std::string &algo = "anlsbpp",
//               const int& nCores = 2,
//               const Rcpp::Nullable<Rcpp::NumericMatrix> &Winit = R_NilValue,
//               const Rcpp::Nullable<Rcpp::NumericMatrix> &Hinit = R_NilValue) {
//    Rcpp::List outlist;
//    try {
//        if (Rf_isS4(x)) {
//            // Assume using dgCMatrix
//            outlist = runNMF<arma::sp_mat>(Rcpp::as<arma::sp_mat>(x), k, algo, niter, nCores, Winit, Hinit);
//        } else {
//            // Assume regular dense matrix
//            outlist = runNMF<arma::mat>(Rcpp::as<arma::mat>(x), k, algo, niter, nCores, Winit, Hinit);
//        }
//    } catch (const std::exception &e) {
//        throw Rcpp::exception(e.what());
//    }
//    return outlist;
//}

arma::mat nullMat = arma::mat{};

template <typename T>
using nmfCall = py::dict(*)(const T&, const arma::uword&, const arma::uword&, const std::string&, const int&, const arma::mat&, const arma::mat&);


// T2 e.g. arma::sp_mat
template <typename T2>
py::dict nmf(const T2& x, const arma::uword &k,
             const arma::uword& niter = 30, const std::string& algo = "anlsbpp",
             const int& nCores = 2, const arma::mat& Winit = nullMat, const arma::mat& Hinit = nullMat) {
    planc::nmfOutput libcall = planc::nmflib<T2>::nmf(x, k, niter, algo, nCores, Winit, Hinit);
    using namespace py::literals;
    py::dict returnNMF("W"_a=libcall.outW, "H"_a=libcall.outH, "objErr"_a=libcall.objErr);
    return returnNMF;
}

template <typename T2>
py::dict nmf(const T2& x, const arma::uword &k,
             const arma::uword& niter = 30, const std::string& algo = "anlsbpp",
             const int& nCores = 2) {
    planc::nmfOutput libcall = planc::nmflib<T2>::nmf(x, k, niter, algo, nCores);
    using namespace py::literals;
    py::dict returnNMF("W"_a=libcall.outW, "H"_a=libcall.outH, "objErr"_a=libcall.objErr);
    return returnNMF;
}

PYBIND11_MODULE(pyplanc, m) {
    m.doc() = "A python wrapper for planc-nmflib";
    using namespace py::literals;
    m.def("nmf", static_cast<nmfCall<arma::mat>>(nmf), "A function that calls NMF with the given arguments", "x"_a, "k"_a, "niter"_a=30, "algo"_a="anlsbpp", "ncores"_a=2,
          py::kw_only(), "Winit"_a, "Hinit"_a);
}