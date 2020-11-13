#pragma once

namespace Shift
{
	enum SC_MessageType
	{
		SC_MessageType_Key,
		SC_MessageType_Mouse,
		SC_MessageType_Callback,
		SC_MessageType_Window,
	};

	struct SC_Message
	{
		SC_MessageType myType;
	};
}