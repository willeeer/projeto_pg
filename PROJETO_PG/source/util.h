#pragma once
#include "glew.h"
#include "freeglut.h"
#include "math.h"
#include "il.h"
#include "ilu.h"
#include "ilut.h"

#define POINT_SIZE 10.0
#define LINE_WIDTH 2.0
#define ROUND(x) ( (int)floor((x)+0.5) )

#define WITHIN_POINT(c, p) ( ( c >= p - POINT_SIZE/2 ) && ( c <= p + POINT_SIZE/2 ) )

extern GLfloat RED[];
extern GLfloat GREEN[];
extern GLfloat BLUE[];
extern GLfloat YELLOW[];
extern GLfloat GRAY[];
extern GLfloat WHITE[];

struct Point {
	GLfloat x;
	GLfloat y;

	GLfloat *color;

	Point() : x{ GLfloat(0.0) }, y{ GLfloat(0.0) }, color{ RED } {};
	Point(float a, float b) : x{ GLfloat(a) }, y{ GLfloat(b) }, color{ RED } {};
	Point(float a, float b, GLfloat c[]) : x{ GLfloat(a) }, y{ GLfloat(b) }, color{ c } {};

	Point& operator=(const Point& p) {
		x = p.x;
		y = p.y;
		color = p.color;
		return *this;
	}
};
typedef struct Point Point;

struct Pixel {
	GLfloat r;
	GLfloat g;
	GLfloat b;

	Point p;//image coordinates
};

struct Quad {
	/*
		A B
		D C
	*/
	Point A;
	Point B;
	Point C;
	Point D;

	Quad() : A{ Point(0,0,GREEN) }, B{ Point(0,0,GREEN) }, C{ Point(0,0,GREEN) }, D{ Point(0,0,GREEN) } {};
	Quad(Point a, Point b, Point c, Point d) : A{ a }, B{ b }, C{ c }, D{ d } {};

	Quad& operator=(const Quad& q) {
		A = q.A;
		B = q.B;
		C = q.C;
		D = q.D;
		return *this;
	}
};
typedef struct Quad Quad;