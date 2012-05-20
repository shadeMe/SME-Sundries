#include "SME_Prefix.h"
#include "MemoryHandler.h"

namespace SME
{
	namespace MemoryHandler
	{
		void Handler_Nop::WriteNop()
		{
			if (m_Address == 0)
				return;

			for (int i = 0; i < m_Size; i++) {
				SafeWrite8(m_Address + i, 0x90);
			}
		}

		Handler_Nop::Handler_Nop( UInt32 PatchAddr, UInt32 Size ) :
			m_Address(PatchAddr),
			m_Size(Size)
		{
			;//
		}

		void Handler_Ace::WriteBuffer()
		{
			if (m_Buffer == 0 || m_BufferSize == 0 || m_AddressA == 0)
				return;

			for (int i = 0; i < m_BufferSize; i++) {
				SafeWrite8(m_AddressA + i, m_Buffer[i]);
			}
		}

		void Handler_Ace::WriteCall()
		{
			if (m_AddressA == 0 || m_AddressB == 0)
				return;

			WriteRelCall(m_AddressA, m_AddressB);
		}

		void Handler_Ace::WriteCall( void* CallAddr )
		{
			if (m_AddressA == 0)
				return;

			WriteRelCall(m_AddressA, (UInt32)CallAddr);
		}

		void Handler_Ace::WriteJump()
		{
			if (m_AddressA == 0 || m_AddressB == 0)
				return;

			WriteRelJump(m_AddressA, m_AddressB);
		}

		void Handler_Ace::WriteUInt32(UInt32 Data)
		{
			if (m_AddressA == 0)
				return;

			SafeWrite32(m_AddressA, Data);
		}

		void Handler_Ace::WriteUInt16(UInt16 Data)
		{
			if (m_AddressA == 0)
				return;

			SafeWrite16(m_AddressA, Data);
		}

		void Handler_Ace::WriteUInt8(UInt8 Data)
		{
			if (m_AddressA == 0)
				return;

			SafeWrite8(m_AddressA, Data);
		}

		Handler_Ace::Handler_Ace( UInt32 HookAddr, UInt32 JumpAddr, UInt8* Buffer, UInt32 BufferSize ) :
			m_AddressA(HookAddr),
			m_AddressB(JumpAddr),
			m_Buffer(Buffer),
			m_BufferSize(BufferSize)
		{
			;//
		}

		Handler_Ace::Handler_Ace( UInt32 HookAddr, void* JumpAddr, UInt8* Buffer, UInt32 BufferSize ) :
			m_AddressA(HookAddr),
			m_AddressB((UInt32)JumpAddr),
			m_Buffer(Buffer),
			m_BufferSize(BufferSize)
		{
			;//
		}

		Handler_Ace::~Handler_Ace()
		{
			SAFEDELETE_ARRAY(m_Buffer);
		}

		void SafeWrite8(UInt32 addr, UInt32 data)
		{
			UInt32	oldProtect;

			VirtualProtect((void *)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
			*((UInt8 *)addr) = data;
			VirtualProtect((void *)addr, 4, oldProtect, &oldProtect);
		}

		void SafeWrite16(UInt32 addr, UInt32 data)
		{
			UInt32	oldProtect;

			VirtualProtect((void *)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
			*((UInt16 *)addr) = data;
			VirtualProtect((void *)addr, 4, oldProtect, &oldProtect);
		}

		void SafeWrite32(UInt32 addr, UInt32 data)
		{
			UInt32	oldProtect;

			VirtualProtect((void *)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
			*((UInt32 *)addr) = data;
			VirtualProtect((void *)addr, 4, oldProtect, &oldProtect);
		}

		void SafeWriteBuf(UInt32 addr, void * data, UInt32 len)
		{
			UInt32	oldProtect;

			VirtualProtect((void *)addr, len, PAGE_EXECUTE_READWRITE, &oldProtect);
			memcpy((void *)addr, data, len);
			VirtualProtect((void *)addr, len, oldProtect, &oldProtect);
		}

		void WriteRelJump(UInt32 jumpSrc, UInt32 jumpTgt)
		{
			// jmp rel32
			SafeWrite8(jumpSrc, 0xE9);
			SafeWrite32(jumpSrc + 1, jumpTgt - jumpSrc - 1 - 4);
		}

		void WriteRelCall(UInt32 jumpSrc, UInt32 jumpTgt)
		{
			// call rel32
			SafeWrite8(jumpSrc, 0xE8);
			SafeWrite32(jumpSrc + 1, jumpTgt - jumpSrc - 1 - 4);
		}

		UInt8* MakeUInt8Array(UInt32 Size, ...)
		{
			va_list Args;
			UInt8* ResultArray = new UInt8[Size];

			va_start(Args, Size);
			for (int i = 0; i < Size; i++)
			{
				UInt8 Value = va_arg(Args, UInt8);
				ResultArray[i] = Value;
			}
			va_end(Args);

			return ResultArray;
		}
	}
}