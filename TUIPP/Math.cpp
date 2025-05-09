#include "Math.hpp"
#include <math.h>

void Math::MulMatrixVector(Vec3D& i, Vec3D& o, Mat4x4& m)
{
	o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
	o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
	o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
	float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

	if (w != 0.0f)
	{
		o.x /= w; o.y /= w; o.z /= w;
	}
}

void Math::MulMatrixTriangle(Vec3D* i, Vec3D* o, Mat4x4& m)
{
	MulMatrixVector(i[0], o[0], m);
	MulMatrixVector(i[1], o[1], m);
	MulMatrixVector(i[2], o[2], m);
}

float Math::Vec3D::Dot(Vec3D _)
{
	return this->x * _.x + this->y * _.y + this->z * _.z;
}

void Math::Vec3D::Normalise()
{
	float length = sqrtf(powf(this->x, 2) + powf(this->y, 2) + powf(this->z, 2));

	if (length != 0.0f)
	{
		this->x /= length;
		this->y /= length;
		this->z /= length;
	}
	else
	{
		x = y = z = 0.0f;
	}
}
