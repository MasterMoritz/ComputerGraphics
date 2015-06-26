/******************************************************************
*
* Random.cpp
*
* Description: Helper routines returning random floats and random vectors.
* 	
*
* Computer Graphics Proseminar SS 2015
* 
* Interactive Graphics and Simulation Group
* Institute of Computer Science
* University of Innsbruck
*
* Taken from OpenGL Programming Guide 8th Edition (Shreiner et al.)
*******************************************************************/

#define GLM_FORCE_RADIANS  /* Use radians in all GLM functions */
#include "../glm/glm.hpp"
#include "../glm/gtc/type_ptr.hpp"         /* Vector/matrix handling */
#include "../glm/gtc/type_precision.hpp"
using namespace glm;

float randf() {
    float res;
    unsigned int tmp;
    static unsigned int seed = 0xFFFF0C59;
    seed *= 16807;
    tmp = seed ^ (seed >> 4) ^ (seed << 15);
    *((unsigned int *) &res) = (tmp >> 9) | 0x3F800000;
    return (res - 1.0f);
}

vec3 randvec(float minmag = 0.0f, float maxmag = 1.0f) {
    vec3 randomvec(randf() * 2.0f - 1.0f, randf() * 2.0f - 1.0f, randf() * 2.0f - 1.0f);
    randomvec = normalize(randomvec);
    randomvec *= (randf() * (maxmag - minmag) + minmag);
    return randomvec;
}
