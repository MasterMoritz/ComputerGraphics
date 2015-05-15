#version 330 core

uniform mat4 tc_rotate;

layout (location = 0) in vec3 Position;

out vec3 tex_coord;

void main()
{
    gl_Position = tc_rotate * vec4(Position, 1.0);\n"
    tex_coord = Position;
}
