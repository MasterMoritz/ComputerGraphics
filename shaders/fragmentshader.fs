/******************************************************************
*
* A simple Phong shader (i.e. lighting is calculated in the fragment and not the vertex shader, employing interpolated normals).
* Based on lecture slides and D. Shreiner, G. Sellers, J. Kessenich, B. Licea-Kane, 
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
	float coneCutOffAngleCos;
	float attenuation;
	float intensity; //light intensity between 0 and 1
};

//actual number of lights in the lights array
uniform int light_count;
// maximum number of lights to be rendered per shader invocation
const int MAX_LIGHTS = 10; 
// the array of lights
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
//the array of materials
uniform Material materials[MAX_MATERIALS];

//how "sharp"/narrow the reflection should be
uniform float Shininess = 20.0;
//how bright the reflection should be
uniform float Strength = 10.0;

//material index for the current face
flat in int materialIndex;
//the vertex normal (should not be used for hard edges)
in vec3 Normal;
//the position of the vertex before the perspective transformation
in vec4 Position;

out vec4 FragColor;

void main()
{ 
    //vector towards viewing position
    vec3 v = vec3(0, 0, 1);
    //orientation of local surface
    vec3 n = normalize(Normal);
    //shininess (i.e. how "sharp"/narrow the reflection should be)
    float m = 0.2; 
    //parameters determining reflection behaviour
    vec3 ka = materials[materialIndex].ambient;
    vec3 kd = materials[materialIndex].diffuse;
    vec3 ks = materials[materialIndex].specular;

    //ambient light
    vec3 Ila = vec3(.2, .2, .2);

    //ambient reflection
    vec3 Ia = vec3(ka[0] * Ila[0], ka[1] * Ila[1], ka[2] * Ila[2]);
    
    //diffuse reflection
    vec3 Id = vec3(0.0);

    //specular reflection
    vec3 Is = vec3(0.0);

    for(int i = 0; i < light_count; i++) {
        if (!lights[i].isEnabled) {
			continue;
		}
        //incoming light direction (pointing away from surface)
        vec3 l = normalize(lights[i].position - vec3(Position));
        //incoming light intensity per channel
        vec3 Il;
        if(lights[i].type == 0) {
            Il = vec3(lights[i].intensity * lights[i].color[0], lights[i].intensity * lights[i].color[1], lights[i].intensity * lights[i].color[2]);
        }
        else {
            float cl = dot(lights[i].coneDirection, -l);
            if(cl > lights[i].coneCutOffAngleCos) {
                Il = vec3(0.0);
            }
            else {
                //spot exponent
                float n = 0.3;
                float cln = pow(cl, n);
                Il = vec3(lights[i].intensity * lights[i].color[0] * cln, lights[i].intensity * lights[i].color[1] * cln, lights[i].intensity * lights[i].color[2] * cln);
            }
        }
        //distance from positional light to surface
        vec3 d = abs(l);
        //attenuation
        float k1 = 0.2;
        float k2 = 0.3;
        float k3 = 0.6;
        Il /= (k1 + k2*d + k3*d*d);

        //reflection vector
        vec3 r = normalize((2*n*(n*l))-l);

        //diffuse reflection
        float x = dot(n, l);
        Id += vec3(kd[0] * Il[0] * x, kd[1] * Il[1] * x, kd[2] * Il[2] * x);

        //specular reflection
        x = pow(dot(r, v), m);
        Is += vec3(ks[0] * Il[0] * x, ks[1] * Il[1] * x, ks[2] * Il[2] * x);
    }

    vec3 I = Ia + Is + Id;
        
    
    FragColor = vec4(I, 1.0);

    /*vec3 scatteredLight = vec3(0.0, 0.0, 0.0);  // the global ambient
    vec3 reflectedLight = vec3(0.0);
    for(int i = 0; i < light_count; i++) {
        if(lights[i].isEnabled) {
        vec3 halfVector;
        vec3 lightDirection = lights[i].position;
        float attenuation = 1.0;
        
        lightDirection -= vec3(Position);
        lightDirection /= length(lightDirection);
        attenuation = 1.0 / lights[i].attenuation;
        if(lights[i].type == 1) {
            float dotProd = dot(lightDirection, -lights[i].coneDirection);
            if (dotProd < lights[i].coneCutOffAngleCos) {
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
    }
    FragColor = vec4(min(scatteredLight + reflectedLight, vec3(1.0)), 1.0);
    */
}
