#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector> // 구 생성을 위해 추가
#include <list>
#include <algorithm>
#include <cmath> // 구 생성을 위해 추가
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <random>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

#define MAXRECT 10 // 최대 사각형 개수
#define point 0
#define line 1
#define triangle 2
#define rectangle 3
#define pentagon 4
#define polygonwidth 100
#define pi 3.14159265358979323846

// M_PI가 정의되지 않은 경우를 대비하여 추가
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


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

//--- 필요한 변수 선언
GLint width = 1200, height = 800;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체
GLuint VAO, VBO; //--- 버텍스 배열 객체, 버텍스 버퍼 객체
int nowdrawstate = 0; // 0: point, 1: line, 2: triangle, 3: rectangle
int selectedshape = -1; // 선택된 도형 인덱스
int spin = 1; //  1: 시계방향, -1: 반시계방향
int animation = 1; // 0: 정지, 1: 회전
int hidetoggle = 1; // 1. 은면제거
int wiretoggle = 0; // 1. 와이어프레임 모드
int culltoggle = 0; // 1. 뒷면 컬링 모드


// Forward declaration
class polygon;
std::list<polygon> polygonmap;
std::list<polygon>::iterator mouse_dest; // 마우스로 선택된 polygon 저장
std::vector<float> allVertices;

int selection[10] = { 1,1,1,1,1,1,0,0,0,0 };

float angle = 0.0f; // y축 회전 각도
float xangle = 0.0f; // x축 회전 각도 (추가)
float polygon_xpos = 0.0f;
float polygon_ypos = 0.0f;
float orbitAngle = 0.0f; // 조명 공전 각도
int turnontoggle = 1; // 조명 ON/OFF (1: ON, 0: OFF)


float lightOrbitRadius = 4.0f; // 조명 궤도 반지름

typedef struct poitment {
	float xpos;
	float ypos;
	float zpos;
} pointment;

// 도형 저장하는 클래스 (현재는 사용되지 않지만 원본 유지를 위해 남겨둠)
class polygon {
private:
	GLdouble Rvalue = 0.0;
	GLdouble Gvalue = 0.0;
	GLdouble Bvalue = 0.0;
	glm::vec4 vpos[2][3];
	int needmove = 0;
	int inner = 0; // mouse 선택되었는지 여부

public:
	//std::vector<ret> rects;
	polygon(pointment p1, pointment p2, pointment p3, GLdouble rv, GLdouble gv, GLdouble bv) {
		vpos[0][0] = glm::vec4(p1.xpos, p1.ypos, p1.zpos, 1.0f);
		vpos[0][1] = glm::vec4(p2.xpos, p2.ypos, p2.zpos, 1.0f);
		vpos[0][2] = glm::vec4(p3.xpos, p3.ypos, p3.zpos, 1.0f);
		vpos[1][0] = glm::vec4(p1.xpos, p1.ypos, p1.zpos, 1.0f);
		vpos[1][1] = glm::vec4(p2.xpos, p2.ypos, p2.zpos, 1.0f);
		vpos[1][2] = glm::vec4(p3.xpos, p3.ypos, p3.zpos, 1.0f);
		Rvalue = rv; Gvalue = gv; Bvalue = bv;
	}

	polygon(pointment p1, pointment p2, pointment p3, pointment p4, GLdouble rv, GLdouble gv, GLdouble bv) {
		vpos[0][0] = glm::vec4(p1.xpos, p1.ypos, p1.zpos, 1.0f);
		vpos[0][1] = glm::vec4(p2.xpos, p2.ypos, p2.zpos, 1.0f);
		vpos[0][2] = glm::vec4(p3.xpos, p3.ypos, p3.zpos, 1.0f);
		vpos[1][0] = glm::vec4(p1.xpos, p1.ypos, p1.zpos, 1.0f);
		vpos[1][1] = glm::vec4(p3.xpos, p3.ypos, p3.zpos, 1.0f);
		vpos[1][2] = glm::vec4(p4.xpos, p4.ypos, p4.zpos, 1.0f);
		Rvalue = rv; Gvalue = gv; Bvalue = bv;
	}

	void update(float theta) {}

	void sendvertexdata(std::vector<float>& vbo) { // vbo에 정점 데이터 추가
		for (int poly = 0; poly < 2; ++poly) {
			glm::vec3 edge1 = glm::vec3(vpos[poly][1] - vpos[poly][0]);
			glm::vec3 edge2 = glm::vec3(vpos[poly][2] - vpos[poly][0]);
			glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
			for (int vert = 0; vert < 3; ++vert) {
				vbo.insert(vbo.end(), { vpos[poly][vert].x, vpos[poly][vert].y, vpos[poly][vert].z });
				vbo.insert(vbo.end(), { normal.x, normal.y, normal.z });
			}
		}
	}
};

// ======================================================================
// ===== 새로 추가된 구 생성 함수 섹션 시작 ===============================
// ======================================================================

/**
 * 구의 정점, 법선, 텍스처 좌표, 인덱스를 생성하는 함수 (원본)
 * @param radius 구의 반지름
 * @param sectorCount 구를 수평으로 나누는 세그먼트 수
 * @param stackCount 구를 수직으로 나누는 스택 수
 * @param outVertices 생성된 정점 좌표를 저장할 벡터
 * @param outNormals 생성된 법선 벡터를 저장할 벡터
 * @param outTexCoords 생성된 텍스처 좌표를 저장할 벡터
 * @param outIndices 렌더링을 위한 인덱스를 저장할 벡터
 */
void generateSphere(float radius, int sectorCount, int stackCount,
	std::vector<float>& outVertices,
	std::vector<float>& outNormals,
	std::vector<float>& outTexCoords,
	std::vector<unsigned int>& outIndices) {

	outVertices.clear();
	outNormals.clear();
	outTexCoords.clear();
	outIndices.clear();

	float x, y, z, xy;
	float nx, ny, nz, lengthInv = 1.0f / radius;
	float s, t;

	float sectorStep = 2 * M_PI / sectorCount;
	float stackStep = M_PI / stackCount;
	float sectorAngle, stackAngle;

	for (int i = 0; i <= stackCount; ++i) {
		stackAngle = M_PI / 2 - i * stackStep;
		xy = radius * cosf(stackAngle);
		z = radius * sinf(stackAngle);

		for (int j = 0; j <= sectorCount; ++j) {
			sectorAngle = j * sectorStep;
			x = xy * cosf(sectorAngle);
			y = xy * sinf(sectorAngle);
			outVertices.push_back(x);
			outVertices.push_back(y);
			outVertices.push_back(z);

			nx = x * lengthInv;
			ny = y * lengthInv;
			nz = z * lengthInv;
			outNormals.push_back(nx);
			outNormals.push_back(ny);
			outNormals.push_back(nz);

			s = (float)j / sectorCount;
			t = (float)i / stackCount;
			outTexCoords.push_back(s);
			outTexCoords.push_back(t);
		}
	}

	unsigned int k1, k2;
	for (int i = 0; i < stackCount; ++i) {
		k1 = i * (sectorCount + 1);
		k2 = k1 + sectorCount + 1;

		for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
			if (i != 0) {
				outIndices.push_back(k1);
				outIndices.push_back(k2);
				outIndices.push_back(k1 + 1);
			}
			if (i != (stackCount - 1)) {
				outIndices.push_back(k1 + 1);
				outIndices.push_back(k2);
				outIndices.push_back(k2 + 1);
			}
		}
	}
}

/**
 * glDrawArrays를 위해 인덱스 없는 평탄화된 정점 배열을 생성하는 함수
 * @param radius 구의 반지름
 * @param sectorCount 수평 분할 수
 * @param stackCount 수직 분할 수
 * @param finalVertices 최종 정점 데이터(위치+법선)를 저장할 벡터
 */
void generateSphereVerticesForDrawArrays(float radius, int sectorCount, int stackCount, std::vector<float>& finalVertices) {
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> texCoords;
	std::vector<unsigned int> indices;

	// 먼저 인덱스 기반의 구 데이터를 생성
	generateSphere(radius, sectorCount, stackCount, vertices, normals, texCoords, indices);

	finalVertices.clear();
	// 인덱스를 순회하면서 각 삼각형의 정점 데이터를 finalVertices에 추가
	for (size_t i = 0; i < indices.size(); ++i) {
		unsigned int index = indices[i];

		// 위치 데이터 추가 (x, y, z)
		finalVertices.push_back(vertices[index * 3]);
		finalVertices.push_back(vertices[index * 3 + 1]);
		finalVertices.push_back(vertices[index * 3 + 2]);

		// 법선 데이터 추가 (nx, ny, nz)
		finalVertices.push_back(normals[index * 3]);
		finalVertices.push_back(normals[index * 3 + 1]);
		finalVertices.push_back(normals[index * 3 + 2]);
	}
}

// ======================================================================
// ===== 새로 추가된 구 생성 함수 섹션 끝 =================================
// ======================================================================

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

	// 정점 속성 설정: 위치 (3개) + 노말 (3개) = 총 6개 float
	// location 0: 위치
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// location 1: 노말
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Sphere Rendering with Lighting");
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

	// ---!!! 핵심 수정 부분 !!!---
	// 프로그램 시작 시 구의 정점 데이터를 미리 생성하여 allVertices에 저장
	allVertices.clear();
	
	// 구 하나만 생성 (재사용할 것임)
	generateSphereVerticesForDrawArrays(1.0f, 36, 18, allVertices);

	//--- 세이더 프로그램 만들기

	glutDisplayFunc(drawScene); //--- 출력 콜백 함수
	glutReshapeFunc(Reshape);

	glutTimerFunc(25, TimerFunction, 1);

	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeys); // 특수 키(화살표 키) 콜백 등록
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion); // 마우스 모션 콜백 등록

	glutMainLoop();
	return 0;
}

void make_vertexShaders()
{
	GLchar* vertexSource;
	//--- 버텍스 세이더 읽어 저장하고 컴파일하기
	//--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
	vertexSource = filetobuf("vertex_light.glsl");
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
	fragmentSource = filetobuf("fragment_light.glsl"); // 프래그세이더 읽어오기
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
	GLchar errorLog[512]; // 에러 로그 크기를 512로 설정
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
		// return false; // main에서 bool을 받지 않으므로 return shaderID; 로 변경
	}
	glUseProgram(shaderID); //--- 만들어진 세이더 프로그램 사용하기
	return shaderID;
}

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	GLfloat rColor, gColor, bColor;
	rColor = gColor = 0.1f; //--- 배경색을 약간 어두운 회색으로 설정
	bColor = 0.1f;
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	// 모델 변환 행렬 설정 (Y축 회전과 X축 회전을 모두 적용)
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f)); // y축 회전
	model = glm::rotate(model, xangle, glm::vec3(1.0f, 0.0f, 0.0f)); // x축 회전


	// 뷰 변환 행렬 설정
	glm::mat4 view = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 4.2f), // 카메라 위치
		glm::vec3(0.0f, 0.0f, 0.0f), // 바라보는 점
		glm::vec3(0.0f, 1.0f, 0.0f)  // 업 벡터
	);

	// 투영 변환 행렬 설정
	glm::mat4 projection = glm::perspective(
		glm::radians(45.0f),          // 시야각
		(float)width / (float)height, // 종횡비
		0.1f,                         // 근평면
		100.0f                        // 원평면
	);

	// Uniform 변수들 설정
	unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "model");
	unsigned int viewLocation = glGetUniformLocation(shaderProgramID, "view");
	unsigned int projectionLocation = glGetUniformLocation(shaderProgramID, "projection");

	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

	// 조명 관련 Uniform 변수 설정
	unsigned int lightPosLocation = glGetUniformLocation(shaderProgramID, "lightPos");
	unsigned int viewPosLocation = glGetUniformLocation(shaderProgramID, "viewPos");
	unsigned int lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor");
	unsigned int objectColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");

	// 조명 설정 - 원 궤도를 따라 회전
	float lightX = lightOrbitRadius * cos(orbitAngle);
	float lightZ = lightOrbitRadius * sin(orbitAngle);

	glm::vec3 lightPos;
	if (turnontoggle) {
		lightPos = glm::vec3(lightX, 0.0f, lightZ); // ZX 평면에서 회전 (Y값을 주어 위에서 비추도록 수정)
	}
	else {
		lightPos = glm::vec3(-500.0f, -500.0f, -500.0f); // 조명 OFF - 멀리 이동
	}

	glm::vec3 viewPos(2.2f, 2.2f, 4.2f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
	glm::vec3 objectColor(1.0f, 0.0f, 0.0f);

	glUniform3fv(lightPosLocation, 1, glm::value_ptr(lightPos));
	glUniform3fv(viewPosLocation, 1, glm::value_ptr(viewPos));
	glUniform3fv(lightColorLocation, 1, glm::value_ptr(lightColor));
	glUniform3fv(objectColorLocation, 1, glm::value_ptr(objectColor));

	// --- 구 그리기 ---
	if (!allVertices.empty()) {
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		// 버퍼에 정점 데이터 업로드 (한 번만)
		glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float), allVertices.data(), GL_STATIC_DRAW);

		int vertexCount = allVertices.size() / 6; // 각 정점은 6개의 float(위치 3, 노말 3)

		// 1. 중앙의 1/2 사이즈 빨간 구
		glm::mat4 model1 = glm::mat4(1.0f);
		model1 = glm::rotate(model1, angle, glm::vec3(0.0f, 1.0f, 0.0f)); // y축 회전
		model1 = glm::rotate(model1, xangle, glm::vec3(1.0f, 0.0f, 0.0f)); // x축 회전
		model1 = glm::scale(model1, glm::vec3(0.5f, 0.5f, 0.5f)); // 1/2 크기
		
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model1));
		objectColor = glm::vec3(1.0f, 0.0f, 0.0f); // 빨간색
		glUniform3fv(objectColorLocation, 1, glm::value_ptr(objectColor));
		glDrawArrays(GL_TRIANGLES, 0, vertexCount);

		// 2. 약간 왼쪽의 1/3 사이즈 초록 구
		glm::mat4 model2 = glm::mat4(1.0f);
		model2 = glm::translate(model2, glm::vec3(-1.2f, 0.0f, 0.0f)); // 왼쪽으로 이동
		model2 = glm::rotate(model2, angle, glm::vec3(0.0f, 1.0f, 0.0f)); // y축 회전
		model2 = glm::rotate(model2, xangle, glm::vec3(1.0f, 0.0f, 0.0f)); // x축 회전
		model2 = glm::scale(model2, glm::vec3(0.333f, 0.333f, 0.333f)); // 1/3 크기
		
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model2));
		objectColor = glm::vec3(0.0f, 1.0f, 0.0f); // 초록색
		glUniform3fv(objectColorLocation, 1, glm::value_ptr(objectColor));
		glDrawArrays(GL_TRIANGLES, 0, vertexCount);

		// 3. 더 왼쪽의 1/4 사이즈 파란 구
		glm::mat4 model3 = glm::mat4(1.0f);
		model3 = glm::translate(model3, glm::vec3(-2.0f, 0.0f, 0.0f)); // 더 왼쪽으로 이동
		model3 = glm::rotate(model3, angle, glm::vec3(0.0f, 1.0f, 0.0f)); // y축 회전
		model3 = glm::rotate(model3, xangle, glm::vec3(1.0f, 0.0f, 0.0f)); // x축 회전
		model3 = glm::scale(model3, glm::vec3(0.25f, 0.25f, 0.25f)); // 1/4 크기
		
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model3));
		objectColor = glm::vec3(0.0f, 0.0f, 1.0f); // 파란색
		glUniform3fv(objectColorLocation, 1, glm::value_ptr(objectColor));
		glDrawArrays(GL_TRIANGLES, 0, vertexCount);

		glBindVertexArray(0);
	}

	glutSwapBuffers(); // 화면에 출력하기
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
	width = w; height = h; // 전역 변수 업데이트
	glViewport(0, 0, w, h);
}

void Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'q': // 프로그램 종료
		glutLeaveMainLoop();
		break;
	case 'r': // 조명 공전 - 반시계방향
	{
		orbitAngle += 0.1f; // 약 5.7도씩 증가
		if (orbitAngle > 2.0f * pi) {
			orbitAngle -= 2.0f * pi; // 2π를 넘으면 리셋
		}
	}
	break;
	case 'R': // 조명 공전 - 시계방향
	{
		orbitAngle -= 0.1f; // 약 5.7도씩 감소
		if (orbitAngle < 0.0f) {
			orbitAngle += 2.0f * pi; // 0 미만이면 2π 더하기
		}
	}
	break;
	case 'y': // y축 기준 양방향 회전
	{
		angle += 0.05f;
	}
	break;
	case 'Y': // y축 기준 음방향 회전
	{
		angle -= 0.05f;
	}
	break;
	case 'x': // x축 기준 양방향 회전
	{
		xangle += 0.05f;
	}
	break;
	case 'X': // x축 기준 음방향 회전
	{
		xangle -= 0.05f;
	}
	break;
	case 'h': // 은면제거 적용/해제
	{
		if (hidetoggle) { // 현재 활성화 상태면
			glDisable(GL_DEPTH_TEST);
			hidetoggle = 0;
		}
		else { // 현재 비활성화 상태면
			glEnable(GL_DEPTH_TEST);
			hidetoggle = 1;
		}
	}
	break;
	case 's': // 초기위치로 리셋(모든 애니메이션 멈추기)
	{
		xangle = 0.0f;
		angle = 0.0f;
		polygon_xpos = 0.0f;
		polygon_ypos = 0.0f;
		orbitAngle = 0.0f; // 조명 궤도 각도도 리셋
		lightOrbitRadius = 2.0f; // 조명 궤도 반지름도 리셋
		turnontoggle = 1; // 조명 ON으로 리셋

	}
	break;
	case 'z': // 조명 궤도 반지름 증가
	{
		lightOrbitRadius += 0.1f;
		if (lightOrbitRadius > 5.0f) {
			lightOrbitRadius = 5.0f; // 최대 반지름 제한
		}
	}
	break;
	case 'Z': // 조명 궤도 반지름 감소
	{
		lightOrbitRadius -= 0.1f;
		if (lightOrbitRadius < 0.5f) {
			lightOrbitRadius = 0.5f; // 최소 반지름 제한
		}
	}
	break;
	case 'm': // 조명 ON/OFF 토글
	{
		turnontoggle = 1 - turnontoggle; // 0 <-> 1 전환
	}
	break;
	case 'u':
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

void SpecialKeys(int key, int x, int y) {
	// 현재 이 함수는 비어있으므로 특별한 동작 없음
	glutPostRedisplay();
}

void Mouse(int button, int state, int x, int y)
{
	// 현재 이 함수는 비어있으므로 특별한 동작 없음
}

void TimerFunction(int value)
{
	glutPostRedisplay();
	glutTimerFunc(25, TimerFunction, 1);
}

void Motion(int x, int y) // 마우스 모션 콜백 함수
{
	// 현재 이 함수는 비어있으므로 특별한 동작 없음
}