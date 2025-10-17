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

#define MAXRECT 10 // �ִ� �簢�� ����
#define point 0
#define line 1
#define triangle 2
#define rectangle 3
#define pentagon 4
#define polygonwidth 100
#define pi 3.14

std::random_device rd;

// random_device �� ���� ���� ���� ������ �ʱ�ȭ �Ѵ�.
std::mt19937 gen(rd());

std::uniform_int_distribution<int> dis(0, 256);
std::uniform_int_distribution<int> posdis(0, 700);
//std::uniform_int_distribution<int> numdis(0, windowWidth - rectspace);

//--- �Ʒ� 5�� �Լ��� ����� ���� �Լ� ��
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
void setupBuffers();
void TimerFunction(int value);

//--- �ʿ��� ���� ����
GLint width = 800, height = 800;
GLuint shaderProgramID; //--- ���̴� ���α׷� �̸�
GLuint vertexShader; //--- ���ؽ� ���̴� ��ü
GLuint fragmentShader; //--- �����׸�Ʈ ���̴� ��ü
GLuint VAO, VBO; //--- ���ؽ� �迭 ��ü, ���ؽ� ���� ��ü
int nowdrawstate = 0; // 0: point, 1: line, 2: triangle, 3: rectangle
int selectedshape = -1; // ���õ� ���� �ε���
int spin = 1; //  1: �ð����, -1: �ݽð����
int animation = 1; // 0: ����, 1: ȸ��

// Forward declaration
class polygon;
std::list<polygon> polygonmap;
std::list<polygon>::iterator mouse_dest; // ���콺�� ���õ� polygon ����
std::vector<float> allVertices;



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


// ���� �����ϴ� Ŭ����
class polygon {
private:
	float vertexpos[2][3][3]; // [������ȣ][������ȣ][x,y,z]
	bool needchange;
	GLdouble Rvalue = 0.0;
	GLdouble Gvalue = 0.0;
	GLdouble Bvalue = 0.0;
	GLdouble x1, y1, z1, x2, y2, z2;
	float angle[3];
	float radius[3];
	int membershape; //  0: line, 1: triangle, 2: rectangle, 3: pentagon
	int xdir = 400; // ����
	int ydir = 400;
	int needmove = 0;
	int inner = 0; // mouse ���õǾ����� ����

public:
	//std::vector<ret> rects;
	polygon(pointment p1, pointment p2, pointment p3, GLdouble rv, GLdouble gv, GLdouble bv) {
		vertexpos[0][0][0] = p1.xpos;
		vertexpos[0][0][1] = p1.ypos;
		vertexpos[0][0][2] = p1.zpos;
		
		vertexpos[0][1][0] = p2.xpos;
		vertexpos[0][1][1] = p2.ypos;
		vertexpos[0][1][2] = p2.zpos;
		
		vertexpos[0][2][0] = p3.xpos;
		vertexpos[0][2][1] = p3.ypos;
		vertexpos[0][2][2] = p3.zpos;

		vertexpos[1][0][0] = p1.xpos;
		vertexpos[1][0][1] = p1.ypos;
		vertexpos[1][0][2] = p1.zpos;
		
		vertexpos[1][1][0] = p2.xpos;
		vertexpos[1][1][1] = p2.ypos;
		vertexpos[1][1][2] = p2.zpos;
		
		vertexpos[1][2][0] = p3.xpos;
		vertexpos[1][2][1] = p3.ypos;
		vertexpos[1][2][2] = p3.zpos;

		Rvalue = rv;
		Gvalue = gv;
		Bvalue = bv;
	}

	polygon(pointment p1, pointment p2, pointment p3, pointment p4, GLdouble rv, GLdouble gv, GLdouble bv) {
		vertexpos[0][0][0] = p1.xpos;
		vertexpos[0][0][1] = p1.ypos;
		vertexpos[0][0][2] = p1.zpos;

		vertexpos[0][1][0] = p2.xpos;
		vertexpos[0][1][1] = p2.ypos;
		vertexpos[0][1][2] = p2.zpos;

		vertexpos[0][2][0] = p3.xpos;
		vertexpos[0][2][1] = p3.ypos;
		vertexpos[0][2][2] = p3.zpos;

		vertexpos[1][0][0] = p1.xpos;
		vertexpos[1][0][1] = p1.ypos;
		vertexpos[1][0][2] = p1.zpos;

		vertexpos[1][2][0] = p3.xpos;
		vertexpos[1][2][1] = p3.ypos;
		vertexpos[1][2][2] = p3.zpos;

		vertexpos[1][1][0] = p4.xpos;
		vertexpos[1][1][1] = p4.ypos;
		vertexpos[1][1][2] = p4.zpos;

		Rvalue = rv;
		Gvalue = gv;
		Bvalue = bv;
	}


	void update(float theta) {

	}

	

	

	void sendvertexdata(std::vector<float>& vbo) { // vbo�� ���� ������ �߰�
		GLdouble centerx = (x1 + x2) / 2;
		GLdouble centery = (y1 + y2) / 2;

		centerx -= polygonwidth / 2;
		centery -= polygonwidth / 2;

		for (int poly = 0; poly < 3; ++poly) {
			for (int vert = 0; vert < 3; ++vert) {


				vbo.insert(vbo.end(), {
					vertexpos[poly][vert][0], vertexpos[poly][vert][1], vertexpos[poly][vert][2], (float)Rvalue, (float)Gvalue, (float)Bvalue
					});
			}
		}


	}

};



bool ptinrect(int x, int y, ret& rect) {
	return (x >= rect.x1 && x <= rect.x2 && y >= rect.y1 && y <= rect.y2);
}

void Keyboard(unsigned char key, int x, int y);
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

void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//width = 800;
	//height = 800;

	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
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

	

	//polygonmap.emplace_back(polygon(400 - 150, 400 + 150, 400 + 150, 400 + 300, 3));

	pointment p1{0.5,0.5,0.5};
	pointment p2{ 0.5,0.5,-0.5 };
	pointment p3{ -0.5,0.5,-0.5 };
	pointment p4{ -0.5,0.5,0.5 };
	pointment p5{ 0.5,-0.5,0.5 };
	pointment p6{ 0.5,-0.5,-0.5 };
	pointment p7{ -0.5,-0.5,-0.5 };
	pointment p8{ -0.5,-0.5,0.5 };

	allVertices.insert(allVertices.end(), {
					1, 0, 0, 
					1, 0, 0});
	allVertices.insert(allVertices.end(), {
					-1, 0, 0,
					1, 0, 0 });

	allVertices.insert(allVertices.end(), {
					0, 1, 0,
					0, 1, 0 });
	allVertices.insert(allVertices.end(), {
					0, -1, 0,
					0, 1, 0 });

	allVertices.insert(allVertices.end(), {
					0, 0, 1,
					0, 0, 1 });
	allVertices.insert(allVertices.end(), {
					0, 0, -1,
					0, 0, 1 });


	polygonmap.emplace_back(polygon(p1, p2, p3, p4, dis(gen) / 100.0f, dis(gen) / 100.0f, dis(gen) / 100.0f));
	polygonmap.emplace_back(polygon(p3, p4, p8, p7, dis(gen) / 100.0f, dis(gen) / 100.0f, dis(gen) / 100.0f));
	polygonmap.emplace_back(polygon(p1, p4, p8, p5, dis(gen) / 100.0f, dis(gen) / 100.0f, dis(gen) / 100.0f));
	polygonmap.emplace_back(polygon(p2, p1, p5, p6, dis(gen) / 100.0f, dis(gen) / 100.0f, dis(gen) / 100.0f));
	polygonmap.emplace_back(polygon(p2, p3, p7, p6, dis(gen) / 100.0f, dis(gen) / 100.0f, dis(gen) / 100.0f));
	polygonmap.emplace_back(polygon(p5, p6, p7, p8, dis(gen) / 100.0f, dis(gen) / 100.0f, dis(gen) / 100.0f));

	for (auto poly = polygonmap.begin(); poly != polygonmap.end(); ++poly) {
		poly->sendvertexdata(allVertices);
	}
	//--- ���̴� ���α׷� �����
	glutDisplayFunc(drawScene); //--- ��� �ݹ� �Լ�
	glutReshapeFunc(Reshape);

	glutTimerFunc(25, TimerFunction, 1);

	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion); // ���콺 ��� �ݹ� ���

	glutMainLoop();
}

void make_vertexShaders()
{
	GLchar* vertexSource;
	//--- ���ؽ� ���̴� �о� �����ϰ� ������ �ϱ�
	//--- filetobuf: ��������� �Լ��� �ؽ�Ʈ�� �о ���ڿ��� �����ϴ� �Լ�
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
		std::cerr << "ERROR: vertex shader ������ ����\n" << errorLog << std::endl;
		return;
	}
}

void make_fragmentShaders()
{
	GLchar* fragmentSource;
	//--- �����׸�Ʈ ���̴� �о� �����ϰ� �������ϱ�
	fragmentSource = filetobuf("fragment.glsl"); // �����׼��̴� �о����
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
	bColor = 1.0; //--- ������ �Ķ������� ����
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	// �� �簢���� 6�� �������� ��ȯ�� ��ü ������
	



	if (!allVertices.empty()) {
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		// ���ۿ� ���� ������ ���ε�
		glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float),
			allVertices.data(), GL_DYNAMIC_DRAW);

		// ��� �簢���� �� ���� �׸��� (�� �簢���� 6�� ����)

	}

	glLineWidth(2.0f);
	glDrawArrays(GL_LINES, 0, 6);
	//for (int i = 0; i < 20; ++i) {
		//glDrawArrays(GL_LINES, 0, 4);
	glDrawArrays(GL_TRIANGLES, 6, 3 * 36);

	//}





	glBindVertexArray(0);

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
	case 'r': // vertexpos[0] ��Ī�̵�
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
		if (state == GLUT_DOWN) {// ��������
			
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

void Motion(int x, int y) // ���콺 ��� �ݹ� �Լ�
{

}
