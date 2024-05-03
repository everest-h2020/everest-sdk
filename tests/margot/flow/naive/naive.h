#ifndef NAIVE_HPP
#define NAIVE_HPP

#include "../inverse_helmholtz.h"


void naive_pre_execution(matrix_t* S, tensor4D_t* D_inv, tensor4D_t* u, int size);
void naive_launch();
void naive_post_execution(tensor4D_t*& r);

#endif