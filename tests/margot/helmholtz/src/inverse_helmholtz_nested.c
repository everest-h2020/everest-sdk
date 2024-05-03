/*inverse_helmholtz_nested.c*/

/* Copyright (C) 2017-2020 TU Dresden */
/* All Rights Reserved                */
/*                                    */
/* Authors: Gerald Hempel             */
#include "stdio.h"
#include "inverse_helmholtz.h"

void inverse_helmholtz_nested(
matrix_t S,
tensor4D_t D_inv,
tensor4D_t u,
tensor4D_t *r)
{
  for (unsigned i = 0; i < LOOP; i++) {
    double t12[X][Y][Z];
    for (unsigned i0 = 0; i0 < X; i0++) {
      for (unsigned i1 = 0; i1 < Y; i1++) {
        for (unsigned i2 = 0; i2 < Z; i2++) {
          double t16 = 0.0;
          for (unsigned i3_contr = 0; i3_contr < X; i3_contr++) {
            t16 += S.std[i0][i3_contr] * u.std[i][i1][i2][i3_contr];
          }
          t12[i0][i1][i2] = t16;
        }
      }
    }
    double t13[X][Y][Z];
    for (unsigned i4 = 0; i4 < X; i4++) {
      for (unsigned i5 = 0; i5 < Y; i5++) {
        for (unsigned i6 = 0; i6 < Z; i6++) {
          double t17 = 0.0;
          for (unsigned i7_contr = 0; i7_contr < X; i7_contr++) {
            t17 += S.std[i4][i7_contr] * t12[i5][i6][i7_contr];
          }
          t13[i4][i5][i6] = t17;
        }
      }
    }
    double t[X][Y][Z];
    double t18;
    for (unsigned i8 = 0; i8 < X; i8++) {
      for (unsigned i9 = 0; i9 < Y; i9++) {
        for (unsigned i10 = 0; i10 < Z; i10++) {
          double t19 = 0.0;
          for (unsigned i11_contr = 0; i11_contr < X; i11_contr++) {
            t19 += S.std[i8][i11_contr] * t13[i9][i10][i11_contr];
          }
          t18 = t19;
          t[i8][i9][i10] = D_inv.std[i][i8][i9][i10] * t18;
        }
      }
    }
    double t14[X][Y][Z];
    for (unsigned i12 = 0; i12 < X; i12++) {
      for (unsigned i13 = 0; i13 < Y; i13++) {
        for (unsigned i14 = 0; i14 < Z; i14++) {
          double t20 = 0.0;
          for (unsigned i15_contr = 0; i15_contr < X; i15_contr++) {
            t20 += S.std[i15_contr][i12] * t[i13][i14][i15_contr];
          }
          t14[i12][i13][i14] = t20;
        }
      }
    }
    double t15[X][Y][Z];
    for (unsigned i16 = 0; i16 < X; i16++) {
      for (unsigned i17 = 0; i17 < Y; i17++) {
        for (unsigned i18 = 0; i18 < Z; i18++) {
          double t21 = 0.0;
          for (unsigned i19_contr = 0; i19_contr < X; i19_contr++) {
            t21 += S.std[i19_contr][i16] * t14[i17][i18][i19_contr];
          }
          t15[i16][i17][i18] = t21;
        }
      }
    }
    for (unsigned i20 = 0; i20 < X; i20++) {
      for (unsigned i21 = 0; i21 < Y; i21++) {
        for (unsigned i22 = 0; i22 < Z; i22++) {
          double t22 = 0.0;
          for (unsigned i23_contr = 0; i23_contr < X; i23_contr++) {
            t22 += S.std[i23_contr][i20] * t15[i21][i22][i23_contr];
          }
          r->std[i][i20][i21][i22] = t22;
        }
      }
    }
  }
}
