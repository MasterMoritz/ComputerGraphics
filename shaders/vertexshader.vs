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

uniform mat4 PVM_Matrix;
uniform mat4 VM_Matrix;
uniform mat4 NormalMatrix;

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in int MaterialIndex;

out vec4 Position;  //the non-projected position
out vec3 Normal;
flat out int materialIndex;

void main()
{
    gl_Position = PVM_Matrix*vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);
    Position = VM_Matrix*vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);
	vec4 n = normalize(NormalMatrix * vec4(vNormal, 1.0));
    Normal = vec3(n.x, n.y, n.z);        
    materialIndex = MaterialIndex;
}
