/*inverse_helmholtz_gemm.c*/

/* Copyright (C) 2017-2020 TU Dresden */
/* All Rights Reserved                */
/*                                    */
/* Authors: Gerald Hempel             */
#include "stdio.h"
#include "inverse_helmholtz.h"

/* general 2D matrix multiplication */
void gemm(
unsigned m1, 
unsigned n1,
unsigned m2, 
unsigned n2, 
double in1[m1][n1],
double in2[m2][n2],
double out[m1][n2])
{
  unsigned i, j, k;
  for (i = 0; i < m1; i++) { 
    for (j = 0; j < n2; j++) { 
      out[i][j] = 0; 
      for (k = 0; k < m2; k++) {
        out[i][j] += in1[i][k] * in2[k][j];
      }
    } 
  } 
}

/* transpose operation for 3D tensor: in_XYZ -> out_YZX */
void tensor_transpose(double in[X][Y][Z], double out[X][Y][Z]) {
  unsigned i,j,k;
  for (i = 0; i < X; i++) { 
    for (j = 0; j < Y; j++) { 
      for (k = 0; k < Z; k++) {
        out[k][i][j] = in[i][j][k];
      }
    }
  }
}

/* transpose operation for matrix: in_XY -> out_YX */
void matrix_transpose(double in[X][Y], double out[X][Y]) {
    unsigned i,j;
    for (i = 0; i < X; i++) { 
        for (j = 0; j < Y; j++) { 
	    out[j][i] = in[i][j];
	}
    }
}

void inverse_helmholtz_gemm(
matrix_t S,
tensor4D_t D_inv,
tensor4D_t u,
tensor4D_t *r)
{
  for (unsigned i0 = 0; i0 < LOOP; i0++) {
    tensor3D_t t12, t13, t14, t15, t16;
    matrix_t S_T;
    unsigned i,j,k; 
    
    matrix_transpose(S.std, S_T.std);
    tensor_transpose(u.std[i0], t12.std);
    gemm(X, Y,  X, Y*Z, S.std, t12.flatted, t13.flatted);
    tensor_transpose(t13.std, t14.std);
    gemm(X, Y,  X, Y*Z, S.std, t14.flatted, t15.flatted);
    tensor_transpose(t15.std, t16.std);
    gemm(X, Y,  X, Y*Z, S.std, t16.flatted, r->flatted[i0]);
    for (unsigned i = 0; i < X; i++) {
      for (unsigned j = 0; j < Y; j++) {
        for (unsigned k = 0; k < Z; k++) {
          r->std[i0][i][j][k] = D_inv.std[i0][i][j][k] * r->std[i0][i][j][k];
        }
      }
    }
    gemm(X, Y,  X, Y*Z, S_T.std, r->flatted[i0], t12.flatted);
    tensor_transpose(t12.std, t13.std);
    gemm(X, Y,  X, Y*Z, S_T.std, t13.flatted, t14.flatted);
    tensor_transpose(t14.std, t15.std);
    gemm(X, Y,  X, Y*Z, S_T.std, t15.flatted, t16.flatted);
    tensor_transpose(t16.std, r->std[i0]);
  }
}

