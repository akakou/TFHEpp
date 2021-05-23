#pragma once

#include <bits/stdint-uintn.h>
#include <cstdint>
#include <array>
#include <cassert>

namespace cuHEpp{
    template <typename T>
    constexpr bool false_v = false;

    constexpr uint64_t P = (((1ULL<<32)-1)<<32)+ 1;

    // this class defines operations over integaer torus.
    class INTorus{
        public:
            uint64_t value;
            INTorus(){value=0;}
            INTorus(uint64_t data,bool modulo = true){
                if(modulo) value = data + static_cast<uint32_t>(-(data >= P));
                else value = data;
            }

            INTorus& operator=(const INTorus &a) { this->value = a.value; return *this; }

            // return this + b mod P.
            INTorus operator +(const INTorus &b) const{
                uint64_t tmp = this->value + b.value;
                return INTorus(tmp + static_cast<uint32_t>(-(tmp < b.value || tmp >= P)),false);
            }

            INTorus& operator += (const INTorus &b){
                this->value += b.value;
                *this = INTorus(this->value + static_cast<uint32_t>(-(this->value < b.value || this->value >= P)),false);
                return *this;
            }
            
            // return this - b mod P.
            INTorus operator -(const INTorus &b) const{
                uint64_t tmp = this->value - b.value;
                return INTorus(tmp - static_cast<uint32_t>(-(tmp>(this->value))),false);
            }

            INTorus operator -= (const INTorus &b){
                INTorus tmp = *this-b;
                *this = tmp;
                return *this;
            }

            INTorus operator * (const INTorus &b) const{
                __uint128_t tmp = static_cast<__uint128_t>(this->value) * b.value;
                uint32_t* tmpa = reinterpret_cast<uint32_t*>(&tmp);
                uint64_t res = ((static_cast<uint64_t>(tmpa[1])+tmpa[2])<<32) + tmpa[0] - tmpa[3] - tmpa[2];
                uint64_t lo = static_cast<uint64_t>(tmp);
                res -= static_cast<uint32_t>(-((res>lo)&&(tmpa[2]==0)));
                res += static_cast<uint32_t>(-((res<lo)&&(tmpa[2]!=0)));
                return INTorus(res);
            }

            INTorus operator *= (const INTorus &b){
                INTorus tmp = *this * b;
                *this = tmp;
                return *this;
            }

            INTorus operator << (uint32_t l) const{
                if(l==0){
                    return *this;
                }
                //t[0] = templ,t[1] = tempul, t[2] = tempuu
                else if(l<32){
                    uint64_t templ,tempu,res;
                    templ = this->value<<l;
                    tempu = this->value>>(64-l);
                    res = templ+(tempu<<32)-tempu;
                    res += static_cast<uint32_t>(-(res<templ));// tempuu is always 0.
                    return INTorus(res);
                }
                else if(l==32){
                    uint64_t templ,tempul,tempuu,res;
                    templ = this->value<<l;
                    tempul = static_cast<uint32_t>(this->value>>(64-l));
                    tempuu = 0;
                    res = templ+(tempul<<32)-tempuu-tempul;
                    res -= static_cast<uint32_t>(-((res>templ)&&(tempul==0)));
                    res += static_cast<uint32_t>(-((res<templ)&&(tempul!=0)));
                    return INTorus(res);
                }
                else if(l<64){
                    uint64_t templ,tempul,tempuu,res;
                    templ = static_cast<uint32_t>(this->value<<(l-32));
                    tempul = static_cast<uint32_t>(this->value>>(64-l));
                    tempuu = this->value>>(96-l);
                    res = ((templ+tempul)<<32)-tempuu-tempul;
                    res -= static_cast<uint32_t>(-((res>(templ<<32))&&(tempul==0)));
                    res += static_cast<uint32_t>(-((res<(templ<<32))&&(tempul!=0)));
                    return INTorus(res);
                }
                // Above are almsot equivalent
                //cuFHE seems to be wrong in Lsh96 and Lsh128. Sign is wrong.
                //mod P is not needed
                else if(l==64){
                    uint64_t templ,tempu,res;
                    templ = static_cast<uint32_t>(this->value);
                    templ = (templ<<32) - templ;
                    // templ += static_cast<uint32_t>(-(templ >= P));//mod P
                    tempu = this->value>>(96-l);
                    res = templ-tempu;
                    res -= static_cast<uint32_t>(-(res > (templ)));
                    return INTorus(res);
                }
                else if(l<96){
                    uint64_t templ,tempu,res;
                    templ = static_cast<uint32_t>(this->value<<(l-64));
                    templ = (templ<<32) - templ;
                    // templ += static_cast<uint32_t>(-(templ >= P)); //mod P
                    tempu = this->value>>(96-l);
                    res = templ-tempu;
                    res -= static_cast<uint32_t>(-(res > (templ)));
                    return INTorus(res);
                }
                else if(l==96){
                    uint64_t templ,tempu,res;
                    templ = P-(this->value);
                    tempu = 0;
                    res = tempu + templ;
                    res += static_cast<uint32_t>(-(res < tempu));
                    return INTorus(res);
                }
                else if(l<128){
                    uint64_t templ,tempu,res;
                    templ = P-(this->value<<(l-96));
                    tempu = this->value>>(160-l);
                    tempu = P-(tempu<<32)+tempu;
                    // tempu += static_cast<uint32_t>(-(tempu >= P)); //mod P
                    res = templ+tempu;
                    res += static_cast<uint32_t>(-(res < tempu));
                    return INTorus(res);
                }
                else if(l==128){
                    uint64_t templ,tempul,tempuu;
                    INTorus res;
                    templ = static_cast<uint32_t>(this->value);
                    tempul = static_cast<uint32_t>(this->value>>(160-l));
                    tempuu = 0;
                    // res = -((templ+tempul)<<32)+tempul-tempuu;
                    res = INTorus(tempul,false)-INTorus(templ<<32,false)-INTorus(tempul<<32,false);//-INTorus(tempuu,false);
                    return res;
                }
                else if(l<160){
                    uint64_t templul,templ,tempul,tempuu;
                    INTorus res;
                    templ = static_cast<uint32_t>(this->value<<(l-128));
                    tempul = static_cast<uint32_t>(this->value>>(160-l));
                    tempuu = this->value>>(192-l);
                    // res = -((templ+tempul)<<32)+tempul-tempuu;
                    res = INTorus(tempul+tempuu,false)-INTorus(templ<<32,false)-INTorus((tempul<<32),false);
                    return res;
                }
                else if(l==160){
                    uint64_t templ,tempu;
                    INTorus res;
                    templ = static_cast<uint32_t>(this->value);
                    tempu = this->value>>(192-l);
                    res = INTorus(templ+tempu,false)-INTorus(templ<<32,false);
                    return res;
                }
                else{
                    uint64_t templ,tempu;
                    INTorus res;
                    templ = static_cast<uint32_t>(this->value)<<(l-160);
                    tempu = this->value>>(192-l);
                    res = INTorus(templ+tempu,false)-INTorus(templ<<32,false);
                    return res;
                }
            }

            INTorus Pow(uint64_t e) const{
                INTorus res(1,false);
                for(uint64_t i = 0;i<e;i++) res *=  *this;
                return res;
            }
    };

    //defined on [1,31]
    INTorus InvPow2(uint8_t nbit){
        uint32_t low,high;
        low = (1 << (32 - nbit)) + 1;
        high = -low;
        return INTorus((static_cast<uint64_t>(high)<<32)+low);
    }

    // NTT implementation
    // https://nufhe.readthedocs.io/en/latest/implementation_details.html
    constexpr uint64_t W = 12037493425763644479ULL;

    template<uint32_t Nbit>
    inline std::array<std::array<INTorus,1U<<Nbit>,2> TwistGen(){
        constexpr uint32_t N = 1U<<Nbit;

        std::array<std::array<INTorus,1U<<Nbit>,2> twist;
        const INTorus w = INTorus(W).Pow(1U<<(32-Nbit-1));
        twist[0][0] = twist[1][0] = INTorus(1,false);
        for(uint32_t i = 1;i < N;i++) twist[1][i] = twist[1][i-1]*w;
        assert((twist[1][N-1]*w).Pow(2).value == 1);
        twist[0][N-1] = twist[1][N-1]*w*w;
        for(uint32_t i = 2;i < N;i++) twist[0][N-i] = twist[0][N-i+1]*w;
        assert((twist[0][1]*w).value == 1);
        return twist;
    }

    template<uint32_t Nbit>
    inline std::array<std::array<INTorus,1U<<Nbit>,2> TableGen(){
        constexpr uint32_t N = 1U<<Nbit;

        std::array<std::array<INTorus,N>,2>table;
        const INTorus w = INTorus(W).Pow(1U<<(32-Nbit));
        table[0][0] = table[1][0] = INTorus(1,false);
        for(uint32_t i = 1;i<N;i++) table[1][i] = table[1][i-1]*w;
        for(uint32_t i = 1;i<N;i++) table[0][i] = table[1][N-i];
        return table;
    }

    template<typename T = uint32_t,uint32_t Nbit>
    inline void TwistMulInvert(std::array<INTorus,1<<Nbit> &res, const std::array<T,1<<Nbit> &a, const std::array<INTorus,1<<Nbit> &twist){
        constexpr uint32_t N = 1<<Nbit;
        for (int i = 0; i < N; i++) res[i] = INTorus(a[i],std::is_same_v<T,uint64_t>) * twist[i];
    }

    template<typename T = uint32_t,uint32_t Nbit>
    inline void TwistMulDirect(std::array<T,1<<Nbit> &res, const std::array<INTorus,1<<Nbit> &a, const std::array<INTorus,1<<Nbit> &twist){
        const INTorus invN = InvPow2(Nbit);
        constexpr uint32_t N = 1<<Nbit;
        for (int i = 0; i < N; i++) res[i] = static_cast<T>((a[i] * twist[i] * invN).value);
    }
    
    inline void ButterflyAdd(INTorus* const res, const uint32_t size){
        for(uint32_t index = 0; index < size/2;index++){
            const INTorus temp = res[index];
            res[index] += res[index+size/2];
            res[index+size/2] = temp - res[index+size/2];
        }
    }

    template<uint8_t radixbit>
    inline void INTTradixButterfly(INTorus* const res,const uint32_t size){
        static_assert(radixbit<=6, "radix 64 is the maximum!");
        if constexpr (radixbit != 0){
            const uint32_t block = size>>radixbit;
            ButterflyAdd(res, size);
            if constexpr (radixbit != 6) for(int i = 1; i<(1<<(radixbit - 1));i++){
                // for(int j = 0;j<block;j++)  res[i*block+j+size/2] = res[i*block+j+size/2]<<(3*(64-(i<<(6-radixbit))));
                for(int j = 0;j<block;j++)  res[i*block+j+size/2] = res[i*block+j+size/2]<<(3*(i<<(6-radixbit)));
            }
            else for(int i =  1; i<(1<<(radixbit - 1));i++) for(int j = 0;j<block;j++)  res[i*block+j+size/2] = res[i*block+j+size/2]<<(3*i);
            INTTradixButterfly<radixbit-1>(&res[0], size/2);
            INTTradixButterfly<radixbit-1>(&res[size/2], size/2);
        }
    }

    template<uint32_t Nbit>
    inline void TwiddleMul(INTorus* const res, const uint32_t size, const uint32_t stride, const std::array<INTorus,1<<Nbit> &table){
        for(uint32_t index = 1; index < size;index++) res[index] *= table[stride*index];
    }

    template<uint32_t Nbit, uint8_t radixbit>
    inline void INTTradix(INTorus* const res, const uint32_t size, const uint32_t num_block, const std::array<INTorus,1<<Nbit> &table){
        INTTradixButterfly<radixbit>(res,size);
        if constexpr (radixbit == 1) TwiddleMul<Nbit>(&res[size>>radixbit], size>>radixbit, num_block, table);
        else if constexpr (radixbit == 2){
            TwiddleMul<Nbit>(&res[size>>radixbit], size>>radixbit, 2*num_block, table);
            TwiddleMul<Nbit>(&res[2*(size>>radixbit)], size>>radixbit, num_block, table);
            TwiddleMul<Nbit>(&res[3*(size>>radixbit)], size>>radixbit, 3*num_block, table);
        }
        else static_assert(false_v<Nbit>(),"radix error");
    }

    template<uint32_t Nbit,uint8_t radixbit>
    inline void INTT(std::array<INTorus,1<<Nbit > &res, const std::array<INTorus,1<<Nbit> &table){
        constexpr uint32_t radix = 1U<<radixbit;
        uint8_t sizebit;
        for(sizebit = Nbit;sizebit>radixbit;sizebit -= radixbit){
            const uint32_t size = 1U<<sizebit;
            const uint32_t num_block  = 1U<<(Nbit-sizebit);
            for(uint32_t block = 0;block<num_block;block++){
                INTTradix<Nbit,radixbit>(&res[size*block],size,num_block,table);
            }
        }
        constexpr uint8_t remainder = ((Nbit-1)%radixbit)+1;
        const uint32_t size = 1U<<remainder;
        const uint32_t num_block = 1U<<(Nbit-remainder);
        for(uint32_t block = 0;block<num_block;block++){
            INTTradixButterfly<remainder>(&res[size*block],size);
        }
    }

    template<typename T, uint32_t Nbit>
    void TwistINTTlvl1(std::array<INTorus ,1<<Nbit> &res, const std::array<T,1<<Nbit> &a, const std::array<INTorus,1<<Nbit> &table, const std::array<INTorus,1<<Nbit> &twist){
        TwistMulInvert<T,Nbit>(res,a,twist);
        INTT<Nbit,2>(res,table);
    }

    template<uint32_t Nbit>
    void NTT(std::array<INTorus,1<<Nbit > &res, const std::array<INTorus,1<<Nbit> &table){
        constexpr uint8_t radixbit = 1;
        constexpr uint32_t radix = 1U<<radixbit;
        for(uint8_t sizebit = radixbit;sizebit<=Nbit;sizebit += radixbit){
            const uint32_t size = 1U<<sizebit;
            uint32_t num_block;
            if(sizebit!=Nbit)num_block  = 1U<<(Nbit-sizebit);
            else num_block = 1;
            for(uint32_t block = 0;block<num_block;block++){
                TwiddleMul<Nbit>(&res[size*block+size/2],size/2,num_block,table);
                ButterflyAdd(&res[size*block],size);
            }
        }
    }

    template<typename T, uint32_t Nbit>
    void TwistNTTlvl1(std::array<T,1<<Nbit> &res, std::array<INTorus,1<<Nbit> &a, const std::array<INTorus,1<<Nbit> &table, const std::array<INTorus,1<<Nbit> &twist){
        NTT<Nbit>(a,table);
        TwistMulDirect<T,Nbit>(res,a,twist);
    }

    template<typename T, uint32_t Nbit>
    void PolyMullvl1(std::array<T,1<<Nbit> &res, std::array<T,1<<Nbit> &a, std::array<T,1<<Nbit> &b, const std::array<std::array<INTorus,1<<Nbit>,2> &table, const std::array<std::array<INTorus,1<<Nbit>,2> &twist){
        std::array<INTorus,1<<Nbit> ntta,nttb;
        TwistINTTlvl1<T,Nbit>(ntta,a,table[1],twist[1]);
        TwistINTTlvl1<T,Nbit>(nttb,b,table[1],twist[1]);
        for(int i = 0;i<(1U<<Nbit);i++) ntta[i]*=nttb[i];
        TwistNTTlvl1<T,Nbit>(res,ntta,table[0],twist[0]);
    }
}