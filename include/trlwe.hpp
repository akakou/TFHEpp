#pragma once

#include <array>

#include "./utils.hpp"

namespace TFHEpp {
using namespace std;

template <class P>
TRLWE<P> trlweSymEncryptZero(const double alpha, const Key<P> &key);

template <class P>
TRLWE<P> trlweSymEncrypt(const array<typename P::T, P::n> &p, const double alpha,
                         const Key<P> &key);

template <class P>
array<bool, P::n> trlweSymDecrypt(const TRLWE<P> &c, const Key<P> &key);

template <class P>
void SampleExtractIndex(TLWE<P> &tlwe, const TRLWE<P> &trlwe, const int index);
}  // namespace TFHEpp