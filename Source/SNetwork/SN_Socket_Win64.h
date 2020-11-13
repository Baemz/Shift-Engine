#pragma once
#include "SN_Socket.h"

#if IS_WINDOWS
namespace Shift
{
	class SN_Socket_Win64 final : public SN_Socket
	{
	public:
		SN_Socket_Win64();
		virtual ~SN_Socket_Win64();

		void Init() override;

	private:
	};
}
#endif