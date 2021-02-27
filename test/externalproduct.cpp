#include <cassert>
#include <iostream>
#include <random>
#include <tfhe++.hpp>

using namespace std;
using namespace TFHEpp;

int main()
{
    const uint32_t num_test = 1000;
    random_device seed_gen;
    default_random_engine engine(seed_gen());
    uniform_int_distribution<uint32_t> binary(0, 1);

    cout << "test p=1" << endl;

    cout << "lvl1" << endl;
    for (int test = 0; test < num_test; test++) {
        lweKey key;

        array<bool, lvl1param::n> p;
        for (bool &i : p) i = (binary(engine) > 0);
        Polynomial<lvl1param> pmu;
        for (int i = 0; i < lvl1param::n; i++) pmu[i] = p[i] ? lvl1param::μ : -lvl1param::μ;
        TRLWE<lvl1param> c = trlweSymEncryptlvl1(pmu, lvl1param::α, key.lvl1);

        TRGSWFFT<lvl1param> trgswfft = trgswfftSymEncryptlvl1(1, lvl1param::α, key.lvl1);
        trgswfftExternalProductlvl1(c, c, trgswfft);
        array<bool, lvl1param::n> p2 = trlweSymDecryptlvl1(c, key.lvl1);
        for (int i = 0; i < lvl1param::n; i++) assert(p[i] == p2[i]);
    }
    cout << "Passed" << endl;

    cout << "lvl2" << endl;
    for (int test = 0; test < num_test; test++) {
        lweKey key;

        array<bool, lvl2param::n> p;
        for (bool &i : p) i = (binary(engine) > 0);
        Polynomial<lvl2param> pmu;
        for (int i = 0; i < lvl2param::n; i++) pmu[i] = p[i] ? lvl2param::μ : -lvl2param::μ;
        TRLWE<lvl2param> c = trlweSymEncryptlvl2(pmu, lvl2param::α, key.lvl2);

        TRGSWFFT<lvl2param> trgswfft =
            trgswfftSymEncryptlvl2(1, lvl2param::α, key.lvl2);
        trgswfftExternalProductlvl2(c, c, trgswfft);
        array<bool, lvl2param::n> p2 = trlweSymDecryptlvl2(c, key.lvl2);
        for (int i = 0; i < lvl2param::n; i++) assert(p[i] == p2[i]);
    }
    cout << "Passed" << endl;

    cout << "test p=-1" << endl;

    cout << "lvl1" << endl;
    for (int test = 0; test < num_test; test++) {
        lweKey key;

        array<bool, lvl1param::n> p;
        for (bool &i : p) i = binary(engine) > 0;
        array<uint32_t, lvl1param::n> pmu;
        for (int i = 0; i < lvl1param::n; i++) pmu[i] = p[i] ? lvl1param::μ : -lvl1param::μ;
        TRLWE<lvl1param> c = trlweSymEncryptlvl1(pmu, lvl1param::α, key.lvl1);

        TRGSWFFT<lvl1param> trgswfft = trgswfftSymEncryptlvl1(-1, lvl1param::α, key.lvl1);
        trgswfftExternalProductlvl1(c, c, trgswfft);
        array<bool, lvl1param::n> p2 = trlweSymDecryptlvl1(c, key.lvl1);
        for (int i = 0; i < lvl1param::n; i++) assert(p[i] == !p2[i]);
    }
    cout << "Passed" << endl;

    cout << "lvl2" << endl;
    for (int test = 0; test < num_test; test++) {
        lweKey key;

        array<bool, lvl2param::n> p;
        for (bool &i : p) i = binary(engine) > 0;
        Polynomial<lvl2param> pmu;
        for (int i = 0; i < lvl2param::n; i++) pmu[i] = p[i] ? lvl2param::μ : -lvl2param::μ;
        TRLWE<lvl2param> c = trlweSymEncryptlvl2(pmu, lvl2param::α, key.lvl2);

        TRGSWFFT<lvl2param> trgswfft =
            trgswfftSymEncryptlvl2(-1, lvl2param::α, key.lvl2);
        trgswfftExternalProductlvl2(c, c, trgswfft);
        array<bool, lvl2param::n> p2 = trlweSymDecryptlvl2(c, key.lvl2);
        for (int i = 0; i < lvl2param::n; i++) assert(p[i] == !p2[i]);
    }
    cout << "Passed" << endl;
}