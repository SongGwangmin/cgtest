#version 330 core

in vec3 outColor;
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D outTexture; 

void main()
{
    // outColor(정점 색상)이 아닌, 텍스처를 샘플링한 색상만 사용해야 합니다.
    FragColor = texture(outTexture, TexCoord); 
    // 만약 다음처럼 outColor와 섞는다면, outColor 값이 1.0, 1.0, 1.0이 아니면 색이 어두워집니다.
    // FragColor = texture(outTexture, TexCoord) * vec4(outColor, 1.0); 
}