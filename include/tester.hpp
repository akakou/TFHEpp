#pragma once

#include <cassert>
#include <iostream>
#include <random>

#include "tfhe++.hpp"

namespace TFHEpp {

constexpr uint32_t num_test = 100;

class AbstructBootstrapTester {
public:
    int dist_max = 100;
    double permit_error = dist_max / 10;

    AbstructFunction *function;
    std::unique_ptr<TFHEpp::SecretKey> sk;
    uniform_int_distribution<> dist;
    default_random_engine engine;

    void init(AbstructFunction *function, random_device &seed_gen, int dist_max,
              double permit_error)
    {
        this->function = function;
        this->dist_max = dist_max;
        this->permit_error = permit_error;

        sk = std::make_unique<TFHEpp::SecretKey>();

        engine = default_random_engine(seed_gen());
        dist = uniform_int_distribution<>(-dist_max, dist_max);
    }

    bool assert_test(double expected, double res, bool output = false)
    {
        double diff = abs(expected - res);
        bool is_succeeded = diff < permit_error;

        if (!is_succeeded || output) {
            std::cerr << "----\nexpected: " << expected << "\nresult :" << res
                      << std::endl;

            std::cerr << diff << "(diff) > " << permit_error << "(permit)"
                      << std::endl;
        }
        return is_succeeded;
    }

    std::unique_ptr<TFHEpp::GateKey> gk(Encoder &encoder)
    {
        return std::make_unique<TFHEpp::GateKey>(*sk, encoder);
    }

    virtual bool test() = 0;
};
}  // namespace TFHEpp