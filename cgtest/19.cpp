#define _CRT_SECURE_NO_WARNINGS //--- ���α׷� �� �տ� ������ ��
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

#define MAXRECT 10 // �ִ� �簢�� ����
#define point 0
#define line 1
#define triangle 2
#define rectangle 3
#define pentagon 4
#define polygonwidth 100
#define pi 3.14159265358979323846

std::random_device rd;

// random_device �� ���� ���� ���� ������ �ʱ�ȭ �Ѵ�.
std::mt19937 gen(rd());

std::uniform_int_distribution<int> dis(0, 256);
std::uniform_int_distribution<int> polyrandom(0, 24);
//std::uniform_int_distribution<int> numdis(0, windowWidth - rectspace);

//--- �Ʒ� 5�� �Լ��� ����� ���� �Լ� ��
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
void setupBuffers();
void TimerFunction(int value);
void drawOrbitsAndPlanets(glm::vec3 cameraPos, glm::vec3 cameraTarget, glm::vec3 cameraUp, float rotationAngle);

//--- �ʿ��� ���� ����
GLint width = 800, height = 800;
GLuint shaderProgramID; //--- ���̴� ���α׷� �̸�
GLuint vertexShader; //--- ���ؽ� ���̴� ��ü
GLuint fragmentShader; //--- �����׸�Ʈ ���̴� ��ü
GLuint VAO, VBO; //--- ���ؽ� �迭 ��ü, ���ؽ� ���� ��ü
int nowdrawstate = 0; // 0: point, 1: line, 2: triangle, 3: rectangle
int selectedshape = -1; // ���õ� ���� �ε���
int spin = 1; //  1: �ð����, -1: �ݽð����
int animation = 0; // 0: ����, 1: ȸ��
int hidetoggle = 1; // 1. ��������
int wiretoggle = 0; // 1. ���̾������� ���
int culltoggle = 0; // 1. �޸� �ø� ���

// cube ���� ��� ����
int zrotoggle = 0; // 1. z�� ȸ�� ���
int opentoggle = 0; // 1. ���� ���� ���
int tiretoggle = 0; // 1. ������ ȸ��
int backsizetoggle = 0; // 1. �޸� size ���

// ��ġ ��ȯ �ִϸ��̼� ���� ����
int swapAnimationActive = 0; // 0: ����, 1: �ִϸ��̼� ��
float swapAnimationProgress = 0.0f; // 0.0 ~ 1.0
glm::vec3 swapStartPos[2]; // ���� ��ġ
glm::vec3 swapEndPos[2]; // ��ǥ ��ġ

// y�� ȸ�� �ִϸ��̼� ���� ����
int yRotationAnimationActive = 0; // 0: ����, 1: �ִϸ��̼� ��

// ���ο� ��� ������
int edgeopentoggle = 0; // 0: edge ����, 1: edge ����
int backscaletoggle = 0; // 1: scale ����, 0: scale ����

// rŰ ���� ������ ���� ������
int rsequence = 0; // 0: ����, 1: ���� ����, 2: ���� �ݱ�
int rcurrentface = 0; // ���� ���� ���� �� (0:t1, 1:t2, 2:t3, 3:t4)

// piriamid ���� ��� ����
int openeverytoggle = 0; // 1. ��� �� ���� ���	
int sequentopnetoggle = 0; // 1. ���� ���������� ����
int sequentoclosetoggle = 0; // 1. ���� ���������� �ݸ�

// ���� ���� ��� ����
int projectiontoggle = 0; // 0: �⺻ ����, 1: �ٸ� ����

// cube ���� ����
float topangle = 0.0f; // ���� ȸ�� ����
float oepnangle = 0.0f; // front ������ ����
float tireangle = 0.0f; // ���� ȸ�� ����
float backsize = 1.0f; // �޸� ũ��

// piramid ���� ����
float t1angle = 0.0f; // ��1 ȸ�� ����
float t2angle = 0.0f; // ��2 ȸ�� ����
float t3angle = 0.0f; // ��3 ȸ�� ����
float t4angle = 0.0f; // ��4 ȸ�� ����


// Forward declaration
class polygon;
std::vector<polygon> polygonmap;
int mouse_dest = -1; // ���콺�� ���õ� polygon �ε��� ����
std::vector<float> allVertices;

int selection[10] = { 1,1,1,1,1,1,0,0,0,0 };
int currentObject = 0; // ���� ���õ� ��ü (0 �Ǵ� 1)

float angle = 0.0f; // ȸ�� ����
float xangle = 0.0f;
float polygon_xpos = 0.0f;
float polygon_ypos = 0.0f;

// �༺ ��ġ
glm::vec3 planetpos = glm::vec3(50.0f, 0.0f, 0.0f);

// ���� ��ġ (�༺ ���� ��� ��ǥ)
glm::vec3 moonpos = glm::vec3(50.0f / 3.0f, 0.0f, 0.0f);

// ���� ȸ������ ���� ���� ����
glm::vec3 current_xaxis;
glm::vec3 current_yaxis;
glm::vec3 current_zaxis;

// ���� Ÿ�� ������
enum ShapeType {
	SHAPE_CUBE = 0,      // ����ü
	SHAPE_SPHERE = 1,    // ��
	SHAPE_CONE = 2,      // 12����
	SHAPE_CYLINDER = 3   // 12�����
};

typedef struct poitment {
	float xpos;
	float ypos;
	float zpos;
} pointment;

// ��ȯ ������ �����ϴ� ����ü
typedef struct TransformInfo {
	float xRotation;      // x�� ���� ����
	float yRotation;      // y�� ���� ����
	float localScale;     // ���ڸ� scale ũ��
	glm::vec3 position;   // ��ġ (x, y, z)
	glm::vec3 midPoint;   // �߰� ���� ����
	ShapeType shapeType;  // ���� Ÿ��
} TransformInfo;

// 2��¥�� �迭 ����
TransformInfo transformArray[2];


// ���� �����ϴ� Ŭ����
class polygon {
private:
	GLdouble Rvalue = 0.0;
	GLdouble Gvalue = 0.0;
	GLdouble Bvalue = 0.0;
	glm::vec4 vpos[2][3];
	int needmove = 0;
	int inner = 0; // mouse ���õǾ����� ����

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



	void sendvertexdata(std::vector<float>& vbo) { // vbo�� ���� ������ �߰�
		for (int poly = 0; poly < 2; ++poly) {
			for (int vert = 0; vert < 3; ++vert) {
				vbo.insert(vbo.end(), {
					vpos[poly][vert].x, vpos[poly][vert].y, vpos[poly][vert].z, (float)Rvalue, (float)Gvalue, (float)Bvalue
					});
			}
		}
	}

	// ��� ��� �Լ�
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
		// �갡 transform�� �ɰ���
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
void SpecialKeys(int key, int x, int y); // Ư�� Ű(ȭ��ǥ Ű) �ݹ� �Լ� ����
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y); // ���콺 ��� �ݹ� �Լ� ����

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

	// ���� �Ӽ� ����: ��ġ (3��) + ���� (3��) = �� 6�� float
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

int main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//width = 800;
	//height = 800;

	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Rectangle Rendering");
	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	glewInit();
	//--- ���̴� �о�ͼ� ���̴� ���α׷� �����: ����� �����Լ� ȣ��
	make_vertexShaders(); //--- ���ؽ� ���̴� �����
	make_fragmentShaders(); //--- �����׸�Ʈ ���̴� �����
	shaderProgramID = make_shaderProgram();

	// ���� ����
	setupBuffers();
	glEnable(GL_DEPTH_TEST);
	allVertices.clear();

	

	// ���� �׸��� ���� ���� ���� (0, 0, -50)�� y������ ȸ��
	int circleSegments = 100; // ���� ������ ���� ����
	float radius = 50.0f; // ������
	glm::vec3 startPoint(0.0f, 0.0f, -radius); // ������
	
	for (int i = 0; i <= circleSegments; ++i) {
		float angle = (2.0f * pi * i) / circleSegments; // 0 ~ 2��
		
		// y�� ȸ�� ��� ����
		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec4 rotatedPoint = rotationMatrix * glm::vec4(startPoint, 1.0f);
		
		// �� �߰�
		allVertices.insert(allVertices.end(), {
			rotatedPoint.x, rotatedPoint.y, rotatedPoint.z,
			0.0f, 0.0f, 0.0f // ������
		});
	}



	//--- ���̴� ���α׷� �����
	glutDisplayFunc(drawScene); //--- ��� �ݹ� �Լ�
	glutReshapeFunc(Reshape);

	glutTimerFunc(25, TimerFunction, 1);

	glutKeyboardFunc(Keyboard);

	glutMainLoop();
	return 0;
}

void make_vertexShaders()
{
	GLchar* vertexSource;
	//--- ���ؽ� ���̴� �о� �����ϰ� ������ �ϱ�
	//--- filetobuf: ��������� �Լ��� �ؽ�Ʈ�� �о ���ڿ��� �����ϴ� �Լ�
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
		std::cerr << "ERROR: vertex shader ������ ����\n" << errorLog << std::endl;
		return;
	}
}

void make_fragmentShaders()
{
	GLchar* fragmentSource;
	//--- �����׸�Ʈ ���̴� �о� �����ϰ� �������ϱ�
	fragmentSource = filetobuf("fragment_matrix.glsl"); // �����׼��̴� �о����
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader ������ ����\n" << errorLog << std::endl;
		return;
	}
}

GLuint make_shaderProgram()
{
	GLint result;
	GLchar* errorLog = NULL;
	GLuint shaderID;
	shaderID = glCreateProgram(); //--- ���̴� ���α׷� �����
	glAttachShader(shaderID, vertexShader); //--- ���̴� ���α׷��� ���ؽ� ���̴� ���̱�
	glAttachShader(shaderID, fragmentShader); //--- ���̴� ���α׷��� �����׸�Ʈ ���̴� ���̱�
	glLinkProgram(shaderID); //--- ���̴� ���α׷� ��ũ�ϱ�
	glDeleteShader(vertexShader); //--- ���̴� ��ü�� ���̴� ���α׷��� ��ũ��������, ���̴� ��ü ��ü�� ���� ����
	glDeleteShader(fragmentShader);
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result); // ---���̴��� �� ����Ǿ����� üũ�ϱ�
	if (!result) {
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program ���� ����\n" << errorLog << std::endl;
		return false;
	}
	glUseProgram(shaderID); //--- ������� ���̴� ���α׷� ����ϱ�
	return shaderID;
}

GLvoid drawScene() //--- �ݹ� �Լ�: �׸��� �ݹ� �Լ�
{
	GLfloat rColor, gColor, bColor;
	rColor = gColor = 1.0;
	bColor = 1.0; //--- ������ ������� ����
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// ī�޶� ����
	glm::vec3 cameraPos = glm::vec3(0.0f, 10.0f, 150.0f);
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	
	// ���������� 1: �˵��� �༺/���� �׸��� (z�� ȸ�� ����)
	drawOrbitsAndPlanets(cameraPos, cameraTarget, cameraUp, 0.0f); // angle�� 0���� ���� (���߿� ���� ����)
	drawOrbitsAndPlanets(cameraPos, cameraTarget, cameraUp, pi / 4.0f);
	drawOrbitsAndPlanets(cameraPos, cameraTarget, cameraUp, -pi / 4.0f);
	
	// ���������� 2: �׼�(�¾�) �׸���
	glUseProgram(0); // ���� ���������� ���
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	if (projectiontoggle == 0) {
		// ���� ����
		gluPerspective(60.0, 1.0, 0.1, 300.0);
	}
	else {
		// ���� ����
		glOrtho(-100.0, 100.0, -100.0, 100.0, 0.1, 300.0);
	}
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z,
			  cameraTarget.x, cameraTarget.y, cameraTarget.z,
			  cameraUp.x, cameraUp.y, cameraUp.z);
	
	// ����(0,0,0)�� �Ķ��� �׼�(�¾�) �׸���
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.0f);
	
	GLUquadricObj* qobj = gluNewQuadric();
	gluQuadricDrawStyle(qobj, GLU_FILL);
	glColor3f(0.0f, 0.0f, 1.0f); // �Ķ���
	gluSphere(qobj, 10.0, 30, 30); // ������ 10
	gluDeleteQuadric(qobj);
	
	glPopMatrix();

	glutSwapBuffers(); // ȭ�鿡 ����ϱ�
}

//--- �ٽñ׸��� �ݹ� �Լ�
GLvoid Reshape(int w, int h) //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
	glViewport(0, 0, w, h);
}

void Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'q': // ���α׷� ����
		glutLeaveMainLoop();
		break;
	case 'Q': // ���α׷� ����
		glutLeaveMainLoop();
		break;
	case 'w': // ���̾������� ��� ����/����
	{

	}
	break;
	case 'z': // z�� ȸ�� ��� ����/����
	{
	}
	break;
	case 'p': // ���� ��� ����
	{
		projectiontoggle = !projectiontoggle;
	}
	break;
	case 'h': // �������� ����/����
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
	case '+':
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

	glutPostRedisplay();
	glutTimerFunc(25, TimerFunction, 1);
}

void Motion(int x, int y) // ���콺 ��� �ݹ� �Լ�
{

}

// �˵��� �༺/���� �׸��� �Լ�
void drawOrbitsAndPlanets(glm::vec3 cameraPos, glm::vec3 cameraTarget, glm::vec3 cameraUp, float rotationAngle)
{
	// ���̴� ���
	glUseProgram(shaderProgramID);
	
	// ���� ��� ����
	glm::mat4 projection;
	if (projectiontoggle == 0) {
		// ���� ���� (Perspective)
		projection = glm::perspective(
			(float)(pi / 3.0f),
			1.0f,
			0.1f,
			300.0f
		);
	}
	else {
		// ���� ���� (Orthographic)
		projection = glm::ortho(
			-100.0f, 100.0f,  // left, right
			-100.0f, 100.0f,  // bottom, top
			0.1f, 300.0f      // near, far
		);
	}
	unsigned int projectionLocation = glGetUniformLocation(shaderProgramID, "projectionTransform");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
	
	// �� ��� ����
	glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
	unsigned int viewLocation = glGetUniformLocation(shaderProgramID, "viewTransform");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
	
	unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "modelTransform");
	
	// ���� ���ε�
	if (!allVertices.empty()) {
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float),
			allVertices.data(), GL_DYNAMIC_DRAW);
	}
	
	glLineWidth(2.0f);
	
	// z�� ȸ�� ���
	glm::mat4 zRotation = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0.0f, 0.0f, 1.0f));
	
	// ���� �˵� �׸��� (z�� ȸ���� ����)
	glm::mat4 orbitModel = zRotation;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(orbitModel));
	glDrawArrays(GL_LINE_LOOP, 0, 101);
	
	// ���ο� �˵� �׸��� - planetpos�� �̵� -> 1/3 ������ -> z�� ȸ�� ����
	glm::mat4 smallOrbitMatrix = glm::mat4(1.0f);
	smallOrbitMatrix = glm::translate(smallOrbitMatrix, planetpos); // 1. �̵�
	smallOrbitMatrix = glm::scale(smallOrbitMatrix, glm::vec3(1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f)); // 2. ������
	smallOrbitMatrix = zRotation * smallOrbitMatrix; // 3. z�� ȸ�� (������ ����)
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(smallOrbitMatrix));
	glDrawArrays(GL_LINE_LOOP, 0, 101);
	
	// GLU ��ü �׸��� (���� ����������)
	glUseProgram(0);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	if (projectiontoggle == 0) {
		// ���� ����
		gluPerspective(60.0, 1.0, 0.1, 300.0);
	}
	else {
		// ���� ����
		glOrtho(-100.0, 100.0, -100.0, 100.0, 0.1, 300.0);
	}
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z,
			  cameraTarget.x, cameraTarget.y, cameraTarget.z,
			  cameraUp.x, cameraUp.y, cameraUp.z);
	
	// z�� ȸ���� GLU ��ü���� ����
	glRotatef(glm::degrees(rotationAngle), 0.0f, 0.0f, 1.0f);
	
	// �ʷϻ� �༺ �׸���
	glPushMatrix();
	glTranslatef(planetpos.x, planetpos.y, planetpos.z);
	
	GLUquadricObj* qobj2 = gluNewQuadric();
	gluQuadricDrawStyle(qobj2, GLU_FILL);
	glColor3f(0.0f, 1.0f, 0.0f);
	gluSphere(qobj2, 5.0, 30, 30);
	gluDeleteQuadric(qobj2);
	
	glPopMatrix();
	
	// ������ ���� �׸���
	glPushMatrix();
	glTranslatef(planetpos.x, planetpos.y, planetpos.z);
	glTranslatef(moonpos.x, moonpos.y, moonpos.z);
	
	GLUquadricObj* qobj3 = gluNewQuadric();
	gluQuadricDrawStyle(qobj3, GLU_FILL);
	glColor3f(1.0f, 0.0f, 0.0f);
	gluSphere(qobj3, 2.5, 30, 30);
	gluDeleteQuadric(qobj3);
	
	glPopMatrix();
}