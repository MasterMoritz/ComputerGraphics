/******************************************************************
*
* ColorConversion.cpp
*
* Description: Helper routine for color space transformations.
* 	
*
* Computer Graphics Proseminar SS 2015
* 
* Interactive Graphics and Simulation Group
* Institute of Computer Science
* University of Innsbruck
*
* Andreas Moritz, Philipp Wirtenberger, Martin Agreiter
*******************************************************************/

#define GLM_FORCE_RADIANS  /* Use radians in all GLM functions */
#include "../glm/glm.hpp"
#include "../glm/gtc/type_ptr.hpp"         /* Vector/matrix handling */
#include "../glm/gtc/type_precision.hpp"
using glm::vec3;

vec3 hsvToRgb(vec3 hsv) {
    hsv[0] = fmod(hsv[0], 360); //limit hue to interval [0;360)
    int M = hsv[0]/60;
    float x = hsv[1]*hsv[2];
    float y = x * (1 - abs(M % 2 - 1));
    float z = hsv[2] - x;
    vec3 rgb;
    if(M < 1) {
        rgb = vec3(x+z, y+z, z);
    }
    else if(M < 2) {
        rgb = vec3(y+z, x+z, z);
    }
    else if(M < 3) {
        rgb = vec3(z, x+z, y+z);
    }
    else if(M < 4) {
        rgb = vec3(z, y+z, x+z);
    }
    else if(M < 5) {
        rgb = vec3(y+z, z, x+z);
    }
    else if(M < 6) {
        rgb = vec3(x+z, z, y+z);
    }
    return rgb;
}
