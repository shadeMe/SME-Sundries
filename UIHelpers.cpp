#include "SME_Prefix.h"
#include "UIHelpers.h"

namespace SME
{
	namespace UIHelpers
	{
		void GetClientRectInitBounds( HWND Window, HWND Parent, RECT* OutBounds )
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