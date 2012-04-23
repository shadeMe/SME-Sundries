#include "SME_Prefix.h"
#include "MiscGunk.h"

namespace SME
{
	namespace MiscGunk
	{
		ElapsedTimeCounter::ElapsedTimeCounter()
		{
			QueryPerformanceCounter(&ReferenceFrame);
			QueryPerformanceFrequency(&TimerFrequency);

			Update();
		}

		void ElapsedTimeCounter::Update(void)
		{
			QueryPerformanceCounter(&FrameBuffer);
			TimePassed = ((FrameBuffer.QuadPart - ReferenceFrame.QuadPart ) * 1000.0 / TimerFrequency.QuadPart);
			ReferenceFrame = FrameBuffer;
		}

		long double ElapsedTimeCounter::GetTimePassed( void ) const
		{
			return TimePassed;
		}
	}
}