#pragma once

#if IS_WINDOWS
#include <winsock2.h>
#define SN_SOCKET_INIT(aVersion, wsaData) WSAStartup(aVersion, wsaData)
#define SN_SOCKET_CLEANUP() WSACleanup()

#elif IS_LINUX

#define SN_SOCKET_INIT(aVersion, wsaData) __noop
#define SN_SOCKET_CLEANUP() __noop

#endif