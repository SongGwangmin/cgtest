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
int opentoggle = 0; // 0: 문 닫힘, 1: 문 열림


// 카메라 변수
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 150.0f);      // 카메라 위치
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);      // 카메라 타겟

// AntiCube 크기 상수
const float ANTICUBE_SIZE = 60.0f;  // 한 변의 길이
const float ANTICUBE_HALF = ANTICUBE_SIZE / 2.0f;  // 반지름 (중심에서 면까지의 거리)
const float minBound = -ANTICUBE_HALF + 3.0f;
const float maxBound = ANTICUBE_HALF - 3.0f;
const float cubeSpeed = 0.1f; // 큐브 이동 속도

// AABB 구조체 정의
struct AABB {
	glm::vec3 min;  // 최소 좌표 (왼쪽 아래 뒤)
	glm::vec3 max;  // 최대 좌표 (오른쪽 위 앞)
};

// 전체 3D AABB 충돌 체크
bool checkAABBCollision(const AABB& a, const AABB& b) {
	return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
		   (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
		   (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

// X, Z축만 충돌 체크 (Y축 제외)
bool checkAABBCollisionXZ(const AABB& a, const AABB& b) {
	return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
		   (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

// Y축만 충돌 체크
bool checkAABBCollisionY(const AABB& a, const AABB& b) {
	return (a.min.y <= b.max.y && a.max.y >= b.min.y);
}


// Cube 위치와 크기를 저장하는 구조체
struct CubePos {
	float xStart;      // x 시작 좌표
	float size;        // 한 변의 길이
	float xend;
	float nowxpos;
	float nowzpos;
};

// 3개의 큐브 정보 저장
CubePos cubepos[3] = {
	{-30.0f, 10.0f, 30.0f - 10.0f, -30.0f, 0.0f},   // cube1: x시작 -30, 크기 10
	{-30.0f, 15.0f, 30.0f - 15.0f, -30.0f, 0.0f},   // cube2: x시작 -30, 크기 15
	{-30.0f, 20.0f, 30.0f - 20.0f, -30.0f, 0.0f}    // cube3: x시작 -30, 크기 20
};

// 3개의 큐브 AABB 정보 저장
AABB cubeAABB[3];

// Player 구조체 정의
struct Player {
	glm::vec3 centerPos;     // 중심 좌표
	glm::vec3 size;          // 크기 (폭, 높이, 깊이)
	glm::vec3 velocity;      // 속도 (물리 시스템용)
	bool isOnGround;         // 바닥에 있는지 여부
	
	// AABB 계산 함수
	AABB getAABB() const {
		AABB box;
		box.min = centerPos - size / 2.0f;  // 중심에서 반 크기만큼 뺌
		box.max = centerPos + size / 2.0f;  // 중심에서 반 크기만큼 더함
		return box;
	}
};


// Player 전역 변수
Player player;

// Forward declaration
class polygon;
std::vector<float> allVertices;

float angle = 0.0f; // 회전 각도 (전역 변수)
float openangle = 0.0f; // EnRjd 각도 (전역 변수)
float yangle = 0.0f; // Y축 회전 각도 (전역 변수)

// 구의 위치 저장 (5개)
glm::vec3 spherePositions[5];
glm::vec3 spheredelta[5];

int xmouse = 400;

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
			{0, 4, 5, 1}, // 아랫면      
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
			{2, 3, 7, 6}, // 윗면 (반대 순서)
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
void rotatetimer(int value);
void SpecialKeys(int key, int x, int y); // 특수 키(화살표 키) 콜백 함수 선언
void Mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			// 마우스 버튼을 누를 때 현재 위치 저장
			xmouse = x;
		}
	}
}
void Motion(int x, int y) { // 마우스 모션 콜백 함수 선언


}
void Mousemove(int x, int y) {
	xmouse = x;
}

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
		glm::vec3(ANTICUBE_HALF, -ANTICUBE_HALF, -ANTICUBE_HALF), // v1: 앞면 오른쪽 아래
		glm::vec3(ANTICUBE_HALF, -ANTICUBE_HALF, ANTICUBE_HALF), // v2: 앞면 오른쪽 위
		glm::vec3(-ANTICUBE_HALF, -ANTICUBE_HALF, ANTICUBE_HALF), // v3: 앞면 왼쪽 위
		glm::vec3(-ANTICUBE_HALF, ANTICUBE_HALF, -ANTICUBE_HALF), // v4: 뒷면 왼쪽 아래
		glm::vec3(ANTICUBE_HALF, ANTICUBE_HALF, -ANTICUBE_HALF), // v5: 뒷면 오른쪽 아래
		glm::vec3(ANTICUBE_HALF, ANTICUBE_HALF, ANTICUBE_HALF), // v6: 뒷면 오른쪽 위
		glm::vec3(-ANTICUBE_HALF, ANTICUBE_HALF, ANTICUBE_HALF), // v7: 뒷면 왼쪽 위
		colors
	);
	antiCube.sendVertexData(allVertices);

	// 큐브들의 랜덤한 시작 위치 설정
	std::uniform_real_distribution<float> cube1XDis(-30.0f, 30.0f - 10.0f);  // cube1: 크기 10
	std::uniform_real_distribution<float> cube1ZDis(-30.0f, 30.0f - 10.0f);
	
	std::uniform_real_distribution<float> cube2XDis(-30.0f, 30.0f - 15.0f);  // cube2: 크기 15
	std::uniform_real_distribution<float> cube2ZDis(-30.0f, 30.0f - 15.0f);
	
	std::uniform_real_distribution<float> cube3XDis(-30.0f, 30.0f - 20.0f);  // cube3: 크기 20
	std::uniform_real_distribution<float> cube3ZDis(-30.0f, 30.0f - 20.0f);

	// Cube1의 랜덤 시작점 계산
	float cube1StartX = cube1XDis(gen);
	float cube1StartZ = cube1ZDis(gen);
	cubepos[0].nowxpos = cube1StartX;
	cubepos[0].nowzpos = cube1StartZ;

	Cube cube1(
		glm::vec3(cube1StartX, -30.0f, cube1StartZ), // v0: 앞면 왼쪽 아래
		glm::vec3(cube1StartX + 10.0f, -30.0f, cube1StartZ), // v1: 앞면 오른쪽 아래
		glm::vec3(cube1StartX + 10.0f, -30.0f, cube1StartZ + 10.0f), // v2: 앞면 오른쪽 위
		glm::vec3(cube1StartX, -30.0f, cube1StartZ + 10.0f), // v3: 앞면 왼쪽 위
		glm::vec3(cube1StartX, -20.0f, cube1StartZ), // v4: 뒷면 왼쪽 아래
		glm::vec3(cube1StartX + 10.0f, -20.0f, cube1StartZ), // v5: 뒷면 오른쪽 아래
		glm::vec3(cube1StartX + 10.0f, -20.0f, cube1StartZ + 10.0f), // v6: 뒷면 오른쪽 위
		glm::vec3(cube1StartX, -20.0f, cube1StartZ + 10.0f), // v7: 뒷면 왼쪽 위
		glm::vec3(0.0f, 0.0f, 1.0f) // 파랑색
	);
	cube1.sendVertexData(allVertices);

	// Cube1의 AABB 정보 저장
	cubeAABB[0].min = glm::vec3(cube1StartX, -30.0f, cube1StartZ);
	cubeAABB[0].max = glm::vec3(cube1StartX + 10.0f, -20.0f, cube1StartZ + 10.0f);

	// Cube2의 랜덤 시작점 계산
	float cube2StartX = cube2XDis(gen);
	float cube2StartZ = cube2ZDis(gen);
	cubepos[1].nowxpos = cube2StartX;
	cubepos[1].nowzpos = cube2StartZ;

	// 두 번째 Cube: 한 변의 길이 15, 빨강색
	Cube cube2(
		glm::vec3(cube2StartX, -30.0f, cube2StartZ),   // v0: 앞면 왼쪽 아래
		glm::vec3(cube2StartX + 15.0f, -30.0f, cube2StartZ),   // v1: 앞면 오른쪽 아래
		glm::vec3(cube2StartX + 15.0f, -30.0f, cube2StartZ + 15.0f),  // v2: 앞면 오른쪽 위
		glm::vec3(cube2StartX, -30.0f, cube2StartZ + 15.0f),  // v3: 앞면 왼쪽 위
		glm::vec3(cube2StartX, -15.0f, cube2StartZ),   // v4: 뒷면 왼쪽 아래
		glm::vec3(cube2StartX + 15.0f, -15.0f, cube2StartZ),   // v5: 뒷면 오른쪽 아래
		glm::vec3(cube2StartX + 15.0f, -15.0f, cube2StartZ + 15.0f),  // v6: 뒷면 오른쪽 위
		glm::vec3(cube2StartX, -15.0f, cube2StartZ + 15.0f),  // v7: 뒷면 왼쪽 위
		glm::vec3(1.0f, 0.0f, 0.0f) // 빨강색
	);
	cube2.sendVertexData(allVertices);

	// Cube2의 AABB 정보 저장
	cubeAABB[1].min = glm::vec3(cube2StartX, -30.0f, cube2StartZ);
	cubeAABB[1].max = glm::vec3(cube2StartX + 15.0f, -15.0f, cube2StartZ + 15.0f);

	// Cube3의 랜덤 시작점 계산
	float cube3StartX = cube3XDis(gen);
	float cube3StartZ = cube3ZDis(gen);
	cubepos[2].nowxpos = cube3StartX;
	cubepos[2].nowzpos = cube3StartZ;

	// 세 번째 Cube: 한 변의 길이 20, 짙은 회색
	Cube cube3(
		glm::vec3(cube3StartX, -30.0f, cube3StartZ),  // v0: 앞면 왼쪽 아래
		glm::vec3(cube3StartX + 20.0f, -30.0f, cube3StartZ),  // v1: 앞면 오른쪽 아래
		glm::vec3(cube3StartX + 20.0f, -30.0f, cube3StartZ + 20.0f),  // v2: 앞면 오른쪽 위
		glm::vec3(cube3StartX, -30.0f, cube3StartZ + 20.0f),  // v3: 앞면 왼쪽 위
		glm::vec3(cube3StartX, -10.0f, cube3StartZ),  // v4: 뒷면 왼쪽 아래
		glm::vec3(cube3StartX + 20.0f, -10.0f, cube3StartZ),  // v5: 뒷면 오른쪽 아래
		glm::vec3(cube3StartX + 20.0f, -10.0f, cube3StartZ + 20.0f),  // v6: 뒷면 오른쪽 위
		glm::vec3(cube3StartX, -10.0f, cube3StartZ + 20.0f),  // v7: 뒷면 왼쪽 위
		glm::vec3(0.3f, 0.3f, 0.3f) // 짙은 회색 (RGB: 76, 76, 76)
	);
	cube3.sendVertexData(allVertices);

	// Cube3의 AABB 정보 저장
	cubeAABB[2].min = glm::vec3(cube3StartX, -30.0f, cube3StartZ);
	cubeAABB[2].max = glm::vec3(cube3StartX + 20.0f, -10.0f, cube3StartZ + 20.0f);

	// Player 초기화 (0, 0, 0 중심에 한 변의 길이 8인 정육면체)
	player.centerPos = glm::vec3(0.0f, 0.0f, 0.0f);
	player.size = glm::vec3(8.0f, 8.0f, 8.0f);
	player.velocity = glm::vec3(cubeSpeed, 0.0f, 0.0f);  // x축 방향으로 초기 속도 0.2 설정
	player.isOnGround = false;

	// Player Cube 생성 (중심이 0,0,0이고 한 변의 길이가 8)
	Cube playerCube(
		glm::vec3(-4.0f, -4.0f, -4.0f), // v0: 앞면 왼쪽 아래
		glm::vec3(4.0f, -4.0f, -4.0f),  // v1: 앞면 오른쪽 아래
		glm::vec3(4.0f, -4.0f, 4.0f),   // v2: 앞면 오른쪽 위
		glm::vec3(-4.0f, -4.0f, 4.0f),  // v3: 앞면 왼쪽 위
		glm::vec3(-4.0f, 4.0f, -4.0f),  // v4: 뒷면 왼쪽 아래
		glm::vec3(4.0f, 4.0f, -4.0f),   // v5: 뒷면 오른쪽 아래
		glm::vec3(4.0f, 4.0f, 4.0f),    // v6: 뒷면 오른쪽 위
		glm::vec3(-4.0f, 4.0f, 4.0f),   // v7: 뒷면 왼쪽 위
		glm::vec3(1.0f, 1.0f, 0.0f)     // 노란색
	);
	playerCube.sendVertexData(allVertices);

	Cube playerbodyCube(
		glm::vec3(-4.0f, -4.0f, -4.0f), // v0: 앞면 왼쪽 아래
		glm::vec3(4.0f, -4.0f, -4.0f),  // v1: 앞면 오른쪽 아래
		glm::vec3(4.0f, -4.0f, 4.0f),   // v2: 앞면 오른쪽 위
		glm::vec3(-4.0f, -4.0f, 4.0f),  // v3: 앞면 왼쪽 위
		glm::vec3(-4.0f, 4.0f, -4.0f),  // v4: 뒷면 왼쪽 아래
		glm::vec3(4.0f, 4.0f, -4.0f),   // v5: 뒷면 오른쪽 아래
		glm::vec3(4.0f, 4.0f, 4.0f),    // v6: 뒷면 오른쪽 위
		glm::vec3(-4.0f, 4.0f, 4.0f),   // v7: 뒷면 왼쪽 위
		glm::vec3(0.0f, 1.0f, 0.0f)     // 초록
	);
	playerbodyCube.sendVertexData(allVertices);

	//--- 세이더 프로그램 만들기

	glutDisplayFunc(drawScene); //--- 출력 콜백 함수
	glutReshapeFunc(Reshape);

	glutTimerFunc(25, TimerFunction, 1);

	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeys); // 특수 키 콜백 함수 등록
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion); // 마우스 모션 콜백 함수 등록
	glutPassiveMotionFunc(Mousemove); // 마우스 이동 콜백 함수 등록 (버튼 누르지 않고 이동)

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
		glm::mat4 yrote = glm::rotate(glm::mat4(1.0f), yangle, glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 gototheorigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, openangle, 0.0f));

		glm::mat4 model = glm::mat4(1.0f);

		model = yrote * model;

		glm::mat4 firstmodel = model * gototheorigin;


		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

		glDrawArrays(GL_TRIANGLES, startVertex, 6); // 6면 * 2삼각형 * 3정점 = 36
		startVertex += 6;
		glDrawArrays(GL_TRIANGLES, startVertex, 6); // 6면 * 2삼각형 * 3정점 = 36
		startVertex += 6;
		glDrawArrays(GL_TRIANGLES, startVertex, 6); // 6면 * 2삼각형 * 3정점 = 36
		startVertex += 6;

		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(firstmodel));
		//glDrawArrays(GL_TRIANGLES, startVertex, 6); // qkekr
		startVertex += 6;

		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, startVertex, 6); // 6면 * 2삼각형 * 3정점 = 36
		startVertex += 6;
		
		glDrawArrays(GL_TRIANGLES, startVertex, 6); // 6면 * 2삼각형 * 3정점 = 36
		startVertex += 6;

		model = glm::mat4(1.0f);
		glm::mat4 rotmodel = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
		model = yrote * rotmodel * model;
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, startVertex, 36);
		startVertex += 36;

		// Cube2 그리기 (x축 이동 후 z축 회전)
		model = glm::mat4(1.0f);
		model = yrote * rotmodel * model;
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, startVertex, 36);
		startVertex += 36;

		// Cube3 그리기 (x축 이동 후 z축 회전)
		model = glm::mat4(1.0f);
		model = yrote * rotmodel * model;
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, startVertex, 36);

		startVertex += 36;

		// Player Cube 그리기 (중심 위치로 이동)
		model = glm::mat4(1.0f);

		glm::mat4 headscale = glm::scale(glm::mat4(1.0f), glm::vec3(0.30f, 0.3f, 0.3f));
		glm::mat4 headmodel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.8f, 0.0f));

		model = glm::translate(model, player.centerPos);  // Player 중심 위치로 이동
		model = yrote * model * headmodel * headscale;  // Y축 회전 적용
		//model = yrote * model
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, startVertex, 36);

		startVertex += 36;

		headscale = glm::scale(glm::mat4(1.0f), glm::vec3(0.50f, 0.5f, 0.5f));
		headmodel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));


		model = glm::mat4(1.0f);
		model = glm::translate(model, player.centerPos);  // Player 중심 위치로 이동
		//model = yrote * model;  // Y축 회전 적용
		model = yrote * model * headmodel * headscale;  // Y축 회전 적용
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
	case ' ': // 스페이스바 - 점프
	{
		
			player.velocity.y = 1.0f;  // 점프 속도 설정
			player.isOnGround = false;
			std::cout << "Jump!" << std::endl;
		
	}
	break;
	case 'w': // 와이어프레임 모드 적용/해제
	{
		player.velocity.z = -cubeSpeed;
		player.velocity.x = 0.0f;
	}
	break;
	case 'a': // 와이어프레임 모드 적용/해제
	{
		player.velocity.x = -cubeSpeed;
		player.velocity.z = 0.0f;
	}
	break;
	case 's': // 와이어프레임 모드 적용/해제
	{
		player.velocity.z = cubeSpeed;
		player.velocity.x = 0.0f;
	}
	break;
	case 'd': // 와이어프레임 모드 적용/해제
	{
		player.velocity.x = cubeSpeed;
		player.velocity.z = 0.0f;
	}
	break;
	case 'z': // z축 양의 방향으로 카메라와 타겟 이동
	{
		cameraPos = glm::vec3(0.0f, 0.0f, cameraPos.z + 1);      // 카메라 위치
	}
	break;
	case 'Z': // z축 음의 방향으로 카메라와 타겟 이동
	{
		cameraPos = glm::vec3(0.0f, 0.0f, cameraPos.z - 1);      // 카메라 위치
	}
	break;
	case 'y': // y축 기준 양의 방향(반시계) 회전
	{
		yangle += 0.1f;
	}
	break;
	case 'Y': // y축 기준 음의 방향(시계) 회전
	{

		yangle -= 0.1f;
	}
	break;
	case 'b':
	case 'B':
	{
		std::uniform_real_distribution<float> posDis(-ANTICUBE_HALF + 3, ANTICUBE_HALF - 3);
		std::uniform_real_distribution<float> angleDis(0.0f, 2.0f * pi);

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
	case 'o':
	case 'O':
	{
		opentoggle = 1;
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

	default:
		break;
	}

	glutPostRedisplay();
}

void TimerFunction(int value)
{
	// 물리 시스템 - 중력 적용
	const float GRAVITY = 0.05f;
	const float GROUND_Y = -30.0f;
	
	// Player의 현재 AABB 계산
	AABB playerAABB = player.getAABB();
	
	// 바닥이나 큐브와 충돌하지 않으면 중력 적용
	bool onGround = false;
	
	// 중력 적용
	player.velocity.y -= GRAVITY;  // 중력 가속도

	if(player.velocity.y < -2.0f) {
		player.velocity.y = -2.0f; // 최대 낙하 속도 제한
	}

	// 위치 업데이트 시도
	glm::vec3 newPos = player.centerPos + player.velocity;
	
	// 새로운 위치에서의 AABB 계산
	AABB newAABB;
	newAABB.min = newPos - player.size / 2.0f;
	newAABB.max = newPos + player.size / 2.0f;

	// Cube들과의 충돌 체크
	for (int i = 0; i < 3; i++) {
		// XZ 평면에서 겹치는지 먼저 확인
		if (checkAABBCollisionXZ(newAABB, cubeAABB[i])) {
			// Y축 충돌 체크
			if (checkAABBCollisionY(newAABB, cubeAABB[i])) {
				// 아래로 떨어지는 중인 경우 (velocity.y < 0)
				if (player.velocity.y <= 0.0f) {
					// 큐브 위에 착지
					if (cubeAABB[i].max.y - (newAABB.min.y) < 2.5f) {
						newPos.y = cubeAABB[i].max.y + player.size.y / 2.0f;
						player.velocity.y = 0.0f;
						player.isOnGround = true;
						onGround = true;
						std::cout << "Player landed on Cube " << (i + 1) << "!" << std::endl;
					}
					else {
						// 옆면 충돌 - 속도 반전하고 위치는 충돌 전으로 유지
						player.velocity.x *= -1.0f;
						player.velocity.z *= -1.0f;
						newPos = player.centerPos;  // 원래 위치로 되돌림
						std::cout << "Hit side of Cube " << (i + 1) << "!" << std::endl;
					}
					break;
				}
			}
		}
	}
	
	// 위치 업데이트
	player.centerPos = newPos;
	
	// 바닥 아래로 떨어지지 않도록 제한
	playerAABB = player.getAABB();
	if (playerAABB.min.y < GROUND_Y) {
		player.centerPos.y = GROUND_Y + player.size.y / 2.0f;
		player.velocity.y = 0.0f;
		player.isOnGround = true;
	}

	// AntiCube 경계 체크 및 속도 반전
	if (playerAABB.min.x <= -ANTICUBE_HALF) {
		player.velocity.x = cubeSpeed;
		player.centerPos.x = -ANTICUBE_HALF + player.size.x / 2.0f;  // 경계 밖으로 나가지 않도록
	}
	else if (playerAABB.max.x >= ANTICUBE_HALF) {
		player.velocity.x = -cubeSpeed;
		player.centerPos.x = ANTICUBE_HALF - player.size.x / 2.0f;
	}

	if (playerAABB.min.z <= -ANTICUBE_HALF) {
		player.velocity.z = cubeSpeed;
		player.centerPos.z = -ANTICUBE_HALF + player.size.z / 2.0f;
	}
	else if (playerAABB.max.z >= ANTICUBE_HALF) {
		player.velocity.z = -cubeSpeed;
		player.centerPos.z = ANTICUBE_HALF - player.size.z / 2.0f;
	}

	// angle에 따라 각 큐브의 nowxpos를 sin 함수로 업데이트
	

	if (opentoggle) {
		openangle += 1.0f;;
		
	}




	glutPostRedisplay();
	glutTimerFunc(25, TimerFunction, 1);
}


void SpecialKeys(int key, int x, int y) // 특수 키(화살표 키) 콜백 함수
{
	const float MOVE_SPEED = 1.0f;
	
	switch (key) {
	case GLUT_KEY_UP: // 위쪽 화살표 - z축 음의 방향으로 이동
		player.centerPos.z -= MOVE_SPEED;
		break;
		
	case GLUT_KEY_DOWN: // 아래쪽 화살표 - z축 양의 방향으로 이동
		player.centerPos.z += MOVE_SPEED;
		break;
		
	case GLUT_KEY_LEFT: // 왼쪽 화살표 - x축 음의 방향으로 이동
		player.centerPos.x -= MOVE_SPEED;
		break;
		
	case GLUT_KEY_RIGHT: // 오른쪽 화살표 - x축 양의 방향으로 이동
		player.centerPos.x += MOVE_SPEED;
		break;
	}

	glutPostRedisplay();
}
