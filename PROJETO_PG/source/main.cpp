#define _CRT_SECURE_NO_DEPRECATE
//#define GLEW_STATIC
//#include <Windows.h>
#include <iostream>
//#include <time.h>
#include "util.h"
#include <string>

#include <GL/gl.h>

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900

using namespace std;

GLuint windowID;
GLuint srcWindow, dstWindow;

int larguraJanela = WINDOW_WIDTH;
int alturaJanela = WINDOW_HEIGHT;

char caminhoImagem[256] = "..\\Release\\sample.bmp";
GLfloat vertices[16];//8 vértices (src + dst) com 2 coordenadas cada
Quad src, dst;
Point *points[8];//Cross-references between quad points and array of vertices

//States for dragging control points
int dragState = -1;
Point* pointDragged = NULL;

char textControlQuadSize[256];

GLuint textura;
int image = -1;

ILubyte dstTexture[1000 * 800 * 3];

void setOrthographicProjection()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, larguraJanela, alturaJanela, 0);
	glMatrixMode(GL_MODELVIEW);
}

void resetPerspectiveProjection()
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void renderizaBitmap(float x, float y, void *font, const char *string)
{
	const char *c;
	glRasterPos2f(x, y);
	for (c = string; *c != '\0'; c++)
		glutBitmapCharacter(font, *c);

}

void displayText(float x, float y, char *str)
{
	setOrthographicProjection();
	glPushMatrix();
	glLoadIdentity();
	glColor3d(1.0, 1.0, 1.0);
	renderizaBitmap(x, y, GLUT_BITMAP_8_BY_13, str);
	glPopMatrix();
	resetPerspectiveProjection();

}

int isControlPoint(int x, int y) {
	int r = -1;

	//for(int i = 0; i < 16; i+=2)
	for (int i = 14; i >= 0; i -= 2) {
		if (WITHIN_POINT(x, vertices[i]) && WITHIN_POINT(y, vertices[i + 1])) {
			//cout << "That's a point!" << endl;
			r = i;
			return r;
		}
	}

	return r;
}

void movePoint(Point* p, int index, int x, int y) {
	vertices[index] = x;
	vertices[index + 1] = y;
	p->x = x;
	p->y = y;
}

void transferPixels(ILubyte* srcData, int srcW, int srcH) {

	Point srcA = Point(src.A.x - 20, src.A.y - 20);
	Point srcB = Point(src.B.x - 20, src.B.y - 20);
	Point srcC = Point(src.C.x - 20, src.C.y - 20);
	Point srcD = Point(src.D.x - 20, src.D.y - 20);

	Point dstA = Point(dst.A.x - 20, dst.A.y - 20);
	Point dstB = Point(dst.B.x - 20, dst.B.y - 20);
	//Point dstC = Point(dst.C.x - 20, dst.C.y - 20);
	Point dstD = Point(dst.D.x - 20, dst.D.y - 20);

	int w = dstB.x - dstA.x;
	int h = dstD.y - dstA.y;


	for (int i = 0; i <= h; i++) {
		//Interpolating in Y-axis

		double t = (double)(i - 0.0) / (double)h;
		double tt = (double)(h - i) / (double)h;

		// A--p1--D
		double p1[2] = {
			tt * srcA.x + t * src.D.x,
			tt * srcA.y + t * src.D.y
		};
		// B--p2--C
		double p2[2] = {
			tt * srcB.x + t * src.C.x,
			tt * srcB.y + t * src.C.y
		};


		for (int j = 0; j <= w; j++) {
			//Interpolating in X-axis

			double s = (double)(j - 0.0) / (double)w;
			double ss = (double)(w - j) / (double)w;

			// p1--iP--p2
			int interpolatedPoint[2] = {
				(int)(ss * p1[0] + s * p2[0]),
				(int)(ss * p1[1] + s * p2[1])
			};


			//Red Channel
			dstTexture[((i * (w + 1)) + j) * 3 + 0] =

				srcData[((interpolatedPoint[1] * srcW) + interpolatedPoint[0]) * 3 + 0];
			//Green Channel
			dstTexture[((i * (w + 1)) + j) * 3 + 1] =
				srcData[((interpolatedPoint[1] * srcW) + interpolatedPoint[0]) * 3 + 1];
			//Blue Channel
			dstTexture[((i * (w + 1)) + j) * 3 + 2] =
				srcData[((interpolatedPoint[1] * srcW) + interpolatedPoint[0]) * 3 + 2];
		}

	}
}

int carregaImagem(char *filename)
{
	ILuint    image = -1;
	ILboolean success;

	ilGenImages(1, &image);//Generate image name
	ilBindImage(image);//Binding image name


	//Try to load image using filename
	if (success = ilLoadImage(filename))
	{
		//Every color component is now represented in an unsigned byte
		success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);

		if (!success)
		{
			return -1;
		}
	}
	else if (success = ilLoadImage("sample.bmp")) {
		//Every color component is now represented in an unsigned byte
		success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);

		if (!success)
		{
			return -1;
		}
	}
	else
		return -1;

	return image;
}

void handleKeyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27:
		glutDestroyWindow(windowID);
		break;

	case 'o':
	case 'O':
		if (image != -1)
			ilDeleteImages(1, (const ILuint *)&image);

		image = carregaImagem(caminhoImagem);
		if (image == -1)
		{
			cout << "Can't load picture file " << caminhoImagem;
		}
		else {
			//usar em coord de tela
			vertices[0] = ilGetInteger(IL_IMAGE_WIDTH) / 4 + 20;
			vertices[1] = ilGetInteger(IL_IMAGE_HEIGHT) / 4 + 20;
			vertices[2] = (3 * ilGetInteger(IL_IMAGE_WIDTH)) / 4 + 20;
			vertices[3] = ilGetInteger(IL_IMAGE_HEIGHT) / 4 + 20;
			vertices[4] = (3 * ilGetInteger(IL_IMAGE_WIDTH)) / 4 + 20;
			vertices[5] = (3 * ilGetInteger(IL_IMAGE_HEIGHT)) / 4 + 20;
			vertices[6] = ilGetInteger(IL_IMAGE_WIDTH) / 4 + 20;
			vertices[7] = (3 * ilGetInteger(IL_IMAGE_HEIGHT)) / 4 + 20;

			//usar em coord de imagem
			src.A = Point(vertices[0], vertices[1], src.A.color);
			src.B = Point(vertices[2], vertices[3], src.B.color);
			src.C = Point(vertices[4], vertices[5], src.C.color);
			src.D = Point(vertices[6], vertices[7], src.D.color);

			//usar em coord de tela
			vertices[8] = vertices[0] + ilGetInteger(IL_IMAGE_WIDTH) + 50;
			vertices[9] = vertices[1];
			vertices[10] = vertices[2] + ilGetInteger(IL_IMAGE_WIDTH) + 50;
			vertices[11] = vertices[3];
			vertices[12] = vertices[4] + ilGetInteger(IL_IMAGE_WIDTH) + 50;
			vertices[13] = vertices[5];
			vertices[14] = vertices[6] + ilGetInteger(IL_IMAGE_WIDTH) + 50;
			vertices[15] = vertices[7];

			//usar em coord de imagem
			dst.A = Point(vertices[8], vertices[9], dst.A.color);
			dst.B = Point(vertices[10], vertices[11], dst.B.color);
			dst.C = Point(vertices[12], vertices[13], dst.C.color);
			dst.D = Point(vertices[14], vertices[15], dst.D.color);
		}

	}
}

void handleMouse(int button, int state, int x, int y) {
	if (state == GLUT_DOWN) {
		switch (button) {
		case GLUT_LEFT_BUTTON:
			//cout << "click at " << x << "," << y << endl;

			if (dragState < 0) {
				dragState = isControlPoint(x, y);
				pointDragged = points[dragState / 2];
			}

			break;
		}
	}
	else { //GLUT_UP
		dragState = -1;
	}
}

void handleDrag(int x, int y) {
	if (dragState >= 0) {
		if (dragState < 8) {//Src quad
			movePoint(pointDragged, dragState, x, y);
		}
		else {//Dst quad
			//A B
			//D C

			switch (dragState) {
				//Point A
			case 8:
				//Mov para esquerda da tela ou para direita, respectivamente
				if (x != vertices[dragState] &&
					(x >= 0 && (x < (vertices[10] - POINT_SIZE)))) {
					movePoint(pointDragged, dragState, x, vertices[9]);
					movePoint(&dst.D, 14, x, vertices[15]);
				}

				//Mov para cima da tela ou para baixo, respectivamente
				if (y != vertices[dragState + 1] &&
					(y >= 0 && (y < (vertices[15] - POINT_SIZE)))) {
					movePoint(pointDragged, dragState, vertices[8], y);
					movePoint(&dst.B, 10, vertices[10], y);
				}
				break;

				//Point B
			case 10:
				//Mov para esquerda da tela ou para direita, respectivamente
				if (x != vertices[dragState] &&
					(x <= WINDOW_WIDTH && (x >(vertices[8] + POINT_SIZE)))) {
					movePoint(pointDragged, dragState, x, vertices[11]);
					movePoint(&dst.C, 12, x, vertices[13]);
				}

				//Mov para cima da tela ou para baixo, respectivamente
				if (y != vertices[dragState + 1] &&
					(y >= 0 && (y < (vertices[15] - POINT_SIZE)))) {
					movePoint(pointDragged, dragState, vertices[10], y);
					movePoint(&dst.A, 8, vertices[8], y);
				}
				break;

				//Point C
			case 12:
				//Mov para esquerda da tela ou para direita, respectivamente
				if (x != vertices[dragState] &&
					(x <= WINDOW_WIDTH && (x >(vertices[8] + POINT_SIZE)))) {
					movePoint(pointDragged, dragState, x, vertices[13]);
					movePoint(&dst.B, 10, x, vertices[11]);
				}

				//Mov para cima da tela ou para baixo, respectivamente
				if (y != vertices[dragState + 1] &&
					(y <= WINDOW_HEIGHT && (y >= (vertices[9] + POINT_SIZE)))) {
					movePoint(pointDragged, dragState, vertices[12], y);
					movePoint(&dst.D, 14, vertices[14], y);
				}
				break;

				//Point D
			case 14:
				//Mov para esquerda da tela ou para direita, respectivamente
				if (x != vertices[dragState] &&
					(x >= 0 && (x < (vertices[10] - POINT_SIZE)))) {
					movePoint(pointDragged, dragState, x, vertices[15]);
					movePoint(&dst.A, 8, x, vertices[9]);
				}

				//Mov para cima da tela ou para baixo, respectivamente
				if (y != vertices[dragState + 1] &&
					(y <= WINDOW_HEIGHT && (y >(vertices[9] + POINT_SIZE)))) {
					movePoint(pointDragged, dragState, vertices[14], y);
					movePoint(&dst.C, 12, vertices[12], y);
				}
				break;

			default:
				cout << dragState << endl;
			}//end switch

		}//end dst quad handling

		//Interpolate
		if (image != -1) {
			transferPixels(ilGetData(), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT));
		}

		glutPostRedisplay();
	}//end down key press verification
}

void initDefaults() {
	//Control Quads
	src = Quad(Point(100.f, 200.f, RED), Point(400.f, 200.f, GREEN), Point(400.f, 500.f, YELLOW), Point(100.f, 500.f, BLUE));
	dst = Quad(Point(500.f, 200.f, RED), Point(800.f, 200.f, GREEN), Point(800.f, 500.f, YELLOW), Point(500.f, 500.f, BLUE));

	//src
	vertices[0] = src.A.x;
	vertices[1] = src.A.y;
	vertices[2] = src.B.x;
	vertices[3] = src.B.y;
	vertices[4] = src.C.x;
	vertices[5] = src.C.y;
	vertices[6] = src.D.x;
	vertices[7] = src.D.y;
	//dst
	vertices[8] = dst.A.x;
	vertices[9] = dst.A.y;
	vertices[10] = dst.B.x;
	vertices[11] = dst.B.y;
	vertices[12] = dst.C.x;
	vertices[13] = dst.C.y;
	vertices[14] = dst.D.x;
	vertices[15] = dst.D.y;

	//References
	points[0] = &src.A;
	points[1] = &src.B;
	points[2] = &src.C;
	points[3] = &src.D;

	points[4] = &dst.A;
	points[5] = &dst.B;
	points[6] = &dst.C;
	points[7] = &dst.D;

}

void renderControlQuads() {


	//source quad
	glColor3f(0.8f, 0.8f, 0.8f);
	glBegin(GL_LINE_LOOP);
	glVertex2f(src.A.x, src.A.y);
	glVertex2f(src.B.x, src.B.y);
	glVertex2f(src.C.x, src.C.y);
	glVertex2f(src.D.x, src.D.y);
	glEnd();
	glFlush();

	glBegin(GL_POINTS);
	glColor3f(src.A.color[0], src.A.color[1], src.A.color[2]);
	glVertex2f(src.A.x, src.A.y);
	glColor3f(src.B.color[0], src.B.color[1], src.B.color[2]);
	glVertex2f(src.B.x, src.B.y);
	glColor3f(src.C.color[0], src.C.color[1], src.C.color[2]);
	glVertex2f(src.C.x, src.C.y);
	glColor3f(src.D.color[0], src.D.color[1], src.D.color[2]);
	glVertex2f(src.D.x, src.D.y);
	glEnd();
	glFlush();
	//glPopMatrix();

	//destination quad
	glColor3f(0.8f, 0.8f, 0.8f);
	glBegin(GL_LINE_LOOP);
	glVertex2f(dst.A.x, dst.A.y);
	glVertex2f(dst.B.x, dst.B.y);
	glVertex2f(dst.C.x, dst.C.y);
	glVertex2f(dst.D.x, dst.D.y);
	glEnd();
	glFlush();

	glBegin(GL_POINTS);
	glColor3f(dst.A.color[0], dst.A.color[1], dst.A.color[2]);
	glVertex2f(dst.A.x, dst.A.y);
	glColor3f(dst.B.color[0], dst.B.color[1], dst.B.color[2]);
	glVertex2f(dst.B.x, dst.B.y);
	glColor3f(dst.C.color[0], dst.C.color[1], dst.C.color[2]);
	glVertex2f(dst.C.x, dst.C.y);
	glColor3f(dst.D.color[0], dst.D.color[1], dst.D.color[2]);
	glVertex2f(dst.D.x, dst.D.y);
	glEnd();
	glFlush();

}

void renderScreen(void) {
	glClear(GL_COLOR_BUFFER_BIT);

	glPixelZoom(1, 1);
	sprintf(textControlQuadSize, "(%.0fx%.0f)", dst.B.x - dst.A.x + 1, dst.D.y - dst.A.y + 1);
	displayText(15, 15, textControlQuadSize);

	if (image != -1) {
		glEnable(GL_TEXTURE_2D);

		
		glBegin(GL_QUADS);
		glColor3f(WHITE[0], WHITE[1], WHITE[2]);
		glTexCoord2i(0, 0); glVertex2i(0 + 20, 0 + 20);
		glTexCoord2i(0, 1); glVertex2i(0 + 20, ilGetInteger(IL_IMAGE_HEIGHT) + 20);
		glTexCoord2i(1, 1); glVertex2i(ilGetInteger(IL_IMAGE_WIDTH) + 20, ilGetInteger(IL_IMAGE_HEIGHT) + 20);
		glTexCoord2i(1, 0); glVertex2i(ilGetInteger(IL_IMAGE_WIDTH) + 20, 0 + 20);
		glEnd();
		glFlush();

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
			ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),
			0, GL_RGB, GL_UNSIGNED_BYTE, ilGetData());
		glFlush();
		glDisable(GL_TEXTURE_2D);

		glRasterPos2f(dst.A.x, dst.A.y);
		glPixelZoom(1, -1);
		glDrawPixels(
			dst.B.x - dst.A.x + 1,
			dst.D.y - dst.A.y + 1,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			dstTexture
			);
		glRasterPos2i(0, 0);
	}
	renderControlQuads();

	glutSwapBuffers();
}

void initGlutCalls(int* argc, char* argv[]) {
	glutInit(argc, argv);

	//Creating window
	glutInitWindowPosition(50, 50);//x,y
	glutInitWindowSize(larguraJanela, alturaJanela);//w,h
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	windowID = glutCreateWindow("Projeto PG");

	glutDisplayFunc(renderScreen);

	//Events
	glutKeyboardFunc(handleKeyboard);
	glutMouseFunc(handleMouse);
	glutMotionFunc(handleDrag);

	//Point and Line size/width
	glPointSize(POINT_SIZE);
	glLineWidth(LINE_WIDTH);

	glViewport(0, 0, larguraJanela, alturaJanela);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glOrtho(0.0f, larguraJanela, alturaJanela, 0.0f, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	//Color of glClear() = black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void init(int* argc, char* argv[]) {
	initDefaults();
	initGlutCalls(argc, argv);

	glewExperimental = GL_TRUE;
	glewInit();

	//DevIL
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	{
		printf("wrong DevIL version \n");
	}
	ilInit();
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glGenTextures(1, &textura);
	glBindTexture(GL_TEXTURE_2D, textura);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (image != -1)
		ilDeleteImages(1, (const ILuint *)&image);

	image = carregaImagem(caminhoImagem);
	if (image == -1)
	{
		//cout << "Can't load picture file " << caminhoImagem;
	}
	else {
		//usar em coord de tela
		vertices[0] = ilGetInteger(IL_IMAGE_WIDTH) / 4 + 20;
		vertices[1] = ilGetInteger(IL_IMAGE_HEIGHT) / 4 + 20;
		vertices[2] = (3 * ilGetInteger(IL_IMAGE_WIDTH)) / 4 + 20;
		vertices[3] = ilGetInteger(IL_IMAGE_HEIGHT) / 4 + 20;
		vertices[4] = (3 * ilGetInteger(IL_IMAGE_WIDTH)) / 4 + 20;
		vertices[5] = (3 * ilGetInteger(IL_IMAGE_HEIGHT)) / 4 + 20;
		vertices[6] = ilGetInteger(IL_IMAGE_WIDTH) / 4 + 20;
		vertices[7] = (3 * ilGetInteger(IL_IMAGE_HEIGHT)) / 4 + 20;

		//usar em coord de imagem
		src.A = Point(vertices[0], vertices[1], src.A.color);
		src.B = Point(vertices[2], vertices[3], src.B.color);
		src.C = Point(vertices[4], vertices[5], src.C.color);
		src.D = Point(vertices[6], vertices[7], src.D.color);

		//usar em coord de tela
		vertices[8] = vertices[0] + ilGetInteger(IL_IMAGE_WIDTH) + 50;
		vertices[9] = vertices[1];
		vertices[10] = vertices[2] + ilGetInteger(IL_IMAGE_WIDTH) + 47;
		vertices[11] = vertices[3];
		vertices[12] = vertices[4] + ilGetInteger(IL_IMAGE_WIDTH) + 47;
		vertices[13] = vertices[5];
		vertices[14] = vertices[6] + ilGetInteger(IL_IMAGE_WIDTH) + 50;
		vertices[15] = vertices[7];

		//usar em coord de imagem
		dst.A = Point(vertices[8], vertices[9], dst.A.color);
		dst.B = Point(vertices[10], vertices[11], dst.B.color);
		dst.C = Point(vertices[12], vertices[13], dst.C.color);
		dst.D = Point(vertices[14], vertices[15], dst.D.color);

		transferPixels(ilGetData(), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT));

	}
}

int main(int argc, char* argv[]) {
	init(&argc, argv);

	glutMainLoop();

	return 0;
}
