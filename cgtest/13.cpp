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
#define pentagon 4
#define polygonwidth 100

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
//--- ���� �Լ�

float GuideFrame[5][3][3][2] = {
	{
		{ {20,20}, {0,0}, {0,20} },
		{ {20,20}, {0, 0}, {0,20} },
		{ {20,20}, {20,0}, {0,0} }
	},

	// [0] �� (3��, �� 3��)
	{
		{ {0,0}, {100,100}, {50,50} },
		{ {0,0}, {100,100}, {50,50} },
		{ {0,0}, {100,100}, {50,50} }
	},

	// [1] �ﰢ�� (3��)
	{
		{ {100,100}, {50,0}, {0,100} },
		{ {100,100}, {50,0}, {0,100} },
		{ {100,100}, {50,0}, {0,100} }
	},

	// [2] �簢�� �� �ﰢ�� ���� 3��
	{
		{ {100,100}, {0,0}, {0,100} },
		{ {100,100}, {0, 0}, {0,100} },
		{ {100,100}, {100,0}, {0,0} }
	},

	// [3] ������ �� �ﰢ�� ���� 3��
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


// ���� �����ϴ� Ŭ����
class polygon {
private:
	float vertexpos[3][3][2];
	bool needchange;
	GLdouble Rvalue = 0.0;
	GLdouble Gvalue = 0.0;
	GLdouble Bvalue = 0.0;
	GLdouble x1, y1, x2, y2;
	int membershape; //  0: line, 1: triangle, 2: rectangle, 3: pentagon
	int xdir = 0;
	int ydir = 0;
	int needmove = 0;
	int selected = 0; // mouse ���õǾ����� ����

public:
	//std::vector<ret> rects;
	polygon(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2,
		GLdouble rvalue, GLdouble gvalue, GLdouble bvalue, int membershape)
		: x1(x1), y1(y1), x2(x2), y2(y2), Rvalue(rvalue), Gvalue(gvalue), Bvalue(bvalue), membershape(membershape) {

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
		if ((membershape + 1) % 4 == targetshape) {
			printf("activate\n");

			bool chaging = false;

			for (int poly = 0; poly < 3; ++poly) {
				for (int vert = 0; vert < 3; ++vert) {
					for (int pos = 0; pos < 2; ++pos) {
						if (vertexpos[poly][vert][pos] == GuideFrame[targetshape][poly][vert][pos]) {

						}
						else {
							chaging = true; // �ϳ��� ���� ���̸� membershape�� �������� �ʴ´�

							if (vertexpos[poly][vert][pos] < GuideFrame[targetshape][poly][vert][pos]) {
								// GuideFrame�� �� Ŭ ���� �����ϰ� �Ѿ�� ���� ���� �ش�

								vertexpos[poly][vert][pos] += 5;
								if (vertexpos[poly][vert][pos] > GuideFrame[targetshape][poly][vert][pos]) {
									vertexpos[poly][vert][pos] = GuideFrame[targetshape][poly][vert][pos];
								}

							}
							else {
								vertexpos[poly][vert][pos] -= 5;
								if (vertexpos[poly][vert][pos] < GuideFrame[targetshape][poly][vert][pos]) {
									vertexpos[poly][vert][pos] = GuideFrame[targetshape][poly][vert][pos];
								}
							}

						}
					}
				}
			}

			if (chaging) {

			}
			else {
				membershape = targetshape;
			}

			return 1;
		}
		else {
			printf("incorrect input, you need to send %d\n", (membershape + 1) % 4);
			return 0;
		}
	}

	void update() {
		if (needmove && !selected) {
			x1 += xdir * 5;
			x2 += xdir * 5;
			y1 += ydir * 5;
			y2 += ydir * 5;
			if (x1 <= 0 || x2 >= width) {
				xdir = -xdir;
			}
			if (y1 <= 0 || y2 >= height) {
				ydir = -ydir;
			}

		}

	}

	void setselect(int select) {
		selected = select;
	}

	void setmove() {
		if (dis(gen) % 2) {
			xdir = -1;
		}
		else {
			xdir = 1;
		}
		
		if (dis(gen) % 2) {
			ydir = -1;
		}
		else {
			ydir = 1;
		}
		needmove = 1;
	}


	void resetShape(int targetshape) {	//	������ ��纯��
		for (int poly = 0; poly < 3; ++poly) {
			for (int vert = 0; vert < 3; ++vert) {
				for (int pos = 0; pos < 2; ++pos) {
					vertexpos[poly][vert][pos] = GuideFrame[targetshape][poly][vert][pos];
				}
			}
		}

		membershape = targetshape;

		Rvalue = dis(gen) / 256.0f;
		Gvalue = dis(gen) / 256.0f;
		Bvalue = dis(gen) / 256.0f;
	}

	void sendvertexdata(std::vector<float>& vbo) { // vbo�� ���� ������ �߰�
		GLdouble centerx = (x1 + x2) / 2;
		GLdouble centery = (y1 + y2) / 2;

		centerx -= polygonwidth / 2;
		centery -= polygonwidth / 2;

		for (int poly = 0; poly < 3; ++poly) {
			for (int vert = 0; vert < 3; ++vert) {
				float virtualx = vertexpos[poly][vert][0] + centerx;
				float virtualy = vertexpos[poly][vert][1] + centery;

				float finalx = (virtualx - (width / 2)) / (width / 2);
				float finaly = (virtualy - (height / 2)) / -(height / 2);

				vbo.insert(vbo.end(), {
					finalx, finaly, 0.0f, (float)Rvalue, (float)Gvalue, (float)Bvalue
					});
			}
		}


	}
};

polygon activePolygon[4] = {
	polygon(0, 0, width / 2, height / 2,
		dis(gen) / 256.0f, dis(gen) / 256.0f, dis(gen) / 256.0f, line), // (0,0) ~ (400, 400)

	polygon(width / 2, 0, width, height / 2,
		dis(gen) / 256.0f, dis(gen) / 256.0f, dis(gen) / 256.0f, triangle), // (400, 0) ~ (800, 400)

	polygon(0, height / 2, width / 2, height,
		dis(gen) / 256.0f, dis(gen) / 256.0f, dis(gen) / 256.0f, rectangle), // (0, 400) ~ (400, 800)

	polygon(width / 2, height / 2, width, height,
		dis(gen) / 256.0f, dis(gen) / 256.0f, dis(gen) / 256.0f, pentagon) // (400, 400) ~ (800, 800)

};



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

	//--- ���̴� ���α׷� �����
	glutDisplayFunc(drawScene); //--- ��� �ݹ� �Լ�
	glutReshapeFunc(Reshape);

	glutTimerFunc(25, TimerFunction, 1);

	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);

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
	std::vector<float> allVertices;

	for (polygon& poly : activePolygon) {
		poly.sendvertexdata(allVertices);
	}


	/*for (int i = 0; i < nowdrawsize; i++) {
		ret after;
		morph(after, showingrect[i]); // morph ��ȯ ����

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

		// �� ��° �ﰢ��: (x1,y1), (x2,y2), (x2,y1)
		allVertices.insert(allVertices.end(), {
			x1, y1, 0.0f, r, g, b,  // (x1, y1)
			x2, y2, 0.0f, r, g, b,  // (x2, y2)
			x2, y1, 0.0f, r, g, b   // (x2, y1)
			});
	}*/

	if (!allVertices.empty()) {
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		// ���ۿ� ���� ������ ���ε�
		glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float),
			allVertices.data(), GL_DYNAMIC_DRAW);

		// ��� �簢���� �� ���� �׸��� (�� �簢���� 6�� ����)

	}

	for (int i = 0; i < 4; ++i) {
		//glLineWidth(5.0f);
		//glDrawArrays(GL_LINES, i * 9, 9);
		glDrawArrays(GL_TRIANGLES, i * 9, 9);

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
	case 'p': // ������ -> ��
	{
		if (selectedshape == -1)
			selectedshape = line;
	}
	break;
	case 'l': // �� -> �ﰢ��
	{
		if (selectedshape == -1)
			selectedshape = triangle;
	}
	break;
	case 't': // �ﰢ�� -> �簢��
	{
		if (selectedshape == -1)
			selectedshape = rectangle;
	}
	break;
	case 'r': // �簢�� -> ������
	{
		if (selectedshape == -1)
			selectedshape = pentagon;
	}
	break;

	case 'a': // ������ �⺻���� ����
	{
		activePolygon[0].resetShape(line);
		activePolygon[1].resetShape(triangle);
		activePolygon[2].resetShape(rectangle);
		activePolygon[3].resetShape(pentagon);
		selectedshape = -1;
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
			// ���ο� �簢�� �߰� (����)

		}
	}
	break;
	default:
		break;
	}
}

void TimerFunction(int value)
{
	/*if (pointcount < sizing * 2) {
		pointcount += 2;
	}
	else {
		pointcount = sizing * 2;
	}*/
	int animationcheck = 0;

	for (polygon& poly : activePolygon) {
		if (poly.changeShape(selectedshape)) {
			animationcheck = 1;
		}
	}

	if (animationcheck) {
		printf("someone change position. %d\n", selectedshape);
	}
	else {
		printf("no one change position. you can move these\n");
		selectedshape = -1;
	}

	printf("timer is playing now\nq");
	glutPostRedisplay();
	glutTimerFunc(25, TimerFunction, 1);
}