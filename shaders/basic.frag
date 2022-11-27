#version 430 core

in vec3 vertexColor;
in vec2 vertexUV;

out vec4 FragColor;

layout (location = 1) uniform sampler2D TexUV;

void main()
{
    FragColor = texture(TexUV, vertexUV) * 0.5f + vec4(vertexColor * 0.5f, 1.0f);
}
