#pragma once

// SME::INI - A windows API GUI used to enumerate and modify INI keys.
// Primarily used as a companion to the SME::INI::INIManager/INISetting.

namespace SME
{
	namespace INI
	{
		class INIManager;
		class INIManagerIterator;

		BOOL CALLBACK						INIEditGUIDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		BOOL CALLBACK						TextEditDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		class INIEditGUI
		{
			bool							CreateListView(HWND ParentHandle);
			bool							PopulateListView(HWND ListViewHandle);
			void							SaveSettings(HWND ListViewHandle);

			friend BOOL CALLBACK			INIEditGUIDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		public:
			INIManager*						ParentManager;
			HINSTANCE						ParentInstance;
			HWND							ListViewHandle;

			void							InitializeGUI(HINSTANCE ParentInstance, HWND ParentHWND, INIManager* ParentINIManager);
		};
	}
}