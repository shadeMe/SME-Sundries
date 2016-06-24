// SME::MiscGunk - "That colored chalk was forged by Lucifer himself!"

#ifndef __SME_SUNDRIES_MISCGUNK_H__
#define __SME_SUNDRIES_MISCGUNK_H__

#include "SME_Prefix.h"


namespace SME
{
	namespace MiscGunk
	{
		class ElapsedTimeCounter
		{
			__int64				ReferenceFrame;
			double				Frequency;
			double				TimePassed;
		public:
			ElapsedTimeCounter()
			{
				LARGE_INTEGER FrameBuffer;

				QueryPerformanceFrequency(&FrameBuffer);
				Frequency = double(FrameBuffer.QuadPart) / 1000.0;
				QueryPerformanceCounter(&FrameBuffer);
				ReferenceFrame = FrameBuffer.QuadPart;
			}

			void						Update(void)
			{
				LARGE_INTEGER FrameBuffer;

				QueryPerformanceCounter(&FrameBuffer);
				TimePassed = double(FrameBuffer.QuadPart - ReferenceFrame) / Frequency;
				ReferenceFrame = FrameBuffer.QuadPart;
			}

			double						GetTimePassed(void) const		// b'ween the last 2 consecutive calls to Update(), in ms
			{
				return TimePassed;
			}
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

		inline const char* GetTimeString(char* Out, size_t Size, const char* FormatString = "%m--%d--%Y %H-%M-%S")
		{
			__time32_t TimeData;
			tm LocalTime;

			_time32(&TimeData);
			_localtime32_s(&LocalTime, &TimeData);
			strftime(Out, Size, FormatString, &LocalTime);
			return Out;
		}

		inline const char* GetTimeString(std::string& Out, const char* FormatString = "%m--%d--%Y %H-%M-%S")
		{
			char Buffer[0x100] = {0};
			GetTimeString(Buffer, sizeof(Buffer), FormatString);
			Out = Buffer;

			return Out.c_str();
		}

		inline void ToggleFlag(UInt8* Flag, UInt32 Mask, bool State)
		{
			if (State)
				*Flag |= Mask;
			else
				*Flag &= ~Mask;
		}

		inline void ToggleFlag(UInt16* Flag, UInt32 Mask, bool State)
		{
			if (State)
				*Flag |= Mask;
			else
				*Flag &= ~Mask;
		}

		inline void ToggleFlag(UInt32* Flag, UInt32 Mask, bool State)
		{
			if (State)
				*Flag |= Mask;
			else
				*Flag &= ~Mask;
		}

#pragma warning (push)
#pragma warning (disable : 4200)
		struct RTTIType
		{
			void	* typeInfo;
			UInt32	pad;
			char	name[0];
		};

		struct RTTILocator
		{
			UInt32		sig, offset, cdOffset;
			RTTIType	* type;
		};
#pragma warning (pop)

		// use the RTTI information to return an object's class name
		inline const char * GetObjectClassName(void * objBase)
		{
			const char	* result = "<no rtti>";

			__try
			{
				void		** obj = (void **)objBase;
				RTTILocator	** vtbl = (RTTILocator **)obj[0];
				RTTILocator	* rtti = vtbl[-1];
				RTTIType	* type = rtti->type;

				// starts with ,?
				if((type->name[0] == '.') && (type->name[1] == '?'))
				{
					// is at most 100 chars long
					for(UInt32 i = 0; i < 100; i++)
					{
						if(type->name[i] == 0)
						{
							// remove the .?AV
							result = type->name + 4;
							break;
						}
					}
				}
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				// return the default
			}

			return result;
		}

		inline const char* DumpClass(void * theClassPtr, UInt32 nIntsToDump, std::string& OutDump)
		{
			OutDump = "DumpClass:";
			UInt32* basePtr = (UInt32*)theClassPtr;

			if (!theClassPtr)
				return OutDump.c_str();

			for (UInt32 ix = 0; ix < nIntsToDump; ix++ )
			{
				UInt32* curPtr = basePtr+ix;
				const char* curPtrName = NULL;
				UInt32 otherPtr = 0;
				float otherFloat = 0.0;
				const char* otherPtrName = NULL;

				if (curPtr)
				{
					curPtrName = GetObjectClassName((void*)curPtr);

					__try
					{
						otherPtr = *curPtr;
						otherFloat = *(float*)(curPtr);
					}
					__except(EXCEPTION_EXECUTE_HANDLER)
					{
						//
					}

					if (otherPtr)
						otherPtrName = GetObjectClassName((void*)otherPtr);
				}

				OutDump += ("\n\t%3d +%03X ptr: 0x%08X: %32s *ptr: 0x%08x | %f: %32s", ix, ix*4, curPtr, curPtrName, otherPtr, otherFloat, otherPtrName);
			}

			return OutDump.c_str();
		}

		class IEventSource;

		struct IEventData
		{
			const IEventSource*			Source;

			IEventData(const IEventSource* Source) : Source(Source) {}
			virtual ~IEventData()
			{
				;//
			}
		};

		class IEventSink
		{
		public:
			virtual ~IEventSink()
			{
				;//
			}

			virtual void					Handle(IEventData* Data) = 0;
		};

		class IEventSource
		{
		protected:
			typedef std::vector<IEventSink*>			SinkListT;

			SinkListT						Sinks;

			virtual void Dispatch(IEventData* Data) const
			{
				for (SinkListT::const_iterator Itr = Sinks.begin(); Itr != Sinks.end(); ++Itr)
					(*Itr)->Handle(Data);
			}
		public:

			IEventSource() : Sinks()
			{
				Sinks.reserve(10);
			}

			virtual ~IEventSource()
			{
				ClearSinks();
			}

			// caller retains ownership of pointer
			virtual bool AddSink(IEventSink* Sink)
			{
				SME_ASSERT(Sink);
				if (std::find(Sinks.begin(), Sinks.end(), Sink) == Sinks.end())
				{
					Sinks.push_back(Sink);
					return true;
				}
				else
					return false;
			}

			virtual bool RemoveSink(IEventSink* Sink)
			{
				SME_ASSERT(Sink);

				SinkListT::iterator Match = std::find(Sinks.begin(), Sinks.end(), Sink);
				if (Match == Sinks.end())
					return false;
				else
				{
					Sinks.erase(Match);
					return false;
				}
			}

			virtual void ClearSinks()
			{
				Sinks.clear();
			}
		};
	}
}
#endif