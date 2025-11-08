#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <random>
#include <list>
#include <algorithm>
#include <cmath>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

#define MAXRECT 10 // 최대 사각형 개수
#define point 0
#define line 1
#define triangle 2
#define rectangle 3
#define pentagon 4
#define spherelevel 15
#define pi 3.14159265358979323846
#define PLATESIZE 150 // 평면 크기

std::random_device rd;

// random_device 를 통해 난수 생성 엔진을 초기화 한다.
std::mt19937 gen(rd());

std::uniform_int_distribution<int> dis(0, 256);
std::uniform_int_distribution<int> polyrandom(0, 24);
//std::uniform_int_distribution<int> numdis(0, windowWidth - rectspace);

//--- 아래 5개 함수는 사용자 정의 함수 임
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
void setupBuffers();
void TimerFunction(int value);
void drawOrbitsAndPlanets(glm::vec3 cameraPos, glm::vec3 cameraTarget, glm::vec3 cameraUp, float rotationAngle);

//--- 필요한 변수 선언
GLint width = 800, height = 800;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체
GLuint VAO, VBO; //--- 버텍스 배열 객체, 버텍스 버퍼 객체

int hidetoggle = 1; // 1. 은면제거
int wiretoggle = 0; // 0: 솔리드 모드, 1: 와이어프레임 모드
int culltoggle = 0; // 1. 뒷면 컬링 모드


// 카메라 변수
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 150.0f);      // 카메라 위치
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);      // 카메라 타겟

// AntiCube 크기 상수
const float ANTICUBE_SIZE = 60.0f;  // 한 변의 길이
const float ANTICUBE_HALF = ANTICUBE_SIZE / 2.0f;  // 반지름 (중심에서 면까지의 거리)

// Cube 위치와 크기를 저장하는 구조체
struct CubePos {
	float xStart;      // x 시작 좌표
	float size;        // 한 변의 길이
	float xend;
	float nowxpos;
};

// 3개의 큐브 정보 저장
CubePos cubepos[3] = {
	{-30.0f, 10.0f, 30.0f - 10.0f, -30.0f},   // cube1: x시작 -30, 크기 10
	{-30.0f, 15.0f, 30.0f - 15.0f, -30.0f},   // cube2: x시작 -30, 크기 15
	{-30.0f, 20.0f, 30.0f - 20.0f, -30.0f}    // cube3: x시작 -30, 크기 20
};

// Forward declaration
class polygon;
int mouse_dest = -1; // 마우스로 선택된 polygon 인덱스 저장
std::vector<float> allVertices;

float angle = 0.0f; // 회전 각도 (전역 변수)

// 육면체 클래스
class Cube {
private:
	glm::vec3 vertices[8]; // 8개의 꼭지점
	glm::vec3 color;       // 색상

public:
	// 생성자: 8개의 점을 받아서 육면체 생성
	Cube(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3,
		glm::vec3 v4, glm::vec3 v5, glm::vec3 v6, glm::vec3 v7,
		glm::vec3 col = glm::vec3(1.0f, 1.0f, 1.0f))
		: color(col)
	{
		vertices[0] = v0;
		vertices[1] = v1;
		vertices[2] = v2;
		vertices[3] = v3;
		vertices[4] = v4;
		vertices[5] = v5;
		vertices[6] = v6;
		vertices[7] = v7;
	}

	// 육면체의 정점 데이터를 VBO에 추가하는 함수
	void sendVertexData(std::vector<float>& vbo) {
		// 육면체는 6개의 면으로 구성되며, 각 면은 2개의 삼각형으로 구성
		// 면의 인덱스 순서 (반시계방향)
		int faceIndices[6][4] = {
			{0, 1, 2, 3}, // 앞면
			{4, 7, 6, 5}, // 뒷면
			{0, 1, 5, 4}, // 아랫면      
			{2, 6, 7, 3}, // 윗면
			{0, 3, 7, 4}, // 왼쪽면
			{1, 5, 6, 2}  // 오른쪽면
		};

		// 각 면을 2개의 삼각형으로 분할하여 VBO에 추가
		for (int face = 0; face < 6; ++face) {
			int* indices = faceIndices[face];

			// 첫 번째 삼각형 (0, 1, 2)
			addTriangle(vbo,
				vertices[indices[0]],
				vertices[indices[1]],
				vertices[indices[2]]);

			// 두 번째 삼각형 (0, 2, 3)
			addTriangle(vbo,
				vertices[indices[0]],
				vertices[indices[2]],
				vertices[indices[3]]);
		}
	}

	// 색상 설정
	void setColor(glm::vec3 col) {
		color = col;
	}

	// 특정 꼭지점 가져오기
	glm::vec3 getVertex(int index) const {
		if (index >= 0 && index < 8) {
			return vertices[index];
		}
		return glm::vec3(0.0f);
	}

	// 특정 꼭지점 설정
	void setVertex(int index, glm::vec3 v) {
		if (index >= 0 && index < 8) {
			vertices[index] = v;
		}
	}

private:
	// 삼각형 데이터를 VBO에 추가하는 헬퍼 함수
	void addTriangle(std::vector<float>& vbo,
		const glm::vec3& v0,
		const glm::vec3& v1,
		const glm::vec3& v2)
	{
		// 정점 0
		vbo.insert(vbo.end(), {
			v0.x, v0.y, v0.z,
			color.r, color.g, color.b
			});

		// 정점 1
		vbo.insert(vbo.end(), {
			v1.x, v1.y, v1.z,
			color.r, color.g, color.b
			});

		// 정점 2
		vbo.insert(vbo.end(), {
			v2.x, v2.y, v2.z,
			color.r, color.g, color.b
			});
	}
};

// 반대 방향 육면체 클래스 (시계 방향 와인딩)
class AntiCube : public Cube {
private:
	glm::vec3 faceColors[6]; // 각 면의 색상

public:
	// 생성자: 부모 클래스와 동일한 생성자
	AntiCube(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3,
		     glm::vec3 v4, glm::vec3 v5, glm::vec3 v6, glm::vec3 v7,
		     glm::vec3 col = glm::vec3(1.0f, 1.0f, 1.0f))
		: Cube(v0, v1, v2, v3, v4, v5, v6, v7, col)
	{
		// 기본적으로 모든 면을 같은 색상으로 초기화
		for (int i = 0; i < 6; ++i) {
			faceColors[i] = col;
		}
	}

	// 각 면의 색상을 개별적으로 설정하는 생성자
	AntiCube(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3,
		     glm::vec3 v4, glm::vec3 v5, glm::vec3 v6, glm::vec3 v7,
		     glm::vec3 colors[6])
		: Cube(v0, v1, v2, v3, v4, v5, v6, v7, glm::vec3(1.0f))
	{
		// 각 면의 색상 저장
		for (int i = 0; i < 6; ++i) {
			faceColors[i] = colors[i];
		}
	}

	// 육면체의 정점 데이터를 VBO에 추가하는 함수 (시계 방향)
	void sendVertexData(std::vector<float>& vbo) {
		// 육면체는 6개의 면으로 구성되며, 각 면은 2개의 삼각형으로 구성
		// 면의 인덱스 순서 (시계방향 - 반대 와인딩)
		int faceIndices[6][4] = {
			{0, 1, 2, 3}, // 앞면 (반대 순서)
			{4, 7, 6, 5}, // 뒷면 (반대 순서)
			{0, 4, 5, 1}, // 아랫면 (반대 순서)
			{2, 6, 7, 3}, // 윗면 (반대 순서)
			{0, 3, 7, 4}, // 왼쪽면 (반대 순서)
			{1, 5, 6, 2}  // 오른쪽면 (반대 순서)
		};

		// 각 면을 2개의 삼각형으로 분할하여 VBO에 추가
		for (int face = 0; face < 6; ++face) {
			int* indices = faceIndices[face];

			// 첫 번째 삼각형 (0, 1, 2) - 시계 방향
			addTriangleReverse(vbo,
				getVertex(indices[0]),
				getVertex(indices[1]),
				getVertex(indices[2]),
				faceColors[face]);

			// 두 번째 삼각형 (0, 2, 3) - 시계 방향
			addTriangleReverse(vbo,
				getVertex(indices[0]),
				getVertex(indices[2]),
				getVertex(indices[3]),
				faceColors[face]);
		}
	}

private:
	// 삼각형 데이터를 VBO에 추가하는 헬퍼 함수 (시계 방향)
	void addTriangleReverse(std::vector<float>& vbo,
		const glm::vec3& v0,
		const glm::vec3& v1,
		const glm::vec3& v2,
		const glm::vec3& color)
	{
		// 정점을 반대 순서로 추가 (v0, v2, v1)
		// 정점 0
		vbo.insert(vbo.end(), {
			v0.x, v0.y, v0.z,
			color.r, color.g, color.b
			});

		// 정점 2 (순서 바꿈)
		vbo.insert(vbo.end(), {
			v2.x, v2.y, v2.z,
			color.r, color.g, color.b
			});

		// 정점 1 (순서 바꿈)
		vbo.insert(vbo.end(), {
			v1.x, v1.y, v1.z,
			color.r, color.g, color.b
			});
	}
};



void Keyboard(unsigned char key, int x, int y);
void SpecialKeys(int key, int x, int y); // 특수 키(화살표 키) 콜백 함수 선언
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y); // 마우스 모션 콜백 함수 선언

char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb"); // Open file for reading
	if (!fptr) // Return NULL on failure
		return NULL;
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file
	length = ftell(fptr); // Find out how many bytes into the file we are
	buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file
	fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer
	fclose(fptr); // Close the file
	buf[length] = 0; // Null terminator
	return buf; // Return the buffer
}

void setupBuffers() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// 정점 속성 설정: 위치 (3개) + 색상 (3개) = 총 6개 float
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//width = 800;
	//height = 800;

	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Rectangle Rendering");
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();
	//--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
	make_vertexShaders(); //--- 버텍스 세이더 만들기
	make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
	shaderProgramID = make_shaderProgram();
	glEnable(GL_CULL_FACE);
	// 버퍼 설정
	setupBuffers();
	glEnable(GL_DEPTH_TEST);

	// 원점을 중심으로 ANTICUBE_SIZE 크기의 AntiCube 생성
	glm::vec3 colors[6] = {
		glm::vec3(125.0f / 255.0f, 125.0f / 255.0f, 125.0f / 255.0f), // 앞면: 회색
		glm::vec3(255.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f),     // 뒷면: 빨강
		glm::vec3(0.0f / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f),     // 아랫면: 초록
		glm::vec3(0.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f),     // 윗면: 파랑
		glm::vec3(255.0f / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f),   // 왼쪽면: 노랑
		glm::vec3(255.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f)    // 오른쪽면: 마젠타
	};

	AntiCube antiCube(
		glm::vec3(-ANTICUBE_HALF, -ANTICUBE_HALF, -ANTICUBE_HALF), // v0: 앞면 왼쪽 아래
		glm::vec3( ANTICUBE_HALF, -ANTICUBE_HALF, -ANTICUBE_HALF), // v1: 앞면 오른쪽 아래
		glm::vec3( ANTICUBE_HALF, -ANTICUBE_HALF,  ANTICUBE_HALF), // v2: 앞면 오른쪽 위
		glm::vec3(-ANTICUBE_HALF, -ANTICUBE_HALF,  ANTICUBE_HALF), // v3: 앞면 왼쪽 위
		glm::vec3(-ANTICUBE_HALF,  ANTICUBE_HALF, -ANTICUBE_HALF), // v4: 뒷면 왼쪽 아래
		glm::vec3( ANTICUBE_HALF,  ANTICUBE_HALF, -ANTICUBE_HALF), // v5: 뒷면 오른쪽 아래
		glm::vec3( ANTICUBE_HALF,  ANTICUBE_HALF,  ANTICUBE_HALF), // v6: 뒷면 오른쪽 위
		glm::vec3(-ANTICUBE_HALF,  ANTICUBE_HALF,  ANTICUBE_HALF), // v7: 뒷면 왼쪽 위
		colors
	);
	antiCube.sendVertexData(allVertices);

	// 첫 번째 Cube: 한 변의 길이 10, z = 20, 파랑색
	Cube cube1(
		glm::vec3(-30.0f, -30.0f, 20.0f), // v0: 앞면 왼쪽 아래
		glm::vec3(-20.0f, -30.0f, 20.0f), // v1: 앞면 오른쪽 아래
		glm::vec3(-20.0f, -30.0f, 30.0f), // v2: 앞면 오른쪽 위
		glm::vec3(-30.0f, -30.0f, 30.0f), // v3: 앞면 왼쪽 위
		glm::vec3(-30.0f, -20.0f, 20.0f), // v4: 뒷면 왼쪽 아래
		glm::vec3(-20.0f, -20.0f, 20.0f), // v5: 뒷면 오른쪽 아래
		glm::vec3(-20.0f, -20.0f, 30.0f), // v6: 뒷면 오른쪽 위
		glm::vec3(-30.0f, -20.0f, 30.0f), // v7: 뒷면 왼쪽 위
		glm::vec3(0.0f, 0.0f, 1.0f) // 파랑색
	);
	cube1.sendVertexData(allVertices);

	// 두 번째 Cube: 한 변의 길이 15, z = 0, 빨강색
	Cube cube2(
		glm::vec3(-30.0f, -30.0f, 0.0f),   // v0: 앞면 왼쪽 아래
		glm::vec3(-15.0f, -30.0f, 0.0f),   // v1: 앞면 오른쪽 아래
		glm::vec3(-15.0f, -30.0f, 15.0f),  // v2: 앞면 오른쪽 위
		glm::vec3(-30.0f, -30.0f, 15.0f),  // v3: 앞면 왼쪽 위
		glm::vec3(-30.0f, -15.0f, 0.0f),   // v4: 뒷면 왼쪽 아래
		glm::vec3(-15.0f, -15.0f, 0.0f),   // v5: 뒷면 오른쪽 아래
		glm::vec3(-15.0f, -15.0f, 15.0f),  // v6: 뒷면 오른쪽 위
		glm::vec3(-30.0f, -15.0f, 15.0f),  // v7: 뒷면 왼쪽 위
		glm::vec3(1.0f, 0.0f, 0.0f) // 빨강색
	);
	cube2.sendVertexData(allVertices);

	// 세 번째 Cube: 한 변의 길이 20, z = -20, 짙은 회색
	Cube cube3(
		glm::vec3(-30.0f, -30.0f, -20.0f),  // v0: 앞면 왼쪽 아래
		glm::vec3(-10.0f, -30.0f, -20.0f),  // v1: 앞면 오른쪽 아래
		glm::vec3(-10.0f, -30.0f, 0.0f),  // v2: 앞면 오른쪽 위
		glm::vec3(-30.0f, -30.0f, 0.0f),  // v3: 앞면 왼쪽 위
		glm::vec3(-30.0f, -10.0f, -20.0f),  // v4: 뒷면 왼쪽 아래
		glm::vec3(-10.0f, -10.0f, -20.0f),  // v5: 뒷면 오른쪽 아래
		glm::vec3(-10.0f, -10.0f, 0.0f),  // v6: 뒷면 오른쪽 위
		glm::vec3(-30.0f, -10.0f, 0.0f),  // v7: 뒷면 왼쪽 위
		glm::vec3(0.3f, 0.3f, 0.3f) // 짙은 회색 (RGB: 76, 76, 76)
	);
	cube3.sendVertexData(allVertices);

	

	//--- 세이더 프로그램 만들기
	glutDisplayFunc(drawScene); //--- 출력 콜백 함수
	glutReshapeFunc(Reshape);

	glutTimerFunc(25, TimerFunction, 1);

	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeys); // 특수 키 콜백 함수 등록

	glutMainLoop();
	return 0;
}

void make_vertexShaders()
{
	GLchar* vertexSource;
	//--- 버텍스 세이더 읽어 저장하고 컴파일 하기
	//--- filebuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
	vertexSource = filetobuf("vertex_projection.glsl");
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

void make_fragmentShaders()
{
	GLchar* fragmentSource;
	//--- 프래그먼트 세이더 읽어 저장하고 컴파일하기
	fragmentSource = filetobuf("fragment_matrix.glsl"); // 프래그세이더 읽어오기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

GLuint make_shaderProgram()
{
	GLint result;
	GLchar* errorLog = NULL;
	GLuint shaderID;
	shaderID = glCreateProgram(); //--- 세이더 프로그램 만들기
	glAttachShader(shaderID, vertexShader); //--- 세이더 프로그램에 버텍스 세이더 붙이기
	glAttachShader(shaderID, fragmentShader); //--- 세이더 프로그램에 프래그먼트 세이더 붙이기
	glLinkProgram(shaderID); //--- 세이더 프로그램 링크하기
	glDeleteShader(vertexShader); //--- 세이더 객체를 세이더 프로그램에 링크했음으로, 세이더 객체 자체는 삭제 가능
	glDeleteShader(fragmentShader);
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result); // ---세이더가 잘 연결되었는지 체크하기
	if (!result) {
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
		return false;
	}
	glUseProgram(shaderID); //--- 만들어진 세이더 프로그램 사용하기
	return shaderID;
}

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	GLfloat rColor, gColor, bColor;
	rColor = gColor = 1.0;
	bColor = 1.0; //--- 배경색을 흰색으로 설정
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 카메라 설정
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	// 셰이더 사용
	glUseProgram(shaderProgramID);

	// 투영 행렬 설정
	glm::mat4 projection = glm::perspective(
		glm::radians(45.0f),
		(float)width / (float)height,
		0.1f,
		300.0f
	);
	unsigned int projectionLocation = glGetUniformLocation(shaderProgramID, "projectionTransform");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

	// 뷰 행렬 설정
	glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
	unsigned int viewLocation = glGetUniformLocation(shaderProgramID, "viewTransform");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));

	// 모델 행렬 위치
	unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "modelTransform");

	// VBO 데이터 바인딩
	if (!allVertices.empty()) {
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float),
			allVertices.data(), GL_STATIC_DRAW);

		// 와이어프레임 모드 설정
		if (wiretoggle) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		int startVertex = 0;

		// AntiCube 그리기 (z축 회전 추가)
		glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, startVertex, 36); // 6면 * 2삼각형 * 3정점 = 36
		startVertex += 36;

		// Cube1 그리기 (x축 이동 후 z축 회전)
		model = glm::translate(glm::mat4(1.0f), glm::vec3(cubepos[0].nowxpos + 30.0f, 0.0f, 0.0f));
		glm::mat4 rotmodel = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
		model = rotmodel * model;
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, startVertex, 36);
		startVertex += 36;

		// Cube2 그리기 (x축 이동 후 z축 회전)
		model = glm::translate(glm::mat4(1.0f), glm::vec3(cubepos[1].nowxpos + 30.0f, 0.0f, 0.0f));
		model = rotmodel * model;
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, startVertex, 36);
		startVertex += 36;

		// Cube3 그리기 (x축 이동 후 z축 회전)
		model = glm::translate(glm::mat4(1.0f), glm::vec3(cubepos[2].nowxpos + 30.0f, 0.0f, 0.0f));
		model = rotmodel * model;
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, startVertex, 36);

		glBindVertexArray(0);
	}

	glutSwapBuffers(); // 화면에 출력하기
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}

void Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'q': // 프로그램 종료
		glutLeaveMainLoop();
		break;
	case 'Q': // 프로그램 종료
		glutLeaveMainLoop();
		break;
	case 'w': // 와이어프레임 모드 적용/해제
	{
		wiretoggle = !wiretoggle;
	}
	break;
	case 'z': // z축 양의 방향으로 카메라와 타겟 이동
	{
	}
	break;
	case 'Z': // z축 음의 방향으로 카메라와 타겟 이동
	{
	}
	break;
	case 'x': // x축 양의 방향으로 카메라와 타겟 이동
	{
	}
	break;
	case 'X': // x축 음의 방향으로 카메라와 타겟 이동
	{
	}
	break;
	case 'y': // y축 기준 양의 방향(반시계) 회전
	{
		
	}
	break;
	case 'Y': // y축 기준 음의 방향(시계) 회전
	{

	}
	break;
	case 'r': // 화면 중심 y축에 대하여 카메라 공전 (반시계)
	{
	
	}
	break;
	case 'R': // 화면 중심 y축에 대하여 카메라 공전 (시계)
	{
	
	}
	break;
	case 'h': // 은면제거 적용/해제
	{
		if (hidetoggle) {
			glEnable(GL_DEPTH_TEST);
			hidetoggle = 0;
		}
		else {
			glDisable(GL_DEPTH_TEST);
			hidetoggle = 1;
		}
	}
	break;
	case '*':
	{
		if (culltoggle == 0) {
			glEnable(GL_CULL_FACE);
			culltoggle = 1;
		}
		else {
			glDisable(GL_CULL_FACE);
			culltoggle = 0;
		}
	}
	break;
	case 't': // 중앙 몸체 y축 회전 토글
	{
		
	}
	break;
	case 'l': // 상부 몸체 위치 교환 토글
	{
	}
	break;
	case 'g': // 포신 y축 회전 토글
	{
		
	}
	break;
	case 'p': // 깃대 x축 회전 토글
	{
		
	}
	break;
	case 'a': // 카메라 자동 공전 토글
	{
		
	}
	break;
	case 'o': // 모든 토글 끄기
	case 'O':
	{
		
	}
	break;
	case 'c': // 탱크 초기화
	case 'C':
	{
		
	}
	break;
	default:
		break;
	}

	glutPostRedisplay();
}

void TimerFunction(int value)
{
	// angle에 따라 각 큐브의 nowxpos를 sin 함수로 업데이트
	for (int i = 0; i < 3; ++i) {
		// sin(angle)의 범위는 -1 ~ 1
		
		float sinValue = sin(angle) * -1;  // -1 ~ 1
		
		// -1 ~ 1을 0 ~ 1로 변환: (sinValue + 1) / 2
		// 0 ~ 1을 xStart ~ xend로 변환
		cubepos[i].nowxpos += sinValue;

		if(cubepos[i].nowxpos > cubepos[i].xend) {
			cubepos[i].nowxpos = cubepos[i].xend;
		}
		if(cubepos[i].nowxpos < cubepos[i].xStart) {
			cubepos[i].nowxpos = cubepos[i].xStart;
		}
	}

	glutPostRedisplay();
	glutTimerFunc(25, TimerFunction, 1);
}

void Motion(int x, int y) // 마우스 모션 콜백 함수
{

}

void SpecialKeys(int key, int x, int y) // 특수 키(화살표 키) 콜백 함수
{
	switch (key) {
	case GLUT_KEY_UP: // 위쪽 화살표 - z축 음의 방향으로 이동
		
		break;
	case GLUT_KEY_DOWN: // 아래쪽 화살표 - z축 양의 방향으로 이동
		
		break;
	case GLUT_KEY_LEFT: // 왼쪽 화살표 - angle 감소
		angle -= 0.1f;
		if (angle < -pi / 3) angle = -pi / 3;
		
		break;
	case GLUT_KEY_RIGHT: // 오른쪽 화살표 - angle 증가
		angle += 0.1f;
		if (angle > pi / 3) angle = pi / 3;
		break;
	}

	glutPostRedisplay();
}
