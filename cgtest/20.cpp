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

// 탱크 애니메이션 토글 변수
int bodyRotateToggle = 0;    // t: 중앙 몸체 y축 회전
int turretSwapToggle = 0;    // l: 상부 몸체 위치 교환
int barrelRotateToggle = 0;  // g: 포신 y축 회전 (양쪽 반대 방향)
int flagRotateToggle = 0;    // p: 깃대 x축 회전 (양쪽 반대 방향)

// 애니메이션 변수
float turretSwapTime = 0.0f;           // 포탑 위치 교환 애니메이션 시간 (0~1)
glm::vec3 turretSwapStartPos(0.0f);    // 포탑 위치 교환 시작 위치
float mixstartpos = 18.0f;             // 선형보간 시작 위치
float mixendpos = -18.0f;              // 선형보간 끝 위치

// Forward declaration
class polygon;
int mouse_dest = -1; // 마우스로 선택된 polygon 인덱스 저장
std::vector<float> allVertices;

float angle = 0.0f; // 회전 각도

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
			{4, 5, 6, 7}, // 뒷면
			{0, 1, 5, 4}, // 아랫면
			{2, 3, 7, 6}, // 윗면
			{0, 3, 7, 4}, // 왼쪽면
			{1, 2, 6, 5}  // 오른쪽면
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

// 탱크 클래스
class Tank {
private:
	glm::vec3 position;      // 탱크의 좌표 (x, y, z)
	float bodyAngle;         // 몸체의 각도 (y축 회전)
	float turretAngle;       // 포탑의 각도 (y축 회전, 몸체 기준 상대 각도)
	glm::vec3 turretpos;     // 포신의 위치 (x, y, z)
	float flagAngle;         // 깃대의 각도 (z축 회전)
	int flagUp;            // 깃대가 올라가는 중인지 여부
	
public:
	// 생성자
	Tank(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f))
		: position(pos), bodyAngle(0.0f), turretAngle(2.0f), 
		  turretpos(glm::vec3(0.0f, 0.0f, 0.0f)), flagAngle(0.0f)
	{
		flagUp = 1;
	}
	
	// 탱크 좌표 설정
	void setPosition(glm::vec3 pos) {
		position = pos;
	}
	
	// 탱크 좌표 가져오기
	glm::vec3 getPosition() const {
		return position;
	}
	
	// 몸체 각도 설정
	void setBodyAngle(float angle) {
		bodyAngle = angle;
	}
	
	// 몸체 각도 가져오기
	float getBodyAngle() const {
		return bodyAngle;
	}
	
	// 포탑 각도 설정 (몸체 기준 상대 각도)
	void setTurretAngle(float angle) {
		turretAngle = angle;
	}
	
	// 포탑 각도 가져오기
	float getTurretAngle() const {
		return turretAngle;
	}
	
	// 포신 위치 설정
	void setTurretPos(glm::vec3 pos) {
		turretpos = pos;
	}
	
	// 포신 위치 가져오기
	glm::vec3 getTurretPos() const {
		return turretpos;
	}
	
	// 깃대 각도 설정
	void setFlagAngle(float angle) {
		flagAngle = angle;
	}
	
	// 깃대 각도 가져오기
	float getFlagAngle() const {
		return flagAngle;
	}
	
	// 탱크 이동
	void move(glm::vec3 delta) {
		position += delta;
	}
	
	// 몸체 회전
	void rotateBody(float delta) {
		bodyAngle += delta;
	}
	
	// 포탑 회전
	void rotateTurret(float delta) {
		turretAngle += delta;
	}
	
	// 포신 위치 이동
	void moveTurret(glm::vec3 delta) {
		turretpos += delta;
	}
	
	// 깃대 회전
	void rotateFlag(float delta) {
		if (flagAngle < -18.0f || 18.0f < flagAngle) {
			flagUp = -flagUp;
		}
		flagAngle += delta * flagUp;
	}
	
	// 변환 행렬 반환 함수들
	
	// 탱크 위치 이동 행렬
	glm::mat4 tankTranslate(int inverse) const {
		return glm::translate(glm::mat4(1.0f), position);
	}
	
	// 몸체 회전 행렬 (탱크 이동 포함)
	glm::mat4 bodyRotate(int inverse) const {
		glm::mat4 matr = glm::rotate(glm::mat4(1.0f), glm::radians(bodyAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		return tankTranslate(inverse) * matr;
	}
	
	// 포탑 회전 및 이동 행렬 (몸체 회전 포함)
	glm::mat4 turretRotateAndTranslate(int inverse) const {
		glm::mat4 matr;
		if (inverse) {
			// 역변환: x값만 반대로 한 위치로 이동
			glm::vec3 turretposInv = glm::vec3(-turretpos.x, turretpos.y, turretpos.z);
			glm::mat4 rotationInv = glm::rotate(glm::mat4(1.0f), -glm::radians(turretAngle), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 translationInv = glm::translate(glm::mat4(1.0f), turretposInv);
			matr = translationInv * rotationInv;
		}
		else {
			// 정변환: 이동 -> 회전
			glm::mat4 translation = glm::translate(glm::mat4(1.0f), turretpos);
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(turretAngle), glm::vec3(0.0f, 1.0f, 0.0f));
			matr = translation * rotation;
		}
		return bodyRotate(inverse) * matr;
	}
	
	// 깃대 회전 행렬 (포탑 변환 포함)
	glm::mat4 flagRotate(int inverse) const {
		glm::mat4 matr;
		if (inverse) {
			matr = glm::rotate(glm::mat4(1.0f), -glm::radians(flagAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		}
		else {
			matr = glm::rotate(glm::mat4(1.0f), glm::radians(flagAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		}
		glm::mat4 matr2;
		matr2 = glm::translate(glm::mat4(1.0f), glm::vec3(0, -27, 0)); // 깃대 위치로 이동 (필요시 조정)

		return turretRotateAndTranslate(inverse) * matr2 * matr;
	}
};

Tank tank(glm::vec3(0.0f, 0.0f, 0.0f)); // 첫 번째 탱크

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

	// 버퍼 설정
	setupBuffers();
	glEnable(GL_DEPTH_TEST);
	

	// y=-50 평면에 회색 사각형 추가
	float planeY = -50.0f;
	glm::vec3 planeColor = glm::vec3(100.0f / 255.0f, 100.0f / 255.0f, 100.0f / 255.0f); // RGB(100,100,100)
	
	// 첫 번째 삼각형
	allVertices.insert(allVertices.end(), {
		-PLATESIZE, planeY, -PLATESIZE,
		planeColor.r, planeColor.g, planeColor.b
	});
	allVertices.insert(allVertices.end(), {
		PLATESIZE, planeY, -PLATESIZE,
		planeColor.r, planeColor.g, planeColor.b
	});
	allVertices.insert(allVertices.end(), {
		PLATESIZE, planeY, PLATESIZE,
		planeColor.r, planeColor.g, planeColor.b
	});
	
	// 두 번째 삼각형
	allVertices.insert(allVertices.end(), {
		-PLATESIZE, planeY, -PLATESIZE,
		planeColor.r, planeColor.g, planeColor.b
	});
	allVertices.insert(allVertices.end(), {
		PLATESIZE, planeY, PLATESIZE,
		planeColor.r, planeColor.g, planeColor.b
	});
	allVertices.insert(allVertices.end(), {
		-PLATESIZE, planeY, PLATESIZE,
		planeColor.r, planeColor.g, planeColor.b
	});

	// 육면체 생성 및 VBO에 추가
	// x: -30~30, y: -50~-40, z: -15~15 (y축과 z축 교환)
	Cube cube(
		glm::vec3(-30.0f, -50.0f, -15.0f), // v0: 앞면 왼쪽 아래
		glm::vec3( 30.0f, -50.0f, -15.0f), // v1: 앞면 오른쪽 아래
		glm::vec3( 30.0f, -50.0f,  15.0f), // v2: 앞면 오른쪽 위
		glm::vec3(-30.0f, -50.0f,  15.0f), // v3: 앞면 왼쪽 위
		glm::vec3(-30.0f, -40.0f, -15.0f), // v4: 뒷면 왼쪽 아래
		glm::vec3( 30.0f, -40.0f, -15.0f), // v5: 뒷면 오른쪽 아래
		glm::vec3( 30.0f, -40.0f,  15.0f), // v6: 뒷면 오른쪽 위
		glm::vec3(-30.0f, -40.0f,  15.0f), // v7: 뒷면 왼쪽 위
		glm::vec3(180.0f / 255.0f, 180.0f / 255.0f, 180.0f / 255.0f) // RGB(180,180,180)
	);
	cube.sendVertexData(allVertices);

	// 두 번째 육면체 생성 및 VBO에 추가
	// x: -20~20, y: -40~-32, z: -13~13
	Cube cube2(
		glm::vec3(-20.0f, -40.0f, -13.0f), // v0: 앞면 왼쪽 아래
		glm::vec3( 20.0f, -40.0f, -13.0f), // v1: 앞면 오른쪽 아래
		glm::vec3( 20.0f, -40.0f,  13.0f), // v2: 앞면 오른쪽 위
		glm::vec3(-20.0f, -40.0f,  13.0f), // v3: 앞면 왼쪽 위
		glm::vec3(-20.0f, -32.0f, -13.0f), // v4: 뒷면 왼쪽 아래
		glm::vec3( 20.0f, -32.0f, -13.0f), // v5: 뒷면 오른쪽 아래
		glm::vec3( 20.0f, -32.0f,  13.0f), // v6: 뒷면 오른쪽 위
		glm::vec3(-20.0f, -32.0f,  13.0f), // v7: 뒷면 왼쪽 위
		glm::vec3(255.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f) // RGB(255,0,255) - 마젠타
	);
	cube2.sendVertexData(allVertices);

	// 세 번째 육면체 생성 및 VBO에 추가
	// x: -12~12, y: -33~-25, z: -15~15
	Cube cube3(
		glm::vec3(-12.0f, -33.0f, -15.0f), // v0: 앞면 왼쪽 아래
		glm::vec3( 12.0f, -33.0f, -15.0f), // v1: 앞면 오른쪽 아래
		glm::vec3( 12.0f, -33.0f,  15.0f), // v2: 앞면 오른쪽 위
		glm::vec3(-12.0f, -33.0f,  15.0f), // v3: 앞면 왼쪽 위
		glm::vec3(-12.0f, -25.0f, -15.0f), // v4: 뒷면 왼쪽 아래
		glm::vec3( 12.0f, -25.0f, -15.0f), // v5: 뒷면 오른쪽 아래
		glm::vec3( 12.0f, -25.0f,  15.0f), // v6: 뒷면 오른쪽 위
		glm::vec3(-12.0f, -25.0f,  15.0f), // v7: 뒷면 왼쪽 위
		glm::vec3(0.0f / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f) // RGB(0,255,0) - 녹색
	);
	cube3.sendVertexData(allVertices);

	// 네 번째 육면체 생성 및 VBO에 추가
	// x: -3~3, y: -32~-26, z: 0~26
	Cube cube4(
		glm::vec3(-3.0f, -32.0f, 0.0f), // v0: 앞면 왼쪽 아래
		glm::vec3( 3.0f, -32.0f, 0.0f), // v1: 앞면 오른쪽 아래
		glm::vec3( 3.0f, -32.0f, 26.0f), // v2: 앞면 오른쪽 위
		glm::vec3(-3.0f, -32.0f, 26.0f), // v3: 앞면 왼쪽 위
		glm::vec3(-3.0f, -26.0f, 0.0f), // v4: 뒷면 왼쪽 아래
		glm::vec3( 3.0f, -26.0f, 0.0f), // v5: 뒷면 오른쪽 아래
		glm::vec3( 3.0f, -26.0f, 26.0f), // v6: 뒷면 오른쪽 위
		glm::vec3(-3.0f, -26.0f, 26.0f), // v7: 뒷면 왼쪽 위
		glm::vec3(255.0f / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f) // RGB(255,255,0) - 노란색
	);
	cube4.sendVertexData(allVertices);

	// 다섯 번째 육면체 생성 및 VBO에 추가
	// x: -3~3, y: 0~18, z: -3~3
	Cube cube5(
		glm::vec3(-3.0f, 0.0f, -3.0f), // v0: 앞면 왼쪽 아래
		glm::vec3( 3.0f, 0.0f, -3.0f), // v1: 앞면 오른쪽 아래
		glm::vec3( 3.0f, 0.0f,  3.0f), // v2: 앞면 오른쪽 위
		glm::vec3(-3.0f, 0.0f,  3.0f), // v3: 앞면 왼쪽 위
		glm::vec3(-3.0f, 18.0f, -3.0f), // v4: 뒷면 왼쪽 아래
		glm::vec3( 3.0f, 18.0f, -3.0f), // v5: 뒷면 오른쪽 아래
		glm::vec3( 3.0f, 18.0f,  3.0f), // v6: 뒷면 오른쪽 위
		glm::vec3(-3.0f, 18.0f,  3.0f), // v7: 뒷면 왼쪽 위
		glm::vec3(180.0f / 255.0f, 180.0f / 255.0f, 0.0f / 255.0f) // RGB(180,180,0) - 올리브색
	);
	cube5.sendVertexData(allVertices);

	tank.setTurretPos(glm::vec3(18.0f, 0.0f, 0.0f));

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
	//--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
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
	glm::vec3 cameraPos = glm::vec3(0.0f, 10.0f, 150.0f);
	glm::vec3 cameraTarget = glm::vec3(cameraPos.x, cameraPos.y - 10.0f, cameraPos.z - 150.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	// 셰이더 사용
	glUseProgram(shaderProgramID);
	
	// 투영 행렬 설정
	glm::mat4 projection = glm::perspective(
		glm::radians(60.0f),
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
	
	// 모델 행렬 설정
	glm::mat4 model = glm::mat4(1.0f);
	unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "modelTransform");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	
	// VBO 데이터 바인딩 및 그리기
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
		
		// 전체 정점 개수 계산 (각 정점은 6개 float: xyz + rgb)
		int vertexCount = allVertices.size() / 6;

		// 바닥
		int startIndex = 0;
		glDrawArrays(GL_TRIANGLES, startIndex, 6);
		startIndex += 6;

		// 탱크 좌표
		model = tank.tankTranslate(0);
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, startIndex, 36);
		startIndex += 36;

		// 탱크 몸체
		model = tank.bodyRotate(0);
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, startIndex, 36);
		startIndex += 36;

		// 탱크 포탑
		model = tank.turretRotateAndTranslate(0);
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, startIndex, 36 * 2);
		model = tank.turretRotateAndTranslate(1);
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, startIndex, 36 * 2);
		startIndex += 36 * 2;


		// 탱크 깃대
		model = tank.flagRotate(0);
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, startIndex, 36);
		model = tank.flagRotate(1);
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, startIndex, 36);
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
	case 'z': // z축 시계방향 회전
	{
		
	}
	break;
	case 'Z': // z축 반시계방향 회전
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
		bodyRotateToggle = !bodyRotateToggle;
	}
	break;
	case 'l': // 상부 몸체 위치 교환 토글
	{
		if (!turretSwapToggle) {
			// 애니메이션 시작: 현재 위치 저장
			//turretSwapStartPos = tank.getTurretPos();
			//turretSwapTime = 0.0f;
		}
		turretSwapToggle = !turretSwapToggle;
	}
	break;
	case 'g': // 포신 y축 회전 토글
	{
		barrelRotateToggle = !barrelRotateToggle;
	}
	break;
	case 'p': // 깃대 x축 회전 토글
	{
		flagRotateToggle = !flagRotateToggle;
	}
	break;
	default:
		break;
	}

	glutPostRedisplay();
}

void TimerFunction(int value)
{
	// 몸체 회전 애니메이션
	if (bodyRotateToggle) {
		tank.rotateBody(1.0f); // 매 프레임마다 1도씩 회전
	}

	// 포탑 회전 애니메이션 (양쪽 반대 방향)
	if (barrelRotateToggle) {
		tank.rotateTurret(1.0f); // 매 프레임마다 1도씩 회전
	}

	// 깃대 회전 애니메이션 (양쪽 반대 방향)
	if (flagRotateToggle) {
		tank.rotateFlag(1.0f); // 매 프레임마다 1도씩 회전
	}

	// 포탑 위치 교환 애니메이션
	if (turretSwapToggle) {
		turretSwapTime += 0.02f; // 시간 증가
		
		if (turretSwapTime >= 1.0f) {
			// 애니메이션 완료: mixstartpos와 mixendpos 둘 다 -1 곱하기
			mixstartpos *= -1.0f;
			mixendpos *= -1.0f;
			turretSwapTime = 0.0f; // 리셋
		}
		else {
			// mixstartpos에서 mixendpos로 선형보간
			float x = glm::mix(mixstartpos, mixendpos, turretSwapTime);
			tank.setTurretPos(glm::vec3(x, 0.0f, 0.0f));
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
	case GLUT_KEY_UP: // 위쪽 화살표
		break;
	case GLUT_KEY_DOWN: // 아래쪽 화살표
		break;
	case GLUT_KEY_LEFT: // 왼쪽 화살표
		break;
	case GLUT_KEY_RIGHT: // 오른쪽 화살표
		break;
	}

	glutPostRedisplay();
}
