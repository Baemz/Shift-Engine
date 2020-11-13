#pragma once

namespace Shift
{
	class SN_Socket
	{
	public:
		SN_Socket();
		virtual ~SN_Socket();

		virtual void Init() = 0;

	protected:

	};

}