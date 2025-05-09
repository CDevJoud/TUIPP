#include "U3DViewer.hpp"

U3DViewer::U3DViewer(const std::string& title, uint16_t width, uint16_t height, FunctionContainer fc) : 
	Panel(title, width, height, fc)
{

}

std::shared_ptr<U3DViewer> U3DViewer::CreateInstance(const std::string& title, uint16_t width, uint16_t height, FunctionContainer fc)
{
	auto component = std::make_shared<U3DViewer>(title, width, height, fc);
	component->SetPosition(1, 1);
	return component;
}

int compareTris(const void* a, const void* b)
{
	U3DViewer::Triangle* t1 = (U3DViewer::Triangle*)a;
	U3DViewer::Triangle* t2 = (U3DViewer::Triangle*)b;

	auto z1 = (t1->p[0].z + t1->p[1].z + t1->p[2].z) / 3.0f;
	auto z2 = (t2->p[0].z + t2->p[1].z + t2->p[2].z) / 3.0f;
	if (z1 < z2) return 1;
	else if (z1 > z2)return -1;
	else return 0;
}

void U3DViewer::Render(RenderTarget* out)
{
	Panel::Render(out);
	if (obj != nullptr)
	{
		Triangles sortTris;
		for (auto& tri : obj->mesh.tris)
		{
			auto triRotated = tri.RotateX(obj->rotation.x).RotateY(obj->rotation.y).RotateZ(obj->rotation.z);
			Triangle triTranslatedX, triTranslatedXY, triTranslatedXYZ;

			triTranslatedX = triRotated.TranslateX(obj->position.x);
			triTranslatedXY = triTranslatedX.TranslateY(obj->position.y);
			triTranslatedXYZ = triTranslatedXY.TranslateZ(obj->position.z);

			auto normal = triTranslatedXYZ.CrossProduct();
			Math::Vec3D diff;
			//Zero will be replaced with camera pos
			diff.x = triTranslatedXYZ.p[0].x - 0;
			diff.y = triTranslatedXYZ.p[0].y - 0;
			diff.z = triTranslatedXYZ.p[0].z - 0;
			auto dot = normal.Dot(triTranslatedXYZ.p[0]);
			if (dot < 0.0f)
			{
				this->light.Normalise();
				CHAR_INFO c = GetShadingColor(normal.Dot(light));

				auto triProjected = ProjectTriangle(triTranslatedXYZ);

				ScaleToView(triProjected, {
					Component::GetPosition().X,
					Component::GetPosition().Y,
					Component::GetSize().X,
					Component::GetSize().Y
					});

				triProjected.c = c.Char.UnicodeChar;
				triProjected.color = c.Attributes;

				sortTris.emplace_back(triProjected);

				
			}
		}
		
		qsort(sortTris.data(), sortTris.size(), sizeof Triangle, compareTris);

		for (auto& tri : sortTris)
		{
			RenderTarget::RasterizeTriangle(
				tri.p[0].x, tri.p[0].y,
				tri.p[1].x, tri.p[1].y,
				tri.p[2].x, tri.p[2].y,
				tri.c, tri.color
			);
		}
	}
}

void U3DViewer::SetObject(Object* obj)
{
	this->obj = obj;
}

bool U3DViewer::Mesh::GenerateTorus(float R, float r, int numU, int numV)
{
	this->tris.clear();
	int triangleIndex = 0;
	for (int i = 0; i < numU - 1; i++)
	{
		float u0 = (float)i / (numU - 1) * 2 * 3.14;
		float u1 = (float)(i + 1) / (numU - 1) * 2 * 3.14;

		for (int j = 0; j < numV - 1; j++)
		{
			float v0 = (float)j / (numV - 1) * 2 * 3.14;
			float v1 = (float)(j + 1) / (numV - 1) * 2 * 3.14;

			Math::Vec3D p00 = { (R + r * cos(v0)) * cos(u0), (R + r * cos(v0)) * sin(u0),r * sin(v0) };
			Math::Vec3D p01 = { (R + r * cos(v1)) * cos(u0), (R + r * cos(v1)) * sin(u0),r * sin(v1) };
			Math::Vec3D p10 = { (R + r * cos(v0)) * cos(u1), (R + r * cos(v0)) * sin(u1),r * sin(v0) };
			Math::Vec3D p11 = { (R + r * cos(v1)) * cos(u1), (R + r * cos(v1)) * sin(u1),r * sin(v1) };

			Triangle tri1{}, tri2{};

			tri1.p[0] = p00;
			tri1.p[1] = p10;
			tri1.p[2] = p11;

			this->tris.push_back(tri1);
			triangleIndex++;

			tri2.p[0] = p00;
			tri2.p[1] = p11;
			tri2.p[2] = p01;

			this->tris.push_back(tri2);
			triangleIndex++;
		}
	}
	return true;
}

void U3DViewer::DefinePerspectiveProjection(float fNear, float fFar, float fFov, float fAspectRatio)
{
	float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * Math::PI);
	this->m_proj.m[0][0] = fAspectRatio * fFovRad;
	this->m_proj.m[1][1] = fFovRad;
	this->m_proj.m[2][2] = fFar / (fFar - fNear);
	this->m_proj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
	this->m_proj.m[2][3] = 1.0f;
	this->m_proj.m[3][3] = 0.0f;
}


void U3DViewer::SetLightPosition(Math::Vec3D pos)
{
	this->light = pos;
}

Math::Vec3D U3DViewer::GetLightPosition()
{
	return this->light;
}

U3DViewer::Triangle U3DViewer::ProjectTriangle(Triangle& i)
{
	Triangle o{};
	o.c = i.c;
	o.color = i.c;
	Math::MulMatrixTriangle(i.p, o.p, this->m_proj);
	return o;
}

void U3DViewer::ScaleToView(Triangle& i, SMALL_RECT viewport)
{
	i.p[0].x += 1.0f; i.p[0].y += 1.0f;
	i.p[1].x += 1.0f; i.p[1].y += 1.0f;
	i.p[2].x += 1.0f; i.p[2].y += 1.0f;
	i.p[0].x *= 0.5f * (float)viewport.Right;
	i.p[0].y *= 0.5f * (float)viewport.Bottom;
	i.p[1].x *= 0.5f * (float)viewport.Right;
	i.p[1].y *= 0.5f * (float)viewport.Bottom;
	i.p[2].x *= 0.5f * (float)viewport.Right;
	i.p[2].y *= 0.5f * (float)viewport.Bottom;
}

CHAR_INFO U3DViewer::GetShadingColor(float lum)
{
	short bg_col, fg_col;
	wchar_t sym;
	int pixel_bw = (int)(13.0f * lum);
	switch (pixel_bw)
	{
	case 0: bg_col = 0x0000; fg_col = 0x0000; sym = 0x2588; break;

	case 1: bg_col = 0x0000; fg_col = 0x0008; sym = 0x2591; break;
	case 2: bg_col = 0x0000; fg_col = 0x0008; sym = 0x2592; break;
	case 3: bg_col = 0x0000; fg_col = 0x0008; sym = 0x2593; break;
	case 4: bg_col = 0x0000; fg_col = 0x0008; sym = 0x2588; break;

	case 5: bg_col = 0x0080; fg_col = 0x0007; sym = 0x2591; break;
	case 6: bg_col = 0x0080; fg_col = 0x0007; sym = 0x2592; break;
	case 7: bg_col = 0x0080; fg_col = 0x0007; sym = 0x2593; break;
	case 8: bg_col = 0x0080; fg_col = 0x0007; sym = 0x2588; break;

	case 9:  bg_col = 0x0070; fg_col = 0x000F; sym = 0x2591; break;
	case 10: bg_col = 0x0070; fg_col = 0x000F; sym = 0x2592; break;
	case 11: bg_col = 0x0070; fg_col = 0x000F; sym = 0x2593; break;
	case 12: bg_col = 0x0070; fg_col = 0x000F; sym = 0x2588; break;
	default:
		bg_col = 0x0000; fg_col = 0x0000; sym = 0x2588;
	}

	CHAR_INFO c;
	c.Attributes = bg_col | fg_col;
	c.Char.UnicodeChar = sym;
	return c;
}
U3DViewer::Mesh::Mesh(Triangles& tris)
{
	this->tris = tris;
}
U3DViewer::Object::Object(Mesh& mesh) :
	mesh(mesh)
{

}

U3DViewer::Triangle U3DViewer::Triangle::RotateX(float rotation)
{
	Triangle tri{};
	Math::Mat4x4 matRotX;
	matRotX.m[0][0] = 1;
	matRotX.m[1][1] = cosf(rotation);
	matRotX.m[1][2] = sinf(rotation);
	matRotX.m[2][1] = -sinf(rotation);
	matRotX.m[2][2] = cosf(rotation);
	matRotX.m[3][3] = 1;
	tri.c = this->c;
	tri.color = this->color;
	Math::MulMatrixTriangle(this->p, tri.p, matRotX);
	return tri;
}

U3DViewer::Triangle U3DViewer::Triangle::RotateY(float rotation)
{
	Triangle tri{};
	Math::Mat4x4 matRotY;
	matRotY.m[0][0] = cosf(rotation);
	matRotY.m[1][1] = 1.0f;
	matRotY.m[2][2] = cosf(rotation);
	matRotY.m[0][2] = -sinf(rotation);
	matRotY.m[2][0] = sinf(rotation);
	matRotY.m[3][3] = 1;
	tri.c = this->c;
	tri.color = this->color;
	Math::MulMatrixTriangle(this->p, tri.p, matRotY);
	return tri;
}

U3DViewer::Triangle U3DViewer::Triangle::RotateZ(float rotation)
{
	Triangle tri{};
	Math::Mat4x4 matRotZ;
	matRotZ.m[0][0] = cosf(rotation);
	matRotZ.m[0][1] = sinf(rotation);
	matRotZ.m[1][0] = -sinf(rotation);
	matRotZ.m[1][1] = cosf(rotation);
	matRotZ.m[2][2] = 1;
	matRotZ.m[3][3] = 1;
	tri.c = this->c;
	tri.color = this->color;
	Math::MulMatrixTriangle(this->p, tri.p, matRotZ);
	return tri;
}

U3DViewer::Triangle U3DViewer::Triangle::TranslateX(float distance)
{
	Triangle tri{};
	tri.c = this->c;
	tri.color = this->color;
	tri.p[0].x = this->p[0].x + distance;
	tri.p[1].x = this->p[1].x + distance;
	tri.p[2].x = this->p[2].x + distance;
	tri.p[0].y = this->p[0].y;
	tri.p[1].y = this->p[1].y;
	tri.p[2].y = this->p[2].y;
	tri.p[0].z = this->p[0].z;
	tri.p[1].z = this->p[1].z;
	tri.p[2].z = this->p[2].z;
	return tri;
}

U3DViewer::Triangle U3DViewer::Triangle::TranslateY(float distance)
{
	Triangle tri{};
	tri.c = this->c;
	tri.color = this->color;
	tri.p[0].x = this->p[0].x;
	tri.p[1].x = this->p[1].x;
	tri.p[2].x = this->p[2].x;
	tri.p[0].y = this->p[0].y + distance;
	tri.p[1].y = this->p[1].y + distance;
	tri.p[2].y = this->p[2].y + distance;
	tri.p[0].z = this->p[0].z;
	tri.p[1].z = this->p[1].z;
	tri.p[2].z = this->p[2].z;
	return tri;
}

U3DViewer::Triangle U3DViewer::Triangle::TranslateZ(float distance)
{
	Triangle tri{};
	tri.c = this->c;
	tri.color = this->color;
	tri.p[0].x = this->p[0].x;
	tri.p[1].x = this->p[1].x;
	tri.p[2].x = this->p[2].x;
	tri.p[0].y = this->p[0].y;
	tri.p[1].y = this->p[1].y;
	tri.p[2].y = this->p[2].y;
	tri.p[0].z = this->p[0].z + distance;
	tri.p[1].z = this->p[1].z + distance;
	tri.p[2].z = this->p[2].z + distance;
	return tri;
}

Math::Vec3D U3DViewer::Triangle::CrossProduct()
{
	Math::Vec3D normal, line1, line2;

	line1.x = this->p[1].x - this->p[0].x;
	line1.y = this->p[1].y - this->p[0].y;
	line1.z = this->p[1].z - this->p[0].z;

	line2.x = this->p[2].x - this->p[0].x;
	line2.y = this->p[2].y - this->p[0].y;
	line2.z = this->p[2].z - this->p[0].z;

	normal.x = line1.y * line2.z - line1.z * line2.y;
	normal.y = line1.z * line2.x - line1.x * line2.z;
	normal.z = line1.x * line2.y - line1.y * line2.x;

	auto length = sqrtf(powf(normal.x, 2) + powf(normal.y, 2) + powf(normal.z, 2));
	if (length != 0.0f)
	{
		normal.x /= length;
		normal.y /= length;
		normal.z /= length;
	}
	else
	{
		normal.x = normal.y = normal.z = 0.0f;
	}
	return normal;
}
