#define TUIPP_USE_UTF16_STRING

#ifdef TUIPP_USE_UTF8_STRING
#define _T(t) u8##t
#undef TUIPP_USE_UTF16_STRING
#undef TUIPP_USE_UTF32_STRING
#endif

#ifdef TUIPP_USE_UTF16_STRING
#define _T(t) u##t
#undef TUIPP_USE_UTF8_STRING
#undef TUIPP_USE_UTF32_STRING
#endif

#ifdef TUIPP_USE_UTF32_STRING
#undef TUIPP_USE_UTF8_STRING
#undef TUIPP_USE_UTF16_STRING
#endif

#if !defined(TUIPP_HEADER)
#define TUIPP_HEADER
#pragma once
#if defined(_WIN32) || defined(_WIN64)
#if defined(TUIPP_USE_UTF8_STRING)
#define TUIPP_STARTUP(Application) INT APIENTRY WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPTSTR, _In_ INT) {Application app; INT nExitCode = app.main(); return nExitCode; }
#elif defined(TUIPP_USE_UTF16_STRING) || defined(TUIPP_USE_UTF32_STRING)
#define TUIPP_STARTUP(Application) INT APIENTRY wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPTSTR, _In_ INT) {Application app; INT nExitCode = app.main(); return nExitCode; }
#endif
#define WIN32_LEAN_AND_MEAN

#ifdef TUIPP_USE_UTF8_STRING
#undef UNICODE
#endif
#define NOMINMAX

#include <Windows.h>

#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

#endif

#include <string>
#include <memory>
#include <vector>
namespace tui {
#if defined(TUIPP_USE_UTF8_STRING)
	using String = std::u8string;
#elif defined(TUIPP_USE_UTF16_STRING)
	using String = std::u16string;
#elif defined(TUIPP_USE_UTF32_STRING)
	using String = std::u32string
#else
#error String Type is not defined!
	using String = void*;
#endif


	typedef signed char Int8;
	typedef signed short Int16;
	typedef signed int Int32;
	typedef signed long long Int64;

	typedef unsigned char Uint8;
	typedef unsigned short Uint16;
	typedef unsigned int Uint32;
	typedef unsigned long long Uint64;

	typedef Uint8 Byte;
	typedef Uint16 Word;
	typedef Uint32 Dword;
	typedef Uint64 Qword;

#if defined(_WIN32) || defined(_WIN64)
	using NativeHandle = HWND;
#else
	using NativeHandle = void*;
#endif
	template<typename T>
	class Vec2 {
	public:
		Vec2() = default;
		Vec2(T _x, T _y) : x(_x), y(_y) {}
		T x, y;
	};

	template <typename T>
	class Rect {
	public:
		Rect();

		Rect(T rectLeft, T rectTop, T rectWidth, T rectHeight);

		Rect(const Vec2<T>& position, const Vec2<T>& size);

		template <typename U>
		explicit Rect(const Rect<U>& rectangle);

		bool contains(T x, T y) const;

		bool contains(const Vec2<T>& point) const;

		bool intersects(const Rect<T>& rectangle) const;

		bool intersects(const Rect<T>& rectangle, Rect<T>& intersection) const;

		bool isInsideBounds(const Vec2<T>& p) const;

		Vec2<T> getPosition() const;

		Vec2<T> getSize() const;

		T x, y, width, height;
	};

	template <typename T>
	bool operator==(const Rect<T>& left, const Rect<T>& right);

	template <typename T>
	bool operator!=(const Rect<T>& left, const Rect<T>& right);

	using Vec2w = Vec2<Word>;
	using Rectw = Rect<Word>;

	class IntrusiveCounted {
	public:
		IntrusiveCounted() = default;
		virtual ~IntrusiveCounted() = default;

		IntrusiveCounted(const IntrusiveCounted&) : IntrusiveCount(0) {

		}
		IntrusiveCounted& operator=(const IntrusiveCounted&) {
			IntrusiveCount.store(0);
			return *this;
		}

		void incIntrusiveCount() const {
			++IntrusiveCount;
		}

		void decIntrusiveCount() const {
			--IntrusiveCount;
		}

		Dword getIntrusiveCount() const { return IntrusiveCount.load(); }
	private:
		mutable std::atomic<Dword> IntrusiveCount = 0;
	};

	template <typename T>
	class Intrusive {
	public:
		Intrusive() = default;
		~Intrusive() {
			release();
		}


		Intrusive(std::nullptr_t n) :
			instance(nullptr) {

		}

		Intrusive(const Intrusive<T>& other) : instance(other.instance) {
			retain();
		}

		Intrusive<T>& operator=(const Intrusive<T>& other) {
			if (this != &other) {
				release();
				instance = other.instance;
				retain();
			}
			return *this;
		}

		Intrusive(Intrusive<T>&& other) noexcept
			: instance(other.instance) {
			other.instance = nullptr;
		}

		Intrusive(T* instance)
			: instance(instance) {
			static_assert(std::is_base_of<IntrusiveCounted, T>::value, "Class is not IntrusiveCounted!");

			retain();
		}

		template<typename T2>
		Intrusive& operator=(Intrusive<T2>&& other) {
			release();

			instance = other.instance;
			other.instance = nullptr;
			return *this;
		}

		operator bool() { return instance != nullptr; }
		operator bool() const { return instance != nullptr; }

		T* operator->() { return instance; }
		T* operator->() const { return instance; }

		T& operator*() { return *instance; }
		const T& operator*() const { return *instance; }

		T* getRawPtr() { return instance; }
		const T* getRawPtr() const { return instance; }

		void reset(T* instance = nullptr) {
			release();
			this->instance = instance;
		}

		template<typename T2>
			requires(std::is_base_of_v<T2, T> || std::is_base_of_v<T, T2>)
		Intrusive<T2> as() const {
			return Intrusive<T2>(*this);
		}

		template<typename... Args>
		static Intrusive<T> make(Args&&...args) {
			return Intrusive<T>(new (std::nothrow) T(std::forward<Args>(args)...));
		}

		bool operator==(const Intrusive<T>& other) const {
			return instance == other.instance;
		}

		bool operator!=(const Intrusive<T>& other) const {
			return !(*this == other);
		}

		bool equalObject(const Intrusive<T>& other) {
			if (!instance || !other.instance) {
				return false;
			}

			return instance == other.instance;
		}

		void retain() const {
			if (instance) {
				instance->incIntrusiveCount();
			}
		}

		void release() const {
			if (instance) {
				instance->decIntrusiveCount();

				if (instance->getIntrusiveCount() == 0) {
					delete instance;
					instance = nullptr;
				}
			}
		}

		template<class T2>
		friend class Intrusive;

		mutable T* instance = nullptr;
	};

	template <typename U, typename T>
	Intrusive<U> dynamicPtrCast(const Intrusive<T>& ptr) {
		const U* casted = dynamic_cast<const U*>(ptr.getRawPtr());

		if (!casted)
			return Intrusive<U>(nullptr);

		return Intrusive<U>(const_cast<U*>(casted));
	}

	class Startup {
		virtual int main() = 0;
	};

	struct CharInfo {
		union {
			Int16 unicodeChar;
			Int8 asciiChar;
		}pixel;
		Int16 attrib;
	};

	struct RenderElement {
		NativeHandle hConsole;
		CharInfo* screenBuffer;
		Rect<Word> viewport;
	};


	class ConsoleInterface {
	public:
		virtual bool isOpen() const = 0;
		virtual NativeHandle getNativeHandle() const = 0;
		virtual void setPosition(const Vec2<Word>& position) = 0;
		virtual void setSize(const Vec2<Word>& size) = 0;
		
		virtual Vec2<Word> getSize() const = 0;
		virtual Vec2<Word> getPosition() const = 0;

		virtual const String getTitle() const = 0;

		virtual void setTitle(const String& title) = 0;

		virtual void display(RenderElement& re) = 0;

		virtual void close() = 0;

		virtual bool initConsoleWindow() = 0;
	};

#if defined(_WIN32) || defined(_WIN64)

	class ConsoleWindow_ImplNativeWin32 : public ConsoleInterface {
	public:
		ConsoleWindow_ImplNativeWin32(NativeHandle window, const String& title, const Vec2<Word>& dimension);
		~ConsoleWindow_ImplNativeWin32();

		virtual bool isOpen() const override;
		virtual NativeHandle getNativeHandle() const override;
		virtual void setPosition(const Vec2<Word>& position) override;
		virtual void setSize(const Vec2<Word>& size) override;

		virtual Vec2<Word> getSize() const override;
		virtual Vec2<Word> getPosition() const override;
		
		virtual const String getTitle() const override;

		virtual void setTitle(const String& title) override;

		virtual void close() override;
		
		virtual void display(RenderElement& re) override;

		virtual bool initConsoleWindow() override;
	private:
		bool bIsOpen;
		NativeHandle nwh;
	};

#endif


	class RenderTarget {
	public:
		RenderTarget() = default;
		RenderTarget(RenderElement* re);
		~RenderTarget();

		void flushTo(RenderTarget* out, Rect<Word> rect);

		void setPixel(const Word& x, const Word& y, Int16 c = 0x2588, Int16 att = 0x00FF);
		void setPixel(const Vec2<Word>& p, Int16 c = 0x2588, Int16 att = 0x00FF);

		void fill(const Word& x1, const Word& y1, const Word& x2, const Word& y2, Int16 c = 0x2588, Int16 att = 0x00FF);
		void fill(const Vec2<Word>& p1, const Vec2<Word>& p2, Int16 c = 0x2588, Int16 att = 0x00FF);
		void fill(const Rect<Word>& rect, Int16 c = 0x2588, Int16 att = 0x00FF);

		void renderLine(const Word& x1, const Word& y1, const Word& x2, const Word& y2, Int16 c = 0x2588, Int16 att = 0x00FF);
		void renderLine(const Vec2<Word>& p1, const Vec2<Word>& p2, Int16 c = 0x2588, Int16 att = 0x00FF);
		void renderLine(const Rect<Word>& p, Int16 c = 0x2588, Int16 att = 0x00FF);

		void renderTriangle(const Word& x1, const Word& y1, const Word& x2, const Word& y2, const Word& x3, const Word& y3, Int16 c = 0x2588, Int16 att = 0x00FF);
		void renderTriangle(const Vec2<Word>& p1, const Vec2<Word>& p2, const Vec2<Word>& p3, Int16 c = 0x2588, Int16 att = 0x00FF);

		void rasterizeTriangle(const Word& x1, const Word& y1, const Word& x2, const Word& y2, const Word& x3, const Word& y3, Int16 c = 0x2588, Int16 att = 0x00FF);
		void rasterizeTriangle(const Vec2<Word>& p1, const Vec2<Word>& p2, const Vec2<Word>& p3, Int16 c = 0x2588, Int16 att = 0x00FF);

		void renderText(const Word& x, const Word& y, const String& txt, Int16 att = 0x000F);
		void renderText(const Vec2<Word>&p, const String& txt, Int16 att = 0x000F);

		void clear(Int16 c = 0x2588, Int16 att = 0x0000);
		void calcClipOn(Int16& x, Int16& y);
	protected:
		RenderElement re;

		CharInfo getPixelAt(const Word& x, const Word& y) const;
		CharInfo getPixelAt(const Vec2<Word>& p) const;
		CharInfo* getPixelScreenBuffer() const;
	};

	class EventProcessor {
	public:
		struct KeyStrokesCondition {
			bool bStrokePressed : 1;
			bool bStrokeReleased : 1;
			bool bStrokeIsHeld : 1;
		};
		KeyStrokesCondition m_KeyboardCondition[256];
		KeyStrokesCondition m_MouseCondition[5];
		short               m_NewKeyboardCondition[256];
		short               m_OldKeyboardCondition[256];
		bool                m_OldMouseCondition[5];
		bool                m_NewMouseCondition[5];

		Vec2<Word> mousePos;  // Mouse position relative to the console window.
		NativeHandle hInput; // Handle to the console input.
		bool ShiftOn; // Flag indicating if the Shift key is pressed.

	public:
		EventProcessor() = default;

		EventProcessor(NativeHandle consoleInput);

		enum class MouseType {
			Left = 0, Right, Middle
		};

		KeyStrokesCondition keyboard(INT ID);

		Vec2<Word> getMousePos();

		KeyStrokesCondition mouse(MouseType ID);

		void processEvents();

		void initEventProcessor(NativeHandle hConsoleInput);
	};

	class Component : public IntrusiveCounted {
	public:
		enum Type {
			None = 0x00,
			Button,
			Panel,
			InputBox
		};

		Component(Type cType, RenderElement& re, const String& id);

		virtual void onUpdate(EventProcessor* ep) = 0;
		virtual void onRender(RenderTarget* out) = 0;
		virtual void onInit() = 0;

		virtual void setSize(const Word& width, const Word& height, bool resizeBuffer = false);
		virtual void setPosition(const Word& x, const Word& y);

		Vec2<Word> getSize() const;
		Vec2<Word> getPosition() const;

		String getID() const;

		Vec2<Word> getDefaultPosition() const;
		Type getType() const;

	protected:
		void setID(const String& id);

		bool bTargeted;
		RenderElement& getRenderElement() const;
		void setComponentType(Type type);

		Rect<Word> viewport;

	private:
		String id;
		Vec2<Word> defaultPosition;
		Type type;
		RenderElement* re;
		friend class Console;
	};

	class Panel : public virtual RenderTarget, public Component {
	public:

		enum class TitleAlignment : Int8 {
			NONE = 0,
			Left,
			Center,
			Right
		};

		struct Properties {
			bool isResizable : 1 = true;
			bool isMovable : 1 = true;
			Int16 titleAlignmentOffset = 0;
			TitleAlignment titleAlignment = TitleAlignment::Center;
			String title = _T("");
			Byte borderColor = 0x0F;
		};


		Panel();
		Panel(const String& title, const Word& width, const Word& height);
		~Panel();

		static Intrusive<Component> createInstance(const String& title, const Word& width, const Word& height);

		bool create(const String& title, const Word& width, const Word& height);

		bool insertComponent(const Intrusive<Component>& comp);
		template<typename T>
		Intrusive<T> getComponent(const String& title) {
			for (auto& i : this->components) {
				if (i->getID() == title) {
					return dynamicPtrCast<T>(i);
				}
			}
			return nullptr;
		}
		virtual void onUpdate(EventProcessor* ep) override;
		virtual void onRender(RenderTarget* out) override;
		virtual void onInit() override;

		Vec2<Word> getMousePos() const;

		Properties& getProperties();
	protected:
		void setUpFrame(RenderTarget* out, Rect<Word>rect, Word color);
	private:
		std::vector<Intrusive<Component>> components;
		bool isDragging = false, resizeDragging = true, isHovering = false, targeted = false;

		bool		   isResizingRight : 1 = false;
		bool		   isResizingLeft : 1 = false;
		bool		   isResizingBottom : 1 = false;

		Vec2<Word> mousePosition, offset, resizeOffset;

		Properties props;
	};

	class U3DViewer : public Panel {
	public:
		class Math {
		public:
			class Mat4x4 {
			public: float m[4][4] = { 0 };
			};
			class Vec3f{
			public:
				float x, y, z;
				inline float Dot(Vec3f _) {
					return this->x * _.x + this->y * _.y + this->z * _.z;
				}
				inline void Normalise() {
					float length = sqrtf(powf(this->x, 2) + powf(this->y, 2) + powf(this->z, 2));

					if (length != 0.0f) {
						this->x /= length;
						this->y /= length;
						this->z /= length;
					}
					else {
						x = y = z = 0.0f;
					}
				}
			};

			static inline void MulMatrixVector(Vec3f& i, Vec3f& o, Mat4x4& m) {
				o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
				o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
				o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
				float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

				if (w != 0.0f) {
					o.x /= w; o.y /= w; o.z /= w;
				}
			}
			static void MulMatrixTriangle(Vec3f* i, Vec3f* o, Mat4x4& m) {
				MulMatrixVector(i[0], o[0], m);
				MulMatrixVector(i[1], o[1], m);
				MulMatrixVector(i[2], o[2], m);
			}
			static constexpr inline float PI = 3.1415927f;
		};
		class Mesh;
		class Object;
		class Triangle {
		public:
			Math::Vec3f p[3];

			inline Triangle rotateX(float rotation) {
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
			inline Triangle rotateY(float rotation) {
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
			inline Triangle rotateZ(float rotation) {
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

			inline Triangle translateX(float distance) {
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
			inline Triangle translateY(float distance) {
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
			inline Triangle translateZ(float distance) {
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

			Math::Vec3f cross() {
				Math::Vec3f normal, line1, line2;

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
				if (length != 0.0f) {
					normal.x /= length;
					normal.y /= length;
					normal.z /= length;
				}
				else {
					normal.x = normal.y = normal.z = 0.0f;
				}
				return normal;
			}

			Word c, color;
		};
		typedef std::vector<Triangle> Triangles;
		class Mesh {
		public:
			Mesh() = default;
			Mesh(Triangles& tris);
			bool generateTorus(float R, float r, Int32 numU, Int32 numV);
			Triangles tris;
		};
		class Object {
		public:
			Math::Vec3f position, rotation;
			Mesh mesh;
		};

	public:
		U3DViewer() = default;
		U3DViewer(const String& title, const Word& width, const Word& height);

		static Intrusive<Component> createInstance(const String& title, const Word& width, const Word& height);

		virtual void onRender(RenderTarget* out) override;

		void setObject(Object* obj);

		void definePerspectiveProjection(float near, float far, float fov, float aspectRatio);
		void setLightPosition(Math::Vec3f pos);
	protected:
		Triangle projectTriangle(Triangle& i);
		void scaleToView(Triangle& i, Rect<Word> viewport);
		CharInfo getShadingColor(float lum);

	private:
		Object* obj = nullptr;
		Math::Vec3f light;
		Math::Mat4x4 proj;
	};

	class Console : public RenderTarget, public EventProcessor, public IntrusiveCounted{
	public:
		enum class Type {
			None = 0,
			NativeOS,
			Custom
		};

		Console(const String& title, const Vec2<Word>& dimension, const Vec2<Word>& pxlDimension, Type type = Type::NativeOS);
		~Console();
		
		std::shared_ptr<ConsoleInterface> getInterface();

		NativeHandle getInputHandle();
		NativeHandle getOutputHandle();

		bool insertComponent(const Intrusive<Component>& component);

		template<typename T>
		Intrusive<T> getComponent(const String& title) {
			for (auto& i : components) {
				if (i->getID() == title) {
					return dynamicPtrCast<T>(i);
				}
			}
			return nullptr;
		}

		void display();
		bool isOpen();
		void close();

		Rect<Word> getViewport() const;
	private:
		std::shared_ptr<ConsoleInterface> interface;

		std::vector<Intrusive<Component>> components;

		NativeHandle hInput, hOutput;

		Type type;
		Rect<Word> viewport;
	};
}

/// <summary>
/// IMPLEMENTATION
/// </summary>

namespace tui {

	template<typename T>
	inline Rect<T>::Rect() : x(0), y(0), width(0), height(0) {}

	template<typename T>
	inline Rect<T>::Rect(T rectLeft, T rectTop, T rectWidth, T rectHeight) : x(rectLeft), y(rectTop), width(rectWidth), height(rectHeight) {}

	template<typename T>
	inline Rect<T>::Rect(const Vec2<T>& p, const Vec2<T>& size) : x(p.x), y(p.y), width(size.x), height(size.y) {}

	template<typename T>
	inline bool Rect<T>::contains(T x, T y) const {

		T minX = std::min(this->x, static_cast<T>(this->x + width));
		T maxX = std::max(this->x, static_cast<T>(this->x + width));
		T minY = std::min(this->y, static_cast<T>(this->y + height));
		T maxY = std::max(this->y, static_cast<T>(this->y + height));

		return (x >= minX) && (x < maxX) && (y >= minY) && (y < maxY);

	}

	template<typename T>
	inline bool Rect<T>::contains(const Vec2<T>& point) const {
		return contains(point.x, point.y);
	}

	template <typename T>
	bool Rect<T>::intersects(const Rect<T>& rectangle) const {
		Rect<T> intersection;
		return intersects(rectangle, intersection);
	}

	template<typename T>
	bool Rect<T>::isInsideBounds(const Vec2<T>& p) const {
		return (p.x >= this->x && p.x <= this->width && p.y >= this->y && p.y <= this->height);
	}


	template <typename T>
	bool Rect<T>::intersects(const Rect<T>& rectangle, Rect<T>& intersection) const {
		// Rectangles with negative dimensions are allowed, so we must handle them correctly

		// Compute the min and max of the first rectangle on both axes
		T r1MinX = std::min(x, static_cast<T>(x+ width));
		T r1MaxX = std::max(x, static_cast<T>(x+ width));
		T r1MinY = std::min(y, static_cast<T>(y+ height));
		T r1MaxY = std::max(y, static_cast<T>(y+ height));

		// Compute the min and max of the second rectangle on both axes
		T r2MinX = std::min(rectangle.x, static_cast<T>(rectangle.x + rectangle.width));
		T r2MaxX = std::max(rectangle.x, static_cast<T>(rectangle.x + rectangle.width));
		T r2MinY = std::min(rectangle.y, static_cast<T>(rectangle.y + rectangle.height));
		T r2MaxY = std::max(rectangle.y, static_cast<T>(rectangle.y + rectangle.height));

		// Compute the intersection boundaries
		T interLeft = std::max(r1MinX, r2MinX);
		T interTop = std::max(r1MinY, r2MinY);
		T interRight = std::min(r1MaxX, r2MaxX);
		T interBottom = std::min(r1MaxY, r2MaxY);

		// If the intersection is valid (positive non zero area), then there is an intersection
		if ((interLeft < interRight) && (interTop < interBottom)) {
			intersection = Rect<T>(interLeft, interTop, interRight - interLeft, interBottom - interTop);
			return true;
		}
		else {
			intersection = Rect<T>(0, 0, 0, 0);
			return false;
		}
	}

	template <typename T>
	Vec2<T> Rect<T>::getPosition() const {
		return Vec2<T>(x, y);
	}

	template <typename T>
	Vec2<T> Rect<T>::getSize() const {
		return Vec2<T>(width, height);
	}

	template <typename T>
	inline bool operator ==(const Rect<T>& left, const Rect<T>& right) {
		return (left.x == right.x) && (left.width == right.width) &&
			(left.y == right.y) && (left.height == right.height);
	}


	template <typename T>
	inline bool operator !=(const Rect<T>& left, const Rect<T>& right) {
		return !(left == right);
	}

	template<typename T>
	template<typename U>
	Rect<T>::Rect(const Rect<U>& rectangle) :
		x(static_cast<T>(rectangle.x)),
		y(static_cast<T>(rectangle.y)),
		width(static_cast<T>(rectangle.width)),
		height(static_cast<T>(rectangle.height)) {}

#if defined(_WIN32) || defined(_WIN64)
	static RenderElement* g_re;
	/*static void VisualDebuggerBreakPoint() {
		SMALL_RECT rect = { 0, 0, g_re->viewport.width, g_re->viewport.height };
		WriteConsoleOutputW(g_re->hConsole, (PCHAR_INFO)g_re->screenBuffer, { (short)g_re->viewport.width, (short)g_re->viewport.height }, {}, &rect);
		while (!(GetAsyncKeyState(VK_SPACE) & 0x8000)) {
			Sleep(1);
		}
	}*/

	/// ////
	ConsoleWindow_ImplNativeWin32::ConsoleWindow_ImplNativeWin32(NativeHandle window, const String& title, const Vec2<Word>& dimension) :
		bIsOpen(false), 
		nwh(nullptr) {
		using CW = ConsoleWindow_ImplNativeWin32;
		if (title.empty() && dimension.x == 0 && dimension.y == 0) {
			if (CW::initConsoleWindow()) {
				CW::bIsOpen = (CW::nwh != nullptr);
			}
		}
		else if (CW::initConsoleWindow()) {
			CW::bIsOpen = (CW::nwh != nullptr);
			CW::setSize(dimension);

			RECT wndRect, deskRect;

			::GetWindowRect(CW::nwh, &wndRect);
			::GetWindowRect(::GetDesktopWindow(), &deskRect);

			auto wnd = Vec2<Int32>(wndRect.right - wndRect.left, wndRect.bottom - wndRect.top);
			auto desk = Vec2<Int32>(deskRect.right - deskRect.left, deskRect.bottom - deskRect.top);

			auto position = Vec2<Word>((desk.x - wnd.x) / 2ui16, (desk.y - wnd.y) / 2ui16);

			CW::setPosition(position);

			CW::setTitle(title);
		}
	}
	inline ConsoleWindow_ImplNativeWin32::~ConsoleWindow_ImplNativeWin32() {
		FreeConsole();
	}
	inline bool ConsoleWindow_ImplNativeWin32::isOpen() const {
		return bIsOpen;
	}
	inline NativeHandle ConsoleWindow_ImplNativeWin32::getNativeHandle() const {
		return nwh;
	}
	inline void ConsoleWindow_ImplNativeWin32::setPosition(const Vec2<Word>& p) {
		RECT wndRect;
		GetWindowRect(nwh, &wndRect);
		MoveWindow(nwh, p.x, p.y, wndRect.right, wndRect.bottom, TRUE);
	}
	inline void ConsoleWindow_ImplNativeWin32::setSize(const Vec2<Word>& size) {
		RECT wndRect;
		GetWindowRect(nwh, &wndRect);
		MoveWindow(nwh, wndRect.left, wndRect.top, size.x, size.y, TRUE);
	}
	
	inline Vec2<Word> ConsoleWindow_ImplNativeWin32::getSize() const {
		RECT wndRect;
		GetWindowRect(nwh, &wndRect);
		return Vec2<Word>(wndRect.right, wndRect.bottom);
	}
	inline Vec2<Word> ConsoleWindow_ImplNativeWin32::getPosition() const {
		RECT wndRect;
		GetWindowRect(nwh, & wndRect);
		return Vec2<Word>(wndRect.left, wndRect.top);
	}
	inline const String ConsoleWindow_ImplNativeWin32::getTitle() const {
#if defined(TUIPP_USE_UTF8_STRING)
		char8_t buffer[MAX_PATH]{};
		GetWindowTextA(nwh, (char*)buffer, MAX_PATH);
		return buffer;
#elif defined(TUIPP_USE_UTF16_STRING)
		char16_t buffer[MAX_PATH]{};
		GetWindowTextW(nwh, (wchar_t*)buffer, MAX_PATH);
		return buffer;
#elif defined(TUIPP_USE_UTF32_STRING)
#error UTF-32 is still in development!
#endif
	}
	inline void ConsoleWindow_ImplNativeWin32::setTitle(const String& title) {
#if defined(TUIPP_USE_UTF8_STRING)
		SetWindowTextA(nwh, (char*)title.c_str());
#elif defined(TUIPP_USE_UTF16_STRING)
		SetWindowTextW(nwh, (wchar_t*)title.c_str());
#elif defined(TUIPP_USE_UTF32_STRING)
#error UTF-32 is still in development!
#endif
	}
	inline void ConsoleWindow_ImplNativeWin32::close() {
		bIsOpen = false;
	}
	inline void ConsoleWindow_ImplNativeWin32::display(RenderElement& re) {
		SMALL_RECT rect = { 0, 0, re.viewport.width, re.viewport.height };
		WriteConsoleOutputW(re.hConsole, (PCHAR_INFO)re.screenBuffer, { (short)re.viewport.width, (short)re.viewport.height }, {}, &rect);
	}
	inline bool ConsoleWindow_ImplNativeWin32::initConsoleWindow() {
		using CW = ConsoleWindow_ImplNativeWin32;
		CW::nwh = GetConsoleWindow();
		if (!CW::nwh) {
			AllocConsole();
			CW::nwh = GetConsoleWindow();
			if (!CW::nwh) {
				MessageBoxA(nullptr, "Could not init console window!", "TUIpp *Error* ", MB_ICONERROR | MB_OK);
				return false;
			}
		}
		return true;
	}
#endif

	inline Component::Component(Type cType, RenderElement& re, const String& id) :
		type(cType), re(&re), id(id), bTargeted(false) {

	}

	inline void Component::setSize(const Word& width, const Word& height, bool resizeBuffer) {
		if (resizeBuffer) {
			int reqSize = width * height;
			Int32 size = re->viewport.width * re->viewport.height;
			if (size != reqSize) {
				Component::re->viewport.width = width;
				Component::re->viewport.height = height;
				CharInfo* _newBuffer = new CharInfo[reqSize]{};
				delete re->screenBuffer;
				re->screenBuffer = _newBuffer;
			}
		}
		Component::viewport.width = width;
		Component::viewport.height = height;
	}

	inline void Component::setPosition(const Word& x, const Word& y) {
		Component::viewport.x = x;
		Component::viewport.y = y;
	}


	inline Vec2<Word> Component::getSize() const {
		return Component::viewport.getSize();
	}

	inline Vec2<Word> Component::getPosition() const {
		return Component::viewport.getPosition();
	}

	inline void Component::setID(const String& id) {
		Component::id = id;
	}

	inline String Component::getID() const {
		return Component::id;
	}
	inline Vec2<Word> Component::getDefaultPosition() const {
		return Component::defaultPosition;
	}

	inline Component::Type Component::getType() const {
		return Component::type;
	}

	inline RenderElement& Component::getRenderElement() const {
		return *this->re;
	}
	inline void Component::setComponentType(Type type) {
		this->type = type;
	}

	RenderTarget::RenderTarget(RenderElement* re) : re(*re) {

	}

	RenderTarget::~RenderTarget() {
		if (re.screenBuffer != nullptr) {
			delete re.screenBuffer;
			re.screenBuffer = nullptr;
		}
	}
	inline void RenderTarget::flushTo(RenderTarget* out, Rect<Word> rect) {
		Vec2<Word> p1 = { rect.x, rect.y };
		Vec2<Word> p2 = { Word(rect.x + rect.width), Word(rect.y + rect.height) };
		for (Int16 x = p1.x; x < p2.x; x++) {
			for (Int16 y = p1.y; y < p2.y; y++) {
				Int16 px = (x - p1.x);
				Int16 py = (y - p1.y);

				auto pxl = RenderTarget::getPixelAt(px, py);
				out->setPixel(x, y, pxl.pixel.unicodeChar, pxl.attrib);
			}
		}
	}
	inline void RenderTarget::setPixel(const Word& x, const Word& y, Int16 c, Int16 att) {
		if (re.viewport.contains(x, y)) {
			re.screenBuffer[y * re.viewport.width + x].pixel.unicodeChar = c;
			re.screenBuffer[y * re.viewport.width + x].attrib = att;
		}
	}

	inline void RenderTarget::setPixel(const Vec2<Word>& p, Int16 c, Int16 att) {
		RenderTarget::setPixel(p.x, p.y, c, att);
	}

	inline void RenderTarget::fill(const Word& x1, const Word& y1, const Word& x2, const Word& y2, Int16 c, Int16 att) {
		Int16 nx1 = x1;
		Int16 nx2 = x2;
		Int16 ny1 = y1;
		Int16 ny2 = y2;
		RenderTarget::calcClipOn(nx1, ny1);
		RenderTarget::calcClipOn(nx2, ny2);

		for (Word h = ny1; h < ny2; h++) {
			for (Word w = nx1; w < nx2; w++) {
				RenderTarget::setPixel(w, h, c, att);
			}
		}
	}

	inline void RenderTarget::fill(const Vec2<Word>& p1, const Vec2<Word>& p2, Int16 c, Int16 att) {
		RenderTarget::fill(p1.x, p1.y, p2.x, p2.y, c, att);
	}

	inline void RenderTarget::fill(const Rect<Word>& rect, Int16 c, Int16 att) {
		RenderTarget::fill(rect.getPosition(), rect.getSize(), c, att);
	}

	inline void RenderTarget::renderLine(const Word& _x1, const Word& _y1, const Word& _x2, const Word& _y2, Int16 c, Int16 att) {
		Int32 x, y, dx, dy, dx1, dy1, px, py, xe, ye, i, x1, x2, y1, y2;
		x1 = static_cast<Int16>(_x1);
		x2 = static_cast<Int16>(_x2);
		y1 = static_cast<Int16>(_y1);
		y2 = static_cast<Int16>(_y2);
		dx = x2 - x1; dy = y2 - y1;
		dx1 = abs(dx); dy1 = abs(dy);
		px = 2 * dy1 - dx1; py = 2 * dx1 - dy1;
		if (dy1 <= dx1) {
			if (dx >= 0) {
				x = x1; y = y1; xe = x2;
			}
			else {
				x = x2; y = y2; xe = x1;
			}

			RenderTarget::setPixel(x, y, c, att);

			for (i = 0; x < xe; i++) {
				x = x + 1;
				if (px < 0) {
					px = px + 2 * dy1;
				}
				else {
					if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) y = y + 1; else y = y - 1;
					px = px + 2 * (dy1 - dx1);
				}
				RenderTarget::setPixel(x, y, c, att);
			}
		}
		else {
			if (dy >= 0) {
				x = x1; y = y1; ye = y2;
			}
			else {
				x = x2; y = y2; ye = y1;
			}

			RenderTarget::setPixel(x, y, c, att);

			for (i = 0; y < ye; i++) {
				y = y + 1;
				if (py <= 0)
					py = py + 2 * dx1;
				else {
					if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) x = x + 1; else x = x - 1;
					py = py + 2 * (dx1 - dy1);
				}
				RenderTarget::setPixel(x, y, c, att);
			}
		}
	}

	inline void RenderTarget::renderLine(const Vec2<Word>& p1, const Vec2<Word>& p2, Int16 c, Int16 att) {
		RenderTarget::renderLine(p1.x, p1.y, p2.x, p2.y, c, att);
	}

	inline void RenderTarget::renderLine(const Rect<Word>& p, Int16 c, Int16 att) {
		RenderTarget::renderLine(p.getPosition(), p.getSize(), c, att);
	}

	inline void RenderTarget::renderTriangle(const Word& x1, const Word& y1, const Word& x2, const Word& y2, const Word& x3, const Word& y3, Int16 c, Int16 att) {
		RenderTarget::renderLine(x1, y1, x2, y2);
		RenderTarget::renderLine(x2, y2, x3, y3);
		RenderTarget::renderLine(x3, y3, x1, y1);
	}

	inline void RenderTarget::renderTriangle(const Vec2<Word>& p1, const Vec2<Word>& p2, const Vec2<Word>& p3, Int16 c, Int16 att) {
		RenderTarget::renderTriangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, c, att);
	}

	inline void RenderTarget::rasterizeTriangle(const Word& _x1, const Word& _y1, const Word& _x2, const Word& _y2, const Word& _x3, const Word& _y3, Int16 c, Int16 att) {
		Int16 x1 = _x1;
		Int16 x2 = _x2;
		Int16 x3 = _x3;
		Int16 y1 = _y1;
		Int16 y2 = _y2;
		Int16 y3 = _y3;
		
		auto SWAP = [](int16_t& x, int16_t& y) { int32_t t = x; x = y; y = t; };
		auto SWAP32 = [](int32_t& x, int32_t& y) { int32_t t = x; x = y; y = t; };
		auto drawline = [&](int32_t sx, int32_t ex, int32_t ny) { for (int32_t i = sx; i <= ex; i++) RenderTarget::setPixel(i, ny, c, att); };

		int32_t t1x, t2x, y, minx, maxx, t1xp, t2xp;
		bool changed1 = false;
		bool changed2 = false;
		int32_t signx1, signx2, dx1, dy1, dx2, dy2;
		int32_t e1, e2;
		// Sort vertices
		if (y1 > y2) { SWAP(y1, y2); SWAP(x1, x2); }
		if (y1 > y3) { SWAP(y1, y3); SWAP(x1, x3); }
		if (y2 > y3) { SWAP(y2, y3); SWAP(x2, x3); }

		t1x = t2x = x1; y = y1;   // Starting poINTs
		dx1 = (int32_t)(x2 - x1); if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
		else signx1 = 1;
		dy1 = (int32_t)(y2 - y1);

		dx2 = (int32_t)(x3 - x1); if (dx2 < 0) { dx2 = -dx2; signx2 = -1; }
		else signx2 = 1;
		dy2 = (int32_t)(y3 - y1);

		if (dy1 > dx1) {   // swap values
			SWAP32(dx1, dy1);
			changed1 = true;
		}
		if (dy2 > dx2) {   // swap values
			SWAP32(dy2, dx2);
			changed2 = true;
		}

		e2 = (int32_t)(dx2 >> 1);
		// Flat top, just process the second half
		if (y1 == y2) goto next;
		e1 = (int32_t)(dx1 >> 1);

		for (int32_t i = 0; i < dx1;) {
			t1xp = 0; t2xp = 0;
			if (t1x < t2x) { minx = t1x; maxx = t2x; }
			else { minx = t2x; maxx = t1x; }
			// process first line until y value is about to change
			while (i < dx1) {
				i++;
				e1 += dy1;
				while (e1 >= dx1) {
					e1 -= dx1;
					if (changed1) t1xp = signx1;//t1x += signx1;
					else          goto next1;
				}
				if (changed1) break;
				else t1x += signx1;
			}
			// Move line
		next1:
			// process second line until y value is about to change
			while (1) {
				e2 += dy2;
				while (e2 >= dx2) {
					e2 -= dx2;
					if (changed2) t2xp = signx2;//t2x += signx2;
					else          goto next2;
				}
				if (changed2)     break;
				else              t2x += signx2;
			}
		next2:
			if (minx > t1x) minx = t1x; if (minx > t2x) minx = t2x;
			if (maxx < t1x) maxx = t1x; if (maxx < t2x) maxx = t2x;
			drawline(minx, maxx, y);    // Draw line from min to max poINTs found on the y
			// Now increase y
			if (!changed1) t1x += signx1;
			t1x += t1xp;
			if (!changed2) t2x += signx2;
			t2x += t2xp;
			y += 1;
			if (y == y2) break;

		}
	next:
		// Second half
		dx1 = (int32_t)(x3 - x2); if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
		else signx1 = 1;
		dy1 = (int32_t)(y3 - y2);
		t1x = x2;

		if (dy1 > dx1) {   // swap values
			SWAP32(dy1, dx1);
			changed1 = true;
		}
		else changed1 = false;

		e1 = (int32_t)(dx1 >> 1);

		for (int32_t i = 0; i <= dx1; i++) {
			t1xp = 0; t2xp = 0;
			if (t1x < t2x) { minx = t1x; maxx = t2x; }
			else { minx = t2x; maxx = t1x; }
			// process first line until y value is about to change
			while (i < dx1) {
				e1 += dy1;
				while (e1 >= dx1) {
					e1 -= dx1;
					if (changed1) { t1xp = signx1; break; }//t1x += signx1;
					else          goto next3;
				}
				if (changed1) break;
				else   	   	  t1x += signx1;
				if (i < dx1) i++;
			}
		next3:
			// process second line until y value is about to change
			while (t2x != x3) {
				e2 += dy2;
				while (e2 >= dx2) {
					e2 -= dx2;
					if (changed2) t2xp = signx2;
					else          goto next4;
				}
				if (changed2)     break;
				else              t2x += signx2;
			}
		next4:

			if (minx > t1x) minx = t1x; if (minx > t2x) minx = t2x;
			if (maxx < t1x) maxx = t1x; if (maxx < t2x) maxx = t2x;
			drawline(minx, maxx, y);
			if (!changed1) t1x += signx1;
			t1x += t1xp;
			if (!changed2) t2x += signx2;
			t2x += t2xp;
			y += 1;
			if (y > y3) return;
		}
	}

	inline void RenderTarget::rasterizeTriangle(const Vec2<Word>& p1, const Vec2<Word>& p2, const Vec2<Word>& p3, Int16 c, Int16 att) {
		RenderTarget::rasterizeTriangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, c, att);
	}

	inline void RenderTarget::renderText(const Word& x, const Word& y, const String& txt, Int16 att) {
		for (int i = 0; i < txt.length(); i++) {
			RenderTarget::setPixel(x + i, y, txt[i], att);
		}
	}

	inline void RenderTarget::renderText(const Vec2<Word>& p, const String& txt, Int16 att) {
		RenderTarget::renderText(p.x, p.y, txt, att);
	}

	inline void RenderTarget::clear(Int16 c, Int16 att) {
		RenderTarget::fill(re.viewport, c, att);
	}

	inline void RenderTarget::calcClipOn(Int16& x, Int16& y) {
		if (x < 0) x = 0;
		if (x > RenderTarget::re.viewport.width) x = RenderTarget::re.viewport.width;
		if (y < 0) y = 0;
		if (y > RenderTarget::re.viewport.height) y = RenderTarget::re.viewport.height;
	}

	inline CharInfo RenderTarget::getPixelAt(const Word& x, const Word& y) const {
		if (re.viewport.contains(x, y)) {
			return re.screenBuffer[y * re.viewport.width + x];
		}
		else {
			return {};
		}
	}

	inline CharInfo RenderTarget::getPixelAt(const Vec2<Word>& p) const {
		return getPixelAt(p.x, p.y);
	}

	inline CharInfo* RenderTarget::getPixelScreenBuffer() const {
		return re.screenBuffer;
	}

	inline EventProcessor::EventProcessor(NativeHandle CIN) {
		this->hInput = CIN;
	}
	inline void EventProcessor::processEvents() {
		//Handle KeyBoard Input
		for (INT i = 0; i < 256; i++) {
			m_NewKeyboardCondition[i] = GetAsyncKeyState(i);

			m_KeyboardCondition[i].bStrokePressed = FALSE;
			m_KeyboardCondition[i].bStrokeReleased = FALSE;

			if (m_NewKeyboardCondition[i] != m_OldKeyboardCondition[i]) {
				if (m_NewKeyboardCondition[i] & 0x8000) {
					m_KeyboardCondition[i].bStrokePressed = !m_KeyboardCondition[i].bStrokeIsHeld;
					m_KeyboardCondition[i].bStrokeIsHeld = TRUE;
				}
				else {
					m_KeyboardCondition[i].bStrokeReleased = TRUE;
					m_KeyboardCondition[i].bStrokeIsHeld = FALSE;
				}
			}
			m_OldKeyboardCondition[i] = m_NewKeyboardCondition[i];
		}

		SetConsoleMode(hInput, ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT | ENABLE_EXTENDED_FLAGS);

		//Handle Mouse Input
		INPUT_RECORD inBuf[32];
		DWORD events = 0;
		GetNumberOfConsoleInputEvents(hInput, &events);
		if (events > 0)
			ReadConsoleInput(hInput, (PINPUT_RECORD)&inBuf, events, &events);

		for (DWORD i = 0; i < events; i++) {
			switch (inBuf[i].EventType) {
			case MOUSE_EVENT:
			{
				switch (inBuf[i].Event.MouseEvent.dwEventFlags) {
				case MOUSE_MOVED:
				{
					mousePos.x = inBuf[i].Event.MouseEvent.dwMousePosition.X;
					mousePos.y = inBuf[i].Event.MouseEvent.dwMousePosition.Y;
				}
				break;

				case 0:
				{
					for (int m = 0; m < 5; m++)
						m_NewMouseCondition[m] = (inBuf[i].Event.MouseEvent.dwButtonState & (1 << m)) > 0;
				}
				break;

				default:
					break;
				}
			}
			break;

			default:
				break;

			}
		}
		for (int m = 0; m < 5; m++) {
			m_MouseCondition[m].bStrokePressed = false;
			m_MouseCondition[m].bStrokeReleased = false;

			if (m_NewMouseCondition[m] != m_OldMouseCondition[m]) {
				if (m_NewMouseCondition[m]) {
					m_MouseCondition[m].bStrokePressed = true;
					m_MouseCondition[m].bStrokeIsHeld = true;
				}
				else {
					m_MouseCondition[m].bStrokeReleased = true;
					m_MouseCondition[m].bStrokeIsHeld = false;
				}
			}

			m_OldMouseCondition[m] = m_NewMouseCondition[m];
		}
	}
	inline void EventProcessor::initEventProcessor(NativeHandle hConsoleInput) {
		hInput = hConsoleInput;
	}
	inline EventProcessor::KeyStrokesCondition EventProcessor::keyboard(INT ID) {
		return m_KeyboardCondition[ID];
	}
	inline Vec2<Word> EventProcessor::getMousePos() {
		return mousePos;
	}
	inline EventProcessor::KeyStrokesCondition EventProcessor::mouse(MouseType ID) {
		switch (ID) {
		case MouseType::Left:
			return m_MouseCondition[0];
		case MouseType::Right:
			return m_MouseCondition[1];
		case MouseType::Middle:
			return m_MouseCondition[2];
		default:
			break;
		}
	}

	inline Panel::Panel() : 
		Component(Component::Type::Panel, RenderTarget::re, _T("")) {

	}

	inline Panel::Panel(const String& title, const Word& width, const Word& height) : 
		Component(Component::Type::Panel, RenderTarget::re, title) {
		Panel::create(title, width, height);
	}

	inline Panel::~Panel() {

	}

	

	inline Intrusive<Component> Panel::createInstance(const String& title, const Word& width, const Word& height) {
		auto comp = Intrusive<Panel>::make(title, width, height);
		if (comp->getRenderElement().screenBuffer == nullptr) {
			return nullptr;
		}
		comp->setPosition(1, 1);
		return dynamicPtrCast<Component>(comp);
	}

	inline bool Panel::create(const String& title, const Word& width, const Word& height) {
		Rect<Word> reqViewport = { 0, 0, 512, 512 };
		if (!reqViewport.contains(width, height)) {
			return false;
		}
		else {
			Component::setID(title);

			Component::viewport.width = width;
			Component::viewport.height = height;
			Panel::props.title = title;
			RenderTarget::re.screenBuffer = new CharInfo[width * height]{};
			RenderTarget::re.viewport = { 0, 0, width, height };
			return true;
		}
	}

	bool Panel::insertComponent(const Intrusive<Component>& comp) {
		auto cType = comp->getType();
		if (cType == Component::Panel) {
			components.push_back(comp);
		}
		return true;
	}

	inline void Panel::onUpdate(EventProcessor* ep) {
		
		Vec2<Int16> pPosition = { Int16(Panel::getPosition().x), Int16(Panel::getPosition().y)};
		Vec2<Word> pSize = Panel::getSize();
		Vec2<Int16> pOverallSize = Vec2<Int16>(pPosition.x + pSize.x, pPosition.y + pSize.y);
		Vec2<Int16> mPos = Vec2<Int16>(ep->getMousePos().x, ep->getMousePos().y);

		{
			EventProcessor epInstance = *ep;
			epInstance.mousePos.x = ep->getMousePos().x - pPosition.x;
			epInstance.mousePos.y = ep->getMousePos().y - pPosition.y;
			for (auto& component : Panel::components) {
				component->onUpdate(&epInstance);
			}
		}

		//Check if the mouse is hovering over the panel
		Rect<Int16> rect(pPosition, pOverallSize);
		if (rect.isInsideBounds(mPos)) {
			Panel::mousePosition = {
				Word(mPos.x - pPosition.x),
				Word(mPos.y - pPosition.y)
			};

			Panel::isHovering = true;
		}
		else {
			Panel::isHovering = false;
		}

		// Handle Mouse press events
		if (ep->mouse(EventProcessor::MouseType::Left).bStrokePressed) {
			if (Panel::props.isMovable) {
				rect = {
					pPosition.x,
					Int16(pPosition.y - 1),
					pOverallSize.x,
					Int16(pPosition.y - 1)
				};
				if (rect.isInsideBounds(mPos)) {
					isDragging = true;
					offset = {
						Word(mPos.x - pPosition.x),
						Word(mPos.y - pPosition.y)
					};
				}
			}

			Panel::targeted = Panel::isHovering;

			if (Panel::props.isResizable) {
				//Resizable logic
				const Word resizeMargin = 0;
				//Right edge resizing
				rect = {
					Int16(pOverallSize.x - resizeMargin),
					pPosition.y,
					pOverallSize.x,
					Int16(pOverallSize.y - 1)
				};
				if (rect.isInsideBounds(mPos)) {
					Panel::resizeDragging = true;
					Panel::isResizingRight = true;
					Panel::resizeOffset = {
						Word(mPos.x - pPosition.x),
						pSize.y
					};
				}

				//Bottom edge resizing
				rect = {
					Int16(pPosition.x + 1),
					Int16(pOverallSize.y - resizeMargin),
					Int16(pOverallSize.x - 1),
					Int16(pOverallSize.y)
				};
				if (rect.isInsideBounds(mPos)) {
					Panel::resizeDragging = true;
					Panel::isResizingBottom = true;
					Panel::resizeOffset = {
						Word(mPos.x - pPosition.x),
						pSize.y
					};
				}

				//Left edge resizing 
				rect = {
					Int16(pPosition.x - 1),
					Int16(pPosition.y),
					Int16(pPosition.x + resizeMargin - 1),
					Int16(pOverallSize.y - 1)
				};
				if (rect.isInsideBounds(mPos)) {
					Panel::isResizingLeft = true;
					Panel::resizeDragging = true;
					Panel::resizeOffset = {
						Word(mPos.x - pPosition.x + 1),
						pSize.y
					};
				}

				// Right Bottom
				if (pOverallSize.x == mPos.x && pOverallSize.y == mPos.y) {
					Panel::isResizingRight = Panel::isResizingBottom = Panel::resizeDragging = true;
					Panel::resizeOffset = {
						Word(mPos.x - pPosition.x),
						Word(mPos.y - pPosition.y)
					};
				}

				if (pPosition.x - 1 == mPos.x && pOverallSize.y == mPos.y) {
					Panel::isResizingLeft = Panel::isResizingBottom = Panel::resizeDragging = true;
					Panel::resizeOffset = {
						Word(mPos.x - pPosition.x),
						Word(mPos.y - pPosition.y)
					};
				}
			}
		}

		// handle dragging
		if (ep->mouse(EventProcessor::MouseType::Left).bStrokeIsHeld) {
			if (Panel::isDragging) {
				Panel::setPosition(mPos.x - offset.x, mPos.y - offset.y);
			}
			// handle resizing

			if (Panel::resizeDragging) {
				Int16 minWidth = 0;

				if (Panel::props.titleAlignment == TitleAlignment::Center) {
					minWidth = Panel::props.title.length() * 4;
				}
				else {
					minWidth = Panel::props.title.length() * 2;
				}

				const Int16 minHeight = 1;

				Int16 newX = pPosition.x;
				Int16 newWidth = pSize.x;
				Int16 newHeight = pSize.y;

				if (Panel::isResizingRight) {
					newWidth = mPos.x - pPosition.x;
					if (newWidth < minWidth) {
						newWidth = minWidth;
					}
				}

				// Left-edge resizing
				if (isResizingLeft) {
					Word leftDelta = pPosition.x - mPos.x;
					newWidth = pSize.x + leftDelta;

					if (newWidth >= minWidth) {
						// Adjust position only if the width is valid
						newX = mPos.x;
					}
					else {
						// Prevent width from going below the minimum
						newWidth = minWidth;
						newX = pOverallSize.x - minWidth;
					}
				}

				// Bottom-edge resizing
				if (isResizingBottom) {
					newHeight = mPos.y - pPosition.y;
					if (newHeight < minHeight)
						newHeight = minHeight;
				}

				Panel::setPosition(newX, pPosition.y);
				Panel::setSize(newWidth, newHeight, true);
			}
		}

		// handle mouse release
		if (ep->mouse(EventProcessor::MouseType::Left).bStrokeReleased) {
			Panel::isDragging = false;
			Panel::resizeDragging = false;
			this->isResizingLeft = this->isResizingBottom = this->isResizingRight = false;
		}
	}

	inline void Panel::onRender(RenderTarget* out) {
		Vec2<Word> pPosition = Panel::getPosition();
		Vec2<Word> pSize = Panel::getSize();
		Vec2<Word> pOverallSize = Vec2<Word>(pPosition.x + pSize.x, pPosition.y + pSize.y);

		//RenderTarget::clear(0x2588, 0x00);

		for (auto& component : Panel::components) {
			component->onRender(this);
		}

		RenderTarget::flushTo(out, Panel::viewport);

		if (Panel::targeted) {
			Panel::setUpFrame(out, Panel::viewport, Panel::props.borderColor);
		}
		else {
			Panel::setUpFrame(out, Panel::viewport, 0x08);
		}

		switch (Panel::props.titleAlignment) {
		default:
			break;

		case TitleAlignment::Left:
			out->renderText(pPosition.x + Panel::props.titleAlignmentOffset + (Panel::props.title.length() * 0.25), pPosition.y - 1, Panel::props.title, 0x000F);
			break;
		case TitleAlignment::Center:
			out->renderText(pPosition.x + (pSize.x / 2u) - (Panel::props.title.length() / 2), pPosition.y - 1, Panel::props.title, 0x000F);
			break;
		case TitleAlignment::Right:
			out->renderText(pOverallSize.x + Panel::props.titleAlignmentOffset - (Panel::props.title.length() * 1.25), pPosition.y - 1, Panel::props.title, 0x000F);
			break;
		};
	}

	inline void Panel::onInit() {
		
	}

	inline Vec2<Word> Panel::getMousePos() const {
		return mousePosition;
	}

	inline Panel::Properties& Panel::getProperties() {
		return props;
	}

	inline void Panel::setUpFrame(RenderTarget* out, Rect<Word> rect, Word color) {
		
		out->renderLine(rect.x - 1, rect.y - 1, rect.x + rect.width, rect.y - 1, 0x2500, color);

		out->renderLine(rect.x - 1, rect.y - 1, rect.x - 1, rect.y + rect.height - 1, 0x2502, color);

		out->renderLine(rect.x - 1, rect.y + rect.height, rect.x + rect.width, rect.y + rect.height, 0x2500, color);

		out->renderLine(rect.x + rect.width, rect.y - 1, rect.x + rect.width, rect.y + rect.height - 1, 0x2502, color);

		out->setPixel(rect.x - 1, rect.y - 1, 0x256D, color);
		out->setPixel(rect.x + rect.width, rect.y - 1, 0x256E, color);
		out->setPixel(rect.x - 1, rect.y + rect.height, 0x2570, color);
		out->setPixel(rect.x + rect.width, rect.y + rect.height, 0x256F, color);
	}

	inline U3DViewer::U3DViewer(const String& title, const Word& width, const Word& height) :
		Panel(title, width, height) {

	}

	inline Intrusive<Component> U3DViewer::createInstance(const String& title, const Word& width, const Word& height) {
		auto comp = Intrusive<U3DViewer>::make(title, width, height);
		if (comp->getRenderElement().screenBuffer == nullptr) {
			return nullptr;
		}
		comp->setPosition(1, 1);
		return dynamicPtrCast<Component>(comp);
	}

	inline Int32 compareTris(const void* a, const void* b) {
		U3DViewer::Triangle* t1 = (U3DViewer::Triangle*)a;
		U3DViewer::Triangle* t2 = (U3DViewer::Triangle*)b;

		auto z1 = (t1->p[0].z + t1->p[1].z + t1->p[2].z) / 3.0f;
		auto z2 = (t2->p[0].z + t2->p[1].z + t2->p[2].z) / 3.0f;
		if (z1 < z2) return 1;
		else if (z1 > z2)return -1;
		else return 0;
	}

	inline void U3DViewer::onRender(RenderTarget* out) {
		Panel::onRender(out);

		if (obj != nullptr) {
			Triangles sortTris;
			for (auto& tri : obj->mesh.tris) {
				auto triRotated = tri.rotateX(obj->rotation.x).rotateY(obj->rotation.y).rotateZ(obj->rotation.z);
				Triangle triTranslatedX, triTranslatedXY, triTranslatedXYZ;

				triTranslatedX = triRotated.translateX(obj->position.x);
				triTranslatedXY = triTranslatedX.translateY(obj->position.y);
				triTranslatedXYZ = triTranslatedXY.translateZ(obj->position.z);

				auto normal = triTranslatedXYZ.cross();
				Math::Vec3f diff;

				diff.x = triTranslatedXYZ.p[0].x - 0;
				diff.y = triTranslatedXYZ.p[0].y - 0;
				diff.z = triTranslatedXYZ.p[0].z - 0;
				auto dot = normal.Dot(triTranslatedXYZ.p[0]);
				if (dot < 0.0f) {
					this->light.Normalise();
					CharInfo c = getShadingColor(normal.Dot(light));

					auto triProjected = projectTriangle(triTranslatedXYZ);

					scaleToView(triProjected, U3DViewer::viewport);

					triProjected.c = c.pixel.unicodeChar;
					triProjected.color = c.attrib;

					sortTris.emplace_back(triProjected);
				}
			}

			qsort(sortTris.data(), sortTris.size(), sizeof Triangle, compareTris);

			for (auto& tri : sortTris) {
				RenderTarget::rasterizeTriangle(
					tri.p[0].x, tri.p[0].y,
					tri.p[1].x, tri.p[1].y,
					tri.p[2].x, tri.p[2].y,
					tri.c, tri.color
				);
			}
		}
	}

	inline void U3DViewer::setObject(Object* obj) {
		this->obj = obj;
	}

	inline void U3DViewer::definePerspectiveProjection(float near, float far, float fov, float aspectRatio) {
		float fFovRad = 1.0f / tanf(fov * 0.5f / 180.0f * Math::PI);
		this->proj.m[0][0] = aspectRatio * fFovRad;
		this->proj.m[1][1] = fFovRad;
		this->proj.m[2][2] = far / (far - near);
		this->proj.m[3][2] = (-far * near) / (far - near);
		this->proj.m[2][3] = 1.0f;
		this->proj.m[3][3] = 0.0f;
	}

	inline void U3DViewer::setLightPosition(Math::Vec3f pos) {
		this->light = pos;
	}

	inline U3DViewer::Mesh::Mesh(U3DViewer::Triangles& tris) {
		this->tris = tris;
	}

	inline bool U3DViewer::Mesh::generateTorus(float R, float r, Int32 numU, Int32 numV) {
		this->tris.clear();

		Int32 triangleIndex = 0;
		for (Int32 i = 0; i < numU - 1; i++) {
			float u0 = (float)i / (numU - 1) * 2 * Math::PI;
			float u1 = (float)(i + 1) / (numU - 1) * 2 * Math::PI;

			for (Int32 j = 0; j < numV - 1; j++) {
				float v0 = (float)j / (numV - 1) * 2 * Math::PI;
				float v1 = (float)(j + 1) / (numV - 1) * 2 * Math::PI;

				Math::Vec3f p00 = { (R + r * cos(v0)) * cos(u0), (R + r * cos(v0)) * sin(u0),r * sin(v0) };
				Math::Vec3f p01 = { (R + r * cos(v1)) * cos(u0), (R + r * cos(v1)) * sin(u0),r * sin(v1) };
				Math::Vec3f p10 = { (R + r * cos(v0)) * cos(u1), (R + r * cos(v0)) * sin(u1),r * sin(v0) };
				Math::Vec3f p11 = { (R + r * cos(v1)) * cos(u1), (R + r * cos(v1)) * sin(u1),r * sin(v1) };

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

	inline U3DViewer::Triangle U3DViewer::projectTriangle(Triangle& i) {
		Triangle o{};
		o.c = i.c;
		o.color = i.c;
		Math::MulMatrixTriangle(i.p, o.p, this->proj);
		return o;
	}

	inline void U3DViewer::scaleToView(Triangle& i, Rect<Word> viewport) {
		i.p[0].x += 1.0f; i.p[0].y += 1.0f;
		i.p[1].x += 1.0f; i.p[1].y += 1.0f;
		i.p[2].x += 1.0f; i.p[2].y += 1.0f;
		i.p[0].x *= 0.5f * (float)viewport.width;
		i.p[0].y *= 0.5f * (float)viewport.height;
		i.p[1].x *= 0.5f * (float)viewport.width;
		i.p[1].y *= 0.5f * (float)viewport.height;
		i.p[2].x *= 0.5f * (float)viewport.width;
		i.p[2].y *= 0.5f * (float)viewport.height;
	}

	inline CharInfo U3DViewer::getShadingColor(float lum) {
		short bg_col, fg_col;
		wchar_t sym;
		int pixel_bw = (int)(13.0f * lum);
		switch (pixel_bw) {
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

		CharInfo c;
		c.attrib = bg_col | fg_col;
		c.pixel.unicodeChar = sym;
		return c;
	}

	inline Console::Console(const String& title, const Vec2<Word>& dimension, const Vec2<Word>& pxlDimension, Type type) : 
		interface(nullptr), type(type) {
		if (Console::type == Console::Type::NativeOS) {
#if defined(_WIN32) || defined(_WIN64)
			g_re = &this->re;
			Console::interface = std::make_shared<ConsoleWindow_ImplNativeWin32>(nullptr, title, Vec2<Word>(dimension.x * pxlDimension.x, dimension.y * pxlDimension.y));
			if (Console::interface->isOpen()) {
				Console::hInput = (NativeHandle)GetStdHandle(STD_INPUT_HANDLE);
				Console::hOutput = (NativeHandle)GetStdHandle(STD_OUTPUT_HANDLE);

				SMALL_RECT rect = { 0, 0, 1, 1 };
				SetConsoleWindowInfo(hOutput, TRUE, &rect);

				if (!SetConsoleScreenBufferSize(hOutput, { (short)dimension.x, (short)dimension.y })) {
					MessageBoxA(nullptr, "Couldn't set the console screen buffer size!", "Error", MB_ICONERROR | MB_OK);
				}

				SetConsoleActiveScreenBuffer(hOutput);

				CONSOLE_FONT_INFOEX cfi{};
				cfi.cbSize = sizeof(cfi);
				cfi.nFont = 0;
				cfi.dwFontSize.X = pxlDimension.x;
				cfi.dwFontSize.Y = pxlDimension.y;
				cfi.FontFamily = FF_DONTCARE;
				cfi.FontWeight = FW_NORMAL;

				wcscpy_s(cfi.FaceName, L"Consolas");
				SetCurrentConsoleFontEx(Console::getOutputHandle(), FALSE, &cfi);

				Console::viewport.x = 0;
				Console::viewport.y = 0;
				Console::viewport.width = dimension.x - 1;
				Console::viewport.height = dimension.y - 1;
				if (!SetConsoleWindowInfo(Console::getOutputHandle(), TRUE, reinterpret_cast<PSMALL_RECT>(&viewport))) {
					// we force the size
					Console::getInterface()->setSize(Vec2<Word>(dimension.x * pxlDimension.x, dimension.y * pxlDimension.y));
				}

				SetConsoleMode(Console::getInputHandle(), ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

				Console::re.hConsole = hOutput;
				Console::re.viewport = viewport;
				Console::re.screenBuffer = new CharInfo[viewport.width * viewport.height]{};
				Console::initEventProcessor(hInput);
			}
#else
#error UNIX based system isnt supported at this moment!
#endif
		}
	}
	Console::~Console() {
		
	}

	inline std::shared_ptr<ConsoleInterface> Console::getInterface() {
		return interface;
	}

	inline NativeHandle Console::getInputHandle() {
		return hInput;
	}

	inline NativeHandle Console::getOutputHandle() {
		return hOutput;
	}
	inline bool Console::insertComponent(const Intrusive<Component>& component) {
		components.push_back(component);
		component->onInit();
		return true;
	}
	inline void Console::display() {
		EventProcessor::processEvents();

		for (Int32 i = 0; i < Console::components.size(); i++) {
			auto& comp = Console::components[i];
			comp->onUpdate(this);

			comp->onRender(this);
		}

		if (Console::interface != nullptr) {
			Console::interface->display(re);
		}
	}
	inline bool Console::isOpen() {
		if (Console::interface != nullptr) {
			return Console::interface->isOpen();
		}
		return false;
	}
	inline void Console::close() {
		if (Console::interface != nullptr) {
			Console::interface->close();
		}
	}
	inline Rect<Word> Console::getViewport() const {
		return viewport;
	}
}

#endif
