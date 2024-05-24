//
// Created by andrew on 5/24/2024.
//

#ifndef PLANC_EMBEDDEDNMFDRIVER_H
#define PLANC_EMBEDDEDNMFDRIVER_H
#include "NMFDriver.hpp"

namespace planc {

    template <typename T>
    class EmbeddedNMFDriver : NMFDriver<T> {
    protected:
        void parseParams(const internalParams<T>& pc) {
            this->A = pc.getMAMat();
            this->Winit = pc.getMWInitMat();
            this->MInit = pc.getMHInitMat();
            this->m_outputfile_name = pc.getMOutputfileName();
            commonParams(pc);
        }
        void loadMat(double t2) {

        }
    public:
        explicit EmbeddedNMFDriver<T>(internalParams<T> pc)
        {
            this->parseParams(pc);
        }
    };



}
#endif //PLANC_EMBEDDEDNMFDRIVER_H