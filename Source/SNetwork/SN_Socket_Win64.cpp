#include "SNetwork_Precompiled.h"
#include "SN_Socket_Win64.h"
#include "SN_NetworkDefines.h"

#if IS_WINDOWS
namespace Shift
{
	SN_Socket_Win64::SN_Socket_Win64()
	{
	}
	SN_Socket_Win64::~SN_Socket_Win64()
	{
	}
	void SN_Socket_Win64::Init()
	{

		WORD winsock_version = 0x202;
		WSADATA winsock_data;
		if (SN_SOCKET_INIT(winsock_version, &winsock_data))
		{
			SC_ERROR_LOG("WSAStartup failed: %d", WSAGetLastError());
			return;
		}

		int address_family = AF_INET;
		int type = SOCK_DGRAM;
		int protocol = IPPROTO_UDP;
		SOCKET sock = socket(address_family, type, protocol);

		if (sock == INVALID_SOCKET)
		{
			SC_ERROR_LOG("Creating socket failed: %d", WSAGetLastError());
			return;
		}

		SOCKADDR_IN local_address;
		local_address.sin_family = AF_INET;
		local_address.sin_port = htons(3791);
		local_address.sin_addr.s_addr = INADDR_ANY;
		if (bind(sock, (SOCKADDR*)&local_address, sizeof(local_address)) == SOCKET_ERROR)
		{
			SC_ERROR_LOG("Binding socket failed: %d", WSAGetLastError());
			return;
		}

	}
}
#endif