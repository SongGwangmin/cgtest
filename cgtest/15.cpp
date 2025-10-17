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

#define MAXRECT 10 // 최대 사각형 개수
#define point 0
#define line 1
#define triangle 2
#define rectangle 3
#define pentagon 4
#define polygonwidth 100
#define pi 3.14

std::random_device rd;

// random_device 를 통해 난수 생성 엔진을 초기화 한다.
std::mt19937 gen(rd());

std::uniform_int_distribution<int> dis(0, 256);
std::uniform_int_distribution<int> posdis(0, 700);
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
int animation = 1; // 0: 정지, 1: 회전

// Forward declaration
class polygon;
std::list<polygon> polygonmap;
std::list<polygon>::iterator mouse_dest; // 마우스로 선택된 polygon 저장

typedef struct poitment {
	GLdouble xpos;
	GLdouble ypos;
	GLdouble zpos;
} pointment;

typedef struct RET {
	GLdouble x1, y1, x2, y2;
	GLdouble Rvalue = 0.0;
	GLdouble Gvalue = 0.0;
	GLdouble Bvalue = 0.0;
	int level = 3;
} ret;


// 도형 저장하는 클래스
class polygon {
private:
	float vertexpos[2][3][3]; // [도형번호][정점번호][x,y,z]
	bool needchange;
	GLdouble Rvalue = 0.0;
	GLdouble Gvalue = 0.0;
	GLdouble Bvalue = 0.0;
	GLdouble x1, y1, z1, x2, y2, z2;
	float angle[3];
	float radius[3];
	int membershape; //  0: line, 1: triangle, 2: rectangle, 3: pentagon
	int xdir = 400; // 중점
	int ydir = 400;
	int needmove = 0;
	int inner = 0; // mouse 선택되었는지 여부

public:
	//std::vector<ret> rects;
	polygon(pointment p1, pointment p2, pointment p3, GLdouble rv, GLdouble gv, GLdouble bv) {
		


		

	}


	void update(float theta) {

	}

	

	

	void sendvertexdata(std::vector<float>& vbo) { // vbo에 정점 데이터 추가
		GLdouble centerx = (x1 + x2) / 2;
		GLdouble centery = (y1 + y2) / 2;

		centerx -= polygonwidth / 2;
		centery -= polygonwidth / 2;


		for (int vert = 0; vert < 3; ++vert) {

			float virtualx = vertexpos[vert][0];
			float virtualy = vertexpos[vert][1];

			/*if (!membershape) {
				virtualx = vertexpos[vert][0] + x1;
				virtualy = vertexpos[vert][1] + y1;
			}*/

			float finalx = (virtualx - (width / 2)) / (width / 2);
			float finaly = (virtualy - (height / 2)) / -(height / 2);

			vbo.insert(vbo.end(), {
				finalx, finaly, 0.0f, (float)Rvalue, (float)Gvalue, (float)Bvalue
				});
		}


	}

};



bool ptinrect(int x, int y, ret& rect) {
	return (x >= rect.x1 && x <= rect.x2 && y >= rect.y1 && y <= rect.y2);
}

void Keyboard(unsigned char key, int x, int y);
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

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//width = 800;
	//height = 800;

	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
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

	

	//polygonmap.emplace_back(polygon(400 - 150, 400 + 150, 400 + 150, 400 + 300, 3));



	//--- 세이더 프로그램 만들기
	glutDisplayFunc(drawScene); //--- 출력 콜백 함수
	glutReshapeFunc(Reshape);

	glutTimerFunc(25, TimerFunction, 1);

	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion); // 마우스 모션 콜백 등록

	glutMainLoop();
}

void make_vertexShaders()
{
	GLchar* vertexSource;
	//--- 버텍스 세이더 읽어 저장하고 컴파일 하기
	//--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
	vertexSource = filetobuf("vertex.glsl");
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
	fragmentSource = filetobuf("fragment.glsl"); // 프래그세이더 읽어오기
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
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	// 각 사각형을 6개 정점으로 변환한 전체 데이터
	std::vector<float> allVertices;


	for (auto poly = polygonmap.begin(); poly != polygonmap.end(); ++poly) {
		poly->sendvertexdata(allVertices);
	}



	if (!allVertices.empty()) {
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		// 버퍼에 정점 데이터 업로드
		glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float),
			allVertices.data(), GL_DYNAMIC_DRAW);

		// 모든 사각형을 한 번에 그리기 (각 사각형당 6개 정점)

	}

	for (int i = 0; i < 20; ++i) {
		glLineWidth(2.0f);
		glDrawArrays(GL_LINES, 0, 4);
		glDrawArrays(GL_TRIANGLES, i * 3, 3);

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
	case 'r': // vertexpos[0] 대칭이동
	{
		
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
