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
#include <vector>

#define MAXRECT 10 // 최대 사각형 개수
#define point 0
#define line 1
#define triangle 2
#define rectangle 3

std::random_device rd;

// random_device 를 통해 난수 생성 엔진을 초기화 한다.
std::mt19937 gen(rd());

std::uniform_int_distribution<int> dis(0, 256);
std::uniform_int_distribution<int> degreedis(0, 628);
//std::uniform_int_distribution<int> numdis(0, windowWidth - rectspace);

//--- 아래 5개 함수는 사용자 정의 함수 임
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
void setupBuffers();

//--- 필요한 변수 선언
GLint width, height;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체
GLuint VAO, VBO; //--- 버텍스 배열 객체, 버텍스 버퍼 객체
int nowdrawstate = 0; // 0: point, 1: line, 2: triangle, 3: rectangle

int outputmode = 0; // 0: fill, 1: line

//--- 메인 함수

typedef struct RET {
	GLdouble x1, y1, x2, y2;
	GLdouble Rvalue = 0.0;
	GLdouble Gvalue = 0.0;
	GLdouble Bvalue = 0.0;
	int movestyle = 0; // 0: 고정, 1: 튕기기, 2: 좌우 지그재구, 3: 사각 스파이럴 4: 원 스파이럴
	GLdouble angle = 0.0;
	int xdir = 0; // x 방향 이동 (1 or -1) / 원 스파이럴 시에는 x중앙값
	int ydir = 0; // y 방향 이동 (1 or -1) / 원 스파이럴 시에는 y중앙값
	int movinglimit = 0; // 움직임 제한 거리 (원 스파이럴 시에는 이게 반지름)
	int gap = 0; // 원 스파이럴 시에는 차이를 저장함
} ret;


ret morph(ret& after, ret& before) {
	int halfwidth = width / 2;
	int halfheight = height / 2;
	after.x1 = (before.x1 - halfwidth) / halfwidth;
	after.y1 = (before.y1 - halfheight) / -halfheight;
	after.x2 = (before.x2 - halfwidth) / halfwidth;
	after.y2 = (before.y2 - halfheight) / -halfheight;

	// 색상은 그대로 복사
	after.Rvalue = before.Rvalue;
	after.Gvalue = before.Gvalue;
	after.Bvalue = before.Bvalue;
	//after.level = before.level;

	return after;
}


bool ptinrect(int x, int y, ret& rect) {
	return (x >= rect.x1 && x <= rect.x2 && y >= rect.y1 && y <= rect.y2);
}

void Keyboard(unsigned char key, int x, int y);
void Mouse(int button, int state, int x, int y);
void TimerFunction(int value);

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
	width = 800;
	height = 800;

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

	//--- 세이더 프로그램 만들기
	glutDisplayFunc(drawScene); //--- 출력 콜백 함수
	glutReshapeFunc(Reshape);

	/*for (ret& rt : showingrect) {
		rt.Rvalue = dis(gen) / 256.0f;
		rt.Gvalue = dis(gen) / 256.0f;
		rt.Bvalue = dis(gen) / 256.0f;
	}*/

	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutTimerFunc(25, TimerFunction, 1);
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
	rColor = gColor = 0.0;
	bColor = 1.0; //--- 배경색을 파랑색으로 설정
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	// 각 사각형을 6개 정점으로 변환한 전체 데이터
	std::vector<float> allVertices;



	/*for (int i = 0; i < 4; ++i) { //사분면 초기화
		for (int j = 0; j < quadrantsize[i]; ++j) {
			ret after;
			morph(after, triangledata[i][j]); // morph 변환 적용

			// level은 그대로 사용 (사각형 그리기용)
			float x1 = (float)after.x1;
			float y1 = (float)after.y1;
			float x2 = (float)after.x2;
			float y2 = (float)after.y2;
			float r = (float)after.Rvalue;
			float g = (float)after.Gvalue;
			float b = (float)after.Bvalue;

			// 사각형을 위한 6개 정점: (x1,y1), (x1,y2), (x2,y2), (x1,y2), (x2,y2), (x2,y1)
			// 각 정점마다 위치(3) + 색상(3) = 6개 값

			// 첫 번째 삼각형: (x1,y1), (x1,y2), (x2,y2)


			allVertices.insert(allVertices.end(), {
				x2, y2, 0.0f, r, g, b,  // (x1, y1)
				(x2 + x1) / 2, y1, 0.0f, r, g, b,  // (x1, y2)
				x1, y2, 0.0f, r, g, b   // (x2, y2)
				});
		}
	}*/

	if (outputmode == 0) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(3.0f);
	}

	if (!allVertices.empty()) {
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		// 버퍼에 정점 데이터 업로드
		glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float),
			allVertices.data(), GL_DYNAMIC_DRAW);

		// 모든 사각형을 한 번에 그리기 (각 사각형당 6개 정점)

		int totalTriangles = 0;
		for (int i = 0; i < 4; ++i) {
			totalTriangles += quadrantsize[i];
		}

		glDrawArrays(GL_TRIANGLES, 0, totalTriangles * 3);
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
	case 'p':
	{
		outputmode = 0; // fill 모드
	}
	break;
	case 'l':
	{
		outputmode = 1; // line 모드
	}
	break;
	case 'c':
	{
		inittriangle(); // 사분면 초기화
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