#include <margot/margot.hpp>
#include <chrono>
#include <thread>
#include <list>
#include <string>
#include <iostream>
#include <cstdio>
extern "C" {
#include "helmholtz.h"
}
extern "C" {
#include "naive.h"
}
extern "C" {
#include "fpga.h"
}
typedef struct
{
    int fpga_condition;
} features;
void helmholtz(matrix_t* S, tensor4D_t* D_inv, tensor4D_t* u, int size, tensor4D_t** r)
{
    margot::init();
    typedef void (*pre) (matrix_t*, tensor4D_t*, tensor4D_t*, int);
    typedef void (*launch) ();
    typedef void (*post) (tensor4D_t*&);
    pre pre_fun[2];
    launch launch_fun[2];
    post post_fun[2];
    char* names[2];
    pre_fun[0] = naive_pre_execution;
    launch_fun[0] = naive_launch;
    post_fun[0] = naive_post_execution;
    names[0] = static_cast<char*>("naive");
    pre_fun[1] = fpga_pre_execution;
    launch_fun[1] = fpga_launch;
    post_fun[1] = fpga_post_execution;
    names[1] = static_cast<char*>("fpga");
    int version = 0;
    features feat;
    feat.fpga_condition = fpga_condition();
    margot::default_block::context().manager.wait_for_knowledge(5);
    if(margot::default_block::update(feat.fpga_condition, version))
    {
        margot::default_block::context().manager.configuration_applied();
        std::cout << "< < < CHANGED VERSION > > >" << std::endl;
    }
    int error = 0;
    switch(version)
    {
        case 1:
        if(!([](int i) -> bool {return i != 0;}(feat.fpga_condition)))
        {
            error = 1;
            version = 0;
        }
        break;
        default:
        break;
    }
    bool done = false;
    while(!done)
    {
        try
        {
            pre_fun[version](S, D_inv, u, size);
            margot::default_block::start_monitors();
            launch_fun[version]();
            margot::default_block::stop_monitors();
            margot::default_block::push_custom_monitor_values(error);
            post_fun[version](*r);
            margot::default_block::log();
            done = true;
        }
        catch(...)
        {
            if(!version)
                throw;
            version = 0;
            error = 2;
        }
    }
}
bool check_dse()
{
    return margot::default_block::context().manager.in_design_space_exploration();
}
