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

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in int MaterialIndex;

out vec4 Position;  //the non-projected position
out vec3 Normal;
out int matInd;

void main()
{
    gl_Position = ProjectionMatrix*ViewMatrix*ModelMatrix*vec4(Position.x, Position.y, Position.z, 1.0);
    Position = ViewMatrix*ModelMatrix*vec4(Position.x, Position.y, Position.z, 1.0);
    Normal = normalize(NormalMatrix * vNormal);        
    matInd = MaterialIndex;
}
