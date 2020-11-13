#pragma once
#include "STools_EditorSubModule.h"

namespace Shift
{
	class SC_Console;
	class STools_LogWindow;
	class STools_ConsoleWindow : public STools_EditorSubModule
	{
		STOOLS_DECLARESUBMODULE(STools_TextureViewer);
	public:
		STools_ConsoleWindow(STools_LogWindow* aLogWindow);
		~STools_ConsoleWindow();

		void PreRender() override;
		void Render() override;

		const char* GetWindowName() const override { return "Console"; }
	private:

		static constexpr uint ourMaxInputLength = 512;

		char myInput[ourMaxInputLength];
		SC_Console* myConsole;
		STools_LogWindow* myLogWindow;
		bool myNeedsRefresh : 1;
	};
}
