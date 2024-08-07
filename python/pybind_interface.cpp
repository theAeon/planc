//
// Created by andrew on 6/19/2024.
//

#include "config.h"
#include <nmf_lib.hpp>
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include "dense_casters.h"
#include "sparse_casters.h"


namespace nb = nanobind;
// void limitpypythreads() {
//     nb::object scope = nb::module_::import_("__main__").attr("__dict__");
//     nb::exec(R"(
//     from threadpoolctl import ThreadpoolController
//     from platform import python_implementation
//     tpcontrol = ThreadpoolController()
//     platform = python_implementation()
//     if platform == "PyPy":
//         tpcontrol.limit(limits=1, user_api="blas")
//         tpcontrol.limit(limits=1, user_api="openmp")
//     )", scope
//     );
    // nb::callable ThreadpoolController = nb::module_::import_("threadpoolctl").attr("ThreadpoolController");
    // nb::object tpcontrol = ThreadpoolController();
    // nb::callable limit = tpcontrol.attr("limit");
    // nb::callable platformimpl = nb::module_::import_("platform").attr("python_implementation");
    // nb::str platform = platformimpl();
    // if (static_cast<std::string>(platform) == "PyPy") {
    //     using namespace nb::literals;
    //     limit("limits"_a=1, "user_api"_a="blas");
    //     limit("limits"_a=1, "user_api"_a="openmp");
    // }
// } // state does not unset, TODO find way to do on import/unload

NB_MODULE(pyplanc, m) {
    m.doc() = "A python wrapper for planc-nmflib";
    using namespace nb::literals;
    nb::class_<planc::nmfOutput<double>>(m, "nmfOutput").def_rw("W", &planc::nmfOutput<double>::outW).def_rw("H", &planc::nmfOutput<double>::outH).def_rw("objErr", &planc::nmfOutput<double>::objErr,  nb::rv_policy::move);
    m.def("nmf", &planc::nmflib<arma::mat>::nmf, "A function that calls NMF with the given arguments", "x"_a, "k"_a, "niter"_a=30, "algo"_a= "anlsbpp", "ncores"_a=2,
          nb::kw_only(), "Winit"_a = arma::mat(), "Hinit"_a = arma::mat());
    m.def("nmf", &planc::nmflib<arma::sp_mat>::nmf, "A function that calls NMF with the given arguments", "x"_a, "k"_a, "niter"_a=30, "algo"_a="anlsbpp", "ncores"_a=2,  nb::kw_only(), "Winit"_a = arma::mat(), "Hinit"_a = arma::mat());
    m.def("symNMF", &planc::nmflib<arma::sp_mat>::symNMF, "A function that calls symNMF with the given arguments", "x"_a, "k"_a, "niter"_a=30, "lambda"_a=0.0, "algo"_a="gnsym", "ncores"_a=2,  nb::kw_only(), "Hinit"_a = arma::mat());
    m.def("symNMF", &planc::nmflib<arma::mat>::symNMF, "A function that calls symNMF with the given arguments", "x"_a, "k"_a, "niter"_a=30, "lambda"_a=0.0, "algo"_a="gnsym", "ncores"_a=2,  nb::kw_only(), "Hinit"_a = arma::mat());

}
