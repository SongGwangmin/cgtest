#version 330 core
 layout (location = 0) in vec3 
vPos; 
//--- ��ġ ����:attribute position 0
 layout (location = 1) in vec3 
vColor; 
//--- �÷� ����:attribute position 1
 out vec3 out_Color;
 uniform mat4 viewTransform;
 uniform mat4 modelTransform;

 void main(void) 
{
 //--- �����׸�Ʈ���̴���������
 gl_Position = viewTransform * modelTransform * vec4(vPos, 1.0); 
 out_Color = vColor;
 }