/******************************************************************
*
* RotatingCube.c
*
* Description: This example demonstrates a colored, rotating
* cube in shader-based OpenGL. The use of transformation
* matrices, perspective projection, and indexed triangle sets 
* are shown.
*
* Computer Graphics Proseminar SS 2015
* 
* Interactive Graphics and Simulation Group
* Institute of Computer Science
* University of Innsbruck
*
*******************************************************************/


/* Standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* OpenGL includes */
#include <GL/glew.h>
#include <GL/freeglut.h>


/* Local includes */
#include "LoadShader.h"   /* Provides loading function for shader code */
#include "Matrix.h"  

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

#ifndef NUM_OF_OBJECTS
	#define NUM_OF_OBJECTS 10
#endif

#ifndef POLE_HEIGHT
	#define POLE_HEIGHT 1
#endif

/*----------------------------------------------------------------*/

GLuint VAO[NUM_OF_OBJECTS];

/* Define handle to a vertex buffer object */
GLuint VBO[NUM_OF_OBJECTS];

/* Define handle to a color buffer object */
GLuint CBO[NUM_OF_OBJECTS]; 

/* Define handle to an index buffer object */
GLuint IBO[NUM_OF_OBJECTS];

/* Indices to vertex attributes; in this case positon and color */ 
enum DataID {vPosition = 0, vColor = 1}; 

/* Strings for loading and storing shader code */
static const char* VertexShaderString;
static const char* FragmentShaderString;

GLuint ShaderProgram;

float ProjectionMatrix[16]; /* Perspective projection matrix */
float ViewMatrix[16]; /* Camera view matrix */ 
float ModelMatrix[NUM_OF_OBJECTS][16]; /* Model matrix */

/* Transformation matrices for initial position */
float TranslateOrigin[16];
float TranslateDown[16];
float RotateX[16];
float RotateZ[16];
float InitialTransform[NUM_OF_OBJECTS][16];

/* Transformation matrices for MasterMoritz */
float T[16];
float R[16];

/*-------------ROOF------------*/

//pyramid vertices
GLfloat vertex_buffer_data[] = {
	0.0, 2.0,  0.0,//0
	2.0, 0.0,  2.0,//1
   -2.0, 0.0,  2.0,//2
	2.0, 0.0, -2.0,//3
   -2.0, 0.0, -2.0,//4
	
	3.0, 0.0,  3.0,//5
   -3.0, 0.0, -3.0,//6
   -3.0, 0.0,  3.0,//7
	3.0, 0.0, -3.0,//8
};   

//colors of the vertices 
GLfloat color_buffer_data[] = {
    0.0, 0.0, 1.0,
    1.0, 0.0, 1.0,
    1.0, 1.0, 1.0,
    0.0, 1.0, 1.0,
	0.0, 1.0, 1.0,

    0.0, 0.0, 1.0,
    1.0, 0.0, 1.0,
    1.0, 1.0, 1.0,
    0.0, 1.0, 1.0,
	0.0, 1.0, 1.0,
}; 

//4 triangles making up the pyramid and another 8 to flatten it with extensions
GLushort index_buffer_data[] = {
	//pyramid
    0, 1, 2,//front
	0, 3, 4,//back
    0, 1, 3,//right
    0, 4, 2,//left

	//flat extensions
	1, 5, 2, //front
	2, 7, 5, //front
	1, 5, 3, //right
	3, 8, 5, //right
	3, 8, 4, //back
	4, 6, 8, //back
	2, 7, 4, //left
	4, 7, 6, //left
};

/*-------------------FLOOR---------------------------*/

//cuboid vertices
GLfloat vertex_buffer_data_2[] = {
	-3.0, -POLE_HEIGHT,  3.0, //0
     3.0, -POLE_HEIGHT,  3.0, //1
     3.0,  0.3,          3.0, //2
    -3.0,  0.3,          3.0, //3
    -3.0, -POLE_HEIGHT, -3.0, //4
     3.0, -POLE_HEIGHT, -3.0, //5
     3.0,  0.3,         -3.0, //6
    -3.0,  0.3,         -3.0, //7
};   

//colors of the vertices
GLfloat color_buffer_data_2[] = {
    1.0, 0.5, 1.0,
    1.0, 0.7, 0.8,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    1.0, 0.5, 1.0,
    1.0, 0.7, 0.8,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
}; 

// 12 Triangles making up a flat cube
GLushort index_buffer_data_2[] = {
    0, 1, 2,
    2, 3, 0,
    1, 5, 6,
    6, 2, 1,
    7, 6, 5,
    5, 4, 7,
    4, 0, 3,
    3, 7, 4,
    4, 5, 1,
    1, 0, 4,
    3, 2, 6,
    6, 7, 3,
};

/*-------------------POLE1 WITH CUBE ON IT---------------------------*/

//vertices [XYZ]
GLfloat vertex_buffer_data_3[] = {
	//pole
	-0.2, -POLE_HEIGHT,  0.2, //0
     0.2, -POLE_HEIGHT,  0.2, //1
     0.2,  POLE_HEIGHT,  0.2, //2
    -0.2,  POLE_HEIGHT,  0.2, //3
    -0.2, -POLE_HEIGHT, -0.2, //4
     0.2, -POLE_HEIGHT, -0.2, //5
     0.2,  POLE_HEIGHT, -0.2, //6
    -0.2,  POLE_HEIGHT, -0.2, //7

	//cube
	-0.4,  POLE_HEIGHT,        0.4, //8
     0.4,  POLE_HEIGHT,        0.4, //9
     0.4,  POLE_HEIGHT + 0.8,  0.4, //10
    -0.4,  POLE_HEIGHT + 0.8,  0.4, //11
    -0.4,  POLE_HEIGHT,       -0.4, //12
     0.4,  POLE_HEIGHT,       -0.4, //13
     0.4,  POLE_HEIGHT + 0.8, -0.4, //14
    -0.4,  POLE_HEIGHT + 0.8, -0.4, //15
};   

//RGB color values
GLfloat color_buffer_data_3[] = {
    1.0, 0.5, 1.0,
    1.0, 0.7, 0.8,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    1.0, 0.5, 1.0,
    1.0, 0.7, 0.8,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,

    1.0, 0.5, 1.0,
    1.0, 0.7, 0.8,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    1.0, 0.5, 1.0,
    1.0, 0.7, 0.8,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
}; 

//Triangles
GLushort index_buffer_data_3[] = {
	//pole
    0, 1, 2,
    2, 3, 0,
    1, 5, 6,
    6, 2, 1,
    7, 6, 5,
    5, 4, 7,
    4, 0, 3,
    3, 7, 4,
    4, 5, 1,
    1, 0, 4,
    3, 2, 6,
    6, 7, 3,

	//cube
    8, 9, 10,
    10, 11, 8,
    9, 13, 14,
    14, 10, 9,
    15, 14, 13,
    13, 12, 15,
    12, 8, 11,
    11, 15, 12,
    12, 13, 9,
    9, 8, 12,
    11, 10, 14,
    14, 15, 11,
};

/*-------------------POLE2 WITH CUBE ON IT---------------------------*/

//vertices [XYZ]
GLfloat vertex_buffer_data_4[] = {
	//pole
	-0.2, -POLE_HEIGHT,  0.2, //0
     0.2, -POLE_HEIGHT,  0.2, //1
     0.2,  POLE_HEIGHT,  0.2, //2
    -0.2,  POLE_HEIGHT,  0.2, //3
    -0.2, -POLE_HEIGHT, -0.2, //4
     0.2, -POLE_HEIGHT, -0.2, //5
     0.2,  POLE_HEIGHT, -0.2, //6
    -0.2,  POLE_HEIGHT, -0.2, //7

	//cube
	-0.4,  POLE_HEIGHT,        0.4, //8
     0.4,  POLE_HEIGHT,        0.4, //9
     0.4,  POLE_HEIGHT + 0.8,  0.4, //10
    -0.4,  POLE_HEIGHT + 0.8,  0.4, //11
    -0.4,  POLE_HEIGHT,       -0.4, //12
     0.4,  POLE_HEIGHT,       -0.4, //13
     0.4,  POLE_HEIGHT + 0.8, -0.4, //14
    -0.4,  POLE_HEIGHT + 0.8, -0.4, //15
};   

//RGB color values
GLfloat color_buffer_data_4[] = {
    1.0, 0.5, 1.0,
    1.0, 0.7, 0.8,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    1.0, 0.5, 1.0,
    1.0, 0.7, 0.8,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,

    1.0, 0.5, 1.0,
    1.0, 0.7, 0.8,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    1.0, 0.5, 1.0,
    1.0, 0.7, 0.8,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
}; 

//Triangles
GLushort index_buffer_data_4[] = {
	//pole
    0, 1, 2,
    2, 3, 0,
    1, 5, 6,
    6, 2, 1,
    7, 6, 5,
    5, 4, 7,
    4, 0, 3,
    3, 7, 4,
    4, 5, 1,
    1, 0, 4,
    3, 2, 6,
    6, 7, 3,

	//cube
    8, 9, 10,
    10, 11, 8,
    9, 13, 14,
    14, 10, 9,
    15, 14, 13,
    13, 12, 15,
    12, 8, 11,
    11, 15, 12,
    12, 13, 9,
    9, 8, 12,
    11, 10, 14,
    14, 15, 11,
};

/*-------------------POLE3 WITH CUBE ON IT---------------------------*/

//vertices [XYZ]
GLfloat vertex_buffer_data_5[] = {
	//pole
	-0.2, -POLE_HEIGHT,  0.2, //0
     0.2, -POLE_HEIGHT,  0.2, //1
     0.2,  POLE_HEIGHT,  0.2, //2
    -0.2,  POLE_HEIGHT,  0.2, //3
    -0.2, -POLE_HEIGHT, -0.2, //4
     0.2, -POLE_HEIGHT, -0.2, //5
     0.2,  POLE_HEIGHT, -0.2, //6
    -0.2,  POLE_HEIGHT, -0.2, //7

	//cube
	-0.4,  POLE_HEIGHT,        0.4, //8
     0.4,  POLE_HEIGHT,        0.4, //9
     0.4,  POLE_HEIGHT + 0.8,  0.4, //10
    -0.4,  POLE_HEIGHT + 0.8,  0.4, //11
    -0.4,  POLE_HEIGHT,       -0.4, //12
     0.4,  POLE_HEIGHT,       -0.4, //13
     0.4,  POLE_HEIGHT + 0.8, -0.4, //14
    -0.4,  POLE_HEIGHT + 0.8, -0.4, //15
};   

//RGB color values
GLfloat color_buffer_data_5[] = {
    1.0, 0.5, 1.0,
    1.0, 0.7, 0.8,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    1.0, 0.5, 1.0,
    1.0, 0.7, 0.8,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,

    1.0, 0.5, 1.0,
    1.0, 0.7, 0.8,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    1.0, 0.5, 1.0,
    1.0, 0.7, 0.8,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
}; 

//Triangles
GLushort index_buffer_data_5[] = {
	//pole
    0, 1, 2,
    2, 3, 0,
    1, 5, 6,
    6, 2, 1,
    7, 6, 5,
    5, 4, 7,
    4, 0, 3,
    3, 7, 4,
    4, 5, 1,
    1, 0, 4,
    3, 2, 6,
    6, 7, 3,

	//cube
    8, 9, 10,
    10, 11, 8,
    9, 13, 14,
    14, 10, 9,
    15, 14, 13,
    13, 12, 15,
    12, 8, 11,
    11, 15, 12,
    12, 13, 9,
    9, 8, 12,
    11, 10, 14,
    14, 15, 11,
};

/*-------------------POLE4 WITH CUBE ON IT---------------------------*/

//vertices [XYZ]
GLfloat vertex_buffer_data_6[] = {
	//pole
	-0.2, -POLE_HEIGHT,  0.2, //0
     0.2, -POLE_HEIGHT,  0.2, //1
     0.2,  POLE_HEIGHT,  0.2, //2
    -0.2,  POLE_HEIGHT,  0.2, //3
    -0.2, -POLE_HEIGHT, -0.2, //4
     0.2, -POLE_HEIGHT, -0.2, //5
     0.2,  POLE_HEIGHT, -0.2, //6
    -0.2,  POLE_HEIGHT, -0.2, //7

	//cube
	-0.4,  POLE_HEIGHT,        0.4, //8
     0.4,  POLE_HEIGHT,        0.4, //9
     0.4,  POLE_HEIGHT + 0.8,  0.4, //10
    -0.4,  POLE_HEIGHT + 0.8,  0.4, //11
    -0.4,  POLE_HEIGHT,       -0.4, //12
     0.4,  POLE_HEIGHT,       -0.4, //13
     0.4,  POLE_HEIGHT + 0.8, -0.4, //14
    -0.4,  POLE_HEIGHT + 0.8, -0.4, //15
};     

//RGB color values
GLfloat color_buffer_data_6[] = {
    1.0, 0.5, 1.0,
    1.0, 0.7, 0.8,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    1.0, 0.5, 1.0,
    1.0, 0.7, 0.8,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,

    1.0, 0.5, 1.0,
    1.0, 0.7, 0.8,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    1.0, 0.5, 1.0,
    1.0, 0.7, 0.8,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
}; 

//Triangles
GLushort index_buffer_data_6[] = {
	//pole
    0, 1, 2,
    2, 3, 0,
    1, 5, 6,
    6, 2, 1,
    7, 6, 5,
    5, 4, 7,
    4, 0, 3,
    3, 7, 4,
    4, 5, 1,
    1, 0, 4,
    3, 2, 6,
    6, 7, 3,

	//cube
    8, 9, 10,
    10, 11, 8,
    9, 13, 14,
    14, 10, 9,
    15, 14, 13,
    13, 12, 15,
    12, 8, 11,
    11, 15, 12,
    12, 13, 9,
    9, 8, 12,
    11, 10, 14,
    14, 15, 11,
};

/*-------------------ROOF POLE 1---------------------------*/

//vertices [XYZ]
GLfloat vertex_buffer_data_7[] = {
	//pole
	-0.1, -2.5,  0.1, //0
     0.1, -2.5,  0.1, //1
     0.1,  2.5,  0.1, //2
    -0.1,  2.5,  0.1, //3
    -0.1, -2.5, -0.1, //4
     0.1, -2.5, -0.1, //5
     0.1,  2.5, -0.1, //6
    -0.1,  2.5, -0.1, //7
};     

//RGB color values
GLfloat color_buffer_data_7[] = {
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    0.4, 0.0, 1.0,
    0.4, 0.0, 1.0,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    0.4, 0.0, 1.0,
    0.4, 0.0, 1.0,
}; 

//Triangles
GLushort index_buffer_data_7[] = {
	//pole
    0, 1, 2,
    2, 3, 0,
    1, 5, 6,
    6, 2, 1,
    7, 6, 5,
    5, 4, 7,
    4, 0, 3,
    3, 7, 4,
    4, 5, 1,
    1, 0, 4,
    3, 2, 6,
    6, 7, 3,
};

/*-------------------ROOF POLE 2---------------------------*/

//vertices [XYZ]
GLfloat vertex_buffer_data_8[] = {
	//pole
	-0.1, -2.5,  0.1, //0
     0.1, -2.5,  0.1, //1
     0.1,  2.5,  0.1, //2
    -0.1,  2.5,  0.1, //3
    -0.1, -2.5, -0.1, //4
     0.1, -2.5, -0.1, //5
     0.1,  2.5, -0.1, //6
    -0.1,  2.5, -0.1, //7
};     

//RGB color values
GLfloat color_buffer_data_8[] = {
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    0.4, 0.0, 1.0,
    0.4, 0.0, 1.0,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    0.4, 0.0, 1.0,
    0.4, 0.0, 1.0,
}; 

//Triangles
GLushort index_buffer_data_8[] = {
	//pole
    0, 1, 2,
    2, 3, 0,
    1, 5, 6,
    6, 2, 1,
    7, 6, 5,
    5, 4, 7,
    4, 0, 3,
    3, 7, 4,
    4, 5, 1,
    1, 0, 4,
    3, 2, 6,
    6, 7, 3,
};

/*-------------------ROOF POLE 3---------------------------*/

//vertices [XYZ]
GLfloat vertex_buffer_data_9[] = {
	//pole
	-0.1, -2.5,  0.1, //0
     0.1, -2.5,  0.1, //1
     0.1,  2.5,  0.1, //2
    -0.1,  2.5,  0.1, //3
    -0.1, -2.5, -0.1, //4
     0.1, -2.5, -0.1, //5
     0.1,  2.5, -0.1, //6
    -0.1,  2.5, -0.1, //7
};     

//RGB color values
GLfloat color_buffer_data_9[] = {
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    0.4, 0.0, 1.0,
    0.4, 0.0, 1.0,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    0.4, 0.0, 1.0,
    0.4, 0.0, 1.0,
}; 

//Triangles
GLushort index_buffer_data_9[] = {
	//pole
    0, 1, 2,
    2, 3, 0,
    1, 5, 6,
    6, 2, 1,
    7, 6, 5,
    5, 4, 7,
    4, 0, 3,
    3, 7, 4,
    4, 5, 1,
    1, 0, 4,
    3, 2, 6,
    6, 7, 3,
};

/*-------------------ROOF POLE 4---------------------------*/

//vertices [XYZ]
GLfloat vertex_buffer_data_10[] = {
	//pole
	-0.1, -2.5,  0.1, //0
     0.1, -2.5,  0.1, //1
     0.1,  2.5,  0.1, //2
    -0.1,  2.5,  0.1, //3
    -0.1, -2.5, -0.1, //4
     0.1, -2.5, -0.1, //5
     0.1,  2.5, -0.1, //6
    -0.1,  2.5, -0.1, //7
};     

//RGB color values
GLfloat color_buffer_data_10[] = {
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    0.4, 0.0, 1.0,
    0.4, 0.0, 1.0,
    1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    0.4, 0.0, 1.0,
    0.4, 0.0, 1.0,
}; 

//Triangles
GLushort index_buffer_data_10[] = {
	//pole
    0, 1, 2,
    2, 3, 0,
    1, 5, 6,
    6, 2, 1,
    7, 6, 5,
    5, 4, 7,
    4, 0, 3,
    3, 7, 4,
    4, 5, 1,
    1, 0, 4,
    3, 2, 6,
    6, 7, 3,
};
/*----------------------------------------------------------------*/


/******************************************************************
*
* Display
*
* This function is called when the content of the window needs to be
* drawn/redrawn. It has been specified through 'glutDisplayFunc()';
* Enable vertex attributes, create binding between C program and 
* attribute name in shader
*
*******************************************************************/

void Display()
{
    /* Clear window; color specified in 'Initialize()' */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Associate program with shader matrices */
    GLint projectionUniform = glGetUniformLocation(ShaderProgram, "ProjectionMatrix");
    if (projectionUniform == -1) 
    {
        fprintf(stderr, "Could not bind uniform ProjectionMatrix\n");
	exit(-1);
    }
    glUniformMatrix4fv(projectionUniform, 1, GL_TRUE, ProjectionMatrix);
    
    GLint ViewUniform = glGetUniformLocation(ShaderProgram, "ViewMatrix");
    if (ViewUniform == -1) 
    {
        fprintf(stderr, "Could not bind uniform ViewMatrix\n");
        exit(-1);
    }
    glUniformMatrix4fv(ViewUniform, 1, GL_TRUE, ViewMatrix);
   
    GLint RotationUniform = glGetUniformLocation(ShaderProgram, "ModelMatrix");
    if (RotationUniform == -1) 
    {
        fprintf(stderr, "Could not bind uniform ModelMatrix\n");
        exit(-1);
    }

	/* Bind Vertex/Color/Index Buffers and draw objects */

	for (int i = 0; i < NUM_OF_OBJECTS; i++) {
		glEnableVertexAttribArray(vPosition);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
		glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(vColor);
		glBindBuffer(GL_ARRAY_BUFFER, CBO[i]);
		glVertexAttribPointer(vColor, 3, GL_FLOAT,GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[i]);

		GLint size; 
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

		
		glUniformMatrix4fv(RotationUniform, 1, GL_TRUE, ModelMatrix[i]); 

		/* Issue draw command, using indexed triangle list */
		glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);

		glDisableVertexAttribArray(vPosition);
		glDisableVertexAttribArray(vColor);
	}

    /* Swap between front and back buffer */ 
    glutSwapBuffers();
}


/******************************************************************
*
* OnIdle
*
* 
*
*******************************************************************/
float moves(double angle, double offset) {
	float tmp = sin((angle + offset) * (M_PI/180));
	if (tmp < 0) {
		tmp *= -1;
	}
	return tmp;
}

void OnIdle()
{
    float angle = (glutGet(GLUT_ELAPSED_TIME) / 1000.0) * (180.0/M_PI);

    /* Time dependent rotation */
    SetRotationY(angle, R);

	/* ROTATE ALL OBJECTS */
	for (int i = 0; i < NUM_OF_OBJECTS; i++) {
		MultiplyMatrix(R, InitialTransform[i], ModelMatrix[i]);
	}

	/* OBJECTS THAT ROTATE AND MOVE UP AND DOWN */
	//Poles
	int j = 0;
	for (int i = 2; i <= 5; i++) {
		SetTranslation(0.0, moves(angle, 20*j), 0.0, T);
    	MultiplyMatrix(T, ModelMatrix[i], ModelMatrix[i]);
		j++;
	}

    /* Request redrawing forof window content */  
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
    glGenBuffers(NUM_OF_OBJECTS, VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

    glGenBuffers(NUM_OF_OBJECTS, IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

    glGenBuffers(NUM_OF_OBJECTS, CBO);
    glBindBuffer(GL_ARRAY_BUFFER, CBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

	glGenVertexArrays(NUM_OF_OBJECTS, VAO);
	glBindVertexArray(VAO[0]);

	glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 3, GL_FLOAT,GL_FALSE, 0, 0); 

    /* Disable attributes */
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);   

	/* Floor */
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data_2), vertex_buffer_data_2, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data_2), index_buffer_data_2, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, CBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data_2), color_buffer_data_2, GL_STATIC_DRAW);

	glBindVertexArray(VAO[1]);

	glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 3, GL_FLOAT,GL_FALSE, 0, 0);

    /* Disable attributes */
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor); 

	/* POLE 1 */
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data_3), vertex_buffer_data_3, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data_3), index_buffer_data_3, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, CBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data_3), color_buffer_data_3, GL_STATIC_DRAW);

	glBindVertexArray(VAO[2]);

	glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 3, GL_FLOAT,GL_FALSE, 0, 0);

    /* Disable attributes */
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);

	/* POLE 2 */
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data_4), vertex_buffer_data_4, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data_4), index_buffer_data_4, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, CBO[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data_4), color_buffer_data_4, GL_STATIC_DRAW);

	glBindVertexArray(VAO[3]);

	glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 3, GL_FLOAT,GL_FALSE, 0, 0);

    /* Disable attributes */
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);

	/* POLE 3 */
    glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data_5), vertex_buffer_data_5, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[4]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data_5), index_buffer_data_5, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, CBO[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data_5), color_buffer_data_5, GL_STATIC_DRAW);

	glBindVertexArray(VAO[4]);

	glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 3, GL_FLOAT,GL_FALSE, 0, 0);

    /* Disable attributes */
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);

	/* POLE 4 */
    glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data_6), vertex_buffer_data_6, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[5]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data_6), index_buffer_data_6, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, CBO[5]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data_6), color_buffer_data_6, GL_STATIC_DRAW);

	glBindVertexArray(VAO[5]);

	glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 3, GL_FLOAT,GL_FALSE, 0, 0);

    /* Disable attributes */
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);

	/* ROOF_POLE 1 */
    glBindBuffer(GL_ARRAY_BUFFER, VBO[6]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data_7), vertex_buffer_data_7, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[6]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data_7), index_buffer_data_7, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, CBO[6]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data_7), color_buffer_data_7, GL_STATIC_DRAW);

	glBindVertexArray(VAO[6]);

	glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 3, GL_FLOAT,GL_FALSE, 0, 0);

    /* Disable attributes */
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);

	/* ROOF_POLE 2 */
    glBindBuffer(GL_ARRAY_BUFFER, VBO[7]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data_8), vertex_buffer_data_8, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[7]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data_8), index_buffer_data_8, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, CBO[7]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data_8), color_buffer_data_8, GL_STATIC_DRAW);

	glBindVertexArray(VAO[7]);

	glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 3, GL_FLOAT,GL_FALSE, 0, 0);

    /* Disable attributes */
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);

	/* ROOF_POLE 3 */
    glBindBuffer(GL_ARRAY_BUFFER, VBO[8]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data_9), vertex_buffer_data_9, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[8]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data_9), index_buffer_data_9, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, CBO[8]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data_9), color_buffer_data_9, GL_STATIC_DRAW);

	glBindVertexArray(VAO[8]);

	glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 3, GL_FLOAT,GL_FALSE, 0, 0);

    /* Disable attributes */
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);

	/* ROOF_POLE 4 */
    glBindBuffer(GL_ARRAY_BUFFER, VBO[9]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data_10), vertex_buffer_data_10, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[9]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data_10), index_buffer_data_10, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, CBO[9]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data_10), color_buffer_data_10, GL_STATIC_DRAW);

	glBindVertexArray(VAO[9]);

	glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 3, GL_FLOAT,GL_FALSE, 0, 0);

    /* Disable attributes */
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
   
}


/******************************************************************
*
* AddShader
*
* This function creates and adds individual shaders
*
*******************************************************************/

void AddShader(GLuint ShaderProgram, const char* ShaderCode, GLenum ShaderType)
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
    VertexShaderString = LoadShader("vertexshader.vs");
    FragmentShaderString = LoadShader("fragmentshader.fs");

    /* Separately add vertex and fragment shader to program */
    AddShader(ShaderProgram, VertexShaderString, GL_VERTEX_SHADER);
    AddShader(ShaderProgram, FragmentShaderString, GL_FRAGMENT_SHADER);

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
* Initialize
*
* This function is called to initialize rendering elements, setup
* vertex buffer objects, and to setup the vertex and fragment shader
*
*******************************************************************/

void Initialize(void)
{   
    /* Set background (clear) color to soft bluegreen */ 
    glClearColor(0.0, 0.2, 0.4, 0.0);

    /* Enable depth testing */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);    

    /* Setup vertex, color, and index buffer objects */
    SetupDataBuffers();

    /* Setup shaders and shader program */
    CreateShaderProgram();  

    /* Initialize matrices */
    SetIdentityMatrix(ProjectionMatrix);
    SetIdentityMatrix(ViewMatrix);

	for (int i = 0; i < NUM_OF_OBJECTS; i++) {
    	SetIdentityMatrix(ModelMatrix[i]);
	}

    /* Set projection transform */
    float fovy = 80.0;
    float aspect = 1.0; 
    float nearPlane = 1.0; 
    float farPlane = 50.0;
    SetPerspectiveMatrix(fovy, aspect, nearPlane, farPlane, ProjectionMatrix);

    /* Set viewing transform */
    float camera_disp = -10.0;
    SetTranslation(0.0, 0.0, camera_disp, ViewMatrix);

    /* Translate and rotate cube onto tip */
    SetTranslation(0, 2, 0, InitialTransform[0]);
	SetTranslation(0, -3, 0, InitialTransform[1]);
	SetTranslation(-1.6, -3, -1.6, InitialTransform[2]);
	SetTranslation(-1.6, -3,  1.6, InitialTransform[3]);
	SetTranslation( 1.6, -3,  1.6, InitialTransform[4]);
	SetTranslation( 1.6, -3, -1.6, InitialTransform[5]);
	SetTranslation(2.6, -0.5,-2.6, InitialTransform[6]);
	SetTranslation(-2.6, -0.5, 2.6, InitialTransform[7]);
	SetTranslation(-2.6, -0.5,-2.6, InitialTransform[8]);
	SetTranslation(2.6, -0.5, 2.6, InitialTransform[9]);
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
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(400, 400);
    glutCreateWindow("CG Proseminar - Rotating Cube");

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
    glutMainLoop();

    /* ISO C requires main to return int */
    return 0;
}
