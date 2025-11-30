#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <vector>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <windows.h> // BITMAPFILEHEADER 등을 위해 필요

//--- 전역 변수
GLint width = 800, height = 800;
GLuint shaderProgramID;
GLuint VAO, VBO; // 객체용
GLuint bVAO, bVBO; // 배경용
GLuint textures[3]; // 0: 배경, 1: 육면체 면1, 2: 육면체 면2 (필요시 추가)

float angle = 0.0f;  // y축 회전
float xangle = 0.0f; // x축 회전
int drawShape = 0;   // 0: Cube, 1: Pyramid

//--- 함수 선언
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
void Keyboard(unsigned char key, int x, int y);
GLubyte* LoadDIBitmap(const char* filename, BITMAPINFO** info);
void InitTexture();
void InitBuffer();
void InitBackgroundBuffer();

// 셰이더 객체
GLuint vertexShader;
GLuint fragmentShader;

// 파일 읽기 헬퍼
char* filetobuf(const char* file) {
    FILE* fptr;
    long length;
    char* buf;
    fptr = fopen(file, "rb");
    if (!fptr) return NULL;
    fseek(fptr, 0, SEEK_END);
    length = ftell(fptr);
    buf = (char*)malloc(length + 1);
    fseek(fptr, 0, SEEK_SET);
    fread(buf, length, 1, fptr);
    fclose(fptr);
    buf[length] = 0;
    return buf;
}

//--- PDF  LoadDIBitmap 구현
GLubyte* LoadDIBitmap(const char* filename, BITMAPINFO** info) {
    FILE* fp;
    GLubyte* bits;
    int bitsize, infosize;
    BITMAPFILEHEADER header;

    // 1. 파일 열기 시도 (여기서 NULL이 나오면 경로/이름/권한 문제)
    if ((fp = fopen(filename, "rb")) == NULL) {
        printf("[ERROR] LoadDIBitmap: Failed to open file '%s'. Check path/name/permissions.\n", filename);
        return NULL;
    }
    printf("[DEBUG] LoadDIBitmap: File '%s' opened successfully.\n", filename);

    // 2. 파일 헤더 읽기 시도
    if (fread(&header, sizeof(BITMAPFILEHEADER), 1, fp) < 1) {
        printf("[ERROR] LoadDIBitmap: Failed to read file header.\n");
        fclose(fp); return NULL;
    }
    printf("[DEBUG] LoadDIBitmap: File header read (Type: %c%c).\n", (char)(header.bfType & 0xFF), (char)((header.bfType >> 8) & 0xFF));

    // 3. BMP 시그니처 확인 (BM인지 확인)
    if (header.bfType != 'MB') {
        printf("[ERROR] LoadDIBitmap: File is not a valid BMP format (Type: 0x%X).\n", header.bfType);
        fclose(fp); return NULL;
    }

    // 4. 정보 헤더 크기 계산 및 메모리 할당
    infosize = header.bfOffBits - sizeof(BITMAPFILEHEADER);
    if ((*info = (BITMAPINFO*)malloc(infosize)) == NULL) {
        printf("[ERROR] LoadDIBitmap: Failed to allocate memory for BITMAPINFO.\n");
        fclose(fp); return NULL;
    }

    // 5. 정보 헤더 읽기 시도
    if (fread(*info, 1, infosize, fp) < (unsigned int)infosize) {
        printf("[ERROR] LoadDIBitmap: Failed to read bitmap info header.\n");
        free(*info); fclose(fp); return NULL;
    }

    // 6. 이미지 데이터 크기 계산
    if ((bitsize = (*info)->bmiHeader.biSizeImage) == 0)
        bitsize = ((*info)->bmiHeader.biWidth * (*info)->bmiHeader.biBitCount + 7) / 8 * abs((*info)->bmiHeader.biHeight);

    // 7. 이미지 데이터 메모리 할당
    if ((bits = (unsigned char*)malloc(bitsize)) == NULL) {
        printf("[ERROR] LoadDIBitmap: Failed to allocate memory for image data.\n");
        free(*info); fclose(fp); return NULL;
    }

    // 8. 이미지 데이터 읽기 시도 (여기서 실패하면 파일이 잘렸거나 포맷 문제)
    if (fread(bits, 1, bitsize, fp) < (unsigned int)bitsize) {
        printf("[ERROR] LoadDIBitmap: Failed to read image data (Expected %d bytes).\n", bitsize);
        free(*info); free(bits); fclose(fp); return NULL;
    }

    printf("[DEBUG] LoadDIBitmap: File '%s' successfully loaded (Size: %d x %d).\n", filename, (*info)->bmiHeader.biWidth, (*info)->bmiHeader.biHeight);
    fclose(fp);
    return bits;
}

//--- 텍스처 초기화 [cite: 205-210]
//--- 텍스처 초기화 함수 (파일이 없으면 하얀색으로 대체)
// 전역 변수: 배열 크기를 7로 늘립니다.
// GLuint textures[7]; // 0: 배경, 1~6: 6개 면

void InitTexture() {
    BITMAPINFO* info;
    glGenTextures(7, textures); // 7개 텍스처 ID 생성 (0 ~ 6)

    // 파일 이름 배열 생성
    const char* filenames[7] = {
        "background.bmp", "face1.bmp", "face2.bmp",
        "face3.bmp", "face4.bmp", "face5.bmp", "face6.bmp"
    };

    for (int i = 0; i < 7; i++) {
        glBindTexture(GL_TEXTURE_2D, textures[i]);

        // 텍스처 파라미터 설정 (이전과 동일)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // 필터링 Linear로 변경
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // 필터링 Linear로 변경

        GLubyte* data = LoadDIBitmap(filenames[i], &info);

        if (data != NULL) {
            // 이미지 로드 성공
            printf("Loaded %s successfully.\n", filenames[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, info->bmiHeader.biWidth, info->bmiHeader.biHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);
            free(data);
            free(info);
        }
        else {
            // 이미지 로드 실패 시 대체 색상 (디버깅용)
            unsigned char color[3];
            if (i == 0) { // 배경: 회색
                color[0] = 50; color[1] = 50; color[2] = 50;
            }
            else if (i <= 3) { // 면 1~3: 빨강 계열
                color[0] = 255; color[1] = 0; color[2] = 0;
            }
            else { // 면 4~6: 파랑 계열
                color[0] = 0; color[1] = 0; color[2] = 255;
            }

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, color);
            printf("Failed to load %s. Using default color texture.\n", filenames[i]);
        }
    }
}

//--- 배경을 위한 버퍼 설정 (화면 가득 채우는 사각형)
void InitBackgroundBuffer() {
    // NDC 좌표계 전체를 덮는 사각형 (z = 0.999, 맨 뒤)
    // 포맷: x, y, z, r, g, b, s, t
    float bgVertices[] = {
        -1.0f, -1.0f, 0.999f,  1,1,1,  0.0f, 0.0f,
         1.0f, -1.0f, 0.999f,  1,1,1,  1.0f, 0.0f,
         1.0f,  1.0f, 0.999f,  1,1,1,  1.0f, 1.0f,
        -1.0f, -1.0f, 0.999f,  1,1,1,  0.0f, 0.0f,
         1.0f,  1.0f, 0.999f,  1,1,1,  1.0f, 1.0f,
        -1.0f,  1.0f, 0.999f,  1,1,1,  0.0f, 1.0f
    };

    glGenVertexArrays(1, &bVAO);
    glBindVertexArray(bVAO);
    glGenBuffers(1, &bVBO);
    glBindBuffer(GL_ARRAY_BUFFER, bVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bgVertices), bgVertices, GL_STATIC_DRAW);

    // Stride = 8 * sizeof(float) (3 pos + 3 color + 2 tex)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

//--- 객체(육면체/피라미드) 버퍼 설정
std::vector<float> objVertices;
void InitBuffer() {
    objVertices.clear();

    // 육면체 6면 정의 (x, y, z, r, g, b, s, t)
    float cube[] = {
        // Front face (z = 0.5)
        -0.5f, -0.5f,  0.5f, 1,0,0, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1,0,0, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1,0,0, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 1,0,0, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1,0,0, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 1,0,0, 0.0f, 1.0f,

        // Back face (z = -0.5)
        -0.5f, -0.5f, -0.5f, 0,1,0, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0,1,0, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 0,1,0, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0,1,0, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0,1,0, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 0,1,0, 0.0f, 1.0f,

         // Top face (y = 0.5)
         -0.5f,  0.5f, -0.5f, 0,0,1, 0.0f, 1.0f,
         -0.5f,  0.5f,  0.5f, 0,0,1, 0.0f, 0.0f,
          0.5f,  0.5f,  0.5f, 0,0,1, 1.0f, 0.0f,
         -0.5f,  0.5f, -0.5f, 0,0,1, 0.0f, 1.0f,
          0.5f,  0.5f,  0.5f, 0,0,1, 1.0f, 0.0f,
          0.5f,  0.5f, -0.5f, 0,0,1, 1.0f, 1.0f,

          // Bottom face (y = -0.5)
          -0.5f, -0.5f, -0.5f, 1,1,0, 0.0f, 0.0f,
           0.5f, -0.5f, -0.5f, 1,1,0, 1.0f, 0.0f,
           0.5f, -0.5f,  0.5f, 1,1,0, 1.0f, 1.0f,
          -0.5f, -0.5f, -0.5f, 1,1,0, 0.0f, 0.0f,
           0.5f, -0.5f,  0.5f, 1,1,0, 1.0f, 1.0f,
          -0.5f, -0.5f,  0.5f, 1,1,0, 0.0f, 1.0f,

          // Right face (x = 0.5)
           0.5f, -0.5f, -0.5f, 1,0,1, 1.0f, 0.0f,
           0.5f,  0.5f, -0.5f, 1,0,1, 1.0f, 1.0f,
           0.5f,  0.5f,  0.5f, 1,0,1, 0.0f, 1.0f,
           0.5f, -0.5f, -0.5f, 1,0,1, 1.0f, 0.0f,
           0.5f,  0.5f,  0.5f, 1,0,1, 0.0f, 1.0f,
           0.5f, -0.5f,  0.5f, 1,0,1, 0.0f, 0.0f,

           // Left face (x = -0.5)
           -0.5f, -0.5f, -0.5f, 0,1,1, 0.0f, 0.0f,
           -0.5f, -0.5f,  0.5f, 0,1,1, 1.0f, 0.0f,
           -0.5f,  0.5f,  0.5f, 0,1,1, 1.0f, 1.0f,
           -0.5f, -0.5f, -0.5f, 0,1,1, 0.0f, 0.0f,
           -0.5f,  0.5f,  0.5f, 0,1,1, 1.0f, 1.0f,
           -0.5f,  0.5f, -0.5f, 0,1,1, 0.0f, 1.0f
    };

    for (int i = 0; i < sizeof(cube) / sizeof(float); i++) objVertices.push_back(cube[i]);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, objVertices.size() * sizeof(float), &objVertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("Lab 30: Texture Mapping & Background");

    glewExperimental = GL_TRUE;
    glewInit();

    make_vertexShaders();
    make_fragmentShaders();
    shaderProgramID = make_shaderProgram();

    InitTexture();        // 텍스처 로드
    InitBackgroundBuffer(); // 배경 VBO 설정
    InitBuffer();           // 객체 VBO 설정 (초기엔 큐브)

    glEnable(GL_DEPTH_TEST);

    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMainLoop();
    return 0;
}

GLvoid drawScene()
{
    // 화면 지우기 색 (검정)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramID);

    unsigned int modelLoc = glGetUniformLocation(shaderProgramID, "modelTransform");
    unsigned int viewLoc = glGetUniformLocation(shaderProgramID, "viewTransform");
    unsigned int projLoc = glGetUniformLocation(shaderProgramID, "projectionTransform");
    unsigned int texLoc = glGetUniformLocation(shaderProgramID, "outTexture");

    glUniform1i(texLoc, 0); // Sampler0 사용

    // --- 1. 배경 그리기 (Depth Test 끄고 맨 뒤에 그리기) ---
    glDisable(GL_DEPTH_TEST);

    // 배경은 변환을 무시하므로 단위 행렬 사용
    glm::mat4 bgModel = glm::mat4(1.0f);
    glm::mat4 bgView = glm::mat4(1.0f);
    glm::mat4 bgProj = glm::mat4(1.0f);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(bgModel));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(bgView));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(bgProj));

    glBindVertexArray(bVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]); // 배경 텍스처
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // --- 2. 객체 그리기 (Depth Test 켜기) ---
    glEnable(GL_DEPTH_TEST);

    // 모델 변환 (회전)
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(xangle), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

    // [✅ 수정된 부분 1: 뷰 행렬] 카메라 위치를 (5, 5, 5)로 설정
    glm::vec3 cameraPos = glm::vec3(5.0f, 5.0f, 5.0f);     // Eye (시점)
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // Center (바라보는 지점)
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);     // Up (월드 상향 벡터)
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

    // [✅ 수정된 부분 2: 투영 행렬] 직교 투영(Orthographic) 사용
    float aspectRatio = (float)width / height;
    float size = 3.0f; // 뷰 볼륨의 절반 크기 (이 값을 조정하여 줌인/줌아웃 효과를 낼 수 있습니다.)

    // glm::ortho(left, right, bottom, top, near, far)
    glm::mat4 projection = glm::ortho(
        -size * aspectRatio,
        size * aspectRatio,
        -size,
        size,
        0.1f, // near
        15.0f // far (카메라 (5,5,5)에서 원점까지의 거리(약 8.66)를 포함하도록 충분히 크게 설정)
    );

    // 행렬 셰이더로 전송
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(texLoc, 0); // Sampler0 사용

    // 육면체 앞, 뒤, 위 (18개 정점) -> Texture 1 (빨강/face1)
    glBindTexture(GL_TEXTURE_2D, textures[1]); // Face 1 (Front)
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // 시작 정점: 6, 정점 수: 6, 텍스처 ID: 2
    glBindTexture(GL_TEXTURE_2D, textures[2]); // Face 2 (Back)
    glDrawArrays(GL_TRIANGLES, 6, 6);

    // 시작 정점: 12, 정점 수: 6, 텍스처 ID: 3
    glBindTexture(GL_TEXTURE_2D, textures[3]); // Face 3 (Top)
    glDrawArrays(GL_TRIANGLES, 12, 6);

    // 시작 정점: 18, 정점 수: 6, 텍스처 ID: 4
    glBindTexture(GL_TEXTURE_2D, textures[4]); // Face 4 (Bottom)
    glDrawArrays(GL_TRIANGLES, 18, 6);

    // 시작 정점: 24, 정점 수: 6, 텍스처 ID: 5
    glBindTexture(GL_TEXTURE_2D, textures[5]); // Face 5 (Right)
    glDrawArrays(GL_TRIANGLES, 24, 6);

    // 시작 정점: 30, 정점 수: 6, 텍스처 ID: 6
    glBindTexture(GL_TEXTURE_2D, textures[6]); // Face 6 (Left)
    glDrawArrays(GL_TRIANGLES, 30, 6);

    glutSwapBuffers();
}

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'c': // 육면체
        drawShape = 0;
        // InitBuffer에서 육면체 데이터로 갱신하는 로직 필요
        break;
    case 'p': // 사각뿔
        drawShape = 1;
        // InitBuffer에서 사각뿔 데이터로 갱신하는 로직 필요
        break;
    case 'x': xangle += 5.0f; break;
    case 'X': xangle -= 5.0f; break;
    case 'y': angle += 5.0f; break;
    case 'Y': angle -= 5.0f; break;
    case 's': // 리셋
        angle = 0.0f; xangle = 0.0f;
        break;
    case 'q': glutLeaveMainLoop(); break;
    }
    glutPostRedisplay();
}

GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
    width = w; height = h;
}

// 셰이더 컴파일 함수들 (vertex_projection.glsl 대신 vertex.glsl 사용 등 이름 맞출것)
void make_vertexShaders() {
    GLchar* vertexSource = filetobuf("vertex_texture.glsl");
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    // 에러 체크 생략
}
void make_fragmentShaders() {
    GLchar* fragmentSource = filetobuf("fragment_texture.glsl");
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    // 에러 체크 생략
}
GLuint make_shaderProgram() {
    GLuint shaderID = glCreateProgram();
    glAttachShader(shaderID, vertexShader);
    glAttachShader(shaderID, fragmentShader);
    glLinkProgram(shaderID);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glUseProgram(shaderID);
    return shaderID;
}