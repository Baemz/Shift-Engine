#pragma once
#include "SC_Module.h"
#include "STools_EditorShelf.h"
#include "STools_TransformGizmo.h"

class SClient_Base;
namespace Shift
{
	class SG_Camera;
	class STools_EditorSubModule;
	class STools_ConsoleWindow;
	class STools_LogWindow;
	class STools_Editor : public SC_Module
	{
		SC_MODULE_DECL(STools_Editor);

	public:
		STools_Editor();
		~STools_Editor();

		bool Init();

		void Update() override;
		void Render() override;

		void GameExitHook();
		void AddClientHook(SClient_Base* aTargetClient);

		template<class T>
		void RegisterSubModule();

		void UnregisterTool(STools_EditorSubModule* aTool);

		static STools_Editor* GetInstance() { return ourInstance; }

		SClient_Base* GetClient() const { return myTargetClientHook; }
		SG_Camera* GetCamera() const { return myCamera; }
		SC_Vector2f GetViewportCursorPos() const { return myLastViewportCursorPos; }

	private:
		static STools_Editor* ourInstance;

		struct SubModuleInfo
		{
			std::string myName;
			std::function<void()> myActivateFunc;

			bool operator==(const SubModuleInfo& aOther) { return myName == aOther.myName; }
		};
		SC_GrowingArray<SubModuleInfo> mySubModuleActivateFuncs;
		SC_GrowingArray<SC_Ref<STools_EditorSubModule>> mySubModules;
		SC_GrowingArray<STools_EditorSubModule*> myRemovedTools;

		STools_EditorShelf myShelf;

		//STools_TransformGizmo myGizmo;

		SC_Vector2f myLastViewportCursorPos;
		SClient_Base* myTargetClientHook;
		SG_Camera* myCamera;
		SC_Ref<STools_LogWindow> myLogWindow;
		SC_Ref<STools_ConsoleWindow> myConsole;

		bool myIsConsoleOpen : 1;
	};

	template<class T>
	inline void STools_Editor::RegisterSubModule()
	{
		auto activateModule = [&]()
		{
			mySubModules.Add(new T());
		};

		SubModuleInfo info;
		SC_UniquePtr<T> temp = new T();
		info.myName = temp->GetWindowName();
		info.myActivateFunc = activateModule;
		mySubModuleActivateFuncs.AddUnique(info);
	}
}