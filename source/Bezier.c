/******************************************************************
*
* Bezier.c
*
* Description: Helper routine for bezier curve computations.
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

/******************************************************************
*
* ComputeBezierPoint
*
*******************************************************************/

void ComputeBezierPoint(const float points[4][3], float t, float result[3]) {
    for(int i = 0; i < 3; i++) {
        result[i] = pow(1-t, 3)*points[0][i] + 3*t*pow(1-t, 2)*points[1][i] + 3*t*t*(1-t)*points[2][i] + t*t*t*points[3][i];
    }
}


