#pragma once

// SME::MiscGunk - "That colored chalk was forged by Lucifer himself!"

namespace SME
{
	namespace MiscGunk
	{
		class ElapsedTimeCounter
		{
			LARGE_INTEGER				ReferenceFrame;
			LARGE_INTEGER				FrameBuffer;
			LARGE_INTEGER				TimerFrequency;
			long double					TimePassed;
		public:
			ElapsedTimeCounter();

			void						Update(void);
			long double					GetTimePassed(void) const;		// in ms
		};

		template<typename T>
		class ScopedSetter
		{
			T&							BaseRef;
			T							Buffer;

			ScopedSetter(const ScopedSetter& rhs);
			ScopedSetter& operator=(const ScopedSetter& rhs);
		public:
			ScopedSetter(T& Base, T Value) :
				BaseRef(Base),
				Buffer(Base)
			{
				Base = Value;
			}
			~ScopedSetter()
			{
				BaseRef = Buffer;
			}
		};

		const char* GetTimeString(char* Out, size_t Size, const char* FormatString = "%m--%d--%Y %H-%M-%S");
		const char* GetTimeString(std::string& Out, const char* FormatString = "%m--%d--%Y %H-%M-%S");

		void ToggleFlag(UInt8* Flag, UInt32 Mask, bool State);
		void ToggleFlag(UInt16* Flag, UInt32 Mask, bool State);
		void ToggleFlag(UInt32* Flag, UInt32 Mask, bool State);
	}
}