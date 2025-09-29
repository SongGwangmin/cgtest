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
//--- ���� �Լ�

typedef struct RET {
	GLdouble x1, y1, x2, y2;
	GLdouble Rvalue = 0.0;
	GLdouble Gvalue = 0.0;
	GLdouble Bvalue = 0.0;
	int level = 3;
} ret;

int nowdrawsize = 0;
ret showingrect[10]; // �ִ� 10������ �簢�� ����
int whereiscursor = -1; // ���� ���콺�� ��ġ�� �簢�� �ε��� (-1�̸� ����)

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
	if(!result)
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
	
	for (int i = 0; i < nowdrawsize; i++) {
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
	}
	
	if (!allVertices.empty()) {
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		
		// ���ۿ� ���� ������ ���ε�
		glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float), 
					allVertices.data(), GL_DYNAMIC_DRAW);
		
		// ��� �簢���� �� ���� �׸��� (�� �簢���� 6�� ����)
		
	}

	for (int i = 0; i < nowdrawsize; ++i) {
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
			// �簢�� �׸���
			// (x1, y1) -> (x1, y2) -> (x2, y2) -> (x2, y1) -> (x1, y1)
			glDrawArrays(GL_TRIANGLES, i * 6, 6);
		}
		break;
		}

		if (whereiscursor >= 0 && whereiscursor == i) { // ���õ� ������ ������ �׸���
			glColor3f(0.0f, 0.0f, 0.0f);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(3.0f);

			switch (showingrect[i].level) { // i ����ؾ� ��!
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
	// WASD Ű�� ���õ� �簢�� �̵�
	case 'w': // ���� �̵�
	{
		if (whereiscursor >= 0 && whereiscursor < nowdrawsize) {
			int moveDistance = 10; // �̵� �Ÿ�
			showingrect[whereiscursor].y1 -= moveDistance;
			showingrect[whereiscursor].y2 -= moveDistance;
		}
	}
	break;
	case 's': // �Ʒ��� �̵�
	{
		if (whereiscursor >= 0 && whereiscursor < nowdrawsize) {
			int moveDistance = 10; // �̵� �Ÿ�
			showingrect[whereiscursor].y1 += moveDistance;
			showingrect[whereiscursor].y2 += moveDistance;
		}
	}
	break;
	case 'a': // �������� �̵�
	{
		if (whereiscursor >= 0 && whereiscursor < nowdrawsize) {
			int moveDistance = 10; // �̵� �Ÿ�
			showingrect[whereiscursor].x1 -= moveDistance;
			showingrect[whereiscursor].x2 -= moveDistance;
		}
	}
	break;
	case 'd': // ���������� �̵�
	{
		if (whereiscursor >= 0 && whereiscursor < nowdrawsize) {
			int moveDistance = 10; // �̵� �Ÿ�
			showingrect[whereiscursor].x1 += moveDistance;
			showingrect[whereiscursor].x2 += moveDistance;
		}
	}
	break;
	// IJKL Ű�� ���õ� �簢�� �밢�� �̵�
	case 'i': // �»������ �̵� (�� + ����)
	{
		if (whereiscursor >= 0 && whereiscursor < nowdrawsize) {
			int moveDistance = 10; // �̵� �Ÿ�
			showingrect[whereiscursor].x1 -= moveDistance; // ����
			showingrect[whereiscursor].x2 -= moveDistance;
			showingrect[whereiscursor].y1 -= moveDistance; // ��
			showingrect[whereiscursor].y2 -= moveDistance;
		}
	}
	break;
	case 'j': // ���ϴ����� �̵� (�Ʒ� + ����)
	{
		if (whereiscursor >= 0 && whereiscursor < nowdrawsize) {
			int moveDistance = 10; // �̵� �Ÿ�
			showingrect[whereiscursor].x1 -= moveDistance; // ����
			showingrect[whereiscursor].x2 -= moveDistance;
			showingrect[whereiscursor].y1 += moveDistance; // �Ʒ�
			showingrect[whereiscursor].y2 += moveDistance;
		}
	}
	break;
	case 'k': // ���ϴ����� �̵� (�Ʒ� + ������)
	{
		if (whereiscursor >= 0 && whereiscursor < nowdrawsize) {
			int moveDistance = 10; // �̵� �Ÿ�
			showingrect[whereiscursor].x1 += moveDistance; // ������
			showingrect[whereiscursor].x2 += moveDistance;
			showingrect[whereiscursor].y1 += moveDistance; // �Ʒ�
			showingrect[whereiscursor].y2 += moveDistance;
		}
	}
	break;
	case 'o': // �������� �̵� (�� + ������) - 'l'�� ���� ��� ����� - change to 'o'
	{
		if (whereiscursor >= 0 && whereiscursor < nowdrawsize) {
			int moveDistance = 10; // �̵� �Ÿ�
			showingrect[whereiscursor].x1 += moveDistance; // ������
			showingrect[whereiscursor].x2 += moveDistance;
			showingrect[whereiscursor].y1 -= moveDistance; // ��
			showingrect[whereiscursor].y2 -= moveDistance;
		}
	}
	break;
    /*case 'r': // ����: ��� ���� �ʱ� ���·� �ǵ�����
    {
       nowdrawsize = 0; // ��� �簢�� ����
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
        if (state == GLUT_DOWN) {// ��������
			for (int i = nowdrawsize - 1; 0 <= i; --i) {
				if (ptinrect(x, y, showingrect[i])) {
					whereiscursor = i; // �ش� �簢�� ����
					break;
				}
			}
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
            if (nowdrawsize < MAXRECT) {

				showingrect[nowdrawsize].x1 = x - 50;
				showingrect[nowdrawsize].y1 = y - 50;
				showingrect[nowdrawsize].x2 = x + 50;
				showingrect[nowdrawsize].y2 = y + 50;
				if (nowdrawstate == point) {
					showingrect[nowdrawsize].x1 = x - 5;
					showingrect[nowdrawsize].y1 = y - 5;
					showingrect[nowdrawsize].x2 = x + 5;
					showingrect[nowdrawsize].y2 = y + 5;
				}
				showingrect[nowdrawsize].Rvalue = dis(gen) / 256.0f;
				showingrect[nowdrawsize].Gvalue = dis(gen) / 256.0f;
				showingrect[nowdrawsize].Bvalue = dis(gen) / 256.0f;
				showingrect[nowdrawsize].level = nowdrawstate; // �簢������ ����
				nowdrawsize++;

				whereiscursor = nowdrawsize - 1; // ���� �߰��� �簢�� ����
				
                glutPostRedisplay();
            }
        }
    }
    break;
    default:
        break;
    }
}