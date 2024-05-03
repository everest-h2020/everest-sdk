#ifndef FPGA_HPP
#define FPGA_HPP

#include "../inverse_helmholtz.h"

void fpga_pre_execution(matrix_t* S, tensor4D_t* D_inv, tensor4D_t* u, int size);
void fpga_launch();
void fpga_post_execution(tensor4D_t*& r);
int fpga_condition();
#endif
