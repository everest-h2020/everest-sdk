#include <margot/margot.hpp>

extern "C" {
#include "inverse_helmholtz.h"
}

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

const char *fin  = "./../data/random.data";
const char *fout = "./../data/golden.data";

/* read some random values betwwen 0..1 from file */
void read_data(const char *file, matrix_t *S, tensor4D_t *D_inv, tensor4D_t *u) {
    FILE *data;
    double value;
    unsigned i,j,k,l;
    
    data = fopen(file, "r");
    /* read S */
    for(i = 0; i < X; i++) {
        for (j = 0 ; j < Y; j++) {
            fscanf(data,"%lf",&value);
	    S->std[i][j] = value;
        }
    }
    /* read D_inv with offset: */
    /* X * Y*/
    for(i = 0; i < LOOP; i++) {
        for (j = 0; j < X; j++) {
            for (k = 0; k < Y; k++) {
                for (l = 0; l < Z; l++) {
                    fscanf(data,"%lf",&value);
		    if (j==k && k==l) {
			D_inv->std[i][j][k][l] = value;
		    }
		    else {
			D_inv->std[i][j][k][l] = 0;
		    }
                }
	    }
        }
    }
    /* read u with offset:*/
    /* (X * Y) + (X * Y * Z * LOOP) */
    for(i = 0; i < LOOP; i++) {
        for (j = 0; j < X; j++) {
            for (k = 0; k < Y; k++) {
                for (l = 0; l < Z; l++) {
                    fscanf(data,"%lf",&value);
		    u->std[i][j][k][l] = value;
                }
	    }
        }
    }
    fclose(data);
}

int almost_equal(double a, double b, double epsilon) {
    return fabs(a-b) <= epsilon;
}

/* compare output with golden run */
int check_outputs(const char *file, tensor4D_t r) {
    FILE *golden_data;
    double value;
    unsigned i,j,k,l;
    unsigned result = 0;
    
    golden_data = fopen(file, "r");
    for(i = 0; i < LOOP; i++) {
        for (j = 0 ; j < X; j++) {
            for (k = 0 ; k < Y; k++) {
                for (l = 0 ; l < Z; l++) {
                    fscanf(golden_data,"%lf",&value);
		    if (! almost_equal(r.std[i][j][k][l], value, DOUBLE_EPSILON)) {
			result++;
                        //printf(" r=%.15f v=%.15f", r.std[i][j][k][l], value);
		    }
                }
		//printf("\r\n");
	    }
        }
    }
    fclose(golden_data);
    return result;
}

void print_output(tensor4D_t r) {
    unsigned i,j,k,l;
    for(i = 0; i < LOOP; i++) {
        for (j = 0 ; j < X; j++) {
            for (k = 0 ; k < Y; k++) {
                for (l = 0 ; l < Z; l++) {
                    printf("%.4f ", r.std[i][j][k][l]);
		}
		printf("\r\n");		
	    }
	}
    }
}

int main(int argc, char *argv[]) {
    matrix_t S;
    tensor4D_t D_inv;
    tensor4D_t u;
    tensor4D_t r;

    read_data(fin, &S, &D_inv, &u);

    //Initialize margot
    margot::init();
    
    //This typedef shall match with the signature of the kernel functions
    typedef void (*signature_t) (matrix_t, tensor4D_t, tensor4D_t, tensor4D_t*);
    signature_t alternatives[3];
    char* names[3];

    //Prepare the version
    alternatives[0] = inverse_helmholtz_gemm;
    alternatives[1] = inverse_helmholtz_nested;
    alternatives[2] = inverse_helmholtz_naive;

    names[0] = (char*)"inverse_helmholtz_gemm";
    names[1] = (char*)"inverse_helmholtz_nested";
    names[2] = (char*)"inverse_helmholtz_naive";

    //Default of the mARGOt knob that handles the version chosen
    int version = 2;

    int repetitions = 100;

    //The second condition waits until agora produces the knowledge, it must be removed before production
    for(int i = 0; i < repetitions || margot::block1::context().manager.in_design_space_exploration(); i++)
    {
        //This if loop checks whether the used version has changed
        if(margot::block1::update(version))
        {
            margot::block1::context().manager.configuration_applied();
            printf("< < < CHANGED VERSION > > >\n");
        }

        //Start monitors and execute
        margot::block1::start_monitors();
        alternatives[version](S, D_inv, u, &r);
        printf("Run %s\r\n", names[version]);
        int errors = check_outputs(fout, r);
        printf("Errors: %d\r\n", errors);

        margot::block1::stop_monitors();
        margot::block1::push_custom_monitor_values(errors);

        margot::block1::log();
    }
}

