// SME::StringUtilities - Various string helper class. Shamelessly copied from OBSE's code base. Most of them, at any rate...

#ifndef __SME_SUNDRIES_STRINGHELPERS_H__
#define __SME_SUNDRIES_STRINGHELPERS_H__

#include "SME_Prefix.h"
#include <mbstring.h>
#include <d3d9.h>

namespace SME
{
	namespace StringHelpers
	{
		class Tokenizer
		{
		public:
			Tokenizer(const char* src, const char* delims)
				: m_offset(0), m_delims(delims), m_data(src)
			{
				//
			}

			~Tokenizer()
			{
				//
			}

			// these return the offset of token in src, or -1 if no token
			UInt32 NextToken(std::string& outStr)
			{
				if (m_offset == m_data.length())
					return -1;

				size_t start = m_data.find_first_not_of(m_delims, m_offset);
				if (start != -1)
				{
					size_t end = m_data.find_first_of(m_delims, start);
					if (end == -1)
						end = m_data.length();

					m_offset = end;
					outStr = m_data.substr(start, end - start);
					return start;
				}

				return -1;
			}

			UInt32 PrevToken(std::string& outStr)
			{
				if (m_offset == 0)
					return -1;

				size_t searchStart = m_data.find_last_of(m_delims, m_offset - 1);
				if (searchStart == -1)
					return -1;

				size_t end = m_data.find_last_not_of(m_delims, searchStart);
				if (end == -1)
					return -1;

				size_t start = m_data.find_last_of(m_delims, end);	// okay if start == -1 here

				m_offset = end + 1;
				outStr = m_data.substr(start + 1, end - start);
				return start + 1;
			}
		private:
			std::string m_delims;
			size_t		m_offset;
			std::string m_data;
		};

		inline bool ci_equal(char ch1, char ch2)
		{
			return tolower((unsigned char)ch1) == tolower((unsigned char)ch2);
		}

		inline bool ci_less(const char* lh, const char* rh)
		{
			while (*lh && *rh)
			{
				char l = toupper(*lh);
				char r = toupper(*rh);

				if (l < r)
				{
					return true;
				}
				else if (l > r)
				{
					return false;
				}

				lh++;
				rh++;
			}

			return toupper(*lh) < toupper(*rh);
		}

#pragma warning(push)
#pragma warning(disable: 4996)	// warning about std::transform()
		inline void MakeUpper(std::string& str)
		{
			std::transform(str.begin(), str.end(), str.begin(), toupper);
		}

		inline void MakeLower(std::string& str)
		{
			std::transform(str.begin(), str.end(), str.begin(), tolower);
		}

		inline void MakeUpper(char* str)
		{
			if (str)
			{
				UInt32 len = strlen(str);
				std::transform(str, str + len, str, toupper);
			}
		}

		inline std::wstring FormatWideString( const char* Format, ... )
		{
			char Buffer[0x1000] = {0};

			va_list Args;
			va_start(Args, Format);
			vsnprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, Format, Args);
			va_end(Args);

			WCHAR WideBuffer[0x1000] = {0};
			mbstowcs(WideBuffer, Buffer, strlen(Buffer));
			return WideBuffer;
		}
#pragma warning(pop)

		inline bool GetHasNonAlnumCharacter( std::string& str )
		{
			for (std::string::iterator Itr = str.begin(); Itr != str.end(); Itr++)
			{
				if (_ismbcalnum(*Itr) == 0)
					return true;
			}

			return false;
		}

		inline void GetRGB(const char* String, int& R, int& G, int& B)
		{
			SME_ASSERT(String);

			SME::StringHelpers::Tokenizer ColorParser(String, ", ");

			for (int i = 0; i < 3; i++)
			{
				std::string Buffer;
				if (ColorParser.NextToken(Buffer) != -1)
				{
					switch (i)
					{
					case 0:		// R
						R = atoi(Buffer.c_str());
						break;
					case 1:		// G
						G = atoi(Buffer.c_str());
						break;
					case 2:		// B
						B = atoi(Buffer.c_str());
						break;
					}
				}
			}
		}

		inline COLORREF GetRGB(const char* String)
		{
			int R = 0, B = 0, G = 0;
			GetRGB(String, R, G, B);
			return RGB(R, G, B);
		}

		inline D3DCOLOR GetRGBD3D(const char* String, int Alpha)
		{
			int R = 0, B = 0, G = 0;
			GetRGB(String, R, G, B);
			return D3DCOLOR_ARGB(Alpha, R, G, B);
		}

		inline void Replace( std::string& Str, char Query, char Replacement )
		{
			std::replace(Str.begin(), Str.end(), Query, Replacement);
		}

		inline void Erase( std::string& Str, char Query )
		{
			for (std::string::iterator Itr = Str.begin(); Itr != Str.end();)
			{
				if (*Itr == Query)
				{
					Itr = Str.erase(Itr);
					continue;
				}

				Itr++;
			}
		}
	}
}
#endif