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

// maximum number of lights to be rendered per shader invocation
const int MAX_LIGHTS = 10; 

uniform Light lights[MAX_LIGHTS];

//structure for material properties
struct Material {
    GLfloat ambient[3];
    GLfloat diffuse[3];
    GLfloat specular[3];
};

uniform int numOfModels;
uniform Material materials[numOfModels];

//material index for the current face
in int materialIndex;

out vec4 FragColor;

void main()
{
    FragColor = vec4(materials[materialIndex].diffuse[0], materials[materialIndex].diffuse[1], materials[materialIndex].diffuse[2]);
}
