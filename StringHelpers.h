#pragma once

// SME::StringUtilities - Various string helper class. Shamelessly copied from OBSE's code base. Most of them, at any rate...

namespace SME
{
	namespace StringHelpers
	{
		class Tokenizer
		{
		public:
			Tokenizer(const char* src, const char* delims);
			~Tokenizer();

			// these return the offset of token in src, or -1 if no token
			UInt32 NextToken(std::string& outStr);
			UInt32 PrevToken(std::string& outStr);

		private:
			void MoveToFirstToken();

			std::string m_delims;
			size_t		m_offset;
			std::string m_data;
		};

		bool ci_equal(char ch1, char ch2);
		bool ci_less(const char* lh, const char* rh);
		void MakeUpper(std::string& str);
		void MakeUpper(char* str);
		void MakeLower(std::string& str);

		std::wstring FormatWideString(const char* Format, ...);
		bool GetHasNonAlnumCharacter(std::string& str);
		COLORREF GetRGB(const char* String);
		void Replace(std::string& Str, char Query, char Replacement);
		void Erase(std::string& Str, char Query);
	}
}