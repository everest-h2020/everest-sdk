extern "C" {
#include "fpga.h"
}
#include <algorithm>
#include <string.h>
#include <iostream>
#include <vector>

//must be a multiple of 4
#define NP 100

#define P 11

#define SIZE_S (P * P)
#define SIZE_D (P * P * P)
#define SIZE_u (P * P * P)
#define SIZE_v (P * P * P)

double fRand()
{
    double fMin = -1;
    double fMax = 1;
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

int main(int argc, char *argv[]) {
    //may be better
    matrix_t S[NP];
    tensor4D_t D_inv[NP];
    tensor4D_t u[NP];
    tensor4D_t r[NP];

    for (int i = 0; i < NP; i++)
    {
        for (int e = 0; e < SIZE_S; e++)
        {
            S[i].flat[e] = fRand();
        }
        for (int e = 0; e < SIZE_D; e++)
        {
            D_inv[i].ultra_flat[e] = fRand();
        }
        for (int e = 0; e < SIZE_u; e++)
        {
            u[i].ultra_flat[e] = fRand();
        }
    }


    int check = fpga_condition();
    if (check == 1)
    {
        std::cout << "[HOST] run code!" << std::endl;
        fpga_pre_execution(S, D_inv, u, NP);
        fpga_launch();
        fpga_post_execution((tensor4D_t*&)r);
    }
    return 0;
}