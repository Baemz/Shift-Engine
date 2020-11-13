#pragma once
#include "STools_EditorSubModule.h"

namespace Shift
{
	class STools_LogWindow final : public STools_EditorSubModule
	{
		STOOLS_DECLARESUBMODULE(STools_LogWindow);
	public:
		STools_LogWindow();
		~STools_LogWindow();

		void Update() override;
		void Render() override;

		const char* GetWindowName() const override { return "Log"; }

		const SC_HybridArray<SC_Pair<SC_LogType, std::string>, 128>& GetSavedMessages() const { return mySavedLogMessages; }

	private:
		void TryOpenLogFile();
		void OnFileChanged(const std::string& aFile, const EFileEvent&);

		SC_HybridArray<SC_Pair<SC_LogType, std::string>, 128> mySavedLogMessages;
		std::ifstream myLogFile;
	};

}