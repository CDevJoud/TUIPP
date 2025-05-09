#pragma once

class Math
{
public:
	class Mat4x4
	{
	public:
		float m[4][4] = { 0 };
	};
	class Vec3D
	{
	public:
		float x, y, z;
		float Dot(Vec3D _);
		void Normalise();
	};
	// ADD inline functionality later
	static void MulMatrixVector(Vec3D& i, Vec3D& o, Mat4x4& m);
	static void MulMatrixTriangle(Vec3D* i, Vec3D* o, Mat4x4& m);
	static constexpr inline float PI = 3.1415927f;
};

