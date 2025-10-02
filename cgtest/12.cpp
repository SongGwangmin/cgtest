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
//--- 메인 함수

float GuideFrame[4][3][3][2] = {
	// [0] 선 (3개, 각 3점)
	{
		{ {0,0}, {100,100}, {50,50} },
		{ {0,0}, {100,100}, {50,50} },
		{ {0,0}, {100,100}, {50,50} }
	},

	// [1] 삼각형 (3개)
	{
		{ {100,100}, {50,0}, {0,100} },
		{ {100,100}, {50,0}, {0,100} },
		{ {100,100}, {50,0}, {0,100} }
	},

	// [2] 사각형 → 삼각형 분해 3개
	{
		{ {100,100}, {0,0}, {0,100} },
		{ {100,100}, {0,100}, {100,0} },
		{ {100,100}, {100,0}, {0,0} }
	},

	// [3] 오각형 → 삼각형 분해 3개
	{
		{ {50,0}, {0,40}, {100,40} },
		{ {0,40}, {20,100}, {100,40} },
		{ {20,100}, {80,100}, {100,40} }
	}
};

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
	float vertexpos[3][3][2];
	bool needchange;
	GLdouble Rvalue = 0.0;
	GLdouble Gvalue = 0.0;
	GLdouble Bvalue = 0.0;
	GLdouble x1, y1, x2, y2;
	int membershape; //  0: line, 1: triangle, 2: rectangle, 3: pentagon

public:
	//std::vector<ret> rects;
	polygon(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2, 
		GLdouble rvalue, GLdouble gvalue, GLdouble bvalue, int membershape) 
		: x1(x1), y1(y1), x2(x2), y2(y2), Rvalue(rvalue), Gvalue(gvalue), Bvalue(bvalue), membershape(membershape){
		
		needchange = false;

		for (int poly = 0; poly < 3; ++poly) {
			for (int vert = 0; vert < 3; ++vert) {
				for (int pos = 0; pos < 2; ++pos) {
					vertexpos[poly][vert][pos] = GuideFrame[membershape][poly][vert][pos];
				}
			}
		}
	}
	
	int changeShape(int targetshape) {

	}

	void resetShape(int targetshape) {

	}

	void sendvertexdata(std::vector<float>& vbo) {

	}
};


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
	after.level = before.level;

	return after;
}

bool ptinrect(int x, int y, ret& rect) {
	return (x >= rect.x1 && x <= rect.x2 && y >= rect.y1 && y <= rect.y2);
}

void Keyboard(unsigned char key, int x, int y);
void Mouse(int button, int state, int x, int y);

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

	

	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);

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

	/*for (int i = 0; i < nowdrawsize; i++) {
		ret after;
		morph(after, showingrect[i]); // morph 변환 적용

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
		if (showingrect[i].level != triangle) {
			allVertices.insert(allVertices.end(), {
				x2, y2, 0.0f, r, g, b,  // (x1, y1)
				x1, y1, 0.0f, r, g, b,  // (x1, y2)
				x1, y2, 0.0f, r, g, b   // (x2, y2)
				});
		}
		else {
			allVertices.insert(allVertices.end(), {
				x2, y2, 0.0f, r, g, b,  // (x1, y1)
				(x2 + x1) / 2, y1, 0.0f, r, g, b,  // (x1, y2)
				x1, y2, 0.0f, r, g, b   // (x2, y2)
				});
		}

		// 두 번째 삼각형: (x1,y1), (x2,y2), (x2,y1)
		allVertices.insert(allVertices.end(), {
			x1, y1, 0.0f, r, g, b,  // (x1, y1)
			x2, y2, 0.0f, r, g, b,  // (x2, y2)
			x2, y1, 0.0f, r, g, b   // (x2, y1)
			});
	}*/

	if (!allVertices.empty()) {
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		// 버퍼에 정점 데이터 업로드
		glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float),
			allVertices.data(), GL_DYNAMIC_DRAW);

		// 모든 사각형을 한 번에 그리기 (각 사각형당 6개 정점)

	}

	/*for (int i = 0; i < nowdrawsize; ++i) {
		switch (showingrect[i].level) {
		case point:
		{
			glDrawArrays(GL_TRIANGLES, i * 6, 6);


		}
		break;
		case line:
		{
			glLineWidth(2.0f);
			glDrawArrays(GL_LINES, i * 6, 2);
		}
		break;
		case triangle:
		{
			glDrawArrays(GL_TRIANGLES, i * 6, 3);
		}
		break;
		case rectangle:
		{
			// 사각형 그리기
			// (x1, y1) -> (x1, y2) -> (x2, y2) -> (x2, y1) -> (x1, y1)
			glDrawArrays(GL_TRIANGLES, i * 6, 6);
		}
		break;
		}

		if (whereiscursor >= 0 && whereiscursor == i) { // 선택된 도형만 윤곽선 그리기
			glColor3f(0.0f, 0.0f, 0.0f);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(3.0f);

			switch (showingrect[i].level) { // i 사용해야 함!
			case point:
				glDrawArrays(GL_TRIANGLES, i * 6, 6);
				break;
			case triangle:
				glDrawArrays(GL_TRIANGLES, i * 6, 3);
				break;
			case rectangle:
				glDrawArrays(GL_TRIANGLES, i * 6, 6);
				break;
			}

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}*/



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
		nowdrawstate = point;
	}
	break;
	case 'l':
	{
		nowdrawstate = line;
	}
	break;
	case 't':
	{
		nowdrawstate = triangle;
	}
	break;
	case 'r':
	{
		nowdrawstate = rectangle;
	}
	break;
	// WASD 키로 선택된 사각형 이동
	case 'w': // 위로 이동
	{
		
	}
	break;
	case 's': // 아래로 이동
	{
		
	}
	break;
	case 'a': // 왼쪽으로 이동
	{

	}
	break;
	case 'd': // 오른쪽으로 이동
	{
		
	}
	break;
	// IJKL 키로 선택된 사각형 대각선 이동
	case 'i': // 좌상단으로 이동 (위 + 왼쪽)
	{
		
	}
	break;
	case 'j': // 좌하단으로 이동 (아래 + 왼쪽)
	{
		
	}
	break;
	case 'k': // 우하단으로 이동 (아래 + 오른쪽)
	{
		
	}
	break;
	case 'o': // 우상단으로 이동 (위 + 오른쪽) - 'l'의 원래 기능 변경됨 - change to 'o'
	{
		
	}
	break;
	/*case 'r': // 리셋: 모든 것을 초기 상태로 되돌리기
	{
	   nowdrawsize = 0; // 모든 사각형 제거
	}
		break;*/
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
			// 새로운 사각형 추가 (예시)
			
		}
	}
	break;
	default:
		break;
	}
}