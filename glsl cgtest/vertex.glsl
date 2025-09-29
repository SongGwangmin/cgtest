#version 330 core
layout (location = 0) in vec3 vPos;    // 정점 위치
layout (location = 1) in vec3 vColor;  // 정점 색상

out vec3 passColor; // 프래그먼트 셰이더로 색상 전달

void main()
{
    gl_Position = vec4(vPos, 1.0);
    passColor = vColor;
}