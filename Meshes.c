/******************************************************************
*
* Basic Meshes.c
*
* Description: Helper routine for calculation of
* vertex positions of basic meshes.
* 	
*
* Computer Graphics Proseminar SS 2015
* 
* Interactive Graphics and Simulation Group
* Institute of Computer Science
* University of Innsbruck
*
*******************************************************************/

/* Standard includes */
#include <math.h>
#include <string.h>


/******************************************************************
*
* simpleCylinder
*
*******************************************************************/

void simpleCylinder(double height, double radius, int segments, float* result) {
    float vertex_buffer_data[segments*3*2];
    int i;
    double phi;
    for(i = 0, phi = 0; phi < 2*M_PI; phi += 2*M_PI/segments, i+=6) {
        double x = radius*cos(i);
        double z = radius*sin(i);
        vertex_buffer_data[i] = x;
        vertex_buffer_data[i+1] = -1.0;
        vertex_buffer_data[i+2] = z;
        vertex_buffer_data[i+3] = x;
        vertex_buffer_data[i+4] = height;
        vertex_buffer_data[i+5] = z;
    }
    memcpy(result, vertex_buffer_data, segments*3*2*sizeof(float));
}
