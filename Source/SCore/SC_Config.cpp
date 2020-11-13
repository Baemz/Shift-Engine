#include "SCore_Precompiled.h"
#include "SC_Config.h"

#include <fstream>

namespace Shift
{
	struct SC_ConfigLayout
	{
		SC_ConfigLayout()
			: myWindowWidth(1280.f)
			, myWindowHeight(720.f)
			, myIsFullscreen(false)
			, myGraphicsAPI(SR_GraphicsAPI::DirectX12)
			, myApplicationName("ShiftEngine")
		{}

		void operator=(const SC_ConfigLayout& aOther)
		{
			myApplicationName = aOther.myApplicationName;
			myWindowWidth = aOther.myWindowWidth;
			myWindowHeight = aOther.myWindowHeight;
			myIsFullscreen = aOther.myIsFullscreen;
			myGraphicsAPI = aOther.myGraphicsAPI;
		}

		std::string myApplicationName;
		float myWindowWidth;
		float myWindowHeight;
		bool myIsFullscreen;
		SR_GraphicsAPI myGraphicsAPI;
	};

	static const char* configFilePath = "settings.secfg"; // TODO: Add documents directory as path.
	SC_ConfigLayout* SC_Config::ourConfig = nullptr;

	std::string GetStringFromAPI(const SR_GraphicsAPI& aAPI)
	{
		switch (aAPI)
		{
		default:
		case SR_GraphicsAPI::DirectX12:
			return "DX12";
		case SR_GraphicsAPI::Vulkan:
			return "Vulkan";
		case SR_GraphicsAPI::PS4:
			return "PS4";
		}
	}
	
	SR_GraphicsAPI GetAPIFromString(const std::string& aAPI)
	{
		if (aAPI == "DX12")
			return SR_GraphicsAPI::DirectX12;
		else if (aAPI == "Vulkan")
			return SR_GraphicsAPI::Vulkan;
		else if (aAPI == "PS4")
			return SR_GraphicsAPI::PS4;

		return SR_GraphicsAPI::DirectX12;
	}

	void SC_Config::SetWindowWidth(const float aWidth)
	{
		aWidth;
		Save();
	}

	void SC_Config::SetWindowHeight(const float aHeight)
	{
		aHeight;
		Save();
	}
	const std::string& SC_Config::GetAppName()
	{
		return ourConfig->myApplicationName;
	}
	const float SC_Config::GetWindowWidth()
	{
		return ourConfig->myWindowWidth;
	}

	const float SC_Config::GetWindowHeight()
	{
		return ourConfig->myWindowHeight;
	}

	const bool SC_Config::IsFullscreen()
	{
		return ourConfig->myIsFullscreen;
	}

	const SR_GraphicsAPI& SC_Config::GetGraphicsAPI()
	{
		return ourConfig->myGraphicsAPI;
	}

	void SC_Config::Load()
	{
		if (ourConfig)
			return;

		ourConfig = new SC_ConfigLayout();

		std::ifstream in(configFilePath);
		if (in.fail())
		{
			Save();
			return;
		}
		SC_Json file;
		in >> file; 
		in.close();

		ourConfig->myApplicationName	= file["Name"];
		ourConfig->myWindowWidth		= file["WindowWidth"];
		ourConfig->myWindowHeight		= file["WindowHeight"];
		ourConfig->myIsFullscreen		= file["Fullscreen"];
		ourConfig->myGraphicsAPI		= GetAPIFromString(file["GraphicsAPI"]);
	}

	void SC_Config::Save()
	{
		std::ofstream out(configFilePath);

		if (out.good())
		{
			SC_Json file;
			file["Name"]			= ourConfig->myApplicationName;
			file["WindowWidth"]		= ourConfig->myWindowWidth;
			file["WindowHeight"]	= ourConfig->myWindowHeight;
			file["Fullscreen"]		= ourConfig->myIsFullscreen;
			file["GraphicsAPI"]		= GetStringFromAPI(ourConfig->myGraphicsAPI);
			out << file << std::endl;
		}

		out.close();
	}
}