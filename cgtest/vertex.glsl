#version 330 core
 layout (location = 0) in vec3 
vPos; 
//--- ��ġ ����:attribute position 0
 layout (location = 1) in vec3 
vColor; 
//--- �÷� ����:attribute position 1
 out vec3 out_Color;
 void main(void) 
{
 //--- �����׸�Ʈ���̴���������
gl_Position = vec4 (vPos.x, vPos.y, vPos.z, 1.0);
 out_Color = vColor;
 }