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



// 투영 관련 토글 변수
int projectiontoggle = 0; // 0: 기본 투영, 1: 다른 투영

// z축 회전 상태 변수
int zrotationdirection = 0; // 0: 정지, 1: 시계방향, -1: 반시계방향

// 궤도 스케일 변수
float orbitscale = 1.0f;

// Forward declaration
class polygon;
std::vector<polygon> polygonmap;
int mouse_dest = -1; // 마우스로 선택된 polygon 인덱스 저장
std::vector<float> allVertices;

int selection[10] = { 1,1,1,1,1,1,0,0,0,0 };
int currentObject = 0; // 현재 선택된 객체 (0 또는 1)

float angle = 0.0f; // 회전 각도
float xangle = 0.0f;
float polygon_xpos = 0.0f;
float polygon_ypos = 0.0f;

// 행성 위치
glm::vec3 planetpos = glm::vec3(50.0f, 0.0f, 0.0f);

// 위성 위치 (행성 기준 상대 좌표)
glm::vec3 moonpos = glm::vec3(50.0f / 3.0f, 0.0f, 0.0f);

// 카메라 추가 위치 변수
glm::vec3 additionalCameraPos = glm::vec3(0.0f, 0.0f, 0.0f);

// 동적 회전축을 위한 전역 변수
glm::vec3 current_xaxis;
glm::vec3 current_yaxis;
glm::vec3 current_zaxis;

// 도형 타입 열거형
enum ShapeType {
	SHAPE_CUBE = 0,      // 육면체
	SHAPE_SPHERE = 1,    // 구
	SHAPE_CONE = 2,      // 12각뿔
	SHAPE_CYLINDER = 3   // 12각기둥
};

typedef struct poitment {
	float xpos;
	float ypos;
	float zpos;
} pointment;

// 변환 정보를 저장하는 구조체
typedef struct TransformInfo {
	float xRotation;      // x축 자전 각도
	float yRotation;      // y축 자전 각도
	float localScale;     // 제자리 scale 크기
	glm::vec3 position;   // 위치 (x, y, z)
	glm::vec3 midPoint;   // 중간 경유 지점
	ShapeType shapeType;  // 도형 타입
} TransformInfo;

// 2개짜리 배열 선언
TransformInfo transformArray[2];


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

	

	// 원을 그리기 위한 점들 생성 (0, 0, -50)을 y축으로 회전
	int circleSegments = 100; // 원을 구성할 선분 개수
	float radius = 50.0f; // 반지름
	glm::vec3 startPoint(0.0f, 0.0f, -radius); // 시작점
	
	for (int i = 0; i <= circleSegments; ++i) {
		float angle = (2.0f * pi * i) / circleSegments; // 0 ~ 2π
		
		// y축 회전 행렬 적용
		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec4 rotatedPoint = rotationMatrix * glm::vec4(startPoint, 1.0f);
		
		// 점 추가
		allVertices.insert(allVertices.end(), {
			rotatedPoint.x, rotatedPoint.y, rotatedPoint.z,
			0.0f, 0.0f, 0.0f // 검은색
		});
	}



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
	glm::vec3 cameraPos = glm::vec3(0.0f, 10.0f, 150.0f) + additionalCameraPos;
	glm::vec3 cameraTarget = glm::vec3(cameraPos.x, cameraPos.y - 10.0f, cameraPos.z - 150.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	
	// 파이프라인 1: 궤도와 행성/위성 그리기 (z축 회전 적용)
	drawOrbitsAndPlanets(cameraPos, cameraTarget, cameraUp, 0.0f + angle); // angle을 0으로 전달 (나중에 변경 가능)
	drawOrbitsAndPlanets(cameraPos, cameraTarget, cameraUp, pi / 4.0f + angle);
	drawOrbitsAndPlanets(cameraPos, cameraTarget, cameraUp, -pi / 4.0f + angle);
	
	// 파이프라인 2: 항성(태양) 그리기
	glUseProgram(0); // 고정 파이프라인 사용
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	if (projectiontoggle == 0) {
		// 원근 투영
		gluPerspective(60.0, 1.0, 0.1, 300.0);
	}
	else {
		// 직교 투영
		glOrtho(-100.0, 100.0, -100.0, 100.0, 0.1, 300.0);
	}
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z,
			  cameraTarget.x, cameraTarget.y, cameraTarget.z,
			  cameraUp.x, cameraUp.y, cameraUp.z);
	
	// 원점(0,0,0)에 파란색 항성(태양) 그리기
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.0f);
	
	GLUquadricObj* qobj = gluNewQuadric();
	if (wiretoggle == 1) {
		gluQuadricDrawStyle(qobj, GLU_LINE); // 와이어프레임
	}
	else {
		gluQuadricDrawStyle(qobj, GLU_FILL); // 솔리드
	}
	glColor3f(0.0f, 0.0f, 1.0f); // 파란색
	gluSphere(qobj, 10.0, spherelevel, spherelevel); // 반지름 10
	gluDeleteQuadric(qobj);
	
	glPopMatrix();

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
		if (zrotationdirection == 1) {
			zrotationdirection = 0; // 이미 1이면 0으로
		}
		else {
			zrotationdirection = 1; // 아니면 1로
		}
	}
	break;
	case 'Z': // z축 반시계방향 회전
	{
		if (zrotationdirection == -1) {
			zrotationdirection = 0; // 이미 -1이면 0으로
		}
		else {
			zrotationdirection = -1; // 아니면 -1로
		}
	}
	break;
	case 'y': // 궤도 스케일 증가
	{
		orbitscale += 0.05f;
	}
	break;
	case 'Y': // 궤도 스케일 감소
	{
		orbitscale -= 0.05f;
		if (orbitscale < 0.1f) orbitscale = 0.1f; // 최소값 제한
	}
	break;
	case 'p': // 투영 모드 변경
	{
		projectiontoggle = !projectiontoggle;
	}
	break;
	case '+': // z축 앞으로 이동
	{
		additionalCameraPos.z -= 5.0f;
	}
	break;
	case '-': // z축 뒤로 이동
	{
		additionalCameraPos.z += 5.0f;
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
	default:
		break;
	}

	glutPostRedisplay();
}

void TimerFunction(int value)
{
	glm::mat4 identity = glm::mat4(1.0f);
	glm::mat4 rotation = glm::rotate(identity, 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
	planetpos = glm::vec3(rotation * glm::vec4(planetpos, 1.0f));

	rotation = glm::rotate(identity, 0.05f, glm::vec3(0.0f, 1.0f, 0.0f));
	moonpos = glm::vec3(rotation * glm::vec4(moonpos, 1.0f));

	// z축 회전 각도 업데이트
	angle += zrotationdirection * 0.03f;

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
		additionalCameraPos.y += 5.0f;
		break;
	case GLUT_KEY_DOWN: // 아래쪽 화살표
		additionalCameraPos.y -= 5.0f;
		break;
	case GLUT_KEY_LEFT: // 왼쪽 화살표
		additionalCameraPos.x -= 5.0f;
		break;
	case GLUT_KEY_RIGHT: // 오른쪽 화살표
		additionalCameraPos.x += 5.0f;
		break;
	}
	
	glutPostRedisplay();
}

// 궤도와 행성/위를 그리는 함수
void drawOrbitsAndPlanets(glm::vec3 cameraPos, glm::vec3 cameraTarget, glm::vec3 cameraUp, float rotationAngle)
{
	// 셰이더 사용
	glUseProgram(shaderProgramID);
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(orbitscale, orbitscale, orbitscale));
	// 투영 행렬 설정
	glm::mat4 projection;
	if (projectiontoggle == 0) {
		// 원근 투영 (Perspective)
		projection = glm::perspective(
			(float)(pi / 3.0f),
			1.0f,
			0.1f,
			300.0f
		);
	}
	else {
		// 직교 투영 (Orthographic)
		projection = glm::ortho(
			-100.0f, 100.0f,  // left, right
			-100.0f, 100.0f,  // bottom, top
			0.1f, 300.0f      // near, far
		);
	}
	unsigned int projectionLocation = glGetUniformLocation(shaderProgramID, "projectionTransform");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
	
	// 뷰 행렬 설정
	glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
	unsigned int viewLocation = glGetUniformLocation(shaderProgramID, "viewTransform");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
	
	unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "modelTransform");
	
	// 버퍼 바인딩
	if (!allVertices.empty()) {
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float),
			allVertices.data(), GL_DYNAMIC_DRAW);
	}
	
	glLineWidth(2.0f);
	
	// z축 회전 행렬
	glm::mat4 zRotation = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0.0f, 0.0f, 1.0f));
	
	// 원래 궤도 그리기 (스케일도 적용)
	glm::mat4 orbitModel = zRotation * scaleMatrix;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(orbitModel));
	glDrawArrays(GL_LINE_LOOP, 0, 101);
	
	// 새로운 궤도 그리기 - planetpos로 이동 -> 1/3 스케일 -> z축 회전 순서
	glm::mat4 smallOrbitMatrix = glm::mat4(1.0f);

	glm::vec3 tplanetpos = orbitscale * planetpos;
	glm::vec3 tmoonpos = orbitscale * moonpos;

	smallOrbitMatrix = glm::translate(smallOrbitMatrix, tplanetpos); // 1. 이동
	smallOrbitMatrix = glm::scale(smallOrbitMatrix, glm::vec3(1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f)); // 2. 스케일
	smallOrbitMatrix = glm::scale(smallOrbitMatrix, glm::vec3(orbitscale, orbitscale, orbitscale)); // 2. 스케일
	smallOrbitMatrix = zRotation * smallOrbitMatrix; // 3. z축 회전 (마지막 적용)
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(smallOrbitMatrix));
	glDrawArrays(GL_LINE_LOOP, 0, 101);
	
	// GLU 객체 그리기 (고정 파이프라인)
	glUseProgram(0);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	if (projectiontoggle == 0) {
		// 원근 투영
		gluPerspective(60.0, 1.0, 0.1, 300.0);
	}
	else {
		// 직교 투영
		glOrtho(-100.0, 100.0, -100.0, 100.0, 0.1, 300.0);
	}
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z,
			  cameraTarget.x, cameraTarget.y, cameraTarget.z,
			  cameraUp.x, cameraUp.y, cameraUp.z);
	
	// z축 회전을 GLU 객체에도 적용
	glRotatef(glm::degrees(rotationAngle), 0.0f, 0.0f, 1.0f);
	
	// 초록색 행성 그리기
	glPushMatrix();
	glTranslatef(tplanetpos.x, tplanetpos.y, tplanetpos.z);
	
	GLUquadricObj* qobj2 = gluNewQuadric();
	if (wiretoggle == 1) {
		gluQuadricDrawStyle(qobj2, GLU_LINE); // 와이어프레임
	}
	else {
		gluQuadricDrawStyle(qobj2, GLU_FILL); // 솔리드
	}
	glColor3f(0.0f, 1.0f, 0.0f);
	gluSphere(qobj2, 5.0, spherelevel, spherelevel);
	gluDeleteQuadric(qobj2);
	
	glPopMatrix();
	
	// 빨간색 위성 그리기
	glPushMatrix();
	glTranslatef(tplanetpos.x, tplanetpos.y, tplanetpos.z);
	glTranslatef(tmoonpos.x, tmoonpos.y, tmoonpos.z);
	
	GLUquadricObj* qobj3 = gluNewQuadric();
	if (wiretoggle == 1) {
		gluQuadricDrawStyle(qobj3, GLU_LINE); // 와이어프레임
	}
	else {
		gluQuadricDrawStyle(qobj3, GLU_FILL); // 솔리드
	}
	glColor3f(1.0f, 0.0f, 0.0f);
	gluSphere(qobj3, 2.5, spherelevel, spherelevel);
	gluDeleteQuadric(qobj3);
	
	glPopMatrix();
}