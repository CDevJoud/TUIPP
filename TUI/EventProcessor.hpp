#pragma once
#include <Windows.h>

class EventProcessor
{
public:
	struct KeyStrokesCondition
	{
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

	COORD m_mousePos;  // Mouse position relative to the console window.
	HANDLE m_handleConsoleInput; // Handle to the console input.
	INPUT_RECORD mapKeys;  // Input record for mapping keys.
	bool ShiftOn; // Flag indicating if the Shift key is pressed.

public:
	/// <summary>
		/// Default constructor for the EventProcessor class.
		/// </summary>
	EventProcessor() = default;

	/// <summary>
	/// Constructor for the EventProcessor class.
	/// </summary>
	/// <param name="consoleInput">Handle to the console input.</param>
	EventProcessor(HANDLE consoleInput);

	enum class MouseType
	{
		Left = 0, Right, Middle
	};

	/// <summary>
	/// Monitors keyboard events associated with the specified ID, such as clicks or interactions.
	/// </summary>
	/// <param name="ID">The unique identifier for the keyboard event being monitored.</param>
	/// <returns>Returns a KeyStrokesCondition indicating the current state of the keyboard event.</returns>
	KeyStrokesCondition Keyboard(INT ID);

	/// <summary>
	/// Retrieves the mouse position relative to the console window.
	/// </summary>
	/// <returns>A Vector2i representing the mouse coordinates.</returns>
	COORD GetMousePos();

	/// <summary>
	/// Checks for mouse click events.
	/// </summary>
	/// <param name="ID">The type of mouse button to check.</param>
	/// <returns>Returns a KeyStrokesCondition indicating the state of the mouse event.</returns>
	KeyStrokesCondition Mouse(MouseType ID);

	/// <summary>
	/// Processes console input events.
	/// </summary>
	VOID ProcessEvents();

	wchar_t GetText();

	void InitEventProcessor(HANDLE hConsoleInput);
};

