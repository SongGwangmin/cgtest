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
bool has_selected = false; // mouse_dest�� ��ȿ���� Ȯ��

// �巡�� ���� ����
bool is_dragging = false;
int last_mouse_x = 0;
int last_mouse_y = 0;
int stopanimation = 0; // 0: ������, 1: ����

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
	float vertexpos[3][2];
	bool needchange;
	GLdouble Rvalue = 0.0;
	GLdouble Gvalue = 0.0;
	GLdouble Bvalue = 0.0;
	GLdouble x1, y1, x2, y2;
	float angle[3];
	float radius[3];
	int membershape; //  0: line, 1: triangle, 2: rectangle, 3: pentagon
	int xdir = 400; // ����
	int ydir = 400;
	int needmove = 0;
	int inner = 0; // mouse ���õǾ����� ����

public:
	//std::vector<ret> rects;
	polygon(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2, int style)
		: x1(x1), y1(y1), x2(x2), y2(y2) {

		needchange = false;

		xdir = 400;
		ydir = 400;



		GLdouble xhalf = (x1 + x2) / 2;
		GLdouble yhalf = (y1 + y2) / 2;

		switch (style) {
		case 1:
		{
			vertexpos[0][0] = xhalf;
			vertexpos[0][1] = y1;

			vertexpos[1][0] = x1;
			vertexpos[1][1] = y2;

			vertexpos[2][0] = x2;
			vertexpos[2][1] = y2;


		}
		break;
		case 2:
		{
			vertexpos[0][0] = x1;
			vertexpos[0][1] = yhalf;

			vertexpos[1][0] = x2;
			vertexpos[1][1] = y2;

			vertexpos[2][0] = x2;
			vertexpos[2][1] = y1;
		}
		break;
		case 3:
		{
			vertexpos[0][0] = xhalf;
			vertexpos[0][1] = y2;

			vertexpos[1][0] = x1;
			vertexpos[1][1] = y1;

			vertexpos[2][0] = x2;
			vertexpos[2][1] = y1;
		}
		break;
		case 4:
		{
			vertexpos[0][0] = x2;
			vertexpos[0][1] = yhalf;

			vertexpos[1][0] = x1;
			vertexpos[1][1] = y2;

			vertexpos[2][0] = x1;
			vertexpos[2][1] = y1;
		}
		break;
		}





		for (int i = 0; i < 3; ++i) {
			radius[i] = std::hypot(vertexpos[i][0] - xdir, vertexpos[i][1] - ydir);
			angle[i] = atan2(vertexpos[i][1] - ydir, vertexpos[i][0] - xdir);
		}

	}


	int changeShape(int targetshape) {
		if ((membershape + 1) % 4 == targetshape) {
			printf("activate\n");

			bool chaging = false;

			/*for (int poly = 0; poly < 3; ++poly) {
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
			}*/

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

	void update(float theta) {

		for (int i = 0; i < 3; ++i) {
			vertexpos[i][0] = xdir + radius[i] * cos(angle[i] + theta);
			vertexpos[i][1] = ydir + radius[i] * sin(angle[i] + theta);
			angle[i] += theta;
		}
	}

	void setselect(int select) {
		//selected = select;
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

	void innerouterchange(float centerx, float centery) {
		radius[0] = std::hypot(vertexpos[0][0] - centerx, vertexpos[0][1] - centery);
		angle[0] = atan2(vertexpos[0][1] - centery, vertexpos[0][0] - centerx);

		if (inner) {
			radius[0] += 250;
			inner = 0;
			printf("%d", inner);
		}
		else {
			radius[0] -= 250;
			inner = 1;
			printf("%d", inner);
		}


		vertexpos[0][0] = centerx + radius[0] * cos(angle[0]);
		vertexpos[0][1] = centery + radius[0] * sin(angle[0]);

		radius[0] = std::hypot(vertexpos[0][0] - xdir, vertexpos[0][1] - ydir);
		angle[0] = atan2(vertexpos[0][1] - ydir, vertexpos[0][0] - xdir);
	}

	void dragmove(int movex, int movey) {
		x1 += movex;
		x2 += movex;
		y1 += movey;
		y2 += movey;
		if (x1 < 0) {
			x1 = 0;
			x2 = polygonwidth;
		}
		if (x2 > width) {
			x2 = width;
			x1 = width - polygonwidth;
		}
		if (y1 < 0) {
			y1 = 0;
			y2 = polygonwidth;
		}
		if (y2 > height) {
			y2 = height;
			y1 = height - polygonwidth;
		}
	}

	void resetShape(int targetshape) {	//	������ ��纯��
		for (int poly = 0; poly < 3; ++poly) {
			for (int vert = 0; vert < 3; ++vert) {
				for (int pos = 0; pos < 2; ++pos) {
					//vertexpos[poly][vert][pos] = GuideFrame[targetshape][poly][vert][pos];
				}
			}
		}



		membershape = targetshape;

		if (!membershape) {
			x2 = x1 + 20;
			y2 = y1 + 20;
		}
		else {

			x2 = x1 + polygonwidth;
			y2 = y1 + polygonwidth;
		}

		Rvalue = dis(gen) / 256.0f;
		Gvalue = dis(gen) / 256.0f;
		Bvalue = dis(gen) / 256.0f;
	}

	void sendvertexdata(std::vector<float>& vbo) { // vbo�� ���� ������ �߰�
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

	bool ptinrect(int x, int y) {
		return (x >= x1 && x <= x2 && y >= y1 && y <= y2);
	}

	void setmid(int xpos, int ypos) {
		xdir = xpos;
		ydir = ypos;

		for (int i = 0; i < 3; ++i) {
			radius[i] = std::hypot(vertexpos[i][0] - xdir, vertexpos[i][1] - ydir);
			angle[i] = atan2(vertexpos[i][1] - ydir, vertexpos[i][0] - xdir);
		}
	}

	// ��ǥ ������ ���� getter �޼����
	GLdouble getX1() const { return x1; }
	GLdouble getY1() const { return y1; }
	GLdouble getX2() const { return x2; }
	GLdouble getY2() const { return y2; }

	float getmainx() const { return vertexpos[0][0]; }
	float getmainy() const { return vertexpos[0][1]; }

	// vertexpos[0]�� vertexpos[1], vertexpos[2]�� �̷�� ������ �������� ��Ī�̵�
	void hardreset() {
		xdir = 400;
		ydir = 400;

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

	polygonmap.emplace_back(polygon(400 - 150, 400 - 300, 400 + 150, 400 - 150, 1));

	polygonmap.emplace_back(polygon(400 - 150, 400 + 150, 400 + 150, 400 + 300, 3));

	polygonmap.emplace_back(polygon(400 - 300, 400 - 150, 400 - 150, 400 + 150, 2));

	polygonmap.emplace_back(polygon(400 + 150, 400 - 150, 400 + 300, 400 + 150, 4));

	//polygonmap.emplace_back(polygon(400 - 150, 400 + 150, 400 + 150, 400 + 300, 3));



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
	std::vector<float> allVertices;


	for (auto poly = polygonmap.begin(); poly != polygonmap.end(); ++poly) {
		poly->sendvertexdata(allVertices);
	}



	if (!allVertices.empty()) {
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		// ���ۿ� ���� ������ ���ε�
		glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float),
			allVertices.data(), GL_DYNAMIC_DRAW);

		// ��� �簢���� �� ���� �׸��� (�� �簢���� 6�� ����)

	}

	for (int i = 0; i < 20; ++i) {
		glLineWidth(2.0f);
		glDrawArrays(GL_LINES, i * 3, 3);
		glDrawArrays(GL_TRIANGLES, i * 3, 3);

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
	case 'c': // �ð���� ȸ��
	{
		spin = 1;
	}
	break;
	case 's': // �ִϸ��̼� ���
	{
		animation = !animation;
	}
	break;
	case 't': // �ݽð���� ȸ��
	{
		spin = -1;
	}
	break;
	case 'r': // vertexpos[0] ��Ī�̵�
	{
		for (auto poly = polygonmap.begin(); poly != polygonmap.end(); ++poly) {
			poly->hardreset();
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
			for (auto poly = polygonmap.begin(); poly != polygonmap.end(); ++poly) {
				poly->setmid(x, y);
			}
		}
		else if (state == GLUT_UP) {
			auto poly = polygonmap.begin();
			//poly++;
			float main1x = poly->getmainx();
			float main1y = poly->getmainy();

			//poly++;
			poly++;
			float main3x = poly->getmainx();
			float main3y = poly->getmainy();

			float centerx = (main1x + main3x) / 2;
			float centery = (main1y + main3y) / 2;
			for (auto poly = polygonmap.begin(); poly != polygonmap.end(); ++poly) {
				poly->setmid(centerx, centery);
			}

			glutPostRedisplay();
		}
	}
	break;
	case GLUT_RIGHT_BUTTON:
	{
		if (state == GLUT_DOWN) {
			auto poly = polygonmap.begin();
			//poly++;
			float main1x = poly->getmainx();
			float main1y = poly->getmainy();

			//poly++;
			poly++;
			float main3x = poly->getmainx();
			float main3y = poly->getmainy();

			float centerx = (main1x + main3x) / 2;
			float centery = (main1y + main3y) / 2;
			for (auto poly = polygonmap.begin(); poly != polygonmap.end(); ++poly) {
				poly->innerouterchange(centerx, centery);
			}
		}
	}
	break;
	default:
		break;
	}
}

void TimerFunction(int value)
{
	if (animation) {
		for (auto poly = polygonmap.begin(); poly != polygonmap.end(); ++poly) {
			poly->update((pi / 30) * spin);
		}
	}


	//printf("timer is playing now\nq");
	glutPostRedisplay();
	glutTimerFunc(25, TimerFunction, 1);
}

void Motion(int x, int y) // ���콺 ��� �ݹ� �Լ�
{

}
