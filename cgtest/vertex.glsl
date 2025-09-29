#version 330 core
 layout (location = 0) in vec3 
vPos; 
//--- 위치 변수:attribute position 0
 layout (location = 1) in vec3 
vColor; 
//--- 컬러 변수:attribute position 1
 out vec3 out_Color;
 void main(void) 
{
 //--- 프래그먼트세이더에게전달
gl_Position = vec4 (vPos.x, vPos.y, vPos.z, 1.0);
 out_Color = vColor;
 }