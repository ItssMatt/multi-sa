/*************************************************************

	The CVector class is used to represent any 3D point.

 *************************************************************/

#ifndef CVECTOR_H
#define CVECTOR_H

#include <math.h>

class CVector3D {
public:
	float x, y, z;

	CVector3D() : x(0), y(0), z(0) { }
	CVector3D(float x, float y, float z) : x(x), y(y), z(z) { }
	static float getDistanceBetween2Points(CVector3D& p1, CVector3D& p2);
	static bool isInCircularRange(CVector3D& p1, CVector3D& p2, float radius);
};

class CVector2D {
public:
	float x, y;
};
#endif