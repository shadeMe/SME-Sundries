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

		inline long double ElapsedTimeCounter::GetTimePassed( void ) const
		{
			return TimePassed;
		}

		const char* GetTimeString( char* Out, size_t Size, const char* FormatString /*= "%m--%d--%Y %H-%M-%S"*/ )
		{
			__time32_t TimeData;
			tm LocalTime;

			_time32(&TimeData);
			_localtime32_s(&LocalTime, &TimeData);
			strftime(Out, Size, FormatString, &LocalTime);
			return Out;
		}
	}
}