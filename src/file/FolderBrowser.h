#pragma once

#include "AssetManager.h"

namespace Ainan {

	class FolderBrowser
	{
	public:
		FolderBrowser(const std::string& startingFolder, const std::string& windowName = "Folder Browser");
		FolderBrowser();

		void DisplayGUI(std::function<void(const std::string&)> onSelect = nullptr);

		std::string GetChosenFolderPath() { return m_CurrentFolder; }

	public:
		bool WindowOpen = false;

	private:
		std::string m_CurrentFolder = "not selected";
		std::string m_WindowName;
		std::string m_InputFolder;
	};
}