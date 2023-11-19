/*************************************************************

	The CVector math is implemented here.

 *************************************************************/

#include "CVector.h"

float CVector3D::getDistanceBetween2Points(CVector3D& p1, CVector3D& p2) {
	return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2) + pow(p2.z - p1.z, 2));
}

bool CVector3D::isInCircularRange(CVector3D& p1, CVector3D& p2, float radius) {
	float distance = getDistanceBetween2Points(p1, p2);
	return (distance <= radius);
}