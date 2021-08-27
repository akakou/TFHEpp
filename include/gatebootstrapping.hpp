#pragma once

#include <bits/stdint-uintn.h>

#include <cmath>
#include <limits>

#include "cloudkey.hpp"
#include "detwfa.hpp"
#include "trlwe.hpp"
#include "utils.hpp"

namespace TFHEpp {

template <class P, uint32_t num_out = 1>
void BlindRotate(TRLWE<typename P::targetP> &res,
                 const TLWE<typename P::domainP> &tlwe,
                 const BootstrappingKeyFFT<P> &bkfft,
                 const Polynomial<typename P::targetP> &testvector)
{
    constexpr uint32_t bitwidth = bits_needed<num_out - 1>();
    uint32_t bara =
        2 * P::targetP::n -
        modSwitchFromTorus<typename P::targetP, bitwidth>(tlwe[P::domainP::n]);
    res[0] = {};
    PolynomialMulByXai<typename P::targetP>(res[1], testvector, bara);
    for (int i = 0; i < P::domainP::n; i++) {
        bara = modSwitchFromTorus<typename P::targetP, bitwidth>(tlwe[i]);
        if (bara == 0) continue;
        // Do not use CMUXFFT to avoid unnecessary copy.
        CMUXFFTwithPolynomialMulByXaiMinusOne<typename P::targetP>(
            res, bkfft[i], bara);
    }
}

template <class P, uint32_t num_out = 1>
void BlindRotate(TRLWE<typename P::targetP> &res,
                 const TLWE<typename P::domainP> &tlwe,
                 const BootstrappingKeyFFT<P> &bkfft,
                 const TRLWE<typename P::targetP> &testvector)
{
    constexpr uint32_t bitwidth = bits_needed<num_out - 1>();
    constexpr typename P::domainP::T flooroffset =
        1ULL << (std::numeric_limits<typename P::domainP::T>::digit -
                 2 * P::targetP::T);  // 1/4N
    uint32_t bara =
        2 * P::targetP::n - modSwitchFromTorus<typename P::targetP, bitwidth>(
                                tlwe[P::domainP::n] - flooroffset);
    PolynomialMulByXai<typename P::targetP>(res[0], testvector[0], bara);
    PolynomialMulByXai<typename P::targetP>(res[1], testvector[1], bara);
    for (int i = 0; i < P::domainP::n; i++) {
        bara = modSwitchFromTorus<typename P::targetP, bitwidth>(tlwe[i]);
        if (bara == 0) continue;
        // Do not use CMUXFFT to avoid unnecessary copy.
        CMUXFFTwithPolynomialMulByXaiMinusOne<typename P::targetP>(
            res, bkfft[i], bara);
    }
}

template <class P>
void ProgrammableBootstrappingTLWE2TRLWEFFTWITHKEY(
    TRLWE<typename P::targetP> &acc, const TLWE<typename P::domainP> &tlwe,
    const BootstrappingKeyFFT<P> &bkfft, Encoder &encoder, Key<lvl0param> sk);
// template <class P>
// void ProgrammableBootstrappingTLWE2TRLWEFFTNegative(TRLWE<typename
// P::targetP> &acc,
//                                    const TLWE<typename P::domainP> &tlwe,
//                                    const BootstrappingKeyFFT<P> &bkfft,
//                                    Encoder &encoder_domain, Encoder
//                                    &encoder_target);

template <class P>
void ProgrammableBootstrappingTLWE2TRLWEFFT(
    TRLWE<typename P::targetP> &acc, const TLWE<typename P::domainP> &tlwe,
    const BootstrappingKeyFFT<P> &bkfft, Encoder &encoder_domain,
    Encoder &encoder_target);

template <class P>
void ProgrammableBootstrappingTLWE2TLWEFFT(
    TLWE<typename P::targetP> &res, const TLWE<typename P::domainP> &tlwe,
    const BootstrappingKeyFFT<P> &bkfft, Encoder &encoder_domain,
    Encoder &encoder_target);

template <class P>
void ProgrammableBootstrappingTLWE2TLWEFFTDEBUG(
    TLWE<typename P::targetP> &res, const TLWE<typename P::domainP> &tlwe,
    const BootstrappingKeyFFT<P> &bkfft, Encoder &encoder_domain,
    Encoder &encoder_target);

template <class P>
void GateBootstrappingTLWE2TRLWEFFT(TRLWE<typename P::targetP> &acc,
                                    const TLWE<typename P::domainP> &tlwe,
                                    const BootstrappingKeyFFT<P> &bkfft);

template <class P>
void GateBootstrappingTLWE2TLWEFFT(TLWE<typename P::targetP> &res,
                                   const TLWE<typename P::domainP> &tlwe,
                                   const BootstrappingKeyFFT<P> &bkfft);

template <class P, uint32_t num_out>
void GateBootstrappingManyLUT(
    std::array<TLWE<typename P::targetP>, num_out> &res,
    const TLWE<typename P::domainP> &tlwe, const BootstrappingKeyFFT<P> &bkfft,
    const Polynomial<typename P::targetP> &testvector)
{
    TRLWE<typename P::targetP> acc;
    BlindRotate<P, num_out>(acc, tlwe, bkfft, testvector);
    for (int i = 0; i < num_out; i++)
        SampleExtractIndex<typename P::targetP>(res[i], acc, i);
}

template <class P>
void GateBootstrappingTLWE2TLWEFFTvariableMu(
    TLWE<typename P::targetP> &res, const TLWE<typename P::domainP> &tlwe,
    const BootstrappingKeyFFT<P> &bkfft, const typename P::targetP::T mus2);

void ProgrammableBootstrapping(TLWE<lvl0param> &res,
                               const TLWE<lvl0param> &tlwe, const GateKey &gk,
                               Encoder &encoder_domain, Encoder &encoder_target,
                               AbstructFunction &function);

void ProgrammableBootstrapping(TLWE<lvl0param> &res,
                               const TLWE<lvl0param> &tlwe, const GateKey &gk,
                               Encoder &encoder_domain, Encoder &encoder_target,
                               AbstructFunction &function);

void ProgrammableBootstrappingWithoutKS(TLWE<lvl1param> &res,
                                        const TLWE<lvl0param> &tlwe,
                                        const GateKey &gk,
                                        Encoder &encoder_domain,
                                        Encoder &encoder_target,
                                        AbstructFunction &function);

// void ProgrammableBootstrappingWithoutSE(TRLWE<lvl1param> &res, const
// TLWE<lvl0param> &tlwe,
//                       const GateKey &gk, Encoder &encoder_domain, Encoder
//                       &encoder_target);

// void ProgrammableBootstrappingWithoutSEWITHKEY(TRLWE<lvl1param> &res, const
// TLWE<lvl0param> &tlwe,
//                       const GateKey &gk, Encoder &encoder_domain, Encoder
//                       &encoder_target, Key<lvl0param> sk);

void GateBootstrapping(TLWE<lvl0param> &res, const TLWE<lvl0param> &tlwe,
                       const GateKey &gk);
template <class P>
void ProgrammableBootstrappingTLWE2TRLWEFFT(
    TRLWE<typename P::targetP> &acc, const TLWE<typename P::domainP> &tlwe,
    const BootstrappingKeyFFT<P> &bkfft, Encoder &encoder_domain,
    Encoder &encoder_target);
}  // namespace TFHEpp