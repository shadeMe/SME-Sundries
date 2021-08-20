// SME::INI - A windows API GUI used to enumerate and modify INI keys.
// Primarily used as a companion to the SME::INI::INIManager/INISetting.

#ifndef __SME_SUNDRIES_INIEDITGUI_H__
#define __SME_SUNDRIES_INIEDITGUI_H__

#include "INIManager.h"
#include "INIEditGUI_Res.h"
#include "UIHelpers.h"

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
			INIManager*						ParentManager;
			HINSTANCE						ParentInstance;
			HWND							ListViewHandle;

			RECT							InitDialogBounds;
			RECT							InitListViewBounds;
			bool							HasChanges;

			bool							CreateListView(HWND ParentHandle);
			bool							PopulateListView(HWND ListViewHandle);
			void							SaveSettings(HWND ListViewHandle);

			friend BOOL CALLBACK			INIEditGUIDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		public:
			void							InitializeGUI(HINSTANCE ParentInstance, HWND ParentHWND, INIManager* ParentINIManager);
		};

		inline void INIEditGUI::InitializeGUI(HINSTANCE ParentInstance, HWND ParentHWND, INIManager* ParentINIManager)
		{
			this->ParentManager = ParentINIManager;
			this->ParentInstance = ParentInstance;

			DialogBoxParam(ParentInstance, MAKEINTRESOURCE(DLG_INIGUI), ParentHWND, INIEditGUIDlgProc, (LPARAM)this);
		}

		inline bool INIEditGUI::CreateListView(HWND ParentHandle)
		{
			ListViewHandle = GetDlgItem(ParentHandle, LV_INILIST);

			char Buffer[256];

			for (int i = 0; i < 4; i++)
			{
				LVCOLUMN lvc;

				lvc.mask =  LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT | LVCF_WIDTH;
				lvc.fmt = LVCFMT_LEFT;
				lvc.iSubItem = i;
				lvc.pszText = (LPSTR)Buffer;

				switch (i)
				{
				case 0:
					_snprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, "Section");
					lvc.cx = 200;
					break;
				case 1:
					_snprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, "Setting");
					lvc.cx = 200;
					break;
				case 2:
					_snprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, "Value");
					lvc.cx = 120;
					break;
				case 3:
					_snprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, "Description");
					lvc.cx = 400;
					break;
				}

				if (ListView_InsertColumn(ListViewHandle, i, &lvc) == -1)
					return false;
			}

			return true;
		}

		inline bool INIEditGUI::PopulateListView(HWND ListViewHandle)
		{
			UInt32 Index = 0;
			char Buffer[0x200] = {0};

			for (INIManagerIterator Itr(ParentManager); Itr.GetDone() == 0; Itr.GetNextSetting())
			{
				const INISetting* Setting = Itr.GetCurrentSetting();

				LVITEM Section, Name, Value, Desc;

				Section.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
				Section.iSubItem = 0;
				Section.state = 0;
				Section.stateMask = 0;
				Section.pszText = (LPSTR)Setting->GetSection();
				Section.iItem = Index;
				Section.lParam = (LPARAM)Setting;

				Name.mask = LVIF_TEXT | LVIF_STATE;
				Name.iSubItem = 1;
				Name.state = 0;
				Name.stateMask = 0;
				Name.pszText = (LPSTR)Setting->GetKey();
				Name.iItem = Index;

				Value.mask = LVIF_TEXT | LVIF_STATE;
				Value.iSubItem = 2;
				Value.state = 0;
				Value.stateMask = 0;
				Value.pszText = (LPSTR)Buffer;
				Value.iItem = Index;

				Desc.mask = LVIF_TEXT | LVIF_STATE;
				Desc.iSubItem = 3;
				Desc.state = 0;
				Desc.stateMask = 0;
				Desc.pszText = (LPSTR)Setting->GetDescription();
				Desc.iItem = Index;

				Setting->GetDataAsString(Buffer, sizeof(Buffer));

				if (ListView_InsertItem(ListViewHandle, &Section) == -1 ||
					ListView_SetItem(ListViewHandle, &Name) == FALSE ||
					ListView_SetItem(ListViewHandle, &Value) == FALSE ||
					ListView_SetItem(ListViewHandle, &Desc) == FALSE)
				{
					return false;
				}

				Index++;
			}

			return true;
		}

		inline void INIEditGUI::SaveSettings(HWND ListViewHandle)
		{
			UInt32 ItemCount = ListView_GetItemCount(ListViewHandle);
			char Buffer[0x200] = {0};

			for (int i = 0; i < ItemCount; i++)
			{
				LVITEM Item;
				Item.mask = LVIF_PARAM;
				Item.iSubItem = 0;
				Item.state = 0;
				Item.stateMask = 0;
				Item.iItem = i;
				ListView_GetItem(ListViewHandle, &Item);

				INISetting* Setting = (INISetting*)Item.lParam;
				ListView_GetItemText(ListViewHandle, i, 2, Buffer, sizeof(Buffer));

				Setting->SetDataAsString(Buffer);
			}
		}

		inline BOOL CALLBACK INIEditGUIDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			INIEditGUI* GUIInstance = (INIEditGUI*)GetWindowLongPtr(hWnd, GWL_USERDATA);

			switch (uMsg)
			{
			case WM_SIZE:
				{
					RECT CurrentRect = {0};
					RECT ButtonRect = {0};

					HWND ListView = GetDlgItem(hWnd, LV_INILIST);

					SetRect(&CurrentRect, 0, 0, LOWORD(lParam), HIWORD(lParam));
					int DeltaDlgWidth = (CurrentRect.right - GUIInstance->InitDialogBounds.right);
					int DeltaDlgHeight = (CurrentRect.bottom - GUIInstance->InitDialogBounds.bottom);
					int VerticalScrollWidth = GetSystemMetrics(SM_CXVSCROLL) + 4;
					HDWP DeferPosData = BeginDeferWindowPos(1);

					DeferWindowPos(DeferPosData, ListView, NULL,
						GUIInstance->InitListViewBounds.left,
						GUIInstance->InitListViewBounds.top,
						DeltaDlgWidth + GUIInstance->InitListViewBounds.right + VerticalScrollWidth,
						CurrentRect.bottom + GUIInstance->InitListViewBounds.bottom - GUIInstance->InitDialogBounds.bottom,
						NULL);

					EndDeferWindowPos(DeferPosData);
				}

				break;
			case WM_CLOSE:
				{
					if (GUIInstance->HasChanges)
					{
						switch (MessageBox(hWnd, "Do you want to save your changes?", "INI Manager GUI", MB_ICONQUESTION|MB_YESNOCANCEL))
						{
						case IDYES:
							GUIInstance->SaveSettings(GetDlgItem(hWnd, LV_INILIST));
							EndDialog(hWnd, NULL);

							break;
						case IDNO:
							EndDialog(hWnd, NULL);

							break;
						default:
							break;
						}
					}
					else
						EndDialog(hWnd, NULL);

					return TRUE;
				}
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case BTN_OK:
					GUIInstance->SaveSettings(GetDlgItem(hWnd, LV_INILIST));
					EndDialog(hWnd, NULL);

					return TRUE;
				}

				break;
			case WM_NOTIFY:
				switch (((LPNMHDR)lParam)->code)
				{
				case LVN_ITEMACTIVATE:
					{
						NMITEMACTIVATE* Data = (NMITEMACTIVATE*)lParam;
						char Buffer[0x200] = {0};

						ListView_GetItemText(Data->hdr.hwndFrom, Data->iItem, 2, Buffer, sizeof(Buffer));

						LPSTR Result = (LPSTR)DialogBoxParam(GUIInstance->ParentInstance, MAKEINTRESOURCE(DLG_TEXTEDIT), hWnd, (DLGPROC)TextEditDlgProc, (LPARAM)Buffer);

						if (Result)
						{
							ListView_SetItemText(Data->hdr.hwndFrom, Data->iItem, 2, Result);
							GUIInstance->HasChanges = true;
						}
					}

					break;
				}

				break;
			case WM_INITDIALOG:
				{
					SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)lParam);
					GUIInstance = (INIEditGUI*)lParam;

					ListView_SetExtendedListViewStyle(GetDlgItem(hWnd, LV_INILIST), LVS_EX_FULLROWSELECT);

					if (GUIInstance->CreateListView(hWnd) == false || GUIInstance->PopulateListView(GetDlgItem(hWnd, LV_INILIST)) == false)
					{
						char Buffer[0x200] = {0};
						FORMAT_STR(Buffer, "Error encountered while creating controls.\n\nWin32API ErrorID: %d", GetLastError());
						MessageBox(hWnd, Buffer, "INI Manager GUI", MB_OK);
						EndDialog(hWnd, NULL);

						return TRUE;
					}

					GetClientRect(hWnd, &GUIInstance->InitDialogBounds);
					SME::UIHelpers::GetClientRectInitBounds(GetDlgItem(hWnd, LV_INILIST), hWnd, &GUIInstance->InitListViewBounds);
					GUIInstance->HasChanges = false;
				}

				break;
			}

			return FALSE;
		}

		inline BOOL CALLBACK TextEditDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			char* Buffer = (char*)GetWindowLongPtr(hWnd, GWL_USERDATA);

			switch (uMsg)
			{
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case BTN_OK:
					GetDlgItemText(hWnd, EDIT_TEXTLINE, Buffer, 0x200);
					EndDialog(hWnd, (INT_PTR)Buffer);

					return TRUE;
				case BTN_CANCEL:
					EndDialog(hWnd, NULL);

					return TRUE;
				}

				break;
			case WM_INITDIALOG:
				SetDlgItemText(hWnd, EDIT_TEXTLINE, (LPSTR)lParam);
				SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)lParam);

				break;
			}

			return FALSE;
		}
	}
}
#endif