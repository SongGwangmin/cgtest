#version 330 core
in vec3 passColor; // 버텍스 셰이더에서 받은 색상
out vec4 color;

void main ()
{
	color = vec4(passColor, 1.0); // 전달받은 색상 사용
}