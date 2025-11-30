#version 330 core

layout (location = 0) in vec3 vPos;       // 위치
layout (location = 1) in vec3 vColor;     // 색상
layout (location = 2) in vec2 vTexCoord;  // 텍스처 좌표

out vec3 outColor;
out vec2 TexCoord;

uniform mat4 modelTransform;
uniform mat4 viewTransform;
uniform mat4 projectionTransform;

void main()
{
    gl_Position = projectionTransform * viewTransform * modelTransform * vec4(vPos, 1.0);
    outColor = vColor;
    TexCoord = vTexCoord;
}