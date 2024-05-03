/*inverse_helmholtz.c*/

/* Copyright (C) 2017-2020 TU Dresden */
/* All Rights Reserved                */
/*                                    */
/* Authors: Gerald Hempel             */

#ifndef _INVERSE_HELMHOLTZ_H__
#define _INVERSE_HELMHOLTZ_H__

/* epsilon for comparison of double values */
#define DOUBLE_EPSILON 1E-10

#define X    11
#define Y    11
#define Z    11

typedef union tensor4D{
    double std[X][Y][Z];
    double flatted[X][Y*Z];
    double ultra_flat[X*Y*Z];
} tensor4D_t;

typedef union tensor3D{
    double std[X][Y][Z];
    double flatted[X][Y*Z];
    double ultra_flat[X*Y*Z];
} tensor3D_t;

typedef union matrix{
    double std[X][Y];
    double flat[X*Y];
} matrix_t;

#endif
