extern "C"
{
#include "fpga.h"
}
#include "xcl2.hpp"

#include <algorithm>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <cmath>

#define P 11

#define SIZE_S (P * P)
#define SIZE_D (P * P * P)
#define SIZE_u (P * P * P)
#define SIZE_v (P * P * P)

#define PING_PONG_SIZE 2
#define INTL 4
#define NUM_CU 1
#define device (*device_ptr)
#define krnl (*krnl_ptr)
#define cont (*cont_ptr)

const unsigned int MAX_POINTS = floor(256 * 1024 * 1024 / ((SIZE_S + SIZE_D + SIZE_u) * sizeof(double)));

unsigned int num_batches = 0;
unsigned int input_size = 0;

cl::Device * device_ptr;
cl::Kernel* krnl_ptr[NUM_CU];
std::vector<std::vector<double, aligned_allocator<double>>> source_full_input;
std::vector<std::vector<double, aligned_allocator<double>>> source_results_final;
cl::Context * cont_ptr;

void fpga_pre_execution(matrix_t *S, tensor4D_t *D_inv, tensor4D_t *u, int size)
{
    std::cout << "[HOST] Start pre execution!" << std::endl;
    cl_int err;

    OCL_CHECK(err, cont_ptr = new cl::Context(device, NULL, NULL, NULL, &err));

    std::string krnl_name = "krnl_helm";
    auto fileBuf = xcl::read_binary_file("/home/rocco/alveo_generator_v3/fpga/hw/krnl_helm.xclbin");

    cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
    cl::Program program(cont, {device}, bins, nullptr, &err);

    if (err != CL_SUCCESS)
    {
        return;
    }
    else
    {
        // std::cout << "[HOST] Device: program successful!" << std::endl;
        //  Creating Kernel object using Compute unit names
        for (unsigned int i = 0; i < NUM_CU; i++)
        {
            std::string cu_id = std::to_string(i + 1);
            std::string krnl_name_full = krnl_name + ":{" + "krnl_helm_" + cu_id + "}";
            // printf("Creating a kernel [%s] for CU(%d)\n", krnl_name_full.c_str(), i + 1);
            OCL_CHECK(err, krnl_ptr[i] = new cl::Kernel(program, krnl_name_full.c_str(), &err));
        }
    }

    num_batches = ceil((double)size / (double)(MAX_POINTS)); // will always be divisible by PING_PONG_SIZE

    std::cout << "[HOST] POINTS       = " << size << std::endl;
    std::cout << "[HOST] NUM_CU       = " << NUM_CU << std::endl;
    std::cout << "[HOST] NUM_BATCHES  = " << num_batches << std::endl;

    input_size = size;

    for (unsigned int nb = 0; nb < num_batches; nb++)
    {
        std::vector<double, aligned_allocator<double>> full_data_batch((SIZE_S + SIZE_D + SIZE_u) * MAX_POINTS);
        unsigned points = size < MAX_POINTS ? size : MAX_POINTS;

        for (unsigned p = 0; p < points; p += INTL)
        {
            for (unsigned int i = 0; i < SIZE_S; i++)
            {
                for (unsigned int l = 0; l < INTL; l++)
                {
                    full_data_batch[(SIZE_S + SIZE_u + SIZE_D) * p + INTL * i + l] =
                        S[p + l].flat[i];
                }
            }
            for (unsigned int i = 0; i < SIZE_u; i++)
            {
                for (unsigned int l = 0; l < INTL; l++)
                {
                    full_data_batch[(SIZE_S + SIZE_u + SIZE_D) * p + SIZE_S * INTL + INTL * i + l] =
                        u[p + l].ultra_flat[i];
                }
            }
            for (unsigned int i = 0; i < SIZE_D; i++)
            {
                for (unsigned int l = 0; l < INTL; l++)
                {
                    full_data_batch[(SIZE_S + SIZE_u + SIZE_D) * p + (SIZE_S + SIZE_u) * INTL + INTL * i + l] =
                        D_inv[p + l].ultra_flat[i];
                }
            }
            size = size - points;
        }
        source_full_input.push_back(full_data_batch);
    }
    std::cout << "[HOST] End pre execution!" << std::endl;
}

void fpga_launch()
{
    std::cout << "[HOST] Start execution!" << std::endl;
    cl_int err;
    std::vector<std::vector<cl::CommandQueue>> in_queues{PING_PONG_SIZE, std::vector<cl::CommandQueue>(NUM_CU)};
    std::vector<std::vector<cl::CommandQueue>> k_queues{PING_PONG_SIZE, std::vector<cl::CommandQueue>(NUM_CU)};
    std::vector<std::vector<cl::CommandQueue>> out_queues{PING_PONG_SIZE, std::vector<cl::CommandQueue>(NUM_CU)};

    std::vector<std::vector<std::vector<double, aligned_allocator<double>>>> cu_data_in{
        PING_PONG_SIZE,
        std::vector<std::vector<double, aligned_allocator<double>>>{
            NUM_CU,
            std::vector<double, aligned_allocator<double>>((SIZE_S + SIZE_D + SIZE_u) * MAX_POINTS)}};
    std::vector<std::vector<std::vector<double, aligned_allocator<double>>>> cu_data_out{
        PING_PONG_SIZE,
        std::vector<std::vector<double, aligned_allocator<double>>>{
            NUM_CU,
            std::vector<double, aligned_allocator<double>>(SIZE_v * MAX_POINTS)}};

    for (unsigned int i = 0; i < NUM_CU; i++)
    {
        for (int p = 0; p < PING_PONG_SIZE; p++)
        {
            OCL_CHECK(err, cl::CommandQueue q(cont, device, CL_QUEUE_PROFILING_ENABLE, &err));
            in_queues[p][i] = q;
        }
    }
    for (unsigned int i = 0; i < NUM_CU; i++)
    {
        for (int p = 0; p < PING_PONG_SIZE; p++)
        {
            OCL_CHECK(err, cl::CommandQueue q(cont, device, CL_QUEUE_PROFILING_ENABLE, &err));
            k_queues[p][i] = q;
        }
    }
    for (unsigned int i = 0; i < NUM_CU; i++)
    {
        for (int p = 0; p < PING_PONG_SIZE; p++)
        {
            OCL_CHECK(err, cl::CommandQueue q(cont, device, CL_QUEUE_PROFILING_ENABLE, &err));
            out_queues[p][i] = q;
        }
    }

    std::vector<std::vector<cl_mem_ext_ptr_t>> inBufExt{PING_PONG_SIZE, std::vector<cl_mem_ext_ptr_t>(NUM_CU)};
    std::vector<std::vector<cl_mem_ext_ptr_t>> outBufExt{PING_PONG_SIZE, std::vector<cl_mem_ext_ptr_t>(NUM_CU)};

    std::vector<std::vector<cl::Buffer>> buffer_input{PING_PONG_SIZE, std::vector<cl::Buffer>(NUM_CU)};
    std::vector<std::vector<cl::Buffer>> buffer_output{PING_PONG_SIZE, std::vector<cl::Buffer>(NUM_CU)};

    // These commands will allocate memory on the FPGA. The cl::Buffer objects can
    // be used to reference the memory locations on the device.
    // Creating Buffers
    std::cout << "[HOST] Create CL buffer!" << std::endl;
    for (unsigned int i = 0; i < NUM_CU; i++)
    {
        for (int p = 0; p < PING_PONG_SIZE; p++)
        {
            inBufExt[p][i].obj = cu_data_in[p][i].data();
            inBufExt[p][i].param = 0;
            inBufExt[p][i].flags = pc[(i * 2 * PING_PONG_SIZE) + p]; // 2 for in + out

            outBufExt[p][i].obj = cu_data_out[p][i].data();
            outBufExt[p][i].param = 0;
            outBufExt[p][i].flags = pc[((i * 2 + 1) * PING_PONG_SIZE) + p]; // 2 for in + out

            OCL_CHECK(err,
                      buffer_input[p][i] = cl::Buffer(cont,
                                                      CL_MEM_READ_ONLY | CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR,
                                                      sizeof(double) * (SIZE_S + SIZE_u + SIZE_D) * MAX_POINTS,
                                                      &inBufExt[p][i],
                                                      &err));

            OCL_CHECK(err,
                      buffer_output[p][i] = cl::Buffer(cont,
                                                       CL_MEM_WRITE_ONLY | CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR,
                                                       sizeof(double) * SIZE_v * MAX_POINTS,
                                                       &outBufExt[p][i],
                                                       &err));
        }
    }

    unsigned ping = 0;
    unsigned last_points[NUM_CU];
    unsigned tot_points = input_size;

    std::cout << "[HOST] Copy points!" << std::endl;

    for (unsigned i = 0; i < NUM_CU; i++)
    {
        memcpy(&cu_data_in[0][i][0], &source_full_input[PING_PONG_SIZE * i][0], (SIZE_S + SIZE_D + SIZE_u) * MAX_POINTS * sizeof(double));
        if (source_full_input.size() >= PING_PONG_SIZE)
            memcpy(&cu_data_in[1][i], &source_full_input[PING_PONG_SIZE * i + 1], (SIZE_S + SIZE_D + SIZE_u) * MAX_POINTS * sizeof(double));
    }

    std::cout << "[HOST] Migrate object!" << std::endl;

    // Copy first batch of input data to Device Global Memory PING (0)
    for (unsigned i = 0; i < NUM_CU; i++)
    {
        OCL_CHECK(err,
                  err = in_queues[ping][i].enqueueMigrateMemObjects({buffer_input[ping][i]}, 0 /* 0 means from host*/));
    }

    // Data in copy done (0)
    for (unsigned int i = 0; i < NUM_CU; i++)
    {
        in_queues[ping][i].finish();
    }

    for (unsigned r = 0; r < num_batches; r += NUM_CU)
    {

        if (r != 0)
        {
            for (unsigned i = 0; i < NUM_CU; i++)
            {
                memcpy(&cu_data_in[!ping][i][0], &source_full_input[r + PING_PONG_SIZE * i + !ping][0], (SIZE_S + SIZE_D + SIZE_u) * MAX_POINTS * sizeof(double));
            }
        }

        for (unsigned i = 0; i < NUM_CU; i++)
        {
            last_points[i] = tot_points < MAX_POINTS ? tot_points : MAX_POINTS;
            tot_points = tot_points - MAX_POINTS > 0 ? tot_points - MAX_POINTS : 0;
        }

        // Setting the k_vadd Arguments
        for (unsigned int i = 0; i < NUM_CU; i++)
        {
            OCL_CHECK(err, err = krnl[i].setArg(0, buffer_input[0][i]));
            OCL_CHECK(err, err = krnl[i].setArg(1, buffer_input[1][i]));
            OCL_CHECK(err, err = krnl[i].setArg(2, buffer_output[0][i]));
            OCL_CHECK(err, err = krnl[i].setArg(3, buffer_output[1][i]));
            OCL_CHECK(err, err = krnl[i].setArg(4, ping));
            OCL_CHECK(err, err = krnl[i].setArg(5, last_points[i]));

            // Invoking the kernel
            OCL_CHECK(err, err = k_queues[ping][i].enqueueTask(krnl[i]));
        }

        if (r != 0)
        {
            for (unsigned int i = 0; i < NUM_CU; i++)
            {
                source_results_final.push_back(cu_data_out[ping][i]);
            }
        }

        // Copy input data to Device Global Memory PONG
        if (r < source_full_input.size() - (NUM_CU * PING_PONG_SIZE))
        { // last iteration, won't need more data in
            for (unsigned int i = 0; i < NUM_CU; i++)
            {
                OCL_CHECK(err,
                          err = in_queues[!ping][i].enqueueMigrateMemObjects({buffer_input[!ping][i]}, 0 /* 0 means from host*/));
            }
        }

        // Kernel done on PING
        for (unsigned int i = 0; i < NUM_CU; i++)
        {
            k_queues[ping][i].finish();
        }

        // Data out copy PING
        for (unsigned int i = 0; i < NUM_CU; i++)
        {
            OCL_CHECK(err,
                      err = out_queues[ping][i].enqueueMigrateMemObjects({buffer_output[ping][i]},
                                                                         CL_MIGRATE_MEM_OBJECT_HOST));
        }

        if (r >= source_full_input.size() - (NUM_CU * PING_PONG_SIZE))
        {
            // Data out copy done PONG
            for (unsigned int i = 0; i < NUM_CU; i++)
            {
                out_queues[ping][i].finish();
            }
        }

        if (r < source_full_input.size() - (NUM_CU * PING_PONG_SIZE))
        {

            for (unsigned int i = 0; i < NUM_CU; i++)
            {
                in_queues[!ping][i].finish();
            }
        }

        ping = !ping;
    }

    for (unsigned int i = 0; i < NUM_CU; i++)
    {
        source_results_final.push_back(cu_data_out[!ping][i]);
    }

    std::cout << "[HOST] End execution!" << std::endl;
}

void fpga_post_execution(tensor4D_t *&r)
{
    std::cout << "[HOST] Start post execution!" << std::endl;
    std::vector<tensor4D_t> rr;

    for (unsigned i = 0; i < num_batches; i++)
    {
        unsigned count = input_size < MAX_POINTS ? input_size : MAX_POINTS;
        input_size = input_size - MAX_POINTS > 0 ? input_size - MAX_POINTS : 0;
        for (unsigned p = 0; p < count; p++)
        {
            tensor4D_t t;
            for (unsigned int j = 0; j < SIZE_v; j++)
            {
                t.ultra_flat[j] = source_results_final[i][SIZE_v * p + j];
            }
            rr.push_back(t);
        }
    }

    r = &(rr[0]);
    delete device_ptr;
    for (unsigned i = 0; i < NUM_CU; i++)
        delete krnl_ptr[i];
    delete cont_ptr;
    source_full_input.clear();
    source_results_final.clear();

    std::cout << "[HOST] ALL DONE!" << std::endl;
}

int fpga_condition()
{
    std::cout << "[HOST] Check condiction!" << std::endl;
    std::vector<cl::Device> devices = xcl::get_devices("Xilinx");
    if (devices.size() == 0)
        return devices.size();
    devices.resize(1);
    device_ptr = new cl::Device(devices[0]);
    return devices.size();
}
