// SME::MemoryHandler - Memory Handler classes
// The executable code handlers were directly ripped off from JRoush's COEF API - Kudos to the mustached llama

#ifndef __SME_SUNDRIES_MEMORYHANDLER_H__
#define __SME_SUNDRIES_MEMORYHANDLER_H__

#include "SME_Prefix.h"

namespace SME
{
	namespace MemoryHandler
	{
		inline void	SafeWrite8(UInt32 addr, UInt32 data)
		{
			UInt32	oldProtect;

			VirtualProtect((void *)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
			*((UInt8 *)addr) = data;
			VirtualProtect((void *)addr, 4, oldProtect, &oldProtect);
		}

		inline void	SafeWrite16(UInt32 addr, UInt32 data)
		{
			UInt32	oldProtect;

			VirtualProtect((void *)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
			*((UInt16 *)addr) = data;
			VirtualProtect((void *)addr, 4, oldProtect, &oldProtect);
		}

		inline void	SafeWrite32(UInt32 addr, UInt32 data)
		{
			UInt32	oldProtect;

			VirtualProtect((void *)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
			*((UInt32 *)addr) = data;
			VirtualProtect((void *)addr, 4, oldProtect, &oldProtect);
		}

		inline void	SafeWriteBuf(UInt32 addr, void * data, UInt32 len)
		{
			UInt32	oldProtect;

			VirtualProtect((void *)addr, len, PAGE_EXECUTE_READWRITE, &oldProtect);
			memcpy((void *)addr, data, len);
			VirtualProtect((void *)addr, len, oldProtect, &oldProtect);
		}

		inline void	WriteRelJump(UInt32 jumpSrc, UInt32 jumpTgt)
		{
			// jmp rel32
			SafeWrite8(jumpSrc, 0xE9);
			SafeWrite32(jumpSrc + 1, jumpTgt - jumpSrc - 1 - 4);
		}

		inline void	WriteRelCall(UInt32 jumpSrc, UInt32 jumpTgt)
		{
			// call rel32
			SafeWrite8(jumpSrc, 0xE8);
			SafeWrite32(jumpSrc + 1, jumpTgt - jumpSrc - 1 - 4);
		}

		inline UInt8*	MakeUInt8Array(UInt32 Size, ...)
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

		class Handler_Nop
		{
			UInt32				m_Address;
			UInt32				m_Size;
		public:
			Handler_Nop(UInt32 PatchAddr, UInt32 Size) :
				m_Address(PatchAddr),
				m_Size(Size)
			{
				;//
			}

			void				WriteNop()
			{
				if (m_Address == 0)
					return;

				for (int i = 0; i < m_Size; i++) {
					SafeWrite8(m_Address + i, 0x90);
				}
			}
		};

		typedef Handler_Nop NopHdlr;

		class Handler_Ace
		{
			UInt32				m_AddressA;
			UInt32				m_AddressB;
			UInt8*				m_Buffer;
			UInt32				m_BufferSize;
		public:
			Handler_Ace(UInt32 HookAddr, UInt32 JumpAddr, UInt8* Buffer, UInt32 BufferSize) :
				m_AddressA(HookAddr),
				m_AddressB(JumpAddr),
				m_Buffer(Buffer),
				m_BufferSize(BufferSize)
			{
				;//
			}

			Handler_Ace(UInt32 HookAddr, void* JumpAddr, UInt8* Buffer, UInt32 BufferSize) :
				m_AddressA(HookAddr),
				m_AddressB((UInt32)JumpAddr),
				m_Buffer(Buffer),
				m_BufferSize(BufferSize)
			{
				;//
			}

			~Handler_Ace()
			{
				SAFEDELETE_ARRAY(m_Buffer);
			}

			void				WriteJump()
			{
				if (m_AddressA == 0 || m_AddressB == 0)
					return;

				WriteRelJump(m_AddressA, m_AddressB);
			}

			void				WriteCall()
			{
				if (m_AddressA == 0 || m_AddressB == 0)
					return;

				WriteRelCall(m_AddressA, m_AddressB);
			}

			void				WriteCall(void* CallAddr)
			{
				if (m_AddressA == 0)
					return;

				WriteRelCall(m_AddressA, (UInt32)CallAddr);
			}

			void				WriteBuffer()
			{
				if (m_Buffer == 0 || m_BufferSize == 0 || m_AddressA == 0)
					return;

				for (int i = 0; i < m_BufferSize; i++) {
					SafeWrite8(m_AddressA + i, m_Buffer[i]);
				}
			}

			void				WriteUInt32(UInt32 Data)
			{
				if (m_AddressA == 0)
					return;

				SafeWrite32(m_AddressA, Data);
			}

			void				WriteUInt16(UInt16 Data)
			{
				if (m_AddressA == 0)
					return;

				SafeWrite16(m_AddressA, Data);
			}

			void				WriteUInt8(UInt8 Data)
			{
				if (m_AddressA == 0)
					return;

				SafeWrite8(m_AddressA, Data);
			}
		};

		typedef Handler_Ace MemHdlr;

		// Helper macros
		// TODO: Remove standard non-compliant leading underscores
		#define _DeclareMemHdlr(Name, Comment)												extern SME::MemoryHandler::MemHdlr		kMemHdlr##Name; void Name##Hook(void)
		#define _DeclareNopHdlr(Name, Comment)												extern SME::MemoryHandler::NopHdlr		kMemHdlr##Name

		#define _DefineCallHdlr(Name, PatchAddr, Function)									SME::MemoryHandler::MemHdlr	kMemHdlr##Name		(##PatchAddr, (UInt32)&##Function, 0, 0)
		#define _DefineHookHdlr(Name, PatchAddr)											SME::MemoryHandler::MemHdlr	kMemHdlr##Name		(##PatchAddr, Name##Hook, 0, 0)
		#define _DefineHookHdlrWithBuffer(Name, PatchAddr, BufferSize, ...)					SME::MemoryHandler::MemHdlr	kMemHdlr##Name		(##PatchAddr, Name##Hook, SME::MemoryHandler::MakeUInt8Array(##BufferSize,  ##__VA_ARGS__), BufferSize)
		#define _DefinePatchHdlr(Name, PatchAddr)											SME::MemoryHandler::MemHdlr	kMemHdlr##Name		(##PatchAddr, (UInt32)0, 0, 0)
		#define _DefinePatchHdlrWithBuffer(Name, PatchAddr, BufferSize, ...)				SME::MemoryHandler::MemHdlr	kMemHdlr##Name		(##PatchAddr, (UInt32)0, SME::MemoryHandler::MakeUInt8Array(##BufferSize,  ##__VA_ARGS__), BufferSize)
		#define _DefineJumpHdlr(Name, PatchAddr, JumpAddr)									SME::MemoryHandler::MemHdlr	kMemHdlr##Name		(##PatchAddr, JumpAddr, 0, 0)
		#define _DefineJumpHdlrWithBuffer(Name, PatchAddr, JumpAddr, BufferSize, ...)		SME::MemoryHandler::MemHdlr	kMemHdlr##Name		(##PatchAddr, JumpAddr, SME::MemoryHandler::MakeUInt8Array(##BufferSize,  ##__VA_ARGS__), BufferSize)
		#define _DefineNopHdlr(Name, PatchAddr, Size)										SME::MemoryHandler::NopHdlr	kMemHdlr##Name		(##PatchAddr, Size)

		#define _MemHdlr(Name)													kMemHdlr##Name
		#define _MemHdlrHook(Name)												Name##Hook

		#define _hhName															HookFnName
		#define _hhBegin()														zz__BeginHookHdlrFn(_hhName)
		#define _hhSetVar(Variable, Address)									zz__DeclareHookHdlrFnVariable(_hhName, Variable, Address)
		#define _hhGetVar(Variable)												zz__HookHdlrFnVariable(_hhName, Variable)

		/**** Define hooks in the following format:
		***********************************************************
			_DeclareMemHdlr(SampleTest, "Sample Hook");
			...
			_DefineHookHdlr(SampleTest, 0x00F586A);
			...
			_MemHdlr(SampleTest).WriteJump();
			...
			#define _hhName SampleTest
			_hhBegin()
			{
				_hhSetVar(Retn, 0x00123345);
				_hhSetVar(Jump, 0x0011AAAA);
				__asm
				{
					mov		eax, 0x005AD3FC				// fn addr
					call	eax
					cmp		eax, ebx
					jz		FAIL

					mov		eax, [eax]
					jmp		[_hhGetVar(Retn)]
				FAIL:
					jmp		[_hhGetVar(Jump)]
				}
			}
		***********************************************************
		****/

		// indirection macros
		#define zz___BeginHookHdlrFn(Name)										void __declspec(naked) Name##Hook(void)
		#define zz__BeginHookHdlrFn(Name)										zz___BeginHookHdlrFn(Name)

		#define zz___DeclareHookHdlrFnVariable(Name, Variable, Address)			static UInt32 k##Name##Hook##Variable##Addr = Address
		#define zz__DeclareHookHdlrFnVariable(Name, Variable, Address)			zz___DeclareHookHdlrFnVariable(Name, Variable, Address)

		#define zz___HookHdlrFnVariable(Name, Variable)							k##Name##Hook##Variable##Addr
		#define zz__HookHdlrFnVariable(Name, Variable)							zz___HookHdlrFnVariable(Name, Variable)

		// used to easily identify mem addrs
		#define VOLATILE(MemoryAddress)											MemoryAddress

		#pragma region Executable Code Handlers
			// cdecl convention
			template <typename TReturn>
			__forceinline TReturn cdeclCall(UInt32 _addr)
			{
				if (!_addr) return TReturn(0);
				typedef TReturn (*f)(void);
				return ((f)_addr)();
			}
			template <typename TReturn, typename T1>
			__forceinline TReturn cdeclCall(UInt32 _addr, T1 arg1)
			{
				if (!_addr) return TReturn(0);
				typedef TReturn (*f)(T1);
				return ((f)_addr)(arg1);
			}
			template <typename TReturn, typename T1,typename T2>
			__forceinline TReturn cdeclCall(UInt32 _addr, T1 arg1,T2 arg2)
			{
				if (!_addr) return TReturn(0);
				typedef TReturn (*f)(T1,T2);
				return ((f)_addr)(arg1,arg2);
			}
			template <typename TReturn, typename T1,typename T2,typename T3>
			__forceinline TReturn cdeclCall(UInt32 _addr, T1 arg1,T2 arg2,T3 arg3)
			{
				if (!_addr) return TReturn(0);
				typedef TReturn (*f)(T1,T2,T3);
				return ((f)_addr)(arg1,arg2,arg3);
			}
			template <typename TReturn, typename T1,typename T2,typename T3,typename T4>
			__forceinline TReturn cdeclCall(UInt32 _addr, T1 arg1,T2 arg2,T3 arg3,T4 arg4)
			{
				if (!_addr) return TReturn(0);
				typedef TReturn (*f)(T1,T2,T3,T4);
				return ((f)_addr)(arg1,arg2,arg3,arg4);
			}
			template <typename TReturn, typename T1,typename T2,typename T3,typename T4,typename T5>
			__forceinline TReturn cdeclCall(UInt32 _addr, T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5)
			{
				if (!_addr) return TReturn(0);
				typedef TReturn (*f)(T1,T2,T3,T4,T5);
				return ((f)_addr)(arg1,arg2,arg3,arg4,arg5);
			}
			template <typename TReturn, typename T1,typename T2,typename T3,typename T4,typename T5,typename T6>
			__forceinline TReturn cdeclCall(UInt32 _addr, T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5,T6 arg6)
			{
				if (!_addr) return TReturn(0);
				typedef TReturn (*f)(T1,T2,T3,T4,T5,T6);
				return ((f)_addr)(arg1,arg2,arg3,arg4,arg5,arg6);
			}
			template <typename TReturn, typename T1,typename T2,typename T3,typename T4,typename T5,typename T6,typename T7>
			__forceinline TReturn cdeclCall(UInt32 _addr, T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5,T6 arg6,T7 arg7)
			{
				if (!_addr) return TReturn(0);
				typedef TReturn (*f)(T1,T2,T3,T4,T5,T6,T7);
				return ((f)_addr)(arg1,arg2,arg3,arg4,arg5,arg6,arg7);
			}
			template <typename TReturn, typename T1,typename T2,typename T3,typename T4,typename T5,typename T6,typename T7,typename T8>
			__forceinline TReturn cdeclCall(UInt32 _addr, T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5,T6 arg6,T7 arg7,T8 arg8)
			{
				if (!_addr) return TReturn(0);
				typedef TReturn (*f)(T1,T2,T3,T4,T5,T6,T7,T8);
				return ((f)_addr)(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8);
			}
			template <typename TReturn, typename T1,typename T2,typename T3,typename T4,typename T5,typename T6,typename T7,typename T8,typename T9>
			__forceinline TReturn cdeclCall(UInt32 _addr, T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5,T6 arg6,T7 arg7,T8 arg8,T9 arg9)
			{
				if (!_addr) return TReturn(0);
				typedef TReturn (*f)(T1,T2,T3,T4,T5,T6,T7,T8,T9);
				return ((f)_addr)(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9);
			}
			template <typename TReturn, typename T1,typename T2,typename T3,typename T4,typename T5,typename T6,typename T7,typename T8,typename T9,typename T10>
			__forceinline TReturn cdeclCall(UInt32 _addr, T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5,T6 arg6,T7 arg7,T8 arg8,T9 arg9,T10 arg10)
			{
				if (!_addr) return TReturn(0);
				typedef TReturn (*f)(T1,T2,T3,T4,T5,T6,T7,T8,T9,T10);
				return ((f)_addr)(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10);
			}

			// thiscall convention
			template <typename TReturn, typename Tthis>
			__forceinline TReturn thisCall(UInt32 _addr, Tthis _this)
			{
				if (!_addr) return TReturn(0);
				class T {}; union { UInt32 x; TReturn (T::*m)(); } u = {_addr};
				return ((T*)_this->*u.m)();
			}
			template <typename TReturn, typename Tthis,typename T1>
			__forceinline TReturn thisCall(UInt32 _addr, Tthis _this, T1 arg1)
			{
				if (!_addr) return TReturn(0);
				class T {}; union { UInt32 x; TReturn (T::*m)(T1); } u = {_addr};
				return ((T*)_this->*u.m)(arg1);
			}
			template <typename TReturn, typename Tthis,typename T1,typename T2>
			__forceinline TReturn thisCall(UInt32 _addr, Tthis _this, T1 arg1,T2 arg2)
			{
				if (!_addr) return TReturn(0);
				class T {}; union { UInt32 x; TReturn (T::*m)(T1,T2); } u = {_addr};
				return ((T*)_this->*u.m)(arg1,arg2);
			}
			template <typename TReturn, typename Tthis,typename T1,typename T2,typename T3>
			__forceinline TReturn thisCall(UInt32 _addr, Tthis _this, T1 arg1,T2 arg2,T3 arg3)
			{
				if (!_addr) return TReturn(0);
				class T {}; union { UInt32 x; TReturn (T::*m)(T1,T2,T3); } u = {_addr};
				return ((T*)_this->*u.m)(arg1,arg2,arg3);
			}
			template <typename TReturn, typename Tthis,typename T1,typename T2,typename T3,typename T4>
			__forceinline TReturn thisCall(UInt32 _addr, Tthis _this, T1 arg1,T2 arg2,T3 arg3,T4 arg4)
			{
				if (!_addr) return TReturn(0);
				class T {}; union { UInt32 x; TReturn (T::*m)(T1,T2,T3,T4); } u = {_addr};
				return ((T*)_this->*u.m)(arg1,arg2,arg3,arg4);
			}
			template <typename TReturn, typename Tthis,typename T1,typename T2,typename T3,typename T4,typename T5>
			__forceinline TReturn thisCall(UInt32 _addr, Tthis _this, T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5)
			{
				if (!_addr) return TReturn(0);
				class T {}; union { UInt32 x; TReturn (T::*m)(T1,T2,T3,T4,T5); } u = {_addr};
				return ((T*)_this->*u.m)(arg1,arg2,arg3,arg4,arg5);
			}
			template <typename TReturn, typename Tthis,typename T1,typename T2,typename T3,typename T4,typename T5,typename T6>
			__forceinline TReturn thisCall(UInt32 _addr, Tthis _this, T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5,T6 arg6)
			{
				if (!_addr) return TReturn(0);
				class T {}; union { UInt32 x; TReturn (T::*m)(T1,T2,T3,T4,T5,T6); } u = {_addr};
				return ((T*)_this->*u.m)(arg1,arg2,arg3,arg4,arg5,arg6);
			}
			template <typename TReturn, typename Tthis,typename T1,typename T2,typename T3,typename T4,typename T5,typename T6,typename T7>
			__forceinline TReturn thisCall(UInt32 _addr, Tthis _this, T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5,T6 arg6,T7 arg7)
			{
				if (!_addr) return TReturn(0);
				class T {}; union { UInt32 x; TReturn (T::*m)(T1,T2,T3,T4,T5,T6,T7); } u = {_addr};
				return ((T*)_this->*u.m)(arg1,arg2,arg3,arg4,arg5,arg6,arg7);
			}
			template <typename TReturn, typename Tthis,typename T1,typename T2,typename T3,typename T4,typename T5,typename T6,typename T7,typename T8>
			__forceinline TReturn thisCall(UInt32 _addr, Tthis _this, T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5,T6 arg6,T7 arg7,T8 arg8)
			{
				if (!_addr) return TReturn(0);
				class T {}; union { UInt32 x; TReturn (T::*m)(T1,T2,T3,T4,T5,T6,T7,T8); } u = {_addr};
				return ((T*)_this->*u.m)(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8);
			}
			template <typename TReturn, typename Tthis,typename T1,typename T2,typename T3,typename T4,typename T5,typename T6,typename T7,typename T8,typename T9>
			__forceinline TReturn thisCall(UInt32 _addr, Tthis _this, T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5,T6 arg6,T7 arg7,T8 arg8,T9 arg9)
			{
				if (!_addr) return TReturn(0);
				class T {}; union { UInt32 x; TReturn (T::*m)(T1,T2,T3,T4,T5,T6,T7,T8,T9); } u = {_addr};
				return ((T*)_this->*u.m)(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9);
			}
			template <typename TReturn, typename Tthis,typename T1,typename T2,typename T3,typename T4,typename T5,typename T6,typename T7,typename T8,typename T9,typename T10>
			__forceinline TReturn thisCall(UInt32 _addr, Tthis _this, T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5,T6 arg6,T7 arg7,T8 arg8,T9 arg9,T10 arg10)
			{
				if (!_addr) return TReturn(0);
				class T {}; union { UInt32 x; TReturn (T::*m)(T1,T2,T3,T4,T5,T6,T7,T8,T9,T10); } u = {_addr};
				return ((T*)_this->*u.m)(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10);
			}

			// thiscall convention, addressed from virtual table
			template <typename TReturn, typename Tthis>
			__forceinline TReturn thisVirtualCall(UInt32 vtblOffset, Tthis _this)
			{
				class T {}; union { UInt32 x; TReturn (T::*m)(); } u = {*(UInt32*)((*((UInt32*)_this)) + vtblOffset)};
				return ((T*)_this->*u.m)();
			}
			template <typename TReturn, typename Tthis,typename T1>
			__forceinline TReturn thisVirtualCall(UInt32 vtblOffset,Tthis _this,T1 arg1)
			{
				class T {}; union { UInt32 x; TReturn (T::*m)(T1); } u = {*(UInt32*)((*((UInt32*)_this)) + vtblOffset)};
				return ((T*)_this->*u.m)(arg1);
			}
			template <typename TReturn, typename Tthis,typename T1,typename T2>
			__forceinline TReturn thisVirtualCall(UInt32 vtblOffset,Tthis _this,T1 arg1,T2 arg2)
			{
				class T {}; union { UInt32 x; TReturn (T::*m)(T1,T2); } u = {*(UInt32*)((*((UInt32*)_this)) + vtblOffset)};
				return ((T*)_this->*u.m)(arg1,arg2);
			}
			template <typename TReturn, typename Tthis,typename T1,typename T2,typename T3>
			__forceinline TReturn thisVirtualCall(UInt32 vtblOffset,Tthis _this,T1 arg1,T2 arg2,T3 arg3)
			{
				class T {}; union { UInt32 x; TReturn (T::*m)(T1,T2,T3); } u = {*(UInt32*)((*((UInt32*)_this)) + vtblOffset)};
				return ((T*)_this->*u.m)(arg1,arg2,arg3);
			}
			template <typename TReturn, typename Tthis,typename T1,typename T2,typename T3,typename T4>
			__forceinline TReturn thisVirtualCall(UInt32 vtblOffset,Tthis _this,T1 arg1,T2 arg2,T3 arg3,T4 arg4)
			{
				class T {}; union { UInt32 x; TReturn (T::*m)(T1,T2,T3,T4); } u = {*(UInt32*)((*((UInt32*)_this)) + vtblOffset)};
				return ((T*)_this->*u.m)(arg1,arg2,arg3,arg4);
			}
			template <typename TReturn, typename Tthis,typename T1,typename T2,typename T3,typename T4,typename T5>
			__forceinline TReturn thisVirtualCall(UInt32 vtblOffset,Tthis _this,T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5)
			{
				class T {}; union { UInt32 x; TReturn (T::*m)(T1,T2,T3,T4,T5); } u = {*(UInt32*)((*((UInt32*)_this)) + vtblOffset)};
				return ((T*)_this->*u.m)(arg1,arg2,arg3,arg4,arg5);
			}
			template <typename TReturn, typename Tthis,typename T1,typename T2,typename T3,typename T4,typename T5,typename T6>
			__forceinline TReturn thisVirtualCall(UInt32 vtblOffset,Tthis _this,T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5,T6 arg6)
			{
				class T {}; union { UInt32 x; TReturn (T::*m)(T1,T2,T3,T4,T5,T6); } u = {*(UInt32*)((*((UInt32*)_this)) + vtblOffset)};
				return ((T*)_this->*u.m)(arg1,arg2,arg3,arg4,arg5,arg6);
			}
			template <typename TReturn, typename Tthis,typename T1,typename T2,typename T3,typename T4,typename T5,typename T6,typename T7>
			__forceinline TReturn thisVirtualCall(UInt32 vtblOffset,Tthis _this,T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5,T6 arg6,T7 arg7)
			{
				class T {}; union { UInt32 x; TReturn (T::*m)(T1,T2,T3,T4,T5,T6,T7); } u = {*(UInt32*)((*((UInt32*)_this)) + vtblOffset)};
				return ((T*)_this->*u.m)(arg1,arg2,arg3,arg4,arg5,arg6,arg7);
			}
			template <typename TReturn, typename Tthis,typename T1,typename T2,typename T3,typename T4,typename T5,typename T6,typename T7,typename T8>
			__forceinline TReturn thisVirtualCall(UInt32 vtblOffset,Tthis _this,T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5,T6 arg6,T7 arg7,T8 arg8)
			{
				class T {}; union { UInt32 x; TReturn (T::*m)(T1,T2,T3,T4,T5,T6,T7,T8); } u = {*(UInt32*)((*((UInt32*)_this)) + vtblOffset)};
				return ((T*)_this->*u.m)(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8);
			}
			template <typename TReturn, typename Tthis,typename T1,typename T2,typename T3,typename T4,typename T5,typename T6,typename T7,typename T8,typename T9>
			__forceinline TReturn thisVirtualCall(UInt32 vtblOffset,Tthis _this,T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5,T6 arg6,T7 arg7,T8 arg8,T9 arg9)
			{
				class T {}; union { UInt32 x; TReturn (T::*m)(T1,T2,T3,T4,T5,T6,T7,T8,T9); } u = {*(UInt32*)((*((UInt32*)_this)) + vtblOffset)};
				return ((T*)_this->*u.m)(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9);
			}
			template <typename TReturn, typename Tthis,typename T1,typename T2,typename T3,typename T4,typename T5,typename T6,typename T7,typename T8,typename T9,typename T10>
			__forceinline TReturn thisVirtualCall(UInt32 vtblOffset,Tthis _this,T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5,T6 arg6,T7 arg7,T8 arg8,T9 arg9,T10 arg10)
			{
				class T {}; union { UInt32 x; TReturn (T::*m)(T1,T2,T3,T4,T5,T6,T7,T8,T9,T10); } u = {*(UInt32*)((*((UInt32*)_this)) + vtblOffset)};
				return ((T*)_this->*u.m)(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10);
			}
		#pragma endregion
	}
}
#endif