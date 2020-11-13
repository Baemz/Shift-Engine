#pragma once
#include "SC_InputStructs.h"

namespace Shift
{
	class SC_InputReceiver;
	class SC_InputManager
	{
	public:
		SC_InputManager();
		~SC_InputManager();

		void HandleMessage(uint aMsg);

		// SR_0
		static bool IsKeyDown(const SC_KeyCodes& aKey);
		bool WasKeyPressed(const SC_KeyCodes& aKey);
		static bool WasKeyReleased(const SC_KeyCodes& aKey);

		bool IsMouseDown(const SC_MouseKeyCodes& aKey);
		bool WasMousePressed(const SC_MouseKeyCodes& aKey);
		bool WasMouseReleased(const SC_MouseKeyCodes& aKey);

		float GetScrollDelta() const;
		SC_Vector2i GetMousePos() const;

	private:
		bool myKeys[SC_Key_COUNT];
		bool myMouseKeys[SC_Key_COUNT];

		SC_Vector2i myMouseScreenPos;
		float myScrollDelta;
	};

}