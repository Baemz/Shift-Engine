#include "SCore_Precompiled.h"
#include "SC_InputManager.h"
#include "SC_Window.h"

namespace Shift
{
	SC_InputManager::SC_InputManager()
	{
	}

	SC_InputManager::~SC_InputManager()
	{
	}

	void SC_InputManager::HandleMessage(uint aMsg)
	{
		switch (aMsg)
		{
		case WM_QUIT:
			break;
		}
	}


	// SR_0
	bool SC_InputManager::IsKeyDown(const SC_KeyCodes& aKey)
	{
		if (GetKeyState(aKey) & 0x8000)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	//bool SC_InputManager::WasKeyPressed(const SC_KeyCodes& aKey)
	//{
	//}
	//bool SC_InputManager::WasKeyReleased(const SC_KeyCodes& aKey)
	//{
	//	if (!GetKeyState(aKey) & 0x8000)
	//	{
	//		return true;
	//	}
	//	else
	//	{
	//		return false;
	//	}
	//}
	//
	//bool SC_InputManager::IsMouseDown(const SC_MouseKeyCodes& aKey)
	//{
	//	return false;
	//}
	//bool SC_InputManager::WasMousePressed(const SC_MouseKeyCodes& aKey)
	//{
	//	return false;
	//}
	//bool SC_InputManager::WasMouseReleased(const SC_MouseKeyCodes& aKey)
	//{
	//	return false;
	//}
	//
	//
	//
	//float SC_InputManager::GetScrollDelta() const
	//{
	//	return 0.0f;
	//}
	//SC_Vector2i SC_InputManager::GetMousePos() const
	//{
	//	return SC_Vector2i();
	//}
}