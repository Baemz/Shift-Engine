#include "STools_Precompiled.h"
#include "STools_RegisterWindows.h"

#include "STools_LogWindow.h"
#include "STools_AssetRepository.h"
#include "STools_AssetImporter.h"
#include "STools_Statistics.h"
#include "STools_DynamicSky.h"
#include "STools_GraphicsSettings.h"
#include "STools_GraphicsWorld.h"
#include "STools_LevelEditor.h"
#include "STools_MaterialEditor.h"
#include "STools_TextureViewer.h"

namespace Shift
{
	void STools_RegisterWindows()
	{
		STools_Editor* editor = STools_Editor::GetInstance();

		editor->RegisterSubModule<STools_LogWindow>();
		editor->RegisterSubModule<STools_AssetRepository>();
		editor->RegisterSubModule<STools_AssetImporter>();
		editor->RegisterSubModule<STools_Statistics>();
		editor->RegisterSubModule<STools_DynamicSky>();
		editor->RegisterSubModule<STools_GraphicsSettings>();
		editor->RegisterSubModule<STools_GraphicsWorld>();
		//editor->RegisterSubModule<STools_LevelEditor>();
		editor->RegisterSubModule<STools_MaterialEditor>();
		editor->RegisterSubModule<STools_TextureViewer>();
	}
}
