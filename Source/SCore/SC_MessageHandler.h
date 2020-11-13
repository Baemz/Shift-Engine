#pragma once
#include "SC_Message.h"

namespace Shift
{
	class SC_MessageReceiver;
	class SC_MessageHandler
	{
	public:
		SC_MessageHandler();
		~SC_MessageHandler();

		void Subscribe(SC_MessageReceiver* aReceiver);
		void Unsubscribe(SC_MessageReceiver* aReceiver);

		void PostMessage(const SC_MessageType& aType, const SC_Message& aMsg);

	private:
		SC_GrowingArray<SC_MessageReceiver*> mySubscribedReceivers;
	};

}