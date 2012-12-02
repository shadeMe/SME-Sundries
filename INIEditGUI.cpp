#include "SME_Prefix.h"
#include "INIEditGUI.h"
#include "INIEditGUI_Res.h"
#include "INIManager.h"

namespace SME
{
	namespace INI
	{
		static char			s_Buffer[0x200] = {0};

		void INIEditGUI::InitializeGUI(HINSTANCE ParentInstance, HWND ParentHWND, INIManager* ParentINIManager)
		{
			this->ParentManager = ParentINIManager;
			this->ParentInstance = ParentInstance;

			DialogBoxParam(ParentInstance, MAKEINTRESOURCE(DLG_INIGUI), ParentHWND, INIEditGUIDlgProc, (LPARAM)this);
		}

		bool INIEditGUI::CreateListView(HWND ParentHandle)
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
					sprintf_s(Buffer, sizeof(Buffer), "Section");
					lvc.cx = 200;
					break;
				case 1:
					sprintf_s(Buffer, sizeof(Buffer), "Setting");
					lvc.cx = 200;
					break;
				case 2:
					sprintf_s(Buffer, sizeof(Buffer), "Value");
					lvc.cx = 120;
					break;
				case 3:
					sprintf_s(Buffer, sizeof(Buffer), "Description");
					lvc.cx = 400;
					break;
				}

				if (ListView_InsertColumn(ListViewHandle, i, &lvc) == -1)
					return false;
			}

			return true;
		}

		bool INIEditGUI::PopulateListView(HWND ListViewHandle)
		{
			UInt32 Index = 0;
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
				Value.pszText = (LPSTR)Setting->GetValueAsString();
				Value.iItem = Index;

				Desc.mask = LVIF_TEXT | LVIF_STATE;
				Desc.iSubItem = 3;
				Desc.state = 0;
				Desc.stateMask = 0;
				Desc.pszText = (LPSTR)Setting->GetDescription();
				Desc.iItem = Index;

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

		void INIEditGUI::SaveSettings(HWND ListViewHandle)
		{
			UInt32 ItemCount = ListView_GetItemCount(ListViewHandle);

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
				ListView_GetItemText(ListViewHandle, i, 2, s_Buffer, sizeof(s_Buffer));

				Setting->SetValue(s_Buffer);
			}
		}

		BOOL CALLBACK INIEditGUIDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
					NMITEMACTIVATE* Data = (NMITEMACTIVATE*)lParam;

					ListView_GetItemText(Data->hdr.hwndFrom, Data->iItem, 2, s_Buffer, sizeof(s_Buffer));

					LPSTR Result = (LPSTR)DialogBoxParam(GUIInstance->ParentInstance, MAKEINTRESOURCE(DLG_TEXTEDIT), hWnd, (DLGPROC)TextEditDlgProc, (LPARAM)s_Buffer);

					if (Result)
					{
						ListView_SetItemText(Data->hdr.hwndFrom, Data->iItem, 2, Result);
						GUIInstance->HasChanges = true;
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
						sprintf_s(s_Buffer, sizeof(s_Buffer), "Error encountered while creating controls.\n\nWin32API ErrorID: %d", GetLastError());
						MessageBox(hWnd, s_Buffer, "INI Manager GUI", MB_OK);
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

		BOOL CALLBACK TextEditDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			switch (uMsg)
			{
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case BTN_OK:
					GetDlgItemText(hWnd, EDIT_TEXTLINE, s_Buffer, sizeof(s_Buffer));
					EndDialog(hWnd, (INT_PTR)s_Buffer);

					return TRUE;
				case BTN_CANCEL:
					EndDialog(hWnd, NULL);

					return TRUE;
				}

				break;
			case WM_INITDIALOG:
				SetDlgItemText(hWnd, EDIT_TEXTLINE, (LPSTR)lParam);

				break;
			}

			return FALSE;
		}
	}
}