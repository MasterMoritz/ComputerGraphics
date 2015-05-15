#version 330 core

uniform samplerCube tex;

in vec3 tex_coord;

layout (location = 0) out vec4 color;

void main()
{
    color = texture(tex, tex_coord);
}
