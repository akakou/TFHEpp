#include <cloudkey.hpp>
#include <detwfa.hpp>
#include <gatebootstrapping.hpp>
#include <keyswitch.hpp>
#include <mulfft.hpp>
#include <params.hpp>
#include <trgsw.hpp>
#include <trlwe.hpp>
#include <utils.hpp>

namespace TFHEpp {
using namespace std;

template <class P>
inline void RotatedTestVector(array<array<typename P::T, P::n>, 2> &testvector,
                              const uint32_t bara, const typename P::T mu)
{
    testvector[0] = {};
    if (bara < P::n) {
        for (int i = 0; i < bara; i++) testvector[1][i] = -mu;
        for (int i = bara; i < P::n; i++) testvector[1][i] = mu;
    }
    else {
        const typename P::T baraa = bara - P::n;
        for (int i = 0; i < baraa; i++) testvector[1][i] = mu;
        for (int i = baraa; i < P::n; i++) testvector[1][i] = -mu;
    }
}

template <class P>
inline void MyCustomTestVector(array<array<typename P::T, P::n>, 2> &testvector,
                              const uint32_t bara, const typename P::T mu)
{
    testvector[0] = {};
    if (bara < P::n) {
        for (int i = 0; i < bara; i++) testvector[1][i] = dtot32(0.5);
        for (int i = bara; i < P::n; i++) testvector[1][i] = dtot32(0.5);
    }
    else {
        const typename P::T baraa = bara - P::n;
        for (int i = 0; i < baraa; i++) testvector[1][i] = dtot32(0.5);
        for (int i = baraa; i < P::n; i++) testvector[1][i] = dtot32(0.5);
    }
}

template <class P>
void ProgrammableBootstrappingTLWE2TRLWEFFT(TRLWE<typename P::targetP> &acc,
                                    const TLWE<typename P::domainP> &tlwe,
                                    const BootstrappingKeyFFT<P> &bkfft)
{
    uint32_t bara = 2 * P::targetP::n - modSwitchFromTorus<typename P::targetP>(
                                            tlwe[P::domainP::n]);
    MyCustomTestVector<typename P::targetP>(acc, bara, P::targetP::mu);
    for (int i = 0; i < P::domainP::n; i++) {
        bara = modSwitchFromTorus<typename P::targetP>(tlwe[i]);
        if (bara == 0) continue;
        // Do not use CMUXFFT to avoid unnecessary copy.
        CMUXFFTwithPolynomialMulByXaiMinusOne<typename P::targetP>(
            acc, bkfft[i], bara);
    }
}
#define INST(P)                                      \
    template void ProgrammableBootstrappingTLWE2TRLWEFFT<P>( \
        TRLWE<typename P::targetP> & acc,            \
        const TLWE<typename P::domainP> &tlwe,       \
        const BootstrappingKeyFFT<P> &bkfft)
TFHEPP_EXPLICIT_INSTANTIATION_LVL01_02(INST);
#undef INST

template <class P>
void GateBootstrappingTLWE2TRLWEFFT(TRLWE<typename P::targetP> &acc,
                                    const TLWE<typename P::domainP> &tlwe,
                                    const BootstrappingKeyFFT<P> &bkfft)
{
    uint32_t bara = 2 * P::targetP::n - modSwitchFromTorus<typename P::targetP>(
                                            tlwe[P::domainP::n]);
    RotatedTestVector<typename P::targetP>(acc, bara, P::targetP::mu);
    for (int i = 0; i < P::domainP::n; i++) {
        bara = modSwitchFromTorus<typename P::targetP>(tlwe[i]);
        if (bara == 0) continue;
        // Do not use CMUXFFT to avoid unnecessary copy.
        CMUXFFTwithPolynomialMulByXaiMinusOne<typename P::targetP>(
            acc, bkfft[i], bara);
    }
}
#define INST(P)                                      \
    template void GateBootstrappingTLWE2TRLWEFFT<P>( \
        TRLWE<typename P::targetP> & acc,            \
        const TLWE<typename P::domainP> &tlwe,       \
        const BootstrappingKeyFFT<P> &bkfft)
TFHEPP_EXPLICIT_INSTANTIATION_LVL01_02(INST);
#undef INST

template <class P>
void ProgrammableBootstrappingTLWE2TLWEFFT(TLWE<typename P::targetP> &res,
                                   const TLWE<typename P::domainP> &tlwe,
                                   const BootstrappingKeyFFT<P> &bkfft)
{
    TRLWE<typename P::targetP> acc;
    ProgrammableBootstrappingTLWE2TRLWEFFT<P>(acc, tlwe, bkfft);
    SampleExtractIndex<typename P::targetP>(res, acc, 0);
}
#define INST(P)                                     \
    template void ProgrammableBootstrappingTLWE2TLWEFFT<P>( \
        TLWE<typename P::targetP> & res,            \
        const TLWE<typename P::domainP> &tlwe,      \
        const BootstrappingKeyFFT<P> &bkfft)
TFHEPP_EXPLICIT_INSTANTIATION_LVL01_02(INST);
#undef INST

template <class P>
void GateBootstrappingTLWE2TLWEFFT(TLWE<typename P::targetP> &res,
                                   const TLWE<typename P::domainP> &tlwe,
                                   const BootstrappingKeyFFT<P> &bkfft)
{
    TRLWE<typename P::targetP> acc;
    GateBootstrappingTLWE2TRLWEFFT<P>(acc, tlwe, bkfft);
    SampleExtractIndex<typename P::targetP>(res, acc, 0);
}
#define INST(P)                                     \
    template void GateBootstrappingTLWE2TLWEFFT<P>( \
        TLWE<typename P::targetP> & res,            \
        const TLWE<typename P::domainP> &tlwe,      \
        const BootstrappingKeyFFT<P> &bkfft)
TFHEPP_EXPLICIT_INSTANTIATION_LVL01_02(INST);
#undef INST

template <class P>
void GateBootstrappingTLWE2TLWEFFTvariableMu(
    TLWE<typename P::targetP> &res, const TLWE<typename P::domainP> &tlwe,
    const BootstrappingKeyFFT<P> &bkfft, const typename P::targetP::T mus2)
{
    TRLWE<typename P::targetP> acc, temp;
    uint32_t bara = 2 * P::targetP::n - modSwitchFromTorus<typename P::targetP>(
                                            tlwe[P::domainP::n]);
    RotatedTestVector<typename P::targetP>(acc, bara, mus2);
    for (int i = 0; i < P::domainP::n; i++) {
        bara = modSwitchFromTorus<typename P::targetP>(tlwe[i]);
        if (bara == 0) continue;
        // Do not use CMUXFFT to avoid unnecessary copy.
        CMUXFFTwithPolynomialMulByXaiMinusOne<typename P::targetP>(
            acc, bkfft[i], bara);
    }
    SampleExtractIndex<typename P::targetP>(res, acc, 0);
    res[P::targetP::n] += mus2;
}
#define INST(P)                                               \
    template void GateBootstrappingTLWE2TLWEFFTvariableMu<P>( \
        TLWE<typename P::targetP> & res,                      \
        const TLWE<typename P::domainP> &tlwe,                \
        const BootstrappingKeyFFT<P> &bkfft, const typename P::targetP::T mus2)
TFHEPP_EXPLICIT_INSTANTIATION_LVL01_02(INST);
#undef INST

void ProgrammableBootstrapping(TLWE<lvl0param> &res, const TLWE<lvl0param> &tlwe,
                       const GateKey &gk)
{
    TLWE<lvl1param> tlwelvl1;
    ProgrammableBootstrappingTLWE2TLWEFFT<lvl01param>(tlwelvl1, tlwe, gk.bkfftlvl01);
    IdentityKeySwitch<lvl10param>(res, tlwelvl1, gk.ksk);
}

void GateBootstrapping(TLWE<lvl0param> &res, const TLWE<lvl0param> &tlwe,
                       const GateKey &gk)
{
    TLWE<lvl1param> tlwelvl1;
    GateBootstrappingTLWE2TLWEFFT<lvl01param>(tlwelvl1, tlwe, gk.bkfftlvl01);
    IdentityKeySwitch<lvl10param>(res, tlwelvl1, gk.ksk);
}
}  // namespace TFHEpp