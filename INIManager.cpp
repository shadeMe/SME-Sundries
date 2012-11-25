#include "SME_Prefix.h"
#include "INIManager.h"

namespace SME
{
	namespace INI
	{
		INIManager::INIManager() : SettingList()
		{
			;//
		}

		INIManager::~INIManager()
		{
			for (INISettingListT::const_iterator Itr = SettingList.begin(); Itr != SettingList.end(); Itr++)
			{
				(*Itr)->Save(this);
				delete *Itr;
			}

			SettingList.clear();
		}

		bool INIManager::RegisterSetting( const char* Key, const char* Section, const char* DefaultValue, const char* Description )
		{
			if (FetchSetting(Key, Section))
				return false;

			INISetting* NewSetting = new INISetting(this, Key, Section, DefaultValue, Description);
			NewSetting->Load(this);

			SettingList.push_back(NewSetting);
			SettingList.sort(SortComparator);
			return true;
		}

		INISetting* INIManager::FetchSetting(const char* Key, const char* Section)
		{
			for (INISettingListT::const_iterator Itr = SettingList.begin(); Itr != SettingList.end(); Itr++)
			{
				if (!_stricmp((*Itr)->GetKey(), Key) && !_stricmp((*Itr)->GetSection(), Section))
				{
					return *Itr;
				}
			}
			return NULL;
		}

		void INIManager::Save(void)
		{
			for (INISettingListT::const_iterator Itr = SettingList.begin(); Itr != SettingList.end(); Itr++)
			{
				(*Itr)->Save(this);
			}
		}

		void INIManager::Load(void)
		{
			for (INISettingListT::const_iterator Itr = SettingList.begin(); Itr != SettingList.end(); Itr++)
			{
				(*Itr)->Load(this);
			}
		}

		const char* INIManager::GetPath() const
		{
			return INIFilePath.c_str();
		}

		int INIManager::DirectRead( const char* Setting, const char* Section, const char* Default, char* OutBuffer, UInt32 Size )
		{
			return GetPrivateProfileString(Section, Setting, Default, OutBuffer, Size, GetPath());
		}

		int INIManager::DirectRead( const char* Section, char* OutBuffer, UInt32 Size )
		{
			return GetPrivateProfileSection(Section, OutBuffer, Size, GetPath());
		}

		bool INIManager::DirectWrite( const char* Setting, const char* Section, const char* Value )
		{
			return WritePrivateProfileString(Section, Setting, Value, GetPath());
		}

		bool INIManager::DirectWrite( const char* Section, const char* Value )
		{
			return WritePrivateProfileSection(Section, Value, GetPath());
		}

		bool INIManager::SortComparator( INISetting* First, INISetting* Second )
		{
			return !(*First < *Second);
		}

		bool INIManager::PopulateFromINI( void )
		{
			bool Result = false;
			std::fstream INIStream(GetPath(), std::fstream::in);

			if (INIStream.fail() == false)
			{
				char SectionNames[0x4000] = {0};
				char SectionData[0x4000] = {0};

				if (GetPrivateProfileSectionNames(SectionNames, sizeof(SectionNames), GetPath()))
				{
					for (const char* Section = SectionNames; *Section != '\0'; Section += strlen(Section) + 1)
					{
						ZeroMemory(SectionData, sizeof(SectionData));
						DirectRead(Section, SectionData, sizeof(SectionData));

						for (const char* Itr = SectionData; *Itr != '\0'; Itr += strlen(Itr) + 1)
						{
							std::string Entry(Itr);
							size_t Delimiter = Entry.find("=");

							if (Delimiter != std::string::npos)
							{
								std::string Key(Entry.substr(0, Delimiter));
								std::string Value(Entry.substr(Delimiter + 1));

								if (RegisterSetting(Key.c_str(), Section, Value.c_str(), ""))
									FetchSetting(Key.c_str(), Section)->SetValue("%s", Value.c_str());
							}
						}
					}

					Result = true;
				}
			}

			INIStream.close();
			INIStream.clear();

			return Result;
		}

		bool INIManager::PopulateFromSection( const char* Section )
		{
			bool Result = false;

			char SectionData[0x4000] = {0};

			if (INIManager::DirectRead(Section, SectionData, sizeof(SectionData)))
			{
				for (const char* Itr = SectionData; *Itr != '\0'; Itr += strlen(Itr) + 1)
				{
					std::string Entry(Itr);
					size_t Delimiter = Entry.find("=");

					if (Delimiter != std::string::npos)
					{
						std::string Key(Entry.substr(0, Delimiter));
						std::string Value(Entry.substr(Delimiter + 1));

						if (RegisterSetting(Key.c_str(), Section, Value.c_str(), ""))
							FetchSetting(Key.c_str(), Section)->SetValue("%s", Value.c_str());
					}
				}

				Result = true;
			}

			return Result;
		}

		INISetting::INISetting(INIManager* Manager, const char* Key, const char* Section, const char* DefaultValue, const char* Description)
		{
			this->Key = Key;
			this->Section = Section;
			this->DefaultValue = DefaultValue;
			this->Value = DefaultValue;
			this->Description = Description;
		}

		bool INISetting::Save(INIManager* Manager)
		{
			if (strlen(Manager->GetPath()) < 2 ||
				!WritePrivateProfileStringA(Section.c_str(), Key.c_str(), Value.c_str(), Manager->GetPath()))
			{
				return false;
			}
			else
				return true;
		}

		void INISetting::Load(INIManager* Manager)
		{
			if (strlen(Manager->GetPath()) < 2)
				return;

			char Buffer[0x200];
			GetPrivateProfileStringA(Section.c_str(), Key.c_str(), DefaultValue.c_str(), Buffer, sizeof(Buffer), Manager->GetPath());
			Value = Buffer;
		}

		INISetting::~INISetting()
		{
			;//
		}

		const char* INISetting::GetValueAsString( void ) const
		{
			return Value.c_str();
		}

		const char* INISetting::GetDescription( void ) const
		{
			return Description.c_str();
		}

		const char* INISetting::GetKey( void ) const
		{
			return Key.c_str();
		}

		const char* INISetting::GetSection( void ) const
		{
			return Section.c_str();
		}

		int INISetting::GetValueAsInteger( void ) const
		{
			return atoi(Value.c_str());
		}

		float INISetting::GetValueAsFloat( void ) const
		{
			return atof(Value.c_str());
		}

		void INISetting::SetValue( const char* Format, ... )
		{
			char Buffer[0x200] = {0};

			va_list Args;
			va_start(Args, Format);
			vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
			va_end(Args);

			Value = Buffer;
		}

		bool INISetting::operator<( const INISetting& Second )
		{
			int Result = _stricmp(Section.c_str(), Second.Section.c_str());
	//		if (Result == 0)
	//			Result = _stricmp(Key.c_str(), Second.Key.c_str());

			return Result >= 0;
		}

		UInt32 INISetting::GetValueAsUnsignedInteger( bool AsHex /*= false*/ ) const
		{
			UInt32 Result = 0;

			if (AsHex)
				sscanf_s(Value.c_str(), "%08X", &Result);
			else
				sscanf_s(Value.c_str(), "%u", &Result);

			return Result;
		}

		INIManagerIterator::INIManagerIterator( INIManager* Manager, const char* Section ) :
			Manager(Manager),
			Section(Section)
		{
			SME_ASSERT(Manager);

			INIListSize = Manager->SettingList.size();
			Iterator = Bookend = Manager->SettingList.end();
			CurrentSetting = NULL;

			if (INIListSize)
			{
				Iterator = Manager->SettingList.begin();
				CurrentSetting = *(Manager->SettingList.begin());
			}
		}

		const INISetting* INIManagerIterator::GetNextSetting()
		{
			CurrentSetting = NULL;

			if (Iterator != Manager->SettingList.end())
			{
				while (++Iterator != Manager->SettingList.end())
				{
					INISetting* Current = *Iterator;

					if (Section == NULL || !_stricmp(Current->GetSection(), Section))
					{
						CurrentSetting = *Iterator;
						break;
					}
				}
			}

			return CurrentSetting;
		}

		INIManagerIterator::~INIManagerIterator()
		{
			INIListSize = 0;
			CurrentSetting = NULL;
			Manager = NULL;
		}

		bool INIManagerIterator::GetDone() const
		{
			return CurrentSetting == NULL;
		}

		const INISetting* INIManagerIterator::GetCurrentSetting() const
		{
			return CurrentSetting;
		}

		const INISetting* INIManagerIterator::operator()() const
		{
			return CurrentSetting;
		}
	}
}