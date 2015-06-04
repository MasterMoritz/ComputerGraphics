/******************************************************************
*
* Based on D. Shreiner, G. Sellers, J. Kessenich, B. Licea-Kane, 
* “OpenGL Programming Guide: The Official Guide to Learning OpenGL, 
* Version 4.3” Addison-Wesley, 8th edition, 978-0321773036, 2013.
*
* Computer Graphics Proseminar SS 2015
* 
* Interactive Graphics and Simulation Group
* Institute of Computer Science
* University of Innsbruck
*
* Andreas Moritz, Philipp Wirtenberger, Martin Agreiter
*
*******************************************************************/


#version 330 core

//structure for our lights
struct Light {
	bool isEnabled;
	int type; //0 = point-light, 1 = spot-light
	vec3 ambient; //ambient light contribution of this light
	vec3 color;
	vec3 position;
	vec3 coneDirection;
	float coneCutOffAngle;
	float attenuation;
	float intensity; //light intensity between 0 and 1
};

//actual number of lights in the lights array
uniform int light_count;
// maximum number of lights to be rendered per shader invocation
const int MAX_LIGHTS = 10; 
uniform Light lights[MAX_LIGHTS];

//structure for material properties
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

//actual number of materials in the materials array
uniform int material_count;
//maximum number of materials in the materials array
const int MAX_MATERIALS = 20;
uniform Material materials[MAX_MATERIALS];


uniform float Shininess;
uniform float Strength;

//material index for the current face
flat in int materialIndex;
in vec3 Normal;
in vec4 Position;

out vec4 FragColor;

void main()
{
    vec3 scatteredLight = vec3(0.0, 0.0, 0.0);  // the global ambient
    vec3 reflectedLight = vec3(0.0);
    for(int i = 0; i < lights_count; i++) {
        vec3 halfVector;
        vec3 lightDirection = lights[i].position;
        flat attenuation = 1.0;
        
        lightDirection -= vec3(Position);
        lightDirection /= length(lightDirection);
        attenuation = 1.0 / lights[i].attenuation;
        if(lights[i].type == 1) {
            float dotProd = dot(lightDirection, -lights[i].coneDirection);
            if (dotProd < lights[i].coneCutOffAngle) {
                attenuation = 0.0;
            }
            else {
                attenuation *= pow(dotProd, 2.0);       //second argument sets the spot exponent
            }    
        }
        halfVector = normalize(lightDirection + vec3(0, 0, 1));
        float diffuse = max(0.0, dot(Normal, lightDirection));  //should be >0
        float specular = max(0.0, dot(Normal, halfVector));  //should be >0

        if(diffuse == 0.0) {
            specular = 0.0;
        }
        else {
            specular = pow(specular, Shininess) * Strength;
        }
        scatteredLight += lights[i].ambient * materials[materialIndex].ambient * attenuation + lights[i].color * materials[materialIndex].diffuse * diffuse * attenuation;
        reflectedLight += lights[i].color * materials[materialIndex].specular * specular * attenuation;
    }
    FragColor = vec4(min(scatteredLight + reflectedLight, vec3(1.0)), 1.0);
}
