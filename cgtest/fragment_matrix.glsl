#version 330 core
 //--- out_Color: ���ؽ� ���̴������Է¹޴»���
//--- FragColor: ����� ������ �����������ӹ��۷����޵�.  
in  vec3 out_Color;
 out vec4 FragColor;
 void main(void) 
{
 //--- ���ؽ����̴����Լ����޹���
//--- ���� ���
FragColor = vec4 (out_Color, 1.0);
 }