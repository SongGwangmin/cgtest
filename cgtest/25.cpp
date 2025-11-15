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

float angle = 0.0f; // 회전 각도
float xangle = 0.0f;
float polygon_xpos = 0.0f;
float polygon_ypos = 0.0f;
float orbitAngle = 0.0f; // 조명 공전 각도

// 동적 회전축을 위한 전역 변수
glm::vec3 current_xaxis;
glm::vec3 current_yaxis;
glm::vec3 current_zaxis;

float lightOrbitRadius = 2.0f; // 조명 궤도 반지름

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

	



	void sendvertexdata(std::vector<float>& vbo) { // vbo에 정점 데이터 추가
		for (int poly = 0; poly < 2; ++poly) {
			// 각 삼각형의 노말 벡터 계산
			// 삼각형의 두 변 계산
			glm::vec3 edge1 = glm::vec3(vpos[poly][1] - vpos[poly][0]);
			glm::vec3 edge2 = glm::vec3(vpos[poly][2] - vpos[poly][0]);
			
			// 외적으로 노말 벡터 계산 및 정규화
			glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
			
			// 세 정점에 대해 동일한 노말 사용
			for (int vert = 0; vert < 3; ++vert) {
				// 정점 위치
				vbo.insert(vbo.end(), {
					vpos[poly][vert].x, vpos[poly][vert].y, vpos[poly][vert].z
					});
				
				// 계산된 노말 벡터
				vbo.insert(vbo.end(), {
					normal.x, normal.y, normal.z
					});
			}
		}
	}

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

	pointment p1{ 0.5,0.5,0.5 };
	pointment p2{ 0.5,0.5,-0.5 };
	pointment p3{ -0.5,0.5,-0.5 };
	pointment p4{ -0.5,0.5,0.5 };
	pointment p5{ 0.5,-0.5,0.5 };
	pointment p6{ 0.5,-0.5,-0.5 };
	pointment p7{ -0.5,-0.5,-0.5 };
	pointment p8{ -0.5,-0.5,0.5 };
	pointment p9{ 0, 0.5 ,0 };

	// glm::vec4로 축 좌표 정의 (회전 없이 기본 축)
	glm::vec4 xaxis1(10, 0, 0, 1);
	glm::vec4 xaxis2(-10, 0, 0, 1);
	glm::vec4 yaxis1(0, 10, 0, 1);
	glm::vec4 yaxis2(0, -10, 0, 1);
	glm::vec4 zaxis1(0, 0, 10, 1);
	glm::vec4 zaxis2(0, 0, -10, 1);

	// 축 데이터를 allVertices에 추가 (vec4의 x, y, z 멤버 사용)
	// 축은 노말 벡터가 필요없으므로 (0,0,0)으로 설정
	allVertices.insert(allVertices.end(), {
					xaxis1.x, xaxis1.y, xaxis1.z,
					1.0f, 0.0f, 0.0f }); // 위치 + 노말
	allVertices.insert(allVertices.end(), {
					xaxis2.x, xaxis2.y, xaxis2.z,
					1.0f, 0.0f, 0.0f });

	allVertices.insert(allVertices.end(), {
					yaxis1.x, yaxis1.y, yaxis1.z,
					1.0f, 0.0f, 0.0f });
	allVertices.insert(allVertices.end(), {
					yaxis2.x, yaxis2.y, yaxis2.z,
					1.0f, 0.0f, 0.0f });

	allVertices.insert(allVertices.end(), {
					zaxis1.x, zaxis1.y, zaxis1.z,
					1.0f, 0.0f, 0.0f });
	allVertices.insert(allVertices.end(), {
					zaxis2.x, zaxis2.y, zaxis2.z,
					1.0f, 0.0f, 0.0f });

	// 현재 축 벡터들 계산 (기본 축 사용)
	current_xaxis = glm::vec3(1.0f, 0.0f, 0.0f);
	current_yaxis = glm::vec3(0.0f, 1.0f, 0.0f);
	current_zaxis = glm::vec3(0.0f, 0.0f, 1.0f);

	// 육면체 면들 (외부를 향하도록 반시계방향 정점 순서)
	// 위 면 (y = 0.5) - 위에서 아래를 볼 때 반시계
	polygonmap.emplace_back(polygon(p1, p2, p3, p4, 1, 0, 0));
	
	// 아래 면 (y = -0.5) - 아래에서 위를 볼 때 반시계
	polygonmap.emplace_back(polygon(p5, p6, p7, p8, 1, 1, 0));
	
	// 앞 면 (z = 0.5) - 앞에서 뒤를 볼 때 반시계
	polygonmap.emplace_back(polygon(p1, p4, p8, p5, 0, 0, 1));
	
	// 뒷 면 (z = -0.5) - 뒤에서 앞을 볼 때 반시계
	polygonmap.emplace_back(polygon(p2, p6, p7, p3, 1, 0, 1));
	
	// 왼쪽 면 (x = -0.5) - 왼쪽에서 오른쪽을 볼 때 반시계
	polygonmap.emplace_back(polygon(p4, p3, p7, p8, 0, 1, 0));
	
	// 오른쪽 면 (x = 0.5) - 오른쪽에서 왼쪽을 볼 때 반시계
	polygonmap.emplace_back(polygon(p1, p5, p6, p2, 0, 1, 1));

	// 피라미드 면들 (외부를 향하도록 반시계방향)
	// 앞 면 (z = 0.5 쪽)
	polygonmap.emplace_back(polygon(p5, p6, p9, 1, 0, 0));
	
	// 오른쪽 면 (x = 0.5 쪽)
	polygonmap.emplace_back(polygon(p6, p7, p9, 0, 1, 0));
	
	// 뒷 면 (z = -0.5 쪽)
	polygonmap.emplace_back(polygon(p7, p8, p9, 0, 0, 1));
	
	// 왼쪽 면 (x = -0.5 쪽)
	polygonmap.emplace_back(polygon(p8, p5, p9, 1, 1, 0));


	// polygon 회전 제거 - 기본 상태로 유지
	for (auto poly = polygonmap.begin(); poly != polygonmap.end(); ++poly) {
		poly->sendvertexdata(allVertices);
	}

	// ZX 평면에 원 그리기 (Y = 0)
	int circleSegments = 36; // 원을 36개의 선분으로
	float radius = 2.0f;
	
	for (int i = 0; i < circleSegments; ++i) {
		float angle1 = (float)i * 2.0f * pi / circleSegments;
		float angle2 = (float)(i + 1) * 2.0f * pi / circleSegments;
		
		// 첫 번째 점 (ZX 평면이므로 y=0)
		float x1 = radius * cos(angle1);
		float z1 = radius * sin(angle1);
		allVertices.insert(allVertices.end(), {
			x1, 0.0f, z1,           // 위치
			0.0f, 1.0f, 0.0f        // 흰색 (노말 대신 색상 정보로 사용)
		});
		
		// 두 번째 점
		float x2 = radius * cos(angle2);
		float z2 = radius * sin(angle2);
		allVertices.insert(allVertices.end(), {
			x2, 0.0f, z2,           // 위치
			0.0f, 1.0f, 0.0f        // 흰색
		});
	}

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
	bColor = 0.0; //--- 배경색을 파랑색으로 설정
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	// 모델 변환 행렬 설정
	
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, angle, current_yaxis);


	// 뷰 변환 행렬 설정 - 카메라를 x, y, z축으로 0.2씩 이동
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::lookAt(
		glm::vec3(2.2f, 2.2f, 4.2f), // 카메라 위치
		glm::vec3(0.0f, 0.0f, 0.0f), // 바라보는 점
		glm::vec3(0.0f, 1.0f, 0.0f)  // 업 벡터
	);

	// 투영 변환 행렬 설정 (직교 투영)
	glm::mat4 projection = glm::perspective(
		glm::radians(45.0f), // 시야각
		(float)width / (float)height, // 종횡비
		0.1f, // 근평면
		100.0f // 원평면
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
	glm::vec3 lightPos(lightX, 0.0f, lightZ); // ZX 평면에서 회전
	
	glm::vec3 viewPos(2.2f, 2.2f, 4.2f); // 카메라 위치와 동일하게
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f); // 흰색 조명으로 수정
	glm::vec3 objectColor(1.0f, 0.5f, 0.31f); // 주황색 객체

	glUniform3fv(lightPosLocation, 1, glm::value_ptr(lightPos));
	glUniform3fv(viewPosLocation, 1, glm::value_ptr(viewPos));
	glUniform3fv(lightColorLocation, 1, glm::value_ptr(lightColor));
	glUniform3fv(objectColorLocation, 1, glm::value_ptr(objectColor));

	if (!allVertices.empty()) {
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		// 버퍼에 정점 데이터 업로드
		glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float),
			allVertices.data(), GL_DYNAMIC_DRAW);
	}

	glLineWidth(2.0f);

	// 축은 변환 없이 그리기 (단위 행렬 적용)
	glm::mat4 identityMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(identityMatrix));

	objectColor = glm::vec3(1.0f, 0.0f, 0.0f); // 빨간 x축
	glUniform3fv(objectColorLocation, 1, glm::value_ptr(objectColor));
	glDrawArrays(GL_LINES, 0, 2);

	objectColor = glm::vec3(0.0f, 1.0f, 0.0f); // 초록 y축
	glUniform3fv(objectColorLocation, 1, glm::value_ptr(objectColor));
	glDrawArrays(GL_LINES, 2, 2);
	objectColor = glm::vec3(0.0f, 0.0f, 1.0f); // 파란 z축
	glUniform3fv(objectColorLocation, 1, glm::value_ptr(objectColor));
	glDrawArrays(GL_LINES, 4, 2);

	// ZX 평면의 원 그리기 (흰색)
	objectColor = glm::vec3(1.0f, 1.0f, 1.0f); // 흰색
	glUniform3fv(objectColorLocation, 1, glm::value_ptr(objectColor));
	
	// 원을 lightOrbitRadius / 2.0 크기로 스케일
	glm::mat4 circleScale = glm::scale(glm::mat4(1.0f), glm::vec3(lightOrbitRadius / 2.0f, 1.0f, lightOrbitRadius / 2.0f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(circleScale));
	glDrawArrays(GL_LINES, 66, 72); // 36개 선분 = 72개 정점 (36*2)
	
	// 다시 단위 행렬로 복원
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(identityMatrix));

	// 조명 위치 표시 (노란색 점)
	float lightMarkerX = lightOrbitRadius * cos(orbitAngle);
	float lightMarkerZ = lightOrbitRadius * sin(orbitAngle);
	
	// 조명 위치에 작은 선으로 표시
	std::vector<float> lightMarker = {
		lightMarkerX, -0.1f, lightMarkerZ, 0.0f, 1.0f, 0.0f,
		lightMarkerX, 0.1f, lightMarkerZ, 0.0f, 1.0f, 0.0f
	};
	
	// VBO에 임시로 조명 마커 추가
	size_t originalSize = allVertices.size();
	allVertices.insert(allVertices.end(), lightMarker.begin(), lightMarker.end());
	glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float),
		allVertices.data(), GL_DYNAMIC_DRAW);
	
	objectColor = glm::vec3(1.0f, 1.0f, 0.0f); // 노란색
	glUniform3fv(objectColorLocation, 1, glm::value_ptr(objectColor));
	glPointSize(10.0f);
	glDrawArrays(GL_LINES, originalSize / 6, 2);
	
	// 원래 크기로 복원
	allVertices.resize(originalSize);

	objectColor = glm::vec3(1.0f, 0.5f, 0.31f); // 주황색 객체
	glUniform3fv(objectColorLocation, 1, glm::value_ptr(objectColor));
	

	// polygon들은 변환 행렬 적용해서 그리기
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	if (wiretoggle) {
		for (int i = 0; i < 10; ++i) {
			if (selection[i]) {
				glDrawArrays(GL_LINES, 6 + 6 * i, 6);
			}
		}
	}
	else {
		for (int i = 0; i < 10; ++i) {
			if (selection[i]) {
				glDrawArrays(GL_TRIANGLES, 6 + 6 * i, 6);
			}
		}
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
	case 'p':
	{
		if (selection[0]) {
			int copy[10] = { 0,2,0,0,0,0,1,1,1,1 };
			for (int i = 0; i < 10; ++i) {
				selection[i] = copy[i];
			}
		}
		else {
			int copy[10] = { 1,1,1,1,1,1,0,0,0,0 };
			for (int i = 0; i < 10; ++i) {
				selection[i] = copy[i];
			}
		}
	}
	break;
	case 'c': // 육면체
	{
		int copy[10] = { 1,1,1,1,1,1,0,0,0,0 };




		for (int i = 0; i < 10; ++i) {
			selection[i] = copy[i];
		}
	}
	break;
	case 't':
	{
		int copy[10] = { 0,0,0,0,0,1,1,1,1,1 };

		for (int i = 0; i < 10; ++i) {
			selection[i] = copy[i];
		}
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
	case 'w': // 와이어객체
	{
		wiretoggle = 1;
	}
	break;
	case 'W': // 솔리드객체
	{
		wiretoggle = 0;

	}
	break;
	case 'x': // x축 기준 양방향 회전애니메이션(자전)
	{
		xangle += 0.02f;
	}
	break;
	case 'X': // x축 기준 음방향 회전애니메이션(자전)
	{
		xangle -= 0.02f;

	}
	break;
	case 'y': // y축 기준 양방향 회전애니메이션(자전)
	{
		angle += 0.02f;
	}
	break;
	case 'Y': // y축 기준 음방향 회전애니메이션(자전)
	{
		angle -= 0.02f;
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