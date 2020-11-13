#pragma once

namespace Shift
{
	class SC_Window
	{
	public:
		static SC_Window* Create(const char* aAppName, const SC_Vector2f& aResolution);

		SC_Window();
		virtual ~SC_Window();
		virtual bool Init() = 0;

		// SR_0 added parameter
		virtual bool HandleMessages(class SC_InputManager* aInputManager) = 0;

		const SC_Vector2f& GetResolution() const;
		bool IsFullscreen() const;
		const char* GetAppName() const { return myAppName.c_str(); }

	protected:
		std::string myAppName;
		SC_Vector2f myResolution;
		bool myIsFullscreen : 1;

	};

}