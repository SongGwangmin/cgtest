#version 330 core
layout (location = 0) in vec3 vPos;    // ���� ��ġ
layout (location = 1) in vec3 vColor;  // ���� ����

out vec3 passColor; // �����׸�Ʈ ���̴��� ���� ����

void main()
{
    gl_Position = vec4(vPos, 1.0);
    passColor = vColor;
}