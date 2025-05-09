#pragma once
#include "Panel.hpp"
#include "Math.hpp"

class U3DViewer : public Panel
{
public:
	class Mesh;
	class Object;
	class Triangle
	{
	public:
		Math::Vec3D p[3];
		uint16_t c, color;
		Triangle RotateX(float rotation);
		Triangle RotateY(float rotation);
		Triangle RotateZ(float rotation);
		Triangle TranslateX(float distance);
		Triangle TranslateY(float distance);
		Triangle TranslateZ(float distance);
		Math::Vec3D CrossProduct();
	};
	typedef std::vector<Triangle> Triangles;
	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(Triangles& tris);
		bool GenerateTorus(float R, float r, int numU, int numV);
		Triangles tris;
	};
	class Object
	{
	public:
		Object() = default;
		Object(Mesh& mesh);
		Math::Vec3D position, rotation;
		Mesh mesh;
	};
	void DefinePerspectiveProjection(float fNear, float fFar, float fFov, float fAspectRatio);
	void SetLightPosition(Math::Vec3D pos);
	Math::Vec3D GetLightPosition();
public:
	U3DViewer() = default;
	U3DViewer(const std::string& title, uint16_t width, uint16_t height, FunctionContainer fc = {});

	static std::shared_ptr<U3DViewer> CreateInstance(const std::string& title, uint16_t width, uint16_t height, FunctionContainer fc = {});
	void Render(RenderTarget* out) override;

	void SetObject(Object* obj);

protected:
	void InitRenderTarget(RenderElement* re);
	Triangle ProjectTriangle(Triangle& i);
	void ScaleToView(Triangle& i, SMALL_RECT viewport);
	CHAR_INFO GetShadingColor(float lum);

private:
	Object* obj = nullptr;
	Math::Vec3D light;
	Math::Mat4x4 m_proj;
};
