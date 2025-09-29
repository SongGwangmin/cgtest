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
#include <vector>

#define MAXRECT 10 // �ִ� �簢�� ����
#define point 0
#define line 1
#define triangle 2
#define rectangle 3

std::random_device rd;

// random_device �� ���� ���� ���� ������ �ʱ�ȭ �Ѵ�.
std::mt19937 gen(rd());

std::uniform_int_distribution<int> dis(0, 256);
std::uniform_int_distribution<int> triangledis(30, 100);
//std::uniform_int_distribution<int> numdis(0, windowWidth - rectspace);

//--- �Ʒ� 5�� �Լ��� ����� ���� �Լ� ��
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
void setupBuffers();

//--- �ʿ��� ���� ����
GLint width, height;
GLuint shaderProgramID; //--- ���̴� ���α׷� �̸�
GLuint vertexShader; //--- ���ؽ� ���̴� ��ü
GLuint fragmentShader; //--- �����׸�Ʈ ���̴� ��ü
GLuint VAO, VBO; //--- ���ؽ� �迭 ��ü, ���ؽ� ���� ��ü
int nowdrawstate = 0; // 0: point, 1: line, 2: triangle, 3: rectangle

int outputmode = 0; // 0: fill, 1: line

//--- ���� �Լ�

typedef struct RET {
	GLdouble x1, y1, x2, y2;
	GLdouble Rvalue = 0.0;
	GLdouble Gvalue = 0.0;
	GLdouble Bvalue = 0.0;
	int movestyle = 0; // 0: ����, 1: ƨ���, 2: �¿� �����籸, 3: �簢 �����̷� 4: �� �����̷�
	GLdouble angle = 0.0;
	int xdir = 0; // x ���� �̵� (1 or -1) / �� �����̷� �ÿ��� x�߾Ӱ�
	int ydir = 0; // y ���� �̵� (1 or -1) / �� �����̷� �ÿ��� y�߾Ӱ�
	int movinglimit = 0; // ������ ���� �Ÿ� (�� �����̷� �ÿ��� �̰� ������)

} ret;

int nowdrawsize = 0;
ret showingrect[10]; // �ִ� 10������ �簢�� ����
int whereiscursor = -1; // ���� ���콺�� ��ġ�� �簢�� �ε��� (-1�̸� ����)

int quadrantsize[4] = { 1,1,1,1 };

ret triangledata[4][4];

ret makenewtriangle(int x, int y, int quadrant);

ret morph(ret& after, ret& before) {
	int halfwidth = width / 2;
	int halfheight = height / 2;
	after.x1 = (before.x1 - halfwidth) / halfwidth;
	after.y1 = (before.y1 - halfheight) / -halfheight;
	after.x2 = (before.x2 - halfwidth) / halfwidth;
	after.y2 = (before.y2 - halfheight) / -halfheight;

	// ������ �״�� ����
	after.Rvalue = before.Rvalue;
	after.Gvalue = before.Gvalue;
	after.Bvalue = before.Bvalue;
	//after.level = before.level;

	return after;
}

void inittriangle() {

	for (int i = 0; i < 4; ++i) { //��и� �ʱ�ȭ
		triangledata[i][0].x1 = 125 - 25;
		triangledata[i][0].y1 = 125 - 25;
		triangledata[i][0].x2 = 125 + 25;
		triangledata[i][0].y2 = 125 + 25;

		if (i % 2) {
			triangledata[i][0].x1 += 250;
			triangledata[i][0].x2 += 250;
		}

		if (i / 2) {
			triangledata[i][0].y1 += 250;
			triangledata[i][0].y2 += 250;
		}

		triangledata[i][0].Rvalue = dis(gen) / 256.0f;
		triangledata[i][0].Gvalue = dis(gen) / 256.0f;
		triangledata[i][0].Bvalue = dis(gen) / 256.0f;

		quadrantsize[i] = 1;

		triangledata[i][0].movestyle = 0;
	}


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

	// ���� �Ӽ� ����: ��ġ (3��) + ���� (3��) = �� 6�� float
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}


void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	width = 500;
	height = 500;

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



	inittriangle();



	// ���� ����
	setupBuffers();

	//--- ���̴� ���α׷� �����
	glutDisplayFunc(drawScene); //--- ��� �ݹ� �Լ�
	glutReshapeFunc(Reshape);

	for (ret& rt : showingrect) {
		rt.Rvalue = dis(gen) / 256.0f;
		rt.Gvalue = dis(gen) / 256.0f;
		rt.Bvalue = dis(gen) / 256.0f;
	}

	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutTimerFunc(25, TimerFunction, 1);
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
	rColor = gColor = 0.0;
	bColor = 1.0; //--- ������ �Ķ������� ����
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	// �� �簢���� 6�� �������� ��ȯ�� ��ü ������
	std::vector<float> allVertices;



	for (int i = 0; i < 4; ++i) { //��и� �ʱ�ȭ
		for (int j = 0; j < quadrantsize[i]; ++j) {
			ret after;
			morph(after, triangledata[i][j]); // morph ��ȯ ����

			// level�� �״�� ��� (�簢�� �׸����)
			float x1 = (float)after.x1;
			float y1 = (float)after.y1;
			float x2 = (float)after.x2;
			float y2 = (float)after.y2;
			float r = (float)after.Rvalue;
			float g = (float)after.Gvalue;
			float b = (float)after.Bvalue;

			// �簢���� ���� 6�� ����: (x1,y1), (x1,y2), (x2,y2), (x1,y2), (x2,y2), (x2,y1)
			// �� �������� ��ġ(3) + ����(3) = 6�� ��

			// ù ��° �ﰢ��: (x1,y1), (x1,y2), (x2,y2)


			allVertices.insert(allVertices.end(), {
				x2, y2, 0.0f, r, g, b,  // (x1, y1)
				(x2 + x1) / 2, y1, 0.0f, r, g, b,  // (x1, y2)
				x1, y2, 0.0f, r, g, b   // (x2, y2)
				});
		}
	}

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

		// ���ۿ� ���� ������ ���ε�
		glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float),
			allVertices.data(), GL_DYNAMIC_DRAW);

		// ��� �簢���� �� ���� �׸��� (�� �簢���� 6�� ����)

		int totalTriangles = 0;
		for (int i = 0; i < 4; ++i) {
			totalTriangles += quadrantsize[i];
		}

		glDrawArrays(GL_TRIANGLES, 0, totalTriangles * 3);
	}




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
	case 'a':
	{
		outputmode = 0; // fill ���
	}
	break;
	case 'b':
	{
		outputmode = 1; // line ���
	}
	break;
	case 'c':
	{
		inittriangle(); // ��и� �ʱ�ȭ
	}
	break;
	case '1': 
	{
		for (int i = 0; i < 4; ++i) { //movingstyle ����
			for (int j = 0; j < quadrantsize[i]; ++j) {
				triangledata[i][j].movestyle = key - '0';
				if(triangledata[i][j].xdir == 0)
					triangledata[i][j].xdir = 1;
				if (triangledata[i][j].ydir == 0)
					triangledata[i][j].ydir = 1;
			}
		}
	}
	break;
	case '2': 
	{
		for (int i = 0; i < 4; ++i) { //movingstyle ����
			for (int j = 0; j < quadrantsize[i]; ++j) {
				triangledata[i][j].movestyle = key - '0';
				if (triangledata[i][j].xdir == 0)
					triangledata[i][j].xdir = 1;
				if (triangledata[i][j].ydir == 0)
					triangledata[i][j].ydir = 1;
			}
		}
	}
	break;
	case '3': 
	{
		for (int i = 0; i < 4; ++i) { //movingstyle ����
			for (int j = 0; j < quadrantsize[i]; ++j) {
				triangledata[i][j].movestyle = key - '0';
				
				triangledata[i][j].xdir = -1;
				
				triangledata[i][j].ydir = 0;

				triangledata[i][j].movinglimit = 0;

			}
		}
	}
	break;
	case '4': {
		for (int i = 0; i < 4; ++i) { //movingstyle ����
			for (int j = 0; j < quadrantsize[i]; ++j) {
				triangledata[i][j].movestyle = key - '0';
			}
		}
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
			int quadrant = (x >= width / 2 ? 1 : 0) + (y >= height / 2 ? 2 : 0);
			//����
			ret newtriangle = makenewtriangle(x, y, quadrant);

			triangledata[quadrant][quadrantsize[quadrant] - 1] = newtriangle;
		}
		else if (state == GLUT_UP) {

			glutPostRedisplay();
		}
	}
	break;
	case GLUT_RIGHT_BUTTON:
	{
		if (state == GLUT_DOWN) {
			int quadrant = (x >= width / 2 ? 1 : 0) + (y >= height / 2 ? 2 : 0);
			if (quadrantsize[quadrant] < 4) { // ��и鿡 ������ 4�� �̸��� ���� �߰�
				ret newtriangle = makenewtriangle(x, y, quadrant);

				triangledata[quadrant][quadrantsize[quadrant]] = newtriangle;
				quadrantsize[quadrant]++;
			}
		}
	}
	break;
	default:
		break;
	}
}

ret makenewtriangle(int x, int y, int quadrant) {
	int rectwidth = triangledis(gen);
	ret newtriangle = triangledata[quadrant][quadrantsize[quadrant] - 1];

	switch (quadrant) {
	case 0: // 2��и�

		newtriangle.x2 = x;
		newtriangle.y2 = y;
		newtriangle.x1 = x - rectwidth;
		newtriangle.y1 = y - rectwidth;
		break;
	case 1: // 1��и�
		newtriangle.x1 = x;
		newtriangle.y1 = y - rectwidth;
		newtriangle.x2 = x + rectwidth;
		newtriangle.y2 = y;
		break;
	case 2: // 3��и�
		newtriangle.x1 = x - rectwidth;
		newtriangle.y1 = y;
		newtriangle.x2 = x;
		newtriangle.y2 = y + rectwidth;
		break;
	case 3: // 4��и�
		newtriangle.x1 = x;
		newtriangle.y1 = y;
		newtriangle.x2 = x + rectwidth;
		newtriangle.y2 = y + rectwidth;
		break;

	}
	newtriangle.Rvalue = dis(gen) / 256.0f;
	newtriangle.Gvalue = dis(gen) / 256.0f;
	newtriangle.Bvalue = dis(gen) / 256.0f;

	return newtriangle;
}



void TimerFunction(int value)
{
	



	for (int i = 0; i < 4; ++i) { //������ ����
		for (int j = 0; j < quadrantsize[i]; ++j) {

			int xmovement = 0;
			int ymovement = 0;
			switch (triangledata[i][j].movestyle) {
			case 1: // ƨ���
			{
				if (triangledata[i][j].x2 >= width) {
					triangledata[i][j].xdir = -1;
				}
				else if (triangledata[i][j].x1 <= 0) {
					triangledata[i][j].xdir = 1;
				}

				if (triangledata[i][j].y2 >= height) {
					triangledata[i][j].ydir = -1;
				}
				else if (triangledata[i][j].y1 <= 0) {
					triangledata[i][j].ydir = 1;
				}

				triangledata[i][j].x1 += triangledata[i][j].xdir * 10;
				triangledata[i][j].x2 += triangledata[i][j].xdir * 10;
				triangledata[i][j].y1 += triangledata[i][j].ydir * 10;
				triangledata[i][j].y2 += triangledata[i][j].ydir * 10;
			}
			break;
			case 2: // �¿� �����籸
			{
				if (triangledata[i][j].x2 >= width) {
					triangledata[i][j].xdir = -1;
					triangledata[i][j].y1 += triangledata[i][j].ydir * 10;
					triangledata[i][j].y2 += triangledata[i][j].ydir * 10;
				}
				else if (triangledata[i][j].x1 <= 0) {
					triangledata[i][j].xdir = 1;
					triangledata[i][j].y1 += triangledata[i][j].ydir * 10;
					triangledata[i][j].y2 += triangledata[i][j].ydir * 10;
				}

				if (triangledata[i][j].y2 >= height) {
					triangledata[i][j].ydir = -1;
				}
				else if (triangledata[i][j].y1 <= 0) {
					triangledata[i][j].ydir = 1;
				}

				triangledata[i][j].x1 += triangledata[i][j].xdir * 10;
				triangledata[i][j].x2 += triangledata[i][j].xdir * 10;
			}
			break;
			case 3: // �簢 �����̷�
			{
				int gap = triangledata[i][j].x2 - triangledata[i][j].x1;
				if (triangledata[i][j].xdir == -1) {
					if (triangledata[i][j].x1 <= 0 + triangledata[i][j].movinglimit) {
						triangledata[i][j].xdir = 0;
						triangledata[i][j].ydir = 1;
					}
				}
				else if (triangledata[i][j].xdir == 1) {
					if (triangledata[i][j].x2 >= width - triangledata[i][j].movinglimit) {
						triangledata[i][j].xdir = 0;
						triangledata[i][j].ydir = -1;
					}
				}
				else if (triangledata[i][j].ydir == 1) {
					if (triangledata[i][j].y2 >= height - triangledata[i][j].movinglimit) {
						triangledata[i][j].xdir = 1;
						triangledata[i][j].ydir = 0;
					}
				}
				else if (triangledata[i][j].ydir == -1) {
					if (triangledata[i][j].y1 <= 0 + triangledata[i][j].movinglimit) {
						triangledata[i][j].xdir = -1;
						triangledata[i][j].ydir = 0;

						triangledata[i][j].movinglimit += gap;
					}
				}

				triangledata[i][j].x1 += triangledata[i][j].xdir * 10;
				triangledata[i][j].x2 += triangledata[i][j].xdir * 10;
				triangledata[i][j].y1 += triangledata[i][j].ydir * 10;
				triangledata[i][j].y2 += triangledata[i][j].ydir * 10;
			}
			break;
			case 4: // �� �����̷�
			{

			}
			break;
			default:
				break;
			}
		}
	}

	glutPostRedisplay();
	glutTimerFunc(25, TimerFunction, 1);
}