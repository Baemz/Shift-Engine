#include "SCore_Precompiled.h"
#include "SC_MessageHandler.h"

#include "SC_MessageReciever.h"

namespace Shift
{
	SC_MessageHandler::SC_MessageHandler()
	{
	}


	SC_MessageHandler::~SC_MessageHandler()
	{
	}

	void SC_MessageHandler::Subscribe(SC_MessageReceiver* aReceiver)
	{
		mySubscribedReceivers.AddUnique(aReceiver);
	}

	void SC_MessageHandler::Unsubscribe(SC_MessageReceiver* aReceiver)
	{
		mySubscribedReceivers.RemoveCyclic(aReceiver);
	}
	void SC_MessageHandler::PostMessage(const SC_MessageType& aType, const SC_Message& aMsg)
	{
		for (auto& receiver : mySubscribedReceivers)
		{
			receiver->ReceiveMessage(aType, aMsg);
		}
	}
}