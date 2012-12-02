#pragma once

// SME::UIHelpers - Helpers for Win32 UI development

namespace SME
{
	namespace UIHelpers
	{
#include "WindowEdgeSnapper.inl"

		void GetClientRectInitBounds(HWND Window, HWND Parent, RECT* OutBounds);
	}
}