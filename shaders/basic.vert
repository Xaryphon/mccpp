#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;

layout (location = 0) uniform mat4 aVP;

out vec3 vertexColor;

void main()
{
    vertexColor = aCol;
    gl_Position = aVP * vec4(aPos, 1.0);
}
