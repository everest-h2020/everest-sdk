#ifndef HELMHOLTZ_H
#define HELMHOLTZ_H
#include "<abs_pat_to>/inverse_helmholtz.h"
void helmholtz(matrix_t* S, tensor4D_t* D_inv, tensor4D_t* u, int size, tensor4D_t** r);
bool check_dse();
#endif