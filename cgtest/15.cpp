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
std::uniform_int_distribution<int> piramidrandom(0, 3);
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
int rnd = 0;
// Forward declaration
class polygon;
std::list<polygon> polygonmap;
std::list<polygon>::iterator mouse_dest; // ���콺�� ���õ� polygon ����
std::vector<float> allVertices;

int selection[10] = { 1,1,1,1,1,1,0,0,0,0 };

typedef struct poitment {
    float xpos;
    float ypos;
    float zpos;
} pointment;

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
                for (int j = 0; j <  2; ++j) {
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

};



/*bool ptinrect(int x, int y, ret& rect) {
    return (x >= rect.x1 && x <= rect.x2 && y >= rect.y1 && y <= rect.y2);
}*/

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
    
    //polygonmap.emplace_back(polygon(400 - 150, 400 + 150, 400 + 150, 400 + 300, 3));

    pointment p1{0.5,0.5,0.5};
    pointment p2{ 0.5,0.5,-0.5 };
    pointment p3{ -0.5,0.5,-0.5 };
    pointment p4{ -0.5,0.5,0.5 };
    pointment p5{ 0.5,-0.5,0.5 };
    pointment p6{ 0.5,-0.5,-0.5 };
    pointment p7{ -0.5,-0.5,-0.5 };
    pointment p8{ -0.5,-0.5,0.5 };
    pointment p9{ 0, 0.5 ,0 };

    // glm::vec4�� �� ��ǥ ����
    glm::vec4 xaxis1(10, 0, 0, 1);
    glm::vec4 xaxis2(-10, 0, 0, 1);
    glm::vec4 yaxis1(0, 10, 0, 1);
    glm::vec4 yaxis2(0, -10, 0, 1);
    glm::vec4 zaxis1(0, 0, 10, 1);
    glm::vec4 zaxis2(0, 0, -10, 1);

    // y������ -30�� ȸ�� (glm::rotate ���)
    xaxis1 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(0.0f, 1.0f, 0.0f)) * xaxis1;
    xaxis2 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(0.0f, 1.0f, 0.0f)) * xaxis2;
    yaxis1 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(0.0f, 1.0f, 0.0f)) * yaxis1;
    yaxis2 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(0.0f, 1.0f, 0.0f)) * yaxis2;
    zaxis1 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(0.0f, 1.0f, 0.0f)) * zaxis1;
    zaxis2 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(0.0f, 1.0f, 0.0f)) * zaxis2;
    
    // x������ 30�� ȸ�� (glm::rotate ���)
    xaxis1 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(1.0f, 0.0f, 0.0f)) * xaxis1;
    xaxis2 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(1.0f, 0.0f, 0.0f)) * xaxis2;
    yaxis1 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(1.0f, 0.0f, 0.0f)) * yaxis1;
    yaxis2 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(1.0f, 0.0f, 0.0f)) * yaxis2;
    zaxis1 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(1.0f, 0.0f, 0.0f)) * zaxis1;
    zaxis2 = glm::rotate(glm::mat4(1.0f), (float)(-pi / 6), glm::vec3(1.0f, 0.0f, 0.0f)) * zaxis2;

    

    // �� �����͸� allVertices�� �߰� (vec4�� x, y, z ��� ���)
    allVertices.insert(allVertices.end(), {
                    xaxis1.x, xaxis1.y, xaxis1.z,
                    1, 0, 0});
    allVertices.insert(allVertices.end(), {
                    xaxis2.x, xaxis2.y, xaxis2.z,
                    1, 0, 0 });

    allVertices.insert(allVertices.end(), {
                    yaxis1.x, yaxis1.y, yaxis1.z,
                    0, 1, 0 });
    allVertices.insert(allVertices.end(), {
                    yaxis2.x, yaxis2.y, yaxis2.z,
                    0, 1, 0 });

    allVertices.insert(allVertices.end(), {
                    zaxis1.x, zaxis1.y, zaxis1.z,
                    0, 0, 1 });
    allVertices.insert(allVertices.end(), {
                    zaxis2.x, zaxis2.y, zaxis2.z,
                    0, 0, 1 });

    polygonmap.emplace_back(polygon(p1, p2, p3, p4, 0, 0, 0));
    polygonmap.emplace_back(polygon(p3, p4, p8, p7, 0, 0, 1));
    polygonmap.emplace_back(polygon(p1, p4, p8, p5, 0, 1, 0));
    polygonmap.emplace_back(polygon(p2, p1, p5, p6, 0, 1, 1));
    polygonmap.emplace_back(polygon(p2, p3, p7, p6, 1, 0, 0));
    polygonmap.emplace_back(polygon(p5, p6, p7, p8, 1, 0, 1));

    polygonmap.emplace_back(polygon(p6, p5, p9, 1, 0, 0));
    polygonmap.emplace_back(polygon(p8, p5, p9, 0, 1, 0));
    polygonmap.emplace_back(polygon(p7, p8, p9, 0, 0, 1));
    polygonmap.emplace_back(polygon(p6, p7, p9, 1, 1, 0));

    
    for (auto poly = polygonmap.begin(); poly != polygonmap.end(); ++poly) {
        poly->rotate(-pi / 6, 'y');
        poly->rotate(-pi / 6, 'x');

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
    return 0;
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
    for (int i = 0; i < 10; ++i) {
        //glDrawArrays(GL_LINES, 0, 4);
        if (selection[i]) {
            glDrawArrays(GL_TRIANGLES, 6 + 6 * i, 6);
        }

    }
    glDrawArrays(GL_LINES, 0, 6);





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
    case 'c': // ����ü ����
    {
        for (int i = 0; i < 10; ++i) {
            selection[i] = 0;
        }
        int copy[6] = { 1,1,0,0,0,0 };



        std::shuffle(&copy[0], &copy[6], gen);

        for (int i = 0; i < 6; ++i) {
            selection[i] = copy[i];
        }
    }
    break;
    case 't':
    {
        for (int i = 0; i < 10; ++i) {
            selection[i] = 0;
        }


        selection[5] = 1;

		rnd = piramidrandom(gen);

        selection[6 + rnd] = 1;
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
            selection[0] = 0;
            rnd = 0;
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