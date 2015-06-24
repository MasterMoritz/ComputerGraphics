/******************************************************************
*
* MerryGoRound.c
*
* Assignment 3
*
* Computer Graphics Proseminar SS 2015
* 
* Interactive Graphics and Simulation Group
* Institute of Computer Science
* University of Innsbruck
*
*
* Andreas Moritz, Philipp Wirtenberger, Martin Agreiter
*******************************************************************/

/************************ DESCRIPTION *****************************
* This program comes with 3 camera modes (automatic, manual free movement, manual examine).
*
* Automatic mode travels around the MerryGoRound in an engaging fashion.
*
* Manual free mode allows the user to manually navigate with keyboard and mouse inputs in an rpg-like flying mode way
* ( note that for moving up and down the corresponding elevate-keybindings[~] should be used, 
*   although 'w' and 's' while looking up/down works too (but not as accurate at certain degrees)
* )
*
* Manual examine mode allows the user to rotate the object and zoom in/out
*
* Switching from Automatic to Manual free leaves the camera position mostly intact (may have to rotate around the y-axis afterwards to look at models again)
* switching from Manual to Automatic resets the position to its initial value.
*
*********************** KEY-BINDINGS ************************
*** General:
* c -> cycle between the 3 camera modes
* r -> reset camera
* o -> reset object animations
* enter (or ctrl+m) -> stop object animations
* q -> quit program
*
*** Automatic Mode only:
* 1 -> set camera speed to low
* 2 -> set camera speed to medium
* 3 -> set camera speed to fast
* i -> invert the camera path
*
*** Manual Free Movement Mode only:
* mouse click + drag mouse -> rotate camera  in dragging direction
* scroll wheel up/down -> increase/decrease movement speed
*
* w -> forward
* s -> backward
* a -> strafe left
* d -> strafe right
*
* [~]
* ctrl + w -> elevate up 
* ctrl + s -> elevate down
*
*** Manual Examine Mode only:
* mouse click + drag mouse -> rotation similar to the way it is in Blender
* scroll wheel up/down -> zoom in/zoom out
*
*
*
*** Lighting:
* l -> cycle through lights to change their hue/value
* + -> add 60° to hue of selected light
* - -> remove 60° from hue of selected light
* hold * -> increase value of selected light
* hold / -> decrease value of selected light
*
* b -> enable/disable ambient rendering
* n -> enable/disable diffuse rendering
* m -> enable/disable specular rendering
*
*/
/******************** ADDITIONAL NOTES **************************
*
* The (seemingly) random lines in the background are actually from the walls and the floor and 100% intended
*
*****************************************************************/

/************************ PROGRAM *******************************/
/* Standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* OpenGL includes */
#include <GL/glew.h>
#include <GL/freeglut.h>

/* GLM includes */
#define GLM_FORCE_RADIANS  /* Use radians in all GLM functions */

/* GLM includes - adjust path as required for local installation */
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" /* Provides glm::translate, glm::rotate, 
                                         * glm::scale, glm::perspective */
#include "glm/gtc/type_ptr.hpp"         /* Vector/matrix handling */
#include "glm/gtc/type_precision.hpp"

/* Local includes */
#define STB_IMAGE_IMPLEMENTATION
extern "C" 
{

    #include "stb_image.h"   /* Provides loading function for texture images https://github.com/nothings/stb */
}

using namespace glm;
#include "LoadShader.hpp"     /* Loading function for shader code */
#include "Matrix.hpp"         /* Functions for matrix handling */
#include "OBJParser.hpp"      /* Loading function for triangle meshes in OBJ format */
#include "Bezier.hpp"         /* Functions for bezier curve computations */
#include "ColorConversion.hpp"/* Function for color space transformations */

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif
#ifndef NUM_STATIC 
	#define NUM_STATIC 4
#endif
#ifndef NUM_BASIC_ANIM
	#define NUM_BASIC_ANIM 1
#endif
#ifndef NUM_ADV_ANIM
	#define NUM_ADV_ANIM 6
#endif
#ifndef NUM_LIGHT
	#define NUM_LIGHT 3
#endif
/*----------------------------------------------------------------*/

/* Flag for starting/stopping animation */
GLboolean anim = GL_TRUE;

/* To switch between automatic and the two manual camera modes */
int camMode = 0;

/* Define handles to vertex buffer objects */
GLuint VBO[NUM_STATIC+NUM_BASIC_ANIM+NUM_ADV_ANIM];

/* Define handles to index buffer objects */
GLuint IBO[NUM_STATIC+NUM_BASIC_ANIM+NUM_ADV_ANIM];

/* Define handles to normal buffer objects */
GLuint NBO[NUM_STATIC+NUM_BASIC_ANIM+NUM_ADV_ANIM];

/* Define handles to material index buffer objects */
GLuint MBO[NUM_STATIC+NUM_BASIC_ANIM+NUM_ADV_ANIM];

/* Define handles to texture coord buffer */
GLuint TBO[NUM_STATIC+NUM_BASIC_ANIM+NUM_ADV_ANIM];

GLuint VAO[NUM_STATIC+NUM_BASIC_ANIM+NUM_ADV_ANIM];

/* texture image, for now just 1 hardcoded for testing puposes */
unsigned char* image;

/* Indices to vertex attributes */ 
enum DataID {vPosition = 0, vNormal = 1, MaterialIndex = 2, texCoord = 3}; 

/* Strings for loading and storing shader code */
static const char* VertexShaderString;
static const char* FragmentShaderString;

GLuint ShaderProgram;


/* Matrices for uniform variables in vertex shader */
mat4 ProjectionMatrix; /* Perspective projection matrix */
mat4 ViewMatrix;       /* Camera view matrix */ 
mat4 ModelMatrix[NUM_STATIC+NUM_BASIC_ANIM+NUM_ADV_ANIM];      /* Model matrices */ 
  
/* Transformation matrices for model rotation */
mat4 RotationMatrixAnimX;
mat4 RotationMatrixAnimY;
mat4 RotationMatrixAnimZ;
mat4 RotationMatrixAnim;

/* Additional transformation matrices */
mat4 T;
mat4 R;
mat4 InitialTransform[NUM_STATIC+NUM_BASIC_ANIM+NUM_ADV_ANIM];
mat4 ViewTransform;

/* Variables for storing current rotation angles */
float angleX, angleY, angleZ = 0.0f; 
float camAngleX, camAngleY, camAngleZ = 0.0f;

/* the speed in manual camera mode */
float manualSpeed = 0.2f;

// last measured mouse coordinates
int xold, yold = 0;

/* Arrays for holding vertex data of models */
GLfloat *vertex_buffer_data[NUM_STATIC+NUM_BASIC_ANIM+NUM_ADV_ANIM];

/* Arrays for holding indices of models */
GLushort *index_buffer_data[NUM_STATIC+NUM_BASIC_ANIM+NUM_ADV_ANIM];

/* Arrays for holding normals of models */
GLfloat *normal_buffer_data[NUM_STATIC+NUM_BASIC_ANIM+NUM_ADV_ANIM];

/* Arrays for holding indices of materials */
GLushort *material_index_buffer_data[NUM_STATIC+NUM_BASIC_ANIM+NUM_ADV_ANIM];

/* Arrays for holding texture coordinates of vertices */
GLfloat *texture_buffer_data[NUM_STATIC+NUM_BASIC_ANIM+NUM_ADV_ANIM];

/* Structures for loading of OBJ data */
obj_scene_data data[NUM_STATIC+NUM_BASIC_ANIM+NUM_ADV_ANIM];

/* Reference time for animation */
int oldTime = 0;

/* The array of bezier curves to use for the automatic camera path */
const float curves[][4][3] = {
{{0, -4, -20}, {0, 0, -18}, {0, 0, -14}, {0, -3, -12}},
{{},{},{}},
{{0, -3, -12}, {0, -3, -22}, {0, -8, -22}, {0, -8, -6}},
{{0, -8, -6}, {0, -8, -6}, {0, -8, -6}, {0, -8, -6}},
{{0, -8, -6}, {0, -8.5, -6.5}, {0, -9.5, -5.5}, {0, -10, -6}},
{{0, -10, -6}, {0, -9.5, -5.5}, {0, -8.5, -6.5}, {0, -8, -6}},
{{0, -8, -6}, {0, -6, -22}, {0, -6, -22}, {0, -4, -20}}
};

/* Bezier curve parameter [0;1] */
float t; 

/* The currently active curve  for automatic mode */
int curve = 0;

/* The camera animation speed for automatic mode */
float camSpeed = 1;

/* Flag for path inversion */
GLboolean invertCam = GL_FALSE;

/* the currently selected light whose values the user may change */
int selectedLight = 0;

/* flags to toggle specific parts of the rendering */
int ambientRendering = 1;
int diffuseRendering = 1;
int specularRendering = 1;

/*-----------------------------Uniforms----------------------------*/
//structure for our lights
struct Light {
	GLboolean isEnabled;
	int type; //0 = point-light, 1 = spot-light
	GLfloat ambient[3]; //ambient light contribution of this light
	vec3 color;
	GLfloat position[3];
	GLfloat coneDirection[3];
	GLfloat coneCutOffAngleCos;
	GLfloat attenuation;
	GLfloat intensity; //light intensity between 0 and 1
};
typedef struct Light Light;
Light lights[NUM_LIGHT];

//structure for material properties
struct Material {
    GLfloat ambient[3];
    GLfloat diffuse[3];
    GLfloat specular[3];
};


/******************************************************************
*
* Display
*
* This function is called when the content of the window needs to be
* drawn/redrawn. It has been specified through 'glutDisplayFunc()';
* Enable vertex attributes, create binding between C program and 
* attribute name in shader, provide data for uniform variables
*
*******************************************************************/

void Display()
{
    /* Clear window; color specified in 'Initialize()' */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Associate program with shader matrices */
    GLint PVM_Uniform = glGetUniformLocation(ShaderProgram, "PVM_Matrix");    
	GLint VM_Uniform = glGetUniformLocation(ShaderProgram, "VM_Matrix");
	GLint NormalUniform = glGetUniformLocation(ShaderProgram, "NormalMatrix");

	GLuint light_attribute;

	/* set lights in shader */
	for (int i = 0; i < NUM_LIGHT; i++) {
		//only support 10 lights maximum
		if (i == 10) {
			break;
		}
		char buffer[32];
		

		//set the light attributes
		strcpy(buffer, "lights[");
		buffer[7] = '0' + i;
		buffer[8] = '\0';
		strcat(buffer, "].isEnabled");
		light_attribute = glGetUniformLocation(ShaderProgram, buffer);
		glUniform1i(light_attribute, lights[i].isEnabled);

		buffer[10] = '\0';
		strcat(buffer, "type");
		light_attribute = glGetUniformLocation(ShaderProgram, buffer);
		glUniform1i(light_attribute, lights[i].type);

		buffer[10] = '\0';
		strcat(buffer, "ambient");
		light_attribute = glGetUniformLocation(ShaderProgram, buffer);
		glUniform3f(light_attribute, lights[i].ambient[0], lights[i].ambient[1], lights[i].ambient[2]);

		buffer[10] = '\0';
		strcat(buffer, "color");
		light_attribute = glGetUniformLocation(ShaderProgram, buffer);
		glUniform3fv(light_attribute, 1, value_ptr(hsvToRgb(lights[i].color)));

		buffer[10] = '\0';
		strcat(buffer, "position");
		light_attribute = glGetUniformLocation(ShaderProgram, buffer);
        vec4 positions = ViewMatrix * vec4(lights[i].position[0], lights[i].position[1], lights[i].position[2], 1.0);
		glUniform3f(light_attribute, positions[0], positions[1], positions[2]);

		buffer[10] = '\0';
		strcat(buffer, "coneDirection");
		light_attribute = glGetUniformLocation(ShaderProgram, buffer);
		glUniform3f(light_attribute, lights[i].coneDirection[0], lights[i].coneDirection[1], lights[i].coneDirection[2]);

		buffer[10] = '\0';
		strcat(buffer, "coneCutOffAngleCos");
		light_attribute = glGetUniformLocation(ShaderProgram, buffer);
		glUniform1f(light_attribute, lights[i].coneCutOffAngleCos);

		buffer[10] = '\0';
		strcat(buffer, "attenuation");
		light_attribute = glGetUniformLocation(ShaderProgram, buffer);
		glUniform1f(light_attribute, lights[i].attenuation);

		buffer[10] = '\0';
		strcat(buffer, "intensity");
		light_attribute = glGetUniformLocation(ShaderProgram, buffer);
		glUniform1f(light_attribute, lights[i].intensity);
	}

    /* animate the animated spotlight */
    light_attribute = glGetUniformLocation(ShaderProgram, "lights[2].position");
    vec4 positions = ViewMatrix * ModelMatrix[NUM_STATIC+NUM_BASIC_ANIM] * vec4(lights[2].position[0], lights[2].position[1], lights[2].position[2], 1.0);
	glUniform3f(light_attribute, positions[0], positions[1], positions[2]);

	/* draw Meshes */
	int numObjects = NUM_STATIC + NUM_BASIC_ANIM + NUM_ADV_ANIM;

	for (int i = 0; i < numObjects; i++) {
		/* bind vertex buffer */
		glEnableVertexAttribArray(vPosition);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
		glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

		/* bind index buffer */
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[i]);
		GLint size; 
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

		/* bind normal buffer */
        glEnableVertexAttribArray(vNormal);
		glBindBuffer(GL_ARRAY_BUFFER, NBO[i]);
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

		/* bind material buffer */
        glEnableVertexAttribArray(MaterialIndex);
		glBindBuffer(GL_ARRAY_BUFFER, MBO[i]);
		glVertexAttribPointer(MaterialIndex, 1, GL_INT, GL_FALSE, 0, 0);

		/* bind texture uv */
		glEnableVertexAttribArray(texCoord);
		glBindBuffer(GL_ARRAY_BUFFER, TBO[i]);
		glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);

		/* set model matrix */
		glUniformMatrix4fv(PVM_Uniform, 1, GL_FALSE, value_ptr(ProjectionMatrix * ViewMatrix * ModelMatrix[i]));
		glUniformMatrix4fv(VM_Uniform, 1, GL_FALSE, value_ptr(ViewMatrix * ModelMatrix[i]));
		glUniformMatrix4fv(NormalUniform, 1, GL_FALSE, value_ptr(transpose(inverse(ModelMatrix[i]*ViewMatrix))));

		/* set material index */
		GLuint material_count = glGetUniformLocation(ShaderProgram, "material_count");
		glUniform1i(material_count, data[i].material_count);

        /* set render flags */
        GLuint ambientRenderingLoc = glGetUniformLocation(ShaderProgram, "ambientRendering");
        glUniform1i(ambientRenderingLoc, ambientRendering);
        GLuint diffuseRenderingLoc = glGetUniformLocation(ShaderProgram, "diffuseRendering");
        glUniform1i(diffuseRenderingLoc, diffuseRendering);
        GLuint specularRenderingLoc = glGetUniformLocation(ShaderProgram, "specularRendering");
        glUniform1i(specularRenderingLoc, specularRendering);
		
		GLuint ambLoc;
		GLuint diffLoc;
		GLuint specLoc;
		GLfloat ambient[3];
		GLfloat diffuse[3];
		GLfloat specular[3];
		char buffer[22];

		for(int z = 0; z < data[i].material_count; z++) {
			strcpy(buffer, "materials[");
			buffer[10] = '0' + z;
			buffer[11] = '\0';
			strcat(buffer, "].ambient");

			ambLoc = glGetUniformLocation(ShaderProgram, buffer);
			ambient[0] = (GLfloat)(*(data[i]).material_list[z]).amb[0];
			ambient[1] = (GLfloat)(*(data[i]).material_list[z]).amb[1];
			ambient[2] = (GLfloat)(*(data[i]).material_list[z]).amb[2];
			glUniform3f(ambLoc, ambient[0], ambient[1], ambient[2]);

			buffer[13] = '\0';
			strcat(buffer, "diffuse");

			diffLoc = glGetUniformLocation(ShaderProgram, buffer);
			diffuse[0] = (GLfloat)(*(data[i]).material_list[z]).diff[0];
			diffuse[1] = (GLfloat)(*(data[i]).material_list[z]).diff[1];
			diffuse[2] = (GLfloat)(*(data[i]).material_list[z]).diff[2];
			glUniform3f(diffLoc, diffuse[0], diffuse[1], diffuse[2]);

			buffer[13] = '\0';
			strcat(buffer, "specular");

			specLoc = glGetUniformLocation(ShaderProgram, buffer);
			specular[0] = (GLfloat)(*(data[i]).material_list[z]).spec[0];
			specular[1] = (GLfloat)(*(data[i]).material_list[z]).spec[1];
			specular[2] = (GLfloat)(*(data[i]).material_list[z]).spec[2];
			glUniform3f(specLoc, specular[0], specular[1], specular[2]);
		}

		/* Issue draw command, using indexed triangle list */
		glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
        
        glDisableVertexAttribArray(vPosition);
        glDisableVertexAttribArray(vNormal);
        glDisableVertexAttribArray(MaterialIndex);
		glDisableVertexAttribArray(texCoord);
	}

    /* Swap between front and back buffer */ 
    glutSwapBuffers();
}


/******************************************************************
*
* Mouse
*
* Function is called on mouse button press; has been seta
* with glutMouseFunc(), x and y specify mouse coordinates,
*
*******************************************************************/

void Mouse(int button, int state, int x, int y) {
    if(state == GLUT_DOWN) {
        xold = x;
		yold = y;
        int scroll_down;
        int scroll_up;
        if (GLUT_MIDDLE_BUTTON == 3) {
            scroll_down = 4;
            scroll_up = 5;
        }
        else {
            scroll_down = 3;
            scroll_up = 4;  
        }

	    if(button == scroll_down) {
            if(camMode == 1) {
	            //increase manual speed
	            manualSpeed *= 1.12f;
            }
            else if(camMode == 2) {
                //zoom in
				ViewTransform = scale(ViewTransform, vec3(1.0, 1.0, 0.95));
            }
	    }
	    else if(button == scroll_up) {
            if(camMode == 1) {
	            //decrease manual speed
	            manualSpeed /= 1.12f;
            }
            else if(camMode == 2) {
                ViewTransform = scale(ViewTransform, vec3(1.0, 1.0, 1.05));
            }
	    }
    }
}


/******************************************************************
*
* RotateCamera
*
* Function is called when mouse pointer moves while one or 
* more buttons are pressed. Only used in manual camera mode.
* Note that in glut the window relative coordinates are "swapped".
*
*******************************************************************/

void RotateCamera(int x, int y) {
    if(camMode != 0) {
        camAngleX = fmod(camAngleX + (y - yold)*0.4, 360.0);
        camAngleY = fmod(camAngleY + (x - xold)*0.4, 360.0);
        xold = x;
        yold = y;
    }
}

/******************************************************************
*
* Keyboard
*
* Function to be called on key press in window; set by
* glutKeyboardFunc(); x and y specify mouse position on keypress;
* not used in this example 
*
*******************************************************************/

void Keyboard(unsigned char key, int x, int y)   
{
	/* Camera free Movement (rpg-like flying mode) bindings */
	if (camMode == 1) {
		switch( key ) {
			
			case 'w':
				//forward
				ViewTransform = translate(ViewTransform, 
					vec3(-sin(camAngleY * (M_PI/180)) * manualSpeed, 
						sin(camAngleX * (M_PI/180)) * manualSpeed, 
						cos(camAngleY * (M_PI/180)) * manualSpeed
					));

				break;
	
			case 's':
				//backward
				ViewTransform = translate(ViewTransform, 
					vec3(sin(camAngleY * (M_PI/180)) * manualSpeed, 
						-sin(camAngleX * (M_PI/180)) * manualSpeed, 
						-cos(camAngleY * (M_PI/180)) * manualSpeed
					));
				break;

			case 23: //ctrl+w
				//up
				ViewTransform = translate(ViewTransform, vec3(0.0, -manualSpeed, 0.0));
				break;

			case 19: //ctrl+s
				//down
				ViewTransform = translate(ViewTransform, vec3(0.0, manualSpeed, 0.0));
				break;

			case 'a':
				//left
				ViewTransform = translate(ViewTransform, vec3(cos(camAngleY * (M_PI/180)) * manualSpeed, 0.0, sin(camAngleY * (M_PI/180)) * manualSpeed));
				break;

			case 'd':
				//right
				ViewTransform = translate(ViewTransform, vec3(-cos(camAngleY * (M_PI/180)) * manualSpeed, 0.0, -sin(camAngleY * (M_PI/180)) * manualSpeed));
				break;
		}
	}

	/* Automatic Camera bindings */
	else if (camMode == 0) {
		switch( key ) {	
			// Set speed of automatic camera
			case '1': 
				camSpeed = .5;
				break;

			case '2':	
				camSpeed = 1;
				break;

			case '3':
				camSpeed = 2;
				break;
			// Pause automatic camera
			case 'p':
				if(camSpeed == 0)
				    camSpeed = 1;
				else
				    camSpeed = 0;
				break;
            // Invert camera path
            case 'i':
                invertCam = !invertCam;
                break;
		}
	}

	/* General Bindings */
	switch(key) {
		/* Switch camera mode */
		case 'c':      
		    if(camMode == 1 || camMode == 2) {
				curve = 0;
				camSpeed = 1;
				ViewTransform = translate(mat4(1.0f), vec3(0.0f, -4.0f, -20.0f));
				camAngleX = 0;
				camAngleY = 0;
				camAngleZ = 0;
		    }
			camMode = (camMode+1)%3;
		    break;
	
		/* Toggle animation */
		case 13: //enter or ctrl+m
			if (anim)
				anim = GL_FALSE;		
			else
				anim = GL_TRUE;
			break;

		/* Reset initial rotation of object */
		case 'o':
			RotationMatrixAnimX = mat4(1.0);
			RotationMatrixAnimY = mat4(1.0);
			RotationMatrixAnimZ = mat4(1.0);
			angleX = 0.0;
			angleY = 0.0;
			angleZ = 0.0;
			break;

		/* Reset camera */
		case 'r':
			camSpeed = 1;
			manualSpeed = 0.2f;
			ViewTransform = translate(mat4(1.0f), vec3(0.0f, -4.0f, -20.0f));
			camAngleX = 0;
			camAngleY = 0;
			camAngleZ = 0;
			break;

        /* cycle through lights */
		case 'l':
			selectedLight = (selectedLight + 1) % NUM_LIGHT;
			break;

		/* change hue of selected light */
		case '+':
			lights[selectedLight].color[0] = lights[selectedLight].color[0] + 60.0;
			if (lights[selectedLight].color[0] > 360.0) {
				lights[selectedLight].color[0] -= 360.0;
			}
			break;
		case '-':
			lights[selectedLight].color[0] = lights[selectedLight].color[0] - 60.0;
			if (lights[selectedLight].color[0] < 0) {
				lights[selectedLight].color[0] = 360.0 - lights[selectedLight].color[0];
			}
            break;

		/* change value of selected light */
		case '*':
			lights[selectedLight].color[2] = lights[selectedLight].color[2] + 0.02;
			if (lights[selectedLight].color[2] > 1.0) {
				lights[selectedLight].color[2] = 1.0;
			}
			break;
		case '/':
			lights[selectedLight].color[2] = lights[selectedLight].color[2] - 0.02;
			if (lights[selectedLight].color[2] < 0) {
				lights[selectedLight].color[2] = 0.0;
			}
			break;

        /* toggle specific components of the rendering */
        case 'b':
                if(ambientRendering)
                    ambientRendering = 0;
                else
                    ambientRendering = 1;
            break;
        case 'n':
                  if(diffuseRendering)
                    diffuseRendering = 0;
                else
                    diffuseRendering = 1;
            break;
        case 'm':
                  if(specularRendering)
                    specularRendering = 0;
                else
                    specularRendering = 1;
            break;
		/* quit program */
		case 'q': case 'Q':  
			exit(0);    
			break;
	}
    /* Issue display refresh 
    glutPostRedisplay();*/
}


/******************************************************************
*
* OnIdle
*
* Function executed when no other events are processed; set by
* call to glutIdleFunc(); holds code for animation  
*
*******************************************************************/

/* calculates transformation value based off an angle
*  offset can serve as a delay
*/
float moves(double angle, double offset) {
	float tmp = sin((angle + offset) * (M_PI/180));
	if (tmp < 0) {
		tmp *= -1;
	}
	return tmp;
}

void OnIdle()
{
	/* Determine delta time between two frames to ensure constant animation */
	int newTime = glutGet(GLUT_ELAPSED_TIME);
	int delta = newTime - oldTime;
	oldTime = newTime;
	
	if(anim) {
		/* Increment rotation angles and update matrix */
		angleY = fmod(angleY + delta/20.0, 360.0); 
		R = rotate(mat4(1.0f), radians(angleY), vec3(0.0f, 1.0f, 0.0f));

		/* rotate all non-static objects */
		int num_non_static = NUM_BASIC_ANIM + NUM_ADV_ANIM;

		for (int i = 0; i < num_non_static; i++) {
			ModelMatrix[i + NUM_STATIC] = R * InitialTransform[i + NUM_STATIC];
		}

		/* move advanced animation objects up and down with individual delay */
		int delay = 0;
		for (int i = 0; i < NUM_ADV_ANIM; i++) {
			T = translate(mat4(1.0f), vec3(0.0f, -moves(angleY, delay), 0.0f));
			ModelMatrix[i + NUM_STATIC + NUM_BASIC_ANIM] = T * ModelMatrix[i + NUM_STATIC + NUM_BASIC_ANIM];
			delay += 20;
		}
	}

    /* Rotate camera */

	//automatic camera mode
    if(camMode == 0) {
         /* Update camera translation */
        ViewMatrix = mat4(1.0f);
        if(!invertCam) {
            t += delta/2000.0*camSpeed;
            if(t >= 1) {
                 if(curve == 1) {
                    camSpeed *= 2;
                }
                t = 0;
                curve = fmod(curve+1, sizeof(curves)/sizeof(curves[0]));
                if(curve == 1) {
                    camSpeed /= 2;
                }
            }
        }
        else {
            t -= delta/2000.0*camSpeed;
            if(t <= 0) {
                 if(curve == 1) {
                    camSpeed *= 2;
                }
                t = 1;
                curve--;
                if(curve < 0)
                    curve = (sizeof(curves)/sizeof(curves[0]))-1;
                if(curve == 1) {
                    camSpeed /= 2;
                }
            }
        }
        if(curve != 1) {
            float p[3];
            ComputeBezierPoint(curves[curve], t, p);
			ViewTransform = translate(mat4(1.0f), vec3(p[0],p[1],p[2]));
        }
        if(curve == 1) {
            camAngleY = t*360;
        }
        if(curve == 4) {
            camAngleY = 360-t*180;
        }
        if(curve == 5) {
            camAngleY = 180-t*180;
        }		
	    /* Update camera view */
        RotationMatrixAnimX = rotate(mat4(1.0f), radians(camAngleX), vec3(1.0f,0.0f,0.0f));
		RotationMatrixAnimY = rotate(mat4(1.0f), radians(camAngleY), vec3(0.0f,1.0f,0.0f));
		RotationMatrixAnimZ = rotate(mat4(1.0f), radians(camAngleZ), vec3(0.0f,0.0f,1.0f));

		ViewMatrix = ViewTransform * RotationMatrixAnimX * RotationMatrixAnimY * RotationMatrixAnimZ;
    }

	//manual camera mode
    else {
        ViewMatrix = mat4(1.0);

        /* Update camera view */
		RotationMatrixAnimX = rotate(mat4(1.0f), radians(camAngleX), vec3(1.0f,0.0f,0.0f));
		RotationMatrixAnimY = rotate(mat4(1.0f), radians(camAngleY), vec3(0.0f,1.0f,0.0f));
		RotationMatrixAnimZ = rotate(mat4(1.0f), radians(camAngleZ), vec3(0.0f,0.0f,1.0f));

		RotationMatrixAnim = RotationMatrixAnimX * RotationMatrixAnimY * RotationMatrixAnimZ;
        
		// free movement mode
        if(camMode == 1) {
			ViewMatrix = RotationMatrixAnim * ViewTransform;
        }

		// examine mode
        else if(camMode == 2) {
			ViewMatrix = ViewTransform * RotationMatrixAnim;
        }
    }	
    
    /* Issue display refresh */
    glutPostRedisplay();
}


/******************************************************************
*
* SetupDataBuffers
*
* Create buffer objects and load data into buffers
*
*******************************************************************/

void SetupDataBuffers()
{
	glGenVertexArrays(NUM_STATIC + NUM_BASIC_ANIM + NUM_ADV_ANIM, VAO);

	for (int i = 0; i < NUM_STATIC + NUM_BASIC_ANIM + NUM_ADV_ANIM; i++) {
		glGenBuffers(1, &(VBO[i]));
		glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
		glBufferData(GL_ARRAY_BUFFER, (data[i]).vertex_count*3*sizeof(GLfloat), vertex_buffer_data[i], GL_STATIC_DRAW);

		glGenBuffers(1, &(IBO[i]));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (data[i]).face_count*3*sizeof(GLushort), index_buffer_data[i], GL_STATIC_DRAW);

        glGenBuffers(1, &(NBO[i]));
		glBindBuffer(GL_ARRAY_BUFFER, NBO[i]);
		glBufferData(GL_ARRAY_BUFFER, (data[i]).vertex_normal_count*3*sizeof(GLfloat), normal_buffer_data[i], GL_STATIC_DRAW);

        glGenBuffers(1, &(MBO[i]));
		glBindBuffer(GL_ARRAY_BUFFER, MBO[i]);
		glBufferData(GL_ARRAY_BUFFER, (data[i]).face_count*sizeof(GLushort), material_index_buffer_data[i], GL_STATIC_DRAW);

        glGenBuffers(1, &(TBO[i]));
		glBindBuffer(GL_ARRAY_BUFFER, TBO[i]);
		glBufferData(GL_ARRAY_BUFFER, (data[i]).vertex_texture_count*2*sizeof(GLfloat), texture_buffer_data[i], GL_STATIC_DRAW);

		glBindVertexArray(VAO[i]);
	}
}


/******************************************************************
*
* AddShader
*
* This function creates and adds individual shaders
*
*******************************************************************/

void AddShader(const char* ShaderCode, GLenum ShaderType)
{
    /* Create shader object */
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) 
    {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }

    /* Associate shader source code string with shader object */
    glShaderSource(ShaderObj, 1, &ShaderCode, NULL);

    GLint success = 0;
    GLchar InfoLog[1024];

    /* Compile shader source code */
    glCompileShader(ShaderObj);
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

    if (!success) 
    {
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }

    /* Associate shader with shader program */
    glAttachShader(ShaderProgram, ShaderObj);
}


/******************************************************************
*
* CreateShaderProgram
*
* This function creates the shader program; vertex and fragment
* shaders are loaded and linked into program; final shader program
* is put into the rendering pipeline 
*
*******************************************************************/

void CreateShaderProgram()
{
    /* Allocate shader object */
    ShaderProgram = glCreateProgram();

    if (ShaderProgram == 0) 
    {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    /* Load shader code from file */
    VertexShaderString = LoadShader("shaders/vertexshader.vs");
    FragmentShaderString = LoadShader("shaders/fragmentshader.fs");

    /* Separately add vertex and fragment shader to program */
    AddShader(VertexShaderString, GL_VERTEX_SHADER);
    AddShader(FragmentShaderString, GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024];

    /* Link shader code into executable shader program */
    glLinkProgram(ShaderProgram);

    /* Check results of linking step */
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);

    if (Success == 0) 
    {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    /* Check if shader program can be executed */ 
    glValidateProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);

    if (!Success) 
    {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    /* Put linked shader program into drawing pipeline */
    glUseProgram(ShaderProgram);
}


/******************************************************************
*
* LoadObjFiles
*
* This function is called to load the different object files
*
*******************************************************************/

void LoadObjFiles() 
{
    int objIndex = 0;
    int success;

	/* Load all models, don't forget to update the macros if you add/remove some */
	
	/* Load all static models */

    char* filename = "models/pillars.obj"; 
    success = parse_obj_scene(&(data[objIndex]), filename);
	InitialTransform[objIndex] = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));

	objIndex += 1;
    if(!success) {
        printf("Could not load file. Exiting.\n");
	}

    filename = "models/floor_static.obj"; 
    success = parse_obj_scene(&(data[objIndex]), filename);
	InitialTransform[objIndex] = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));

	objIndex += 1;
    if(!success) {
        printf("Could not load file. Exiting.\n");
	}

    filename = "models/roof.obj"; 
    success = parse_obj_scene(&(data[objIndex]), filename);
	InitialTransform[objIndex] = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));

	objIndex += 1;
    if(!success) {
        printf("Could not load file. Exiting.\n");
	}

    filename = "models/dragonHead.obj"; 
    success = parse_obj_scene(&(data[objIndex]), filename);
	InitialTransform[objIndex] = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));

	objIndex += 1;
    if(!success) {
        printf("Could not load file. Exiting.\n");
	}

	/* Load all Basic animation models */
    filename = "models/floor_rotating.obj"; 
    success = parse_obj_scene(&(data[objIndex]), filename);
	InitialTransform[objIndex] = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));

	objIndex += 1;
    if(!success) {
        printf("Could not load file. Exiting.\n");
	}

	/* Load all Advanced animation models */
	//add 6 horsies at different positions, note that horsies are special to transform because they bugged out in maya :D
	for (int i = 0; i < 6; i++) {

		filename = "models/myLittleDragon.obj"; 
		success = parse_obj_scene(&(data[objIndex]), filename);
		InitialTransform[objIndex] = rotate(mat4(1.0f), radians(float(60*i)), vec3(0.0f,1.0f,0.0f));
		InitialTransform[objIndex] = translate(InitialTransform[objIndex], vec3(-4.0f, 0.6f, 0.0f));
		InitialTransform[objIndex] = scale(InitialTransform[objIndex], vec3(-0.4f, 0.4f, 0.4f));

		objIndex += 1;
		if(!success) {
		    printf("Could not load file. Exiting.\n");
		}
	}

    /*  Copy mesh data from structs into appropriate arrays */ 
	objIndex = 0;
    int vert = 0;
    int indx = 0;
	int nrml = 0;
	int texv = 0;

	for (int z = 0; z < NUM_STATIC + NUM_BASIC_ANIM + NUM_ADV_ANIM; z++) {
		vert = data[z].vertex_count;
		indx = data[z].face_count;
		nrml = data[z].vertex_normal_count;
		texv = data[z].vertex_texture_count;

    	vertex_buffer_data[z] = (GLfloat*) calloc (vert*3, sizeof(GLfloat));
    	index_buffer_data[z] = (GLushort*) calloc (indx*3, sizeof(GLushort));
        normal_buffer_data[z] = (GLfloat*) calloc (nrml*3, sizeof(GLfloat));
        material_index_buffer_data[z] = (GLushort*) calloc (indx, sizeof(GLushort));
    	texture_buffer_data[z] = (GLfloat*) calloc (texv*2, sizeof(GLfloat));

		/* Vertices */
		for(int i=0; i<vert; i++) {
		    vertex_buffer_data[z][i*3] = (GLfloat)(*(data[z]).vertex_list[i]).e[0];
			vertex_buffer_data[z][i*3+1] = (GLfloat)(*(data[z]).vertex_list[i]).e[1];
			vertex_buffer_data[z][i*3+2] = (GLfloat)(*(data[z]).vertex_list[i]).e[2];
		}

		/* Indices */
		for(int i=0; i<indx; i++) {
			index_buffer_data[z][i*3] = (GLushort)(*(data[z]).face_list[i]).vertex_index[0];
			index_buffer_data[z][i*3+1] = (GLushort)(*(data[z]).face_list[i]).vertex_index[1];
			index_buffer_data[z][i*3+2] = (GLushort)(*(data[z]).face_list[i]).vertex_index[2];
		}

        /* Normals */
		for(int i=0; i<nrml; i++) {
		    normal_buffer_data[z][i*3] = (GLfloat)(*(data[z]).vertex_normal_list[i]).e[0];
			normal_buffer_data[z][i*3+1] = (GLfloat)(*(data[z]).vertex_normal_list[i]).e[1];
			normal_buffer_data[z][i*3+2] = (GLfloat)(*(data[z]).vertex_normal_list[i]).e[2];
		}

        /* Material indices */
		for(int i=0; i<indx; i++) {
			material_index_buffer_data[z][i] = (GLushort)(*(data[z]).face_list[i]).material_index;
		}

		/* texture coordinates */
		for (int i = 0; i < texv; i++) {
			texture_buffer_data[z][i*2] = (GLfloat)(*(data[z]).vertex_texture_list[i]).t[0];
			texture_buffer_data[z][i*2+1] = (GLfloat)(*(data[z]).vertex_texture_list[i]).t[1];
		}
	}
}


/******************************************************************
*
* Initialize
*
* This function is called to initialize rendering elements, setup
* vertex buffer objects, and to setup the vertex and fragment shader;
* meshes are loaded from files in OBJ format; data is copied from
* structures into vertex and index arrays
*
*******************************************************************/

void Initialize()
{   
    /* Load the object files */
    LoadObjFiles();

	/* Set background (clear) color to soft bluegreen */ 
    glClearColor(0.0, 0.2, 0.4, 0.0);
	
	/* Enable culling */
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

    /* Enable depth testing */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);    

    /* Setup vertex and (material) index buffer objects */
    SetupDataBuffers();

    /* Setup shaders and shader program */
    CreateShaderProgram();  
    
    /* Set projection transform */
    float fovy = 45.0;
    float aspect = 1.0; 
    float nearPlane = 1.0; 
    float farPlane = 50.0;
	ProjectionMatrix = perspective(fovy, aspect, nearPlane, farPlane);

    /* Set camera transform */
	ViewTransform = translate(mat4(1.0f), vec3(0.0f, -4.0f, -20.0f));
	ViewMatrix = ViewTransform * ViewMatrix;

	/* setup lights */
	lights[0].isEnabled = GL_TRUE;
	lights[0].type = 0; // light is point light
	lights[0].ambient[0] = 0.0f;
	lights[0].ambient[1] = 0.0f;
	lights[0].ambient[2] = 0.0f;
	lights[0].color = vec3 (360.0f, 1.0f, 1.0f); //red
	lights[0].position[0] = 0.0f;
	lights[0].position[1] = 2.0f;
	lights[0].position[2] = 0.0f;
	lights[0].attenuation = 0.05f;
	lights[0].intensity = 0.2f;

	lights[1].isEnabled = GL_TRUE;
	lights[1].type = 1; // light is spot light
	lights[1].ambient[0] = 0.0f;
	lights[1].ambient[1] = 0.0f;
	lights[1].ambient[2] = 0.0f;
	lights[1].color = vec3 (240.0f, 1.0f, 1.0f); //blue
	lights[1].position[0] = 0.0f;
	lights[1].position[1] = 0.0f;
	lights[1].position[2] = 0.0f;
	lights[1].coneDirection[0] = 0.0f;
	lights[1].coneDirection[1] = 1.0f;
	lights[1].coneDirection[2] = 0.0f;
	lights[1].coneCutOffAngleCos = cos(radians(20.0f)); //cutoff cone at 20 degrees to either side
	lights[1].attenuation = 0.5f;
	lights[1].intensity = .2f;

    lights[2].isEnabled = GL_TRUE;
	lights[2].type = 1; // light is point light
	lights[2].ambient[0] = 0.0f;
	lights[2].ambient[1] = 0.0f;
	lights[2].ambient[2] = 0.0f;
	lights[2].color = vec3 (0.0f, 0.0f, 1.0f); //white
	lights[2].position[0] = -3.0f;
	lights[2].position[1] = 1.0f;
	lights[2].position[2] = 0.0f;
    lights[2].coneDirection[0] = 3.0f;
	lights[2].coneDirection[1] = -1.0f;
	lights[2].coneDirection[2] = 0.0f;
	lights[2].coneCutOffAngleCos = cos(radians(20.0f)); //cutoff cone at 20 degrees to either side
	lights[2].attenuation = .2f;
	lights[2].intensity = .1f;

	//set the number of lights in shader
	GLuint light_count = glGetUniformLocation(ShaderProgram, "light_count");
	glUniform1i(light_count, NUM_LIGHT);
}
/******
*
* load textures
*
*******/
void loadTextures() {

	glEnable(GL_TEXTURE_2D);
 	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	int width, height, n;
	image = stbi_load("512X512.png", &width, &height, &n, 0);

	if (image == NULL) {
	    fprintf(stderr, "Could not load texture image\n");
	    exit(-1);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	stbi_image_free(image);
}

/******************************************************************
*
* main
*
* Main function to setup GLUT, GLEW, and enter rendering loop
*
*******************************************************************/

int main(int argc, char** argv)
{
    /* Initialize GLUT; set double buffered window and RGBA color model */
    glutInit(&argc, argv);
    glutInitContextVersion(3,3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(400, 400);
    glutCreateWindow("CG Proseminar - User Interaction");

    /* Initialize GL extension wrangler */
    glewExperimental = GL_TRUE;
    GLenum res = glewInit();
    if (res != GLEW_OK) 
    {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    /* Setup scene and rendering parameters */
    Initialize();

    /* Specify callback functions;enter GLUT event processing loop, 
     * handing control over to GLUT */
    glutIdleFunc(OnIdle);
    glutDisplayFunc(Display);
    glutKeyboardFunc(Keyboard); 
    glutMouseFunc(Mouse);
    glutMotionFunc(RotateCamera);  

	/* load all relevant textures */
	loadTextures();

	/* start loop */
    glutMainLoop();

    /* ISO C requires main to return int */
    return 0;
}
