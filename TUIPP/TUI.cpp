//#include <windows.h>
//#include <iostream>
//#include <sstream>
//
//// Custom stream buffer to capture std::cout output
//class TextBoxStream : public std::wstreambuf {
//private:
//	HWND hEdit; // Handle to the edit control
//	std::wostringstream buffer;
//
//public:
//	TextBoxStream(HWND hEdit) : hEdit(hEdit) {}
//
//protected:
//	virtual std::wstreambuf::int_type overflow(std::wstreambuf::int_type c) override {
//		if (c == '\n' || c == EOF || c == 0) {
//			// Append to the edit control
//			std::wstring content = buffer.str();
//			buffer.str(L""); // Clear buffer
//
//			int len = GetWindowTextLength(hEdit);
//			SendMessage(hEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
//			SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)content.c_str());
//		}
//		else {
//			buffer.put(c);
//		}
//		return c;
//	}
//};
//
//LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
//	static HWND hEdit;
//	static TextBoxStream* tbStream = nullptr;
//
//	switch (uMsg) {
//	case WM_CREATE:
//		// Create an edit control
//		hEdit = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE,
//			10, 10, 400, 300, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
//
//		// Redirect std::cout
//		tbStream = new TextBoxStream(hEdit);
//		std::wcout.rdbuf(tbStream);
//		std::wcout << "Redirected output will appear here.sds\n";
//		std::wcout << "#\n";
//		break;
//
//	case WM_DESTROY:
//		PostQuitMessage(0);
//		break;
//	}
//	
//	return DefWindowProc(hwnd, uMsg, wParam, lParam);
//}
//
//int main() {
//	// Register the window class
//	WNDCLASS wc = { 0 };
//	wc.lpfnWndProc = WindowProc;
//	wc.hInstance = GetModuleHandle(NULL);
//	wc.lpszClassName = L"OutputWindow";
//	RegisterClass(&wc);
//
//	// Create the window
//	HWND hwnd = CreateWindow(L"OutputWindow", L"Output Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
//		CW_USEDEFAULT, CW_USEDEFAULT, 450, 350, NULL, NULL, GetModuleHandle(NULL), NULL);
//
//	// Message loop
//	MSG msg = { 0 };
//	while (GetMessage(&msg, NULL, 0, 0)) {
//		TranslateMessage(&msg);
//		DispatchMessage(&msg);
//		std::wcout << "#\n";
//	}
//
//	return 0;
//}
//
//
//#include "Console.hpp"
//#include "InputBox.hpp"
//
//class Application : public Console, public TUIStartup {
//public:
//	Application() : Console("App", 240, 64, 8, 16) {
//		Console::InsertComponent(Panel::CreateInstance("main", 120, 32, {
//			{
//				"OnInit", std::function<void(Panel&)>([&](Panel& main) {
//					auto in = InputBox::CreateInstance("Enter your name", 40, 1);
//					main.InsertComponent(in);
//					in->SetTitleAlignment(InputBox::TitleAlignment::Right);
//					in->SetPosition(main.GetSize().X / 2 - in->GetSize().X / 2, main.GetSize().Y * 0.25);
//					in->SetResizability(true);
//					in->SetMovability(true);
//					in->SetInputType(InputBox::Type::Password);
//
//					auto btn = Button::CreateInstance("send", Button::Default, 1, {
//						{
//							"OnClicked", std::function<void(EventProcessor::MouseType)>([&](EventProcessor::MouseType mType) {
//								if (mType == EventProcessor::MouseType::Left) {
//									this->username = main.GetComponent<InputBox>("Enter your name")->value.str;
//								}
//							})
//						}
//						});
//					main.InsertComponent(btn);
//
//					btn->SetPosition(main.GetSize().X / 2 - btn->GetSize().X / 2, btn->GetPosition().Y);
//				})
//			},{
//				"OnRender", std::function<void(Panel&)>([&](Panel& main) {
//					main.ClearScreen(0x2591, 0x1f);
//					if (!this->username.empty()) {
//						std::string greeting = "Hello " + username;
//						main.RenderText((main.GetSize().X / 2) - (greeting.length() / 2), main.GetSize().Y / 2, greeting, 0x0f);
//					}
//				})
//			},{
//				"OnResize", std::function<void(Panel&)>([&](Panel& main) {
//					auto& in = *main.GetComponent<InputBox>("Enter your name");
//					in.SetPosition(main.GetSize().X / 2 - in.GetSize().X / 2, main.GetSize().Y * 0.25);
//					auto& btn = *main.GetComponent<Button>("send");
//					btn.SetPosition(main.GetSize().X / 2 - btn.GetSize().X / 2, main.GetSize().Y * 0.25 + 2);
//				})
//			}
//			}));
//	}
//	int main() {
//		while (Console::Window::IsOpen()) {
//			if (Console::EventProcessor::Keyboard(VK_ESCAPE).bStrokeReleased) {
//				Console::Window::Close();
//			}
//
//			Console::RenderTarget::ClearScreen();
//			
//			Console::Display();
//		}
//		return 0;
//	}
//private:
//	std::string username;
//};
//
//TUIWIN32_STARTUP(Application);


#include "Console.hpp"
#include "U3DViewer.hpp"

class Application : public virtual Console, public TUIStartup {
public:
	Application() : Console("App", 240, 64, 8, 16)
	{
		Console::InsertComponent(U3DViewer::CreateInstance("Main", 120, 32, {
			{
				"OnInit", Panel::FD::OnInit([&](Panel& Main) {
					auto& instance = *Console::GetComponent<U3DViewer>("Main");
					instance.DefinePerspectiveProjection(0.1f, 1000.0f, 15.0f, (float)instance.GetSize().Y * 2 / instance.GetSize().X);
					obj.mesh.GenerateTorus(2, 1, 20, 20);
					obj.position.z = 30;
					instance.SetObject(&obj);
					Main.InsertComponent(Button::CreateInstance("Move Up", Button::Default, 1, {
						{
							"OnClicked", Button::FD::OnClicked([&](Button& btn, EventProcessor::MouseType mType) {
								if (mType == EventProcessor::MouseType::Left) {
									obj.position.y -= 0.5f;
								}
							})
						}
						}));
					Main.InsertComponent(Button::CreateInstance("Move Down", Button::Default, 1, {
						{
							"OnClicked", Button::FD::OnClicked([&](Button& btn, EventProcessor::MouseType mType) {
								if (mType == EventProcessor::MouseType::Left) {
									obj.position.y += 0.5f;
								}
							})
						}
						}));
					})
			},
			{
				"OnRender", Panel::FD::OnRender([&](Panel& Main) {
					auto& instance = *Console::GetComponent<U3DViewer>("Main");
					obj.rotation.y += 0.005f;
					instance.SetLightPosition({ 1, 1, 0 });
					Main.ClearScreen();
				})
			},
			{
				"OnResize", Panel::FD::OnResize([&](Panel& Main) {
					auto& instance = *Console::GetComponent<U3DViewer>("Main");
					instance.DefinePerspectiveProjection(0.1f, 1000.0f, 15.0f, (float)instance.GetSize().Y * 2 / instance.GetSize().X);
				})
			}
			}));
	}
	~Application()
	{

	}
	virtual int main() override
	{
		while (Console::IsOpen()) {
			if (Console::Keyboard(VK_ESCAPE).bStrokeReleased)
				Console::Close();

			Console::ClearScreen();

			Console::Display();
		}
		return 0;
	}
private:
	U3DViewer::Object obj;
};

TUIWIN32_STARTUP(Application)