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
#define polygonwidth 100
#define pi 3.14159265358979323846

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
GLint width = 800, height = 800;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체
GLuint VAO, VBO; //--- 버텍스 배열 객체, 버텍스 버퍼 객체
int nowdrawstate = 0; // 0: point, 1: line, 2: triangle, 3: rectangle
int selectedshape = -1; // 선택된 도형 인덱스
int spin = 1; //  1: 시계방향, -1: 반시계방향
int animation = 0; // 0: 정지, 1: 회전
int hidetoggle = 1; // 1. 은면제거
int wiretoggle = 0; // 1. 와이어프레임 모드
int culltoggle = 0; // 1. 뒷면 컬링 모드

// cube 관련 토글 변수
int zrotoggle = 0; // 1. z축 회전 모드
int opentoggle = 0; // 1. 도형 열기 모드
int tiretoggle = 0; // 1. 옆면이 회전
int backsizetoggle = 0; // 1. 뒷면 size 모드

// 새로운 토글 변수들
int edgeopentoggle = 0; // 0: edge 열림, 1: edge 닫힘
int backscaletoggle = 0; // 1: scale 증가, 0: scale 감소

// r키 순차 동작을 위한 변수들
int rsequence = 0; // 0: 정지, 1: 순차 열기, 2: 순차 닫기
int rcurrentface = 0; // 현재 동작 중인 면 (0:t1, 1:t2, 2:t3, 3:t4)

// piriamid 관련 토글 변수
int openeverytoggle = 0; // 1. 모든 면 열기 모드	
int sequentopnetoggle = 0; // 1. 면이 순차적으로 열림
int sequentoclosetoggle = 0; // 1. 면이 순차적으로 닫힘

// cube 관련 변수
float topangle = 0.0f; // 윗면 회전 각도
float oepnangle = 0.0f; // front 열리는 각도
float tireangle = 0.0f; // 옆면 회전 각도
float backsize = 1.0f; // 뒷면 크기

// piramid 관련 변수
float t1angle = 0.0f; // 면1 회전 각도
float t2angle = 0.0f; // 면2 회전 각도
float t3angle = 0.0f; // 면3 회전 각도
float t4angle = 0.0f; // 면4 회전 각도

// 10개짜리 float 포인터 배열 - 각각의 angle 변수들을 가리킴
float* anglelist[10] = {
	&topangle,   // 0
	&tireangle,  // 1
	&backsize,   // 2
	&tireangle,  // 3
	&oepnangle,  // 4
	&backsize,   // 5
	&t1angle,    // 6
	&t2angle,    // 7
	&t3angle,    // 8
	&t4angle     // 9
};


// Forward declaration
class polygon;
std::vector<polygon> polygonmap;
int mouse_dest = -1; // 마우스로 선택된 polygon 인덱스 저장
std::vector<float> allVertices;

int selection[10] = { 1,1,1,1,1,1,0,0,0,0 };

float angle = 0.0f; // 회전 각도
float xangle = 0.0f;
float polygon_xpos = 0.0f;
float polygon_ypos = 0.0f;

// 동적 회전축을 위한 전역 변수
glm::vec3 current_xaxis;
glm::vec3 current_yaxis;
glm::vec3 current_zaxis;

typedef struct poitment {
	float xpos;
	float ypos;
	float zpos;
} pointment;

// 도형 저장하는 클래스
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

		Rvalue = rv;
		Gvalue = gv;
		Bvalue = bv;
	}

	polygon(pointment p1, pointment p2, pointment p3, pointment p4, GLdouble rv, GLdouble gv, GLdouble bv) {
		vpos[0][0] = glm::vec4(p1.xpos, p1.ypos, p1.zpos, 1.0f);

		vpos[0][1] = glm::vec4(p2.xpos, p2.ypos, p2.zpos, 1.0f);

		vpos[0][2] = glm::vec4(p3.xpos, p3.ypos, p3.zpos, 1.0f);

		vpos[1][0] = glm::vec4(p1.xpos, p1.ypos, p1.zpos, 1.0f);

		vpos[1][1] = glm::vec4(p3.xpos, p3.ypos, p3.zpos, 1.0f);

		vpos[1][2] = glm::vec4(p4.xpos, p4.ypos, p4.zpos, 1.0f);

		Rvalue = rv;
		Gvalue = gv;
		Bvalue = bv;
	}


	void update(float theta) {

	}

	void rotate(float theta, char command) {
		switch (command) {
		case 'x':
		{
			for (int j = 0; j < 2; ++j) {
				for (int i = 0; i < 3; ++i) {
					vpos[j][i] = glm::rotate(glm::mat4(1.0f), theta, glm::vec3(1.0f, 0.0f, 0.0f)) * vpos[j][i];
				}
			}

		}
		break;
		case 'y':
		{
			for (int j = 0; j < 2; ++j) {
				for (int i = 0; i < 3; ++i) {
					vpos[j][i] = glm::rotate(glm::mat4(1.0f), theta, glm::vec3(0.0f, 1.0f, 0.0f)) * vpos[j][i];
				}
			}
		}
		break;
		case 'z':
		{
			for (int j = 0; j < 2; ++j) {
				for (int i = 0; i < 3; ++i) {
					vpos[j][i] = glm::rotate(glm::mat4(1.0f), theta, glm::vec3(0.0f, 0.0f, 1.0f)) * vpos[j][i];
				}
			}
		}
		break;
		}
	}



	void sendvertexdata(std::vector<float>& vbo) { // vbo에 정점 데이터 추가
		for (int poly = 0; poly < 2; ++poly) {
			for (int vert = 0; vert < 3; ++vert) {
				vbo.insert(vbo.end(), {
					vpos[poly][vert].x, vpos[poly][vert].y, vpos[poly][vert].z, (float)Rvalue, (float)Gvalue, (float)Bvalue
					});
			}
		}
	}

	// 행렬 뱉는 함수
	glm::mat4 getedge(float* angles) {
		glm::vec3 edge = glm::normalize(glm::vec3(vpos[0][0] - vpos[0][1]));
		glm::mat4 model1 = glm::mat4(1.0f);

		glm::mat4 transparant = glm::mat4(1.0f);
		transparant = glm::translate(transparant, glm::vec3(-vpos[0][1].x, -vpos[0][1].y, -vpos[0][1].z));


		model1 = glm::rotate(model1, *angles, edge);

		glm::mat4 rev = glm::mat4(1.0f);
		rev = glm::translate(rev, glm::vec3(vpos[0][1].x, vpos[0][1].y, vpos[0][1].z));
		return rev * model1 * transparant;
		return model1;
	}

	glm::mat4 getnomal(float* angles) {
		glm::vec4 u((vpos[0][0] + vpos[0][2]));
		u.x /= -2.0f;
		u.y /= -2.0f;
		u.z /= -2.0f;
		u.y -= 0.15f;
		u.x += 0.16f;
		// 얘가 transform이 될거임
		glm::mat4 transport = glm::mat4(1.0f);
		//transport = glm::translate(transport, glm::vec3(u.x, u.y, u.z));
		transport = glm::translate(transport, glm::vec3(u.x, u.y, 0));
		glm::mat4 model1 = glm::mat4(1.0f);
		model1 = glm::rotate(model1, *angles, current_zaxis);

		glm::mat4 rev = glm::mat4(1.0f);
		rev = glm::translate(rev, glm::vec3(-u.x, -u.y, 0));

		return rev * model1 * transport;

		//model1 = glm::translate(model1, glm::vec3(-u.x, -u.y, -u.z));

	}

	glm::mat4 getunit(float* angles) {
		//glm::vec3 u = glm::vec3(vpos[0][0] - vpos[0][1]);

		return glm::mat4(1.0f);
	}

	glm::mat4 gettirerotate(float* angles) {

		glm::mat4 model1 = glm::mat4(1.0f);
		model1 = glm::rotate(model1, *angles, current_xaxis);

		return model1;
	}

	glm::mat4 getbackscale(float* angles) {
		glm::vec4 u((vpos[0][0] + vpos[0][2]));
		u /= 2.0f;
		glm::mat4 transport = glm::mat4(1.0f);
		transport = glm::translate(transport, glm::vec3(-u.x, -u.y, -u.z));

		glm::mat4 model1 = glm::mat4(1.0f);
		model1 = glm::scale(model1, glm::vec3(*angles, *angles, *angles));

		glm::mat4 rev = glm::mat4(1.0f);
		rev = glm::translate(rev, glm::vec3(u.x, u.y, u.z));
		return rev * model1 * transport;

	}
};


using ActionFunc = glm::mat4(polygon::*)(float* angles);

// 각 객체가 어떤 행동을 할지 지정
ActionFunc actions[10] = {
	&polygon::getnomal, &polygon::gettirerotate, &polygon::getbackscale,
	&polygon::gettirerotate, &polygon::getedge, &polygon::getunit,
	&polygon::getedge, &polygon::getedge, &polygon::getedge, &polygon::getedge
};
/*bool ptinrect(int x, int y, ret& rect) {
	return (x >= rect.x1 && x <= rect.x2 && y >= rect.y1 && y <= rect.y2);
}*/

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
	allVertices.clear();

	//polygonmap.emplace_back(polygon(400 - 150, 400 + 150, 400 + 150, 400 + 300, 3));


	// glm::vec4로 축 좌표 정의
	glm::vec4 xaxis1(10, 0, 0, 1);
	glm::vec4 xaxis2(-10, 0, 0, 1);
	glm::vec4 yaxis1(0, 10, 0, 1);
	glm::vec4 yaxis2(0, -10, 0, 1);
	glm::vec4 zaxis1(0, 0, 10, 1);
	glm::vec4 zaxis2(0, 0, -10, 1);

	// y축으로 -30도 회전 (glm::rotate 사용)
	/*xaxis1 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(0.0f, 1.0f, 0.0f)) * xaxis1;
	xaxis2 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(0.0f, 1.0f, 0.0f)) * xaxis2;
	yaxis1 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(0.0f, 1.0f, 0.0f)) * yaxis1;
	yaxis2 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(0.0f, 1.0f, 0.0f)) * yaxis2;
	zaxis1 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(0.0f, 1.0f, 0.0f)) * zaxis1;
	zaxis2 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(0.0f, 1.0f, 0.0f)) * zaxis2;

	// x축으로 30도 회전 (glm::rotate 사용)
	xaxis1 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(1.0f, 0.0f, 0.0f)) * xaxis1;
	xaxis2 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(1.0f, 0.0f, 0.0f)) * xaxis2;
	yaxis1 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(1.0f, 0.0f, 0.0f)) * yaxis1;
	yaxis2 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(1.0f, 0.0f, 0.0f)) * yaxis2;
	zaxis1 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(1.0f, 0.0f, 0.0f)) * zaxis1;
	zaxis2 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(1.0f, 0.0f, 0.0f)) * zaxis2;*/



	// 축 데이터를 allVertices에 추가 (vec4의 x, y, z 멤버 사용)
	allVertices.insert(allVertices.end(), {
					xaxis1.x, xaxis1.y, xaxis1.z,
					1, 0, 0 });
	allVertices.insert(allVertices.end(), {
					xaxis2.x, xaxis2.y, xaxis2.z,
					1, 0, 0 });

	allVertices.insert(allVertices.end(), {
					yaxis1.x, yaxis1.y, yaxis1.z,
					0, 1, 0 });
	allVertices.insert(allVertices.end(), {
					yaxis2.x, yaxis2.y, yaxis2.z,
					0, 1, 0 });

	allVertices.insert(allVertices.end(), {
					zaxis1.x, zaxis1.y, zaxis1.z,
					0, 0, 1 });
	allVertices.insert(allVertices.end(), {
					zaxis2.x, zaxis2.y, zaxis2.z,
					0, 0, 1 });


	/*for (auto& poly : polygonmap) {
		poly.rotate(-pi / 6, 'y');
		poly.rotate(-pi / 6, 'x');

		poly.sendvertexdata(allVertices);
	}*/



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
	//--- 버텍스 세이더 읽어 저장하고 컴파일 하기
	//--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
	vertexSource = filetobuf("vertex_view.glsl");
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
	bColor = 1.0; //--- 배경색을 파랑색으로 설정
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	// 각 사각형을 6개 정점으로 변환한 전체 데이터
	glm::mat4 view = glm::mat4(1.0f);

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.4f);
	// y축으로 -pi/6 회전
	glm::vec4 tempPos = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(cameraPos, 1.0f);
	cameraPos = glm::vec3(tempPos);
	// x축으로 -pi/6 회전
	tempPos = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::vec4(cameraPos, 1.0f);
	cameraPos = glm::vec3(tempPos);
	
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
	unsigned int viewLocation = glGetUniformLocation(shaderProgramID, "viewTransform"); //--- 버텍스 세이더에서 viewTransform 변수위치
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, angle, current_yaxis);

	unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "modelTransform");

	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

	if (!allVertices.empty()) {
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		// 버퍼에 정점 데이터 업로드
		glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float),
			allVertices.data(), GL_DYNAMIC_DRAW);

		// 모든 사각형을 한 번에 그리기 (각 사각형당 6개 정점)

	}

	glLineWidth(2.0f);

	// 축은 변환 없이 그리기 (단위 행렬 적용)
	glm::mat4 identityMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(identityMatrix)); // 버텍스 셰이더에 있는 modelTransform에 단위 행렬 전달
	glDrawArrays(GL_LINES, 0, 6);

	// polygon들은 변환 행렬 적용해서 그리기
	glm::mat4 temp = model;

	for (int i = 0; i < 10; ++i) {
		

	}






	glBindVertexArray(0);

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
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	{
		int idx = key - '0';
		for (int i = 0; i < 10; ++i) {
			selection[i] = 0;
		}
		idx += 9;
		idx %= 10;
		selection[idx] = 1;
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
	case 'y': // 
	{
		animation = !animation;
	}
	break;
	case 't': // zrotoggle
	{
		zrotoggle = !zrotoggle;
	}
	break;
	case 'f': // opentoggle
	{
		opentoggle = !opentoggle;
	}
	break;
	case 's': // tiretoggle
	{
		tiretoggle = !tiretoggle;
	}
	break;
	case 'b': // backsizetoggle
	{
		backsizetoggle = !backsizetoggle;
	}
	break;
	case 'o': // openeverytoggle
	{
		openeverytoggle = !openeverytoggle;
	}
	break;
	case 'r': // sequentoclosetoggle
	{
		
	}
	break;
	case 'p':
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
	case 'c': // 초기화
	{
		
	}
	default:
		break;
	}

	glutPostRedisplay();
}

void SpecialKeys(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_LEFT: // ← 좌로 객체 이동 (current_xaxis 음의 방향)
	{
		polygon_xpos -= current_xaxis.x * 0.02f;
		polygon_ypos -= current_xaxis.y * 0.02f;
	}
	break;
	case GLUT_KEY_RIGHT: // → 우로 객체 이동 (current_xaxis 양의 방향)
	{
		polygon_xpos += current_xaxis.x * 0.02f;
		polygon_ypos += current_xaxis.y * 0.02f;
	}
	break;
	case GLUT_KEY_UP: // ↑ 상으로 객체 이동 (current_yaxis 양의 방향)
	{
		polygon_xpos += current_yaxis.x * 0.02f;
		polygon_ypos += current_yaxis.y * 0.02f;
	}
	break;
	case GLUT_KEY_DOWN: // ↓ 하로 객체 이동 (current_yaxis 음의 방향)
	{
		polygon_xpos -= current_yaxis.x * 0.02f;
		polygon_ypos -= current_yaxis.y * 0.02f;
	}
	break;
	default:
		break;
	}

	glutPostRedisplay();
}

void Mouse(int button, int state, int x, int y)
{
	switch (button) {
	case GLUT_LEFT_BUTTON:
	{
		if (state == GLUT_DOWN) {// 도형선택

		}
		else if (state == GLUT_UP) {

			glutPostRedisplay();
		}
	}
	break;
	case GLUT_RIGHT_BUTTON:
	{
		if (state == GLUT_DOWN) {

		}
	}
	break;
	default:
		break;
	}
}

void TimerFunction(int value)
{
	

	glutPostRedisplay();
	glutTimerFunc(25, TimerFunction, 1);
}

void Motion(int x, int y) // 마우스 모션 콜백 함수
{

}