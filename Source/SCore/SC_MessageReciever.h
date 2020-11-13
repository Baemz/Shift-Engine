#pragma once
#include "SC_Message.h"

namespace Shift
{
	class SC_MessageReceiver
	{
	public:
		virtual ~SC_MessageReceiver() {}
		virtual bool ReceiveMessage(const SC_MessageType& aType, const SC_Message& aMsg) = 0;
	};

}