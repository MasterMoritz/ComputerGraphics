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
