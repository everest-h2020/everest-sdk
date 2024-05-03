extern "C" {
#include "naive.h"
}
#include <cstring>
#include <cstdlib>

matrix_t* S_loc;
tensor4D_t* D_inv_loc;
tensor4D_t* u_loc;
int size_loc;
tensor4D_t* r_loc;

void naive_pre_execution(matrix_t* S, tensor4D_t* D_inv, tensor4D_t* u, int size)
{
    S_loc = S;
    D_inv_loc = D_inv;
    u_loc = u;
    size_loc = size;
    r_loc = (tensor4D_t*) malloc(sizeof(tensor4D_t) * size);
    if(r_loc == NULL)
        throw 75;
}

void naive_launch()
{
    for(int index = 0; index < size_loc; index++)
    {
        matrix_t S = S_loc[index];
        tensor4D_t D_inv = D_inv_loc[index];
        tensor4D_t u = u_loc[index];
        tensor4D_t* r = &(r_loc[index]);
        double t12[X][Y][Z];
        for (unsigned i0 = 0; i0 < X; i0++) {
            for (unsigned i1 = 0; i1 < Y; i1++) {
                for (unsigned i2 = 0; i2 < Z; i2++) {
                    double t18 = 0.0;
                    for (unsigned i3_contr = 0; i3_contr < X; i3_contr++) {
                        t18 += S.std[i0][i3_contr] * u.std[i1][i2][i3_contr];
                    }
                    t12[i0][i1][i2] = t18;
                }
            }
        }
        double t13[X][Y][Z];
        for (unsigned i4 = 0; i4 < X; i4++) {
            for (unsigned i5 = 0; i5 < Y; i5++) {
                for (unsigned i6 = 0; i6 < Z; i6++) {
                    double t19 = 0.0;
                    for (unsigned i7_contr = 0; i7_contr < X; i7_contr++) {
                        t19 += S.std[i4][i7_contr] * t12[i5][i6][i7_contr];
                    }
                    t13[i4][i5][i6] = t19;
                }
            }
        }
        double t14[X][Y][Z];
        for (unsigned i8 = 0; i8 < X; i8++) {
            for (unsigned i9 = 0; i9 < Y; i9++) {
                for (unsigned i10 = 0; i10 < Z; i10++) {
                    double t20 = 0.0;
                    for (unsigned i11_contr = 0; i11_contr < X; i11_contr++) {
                        t20 += S.std[i8][i11_contr] * t13[i9][i10][i11_contr];
                    }
                    t14[i8][i9][i10] = t20;
                }
            }
        }
        double t17[X][Y][Z];
        for (unsigned i12 = 0; i12 < X; i12++) {
            for (unsigned i13 = 0; i13 < Y; i13++) {
                for (unsigned i14 = 0; i14 < Z; i14++) {
                    t17[i12][i13][i14] = D_inv.std[i12][i13][i14] * t14[i12][i13][i14];
                }
            }
        }
        double t15[X][Y][Z];
        for (unsigned i15 = 0; i15 < X; i15++) {
            for (unsigned i16 = 0; i16 < Y; i16++) {
                for (unsigned i17 = 0; i17 < Z; i17++) {
                    double t21 = 0.0;
                    for (unsigned i18_contr = 0; i18_contr < X; i18_contr++) {
                        t21 += S.std[i18_contr][i15] * t17[i16][i17][i18_contr];
                    }
                    t15[i15][i16][i17] = t21;
                }
            }
        }
        double t16[X][Y][Z];
        for (unsigned i19 = 0; i19 < X; i19++) {
            for (unsigned i20 = 0; i20 < Y; i20++) {
                for (unsigned i21 = 0; i21 < Z; i21++) {
                    double t22 = 0.0;
                    for (unsigned i22_contr = 0; i22_contr < X; i22_contr++) {
                        t22 += S.std[i22_contr][i19] * t15[i20][i21][i22_contr];
                    }
                    t16[i19][i20][i21] = t22;
                }
            }
        }
        for (unsigned i23 = 0; i23 < X; i23++) {
            for (unsigned i24 = 0; i24 < Y; i24++) {
                for (unsigned i25 = 0; i25 < Z; i25++) {
                    double t23 = 0.0;
                    for (unsigned i26_contr = 0; i26_contr < X; i26_contr++) {
                        t23 += S.std[i26_contr][i23] * t16[i24][i25][i26_contr];
                    }
                    r->std[i23][i24][i25] = t23;
                }
            }
        }
    }
}

void naive_post_execution(tensor4D_t*& r)
{
    memcpy(&r, r_loc, sizeof(tensor4D_t) * size_loc);
    free(r_loc);
}