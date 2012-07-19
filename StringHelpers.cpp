#include "SME_Prefix.h"
#include "StringHelpers.h"
#include <mbstring.h>

namespace SME
{
	namespace StringHelpers
	{
		Tokenizer::Tokenizer(const char* src, const char* delims)
			: m_offset(0), m_delims(delims), m_data(src)
		{
			//
		}

		Tokenizer::~Tokenizer()
		{
			//
		}

		UInt32 Tokenizer::NextToken(std::string& outStr)
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

		UInt32 Tokenizer::PrevToken(std::string& outStr)
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

		bool ci_equal(char ch1, char ch2)
		{
			return tolower((unsigned char)ch1) == tolower((unsigned char)ch2);
		}

		bool ci_less(const char* lh, const char* rh)
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

		void MakeUpper(std::string& str)
		{
			std::transform(str.begin(), str.end(), str.begin(), toupper);
		}

		void MakeLower(std::string& str)
		{
			std::transform(str.begin(), str.end(), str.begin(), tolower);
		}

#pragma warning(push)
#pragma warning(disable: 4996)	// warning about std::transform()
		void MakeUpper(char* str)
		{
			if (str)
			{
				UInt32 len = strlen(str);
				std::transform(str, str + len, str, toupper);
			}
		}
#pragma warning(pop)

		std::wstring FormatWideString( const char* Format, ... )
		{
			char Buffer[0x1000] = {0};

			va_list Args;
			va_start(Args, Format);
			vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
			va_end(Args);

			WCHAR WideBuffer[0x1000] = {0};
			mbstowcs(WideBuffer, Buffer, strlen(Buffer));
			return WideBuffer;
		}

		bool GetHasNonAlnumCharacter( std::string& str )
		{
			for (std::string::iterator Itr = str.begin(); Itr != str.end(); Itr++)
			{
				if (_ismbcalnum(*Itr) == 0)
					return true;
			}

			return false;
		}
	}
}