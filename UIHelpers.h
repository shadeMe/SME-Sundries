// SME::UIHelpers - Helpers for Win32 UI development

#ifndef __SME_SUNDRIES_UIHELPERS_H__
#define __SME_SUNDRIES_UIHELPERS_H__

#include "SME_Prefix.h"

namespace SME
{
	namespace UIHelpers
	{
#include "WindowEdgeSnapper.inl"

		inline void GetClientRectInitBounds( HWND Window, HWND Parent, RECT* OutBounds )
		{
			POINT Position = {0};
			RECT Bounds = {0};

			GetWindowRect(Window, OutBounds);
			Position.x = OutBounds->left;
			Position.y = OutBounds->top;
			ScreenToClient(Parent, &Position);
			OutBounds->left = Position.x;
			OutBounds->top = Position.y;
			GetClientRect(Window, &Bounds);
			OutBounds->right = Bounds.right;
			OutBounds->bottom = Bounds.bottom;
		}
	}
}
#endif