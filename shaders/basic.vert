#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNor;
layout (location = 2) in vec3 aCol;
layout (location = 3) in vec2 aUV;

layout (location = 0) uniform mat4 aVP;

out vec3 vertexColor;
out vec2 vertexUV;

void main()
{
    vertexColor = aCol;
    vertexUV = aUV;
    gl_Position = aVP * vec4(aPos, 1.0);
}
