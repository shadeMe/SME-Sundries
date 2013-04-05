#pragma warning(disable: 4800)

#include "INIManager.h"

namespace SME
{
	namespace INI
	{
		void INISetting::SetCString( char** Dest, const char* Source )
		{
			SME_ASSERT(Dest);

			SAFEDELETE_ARRAY(*Dest);

			if (Source == NULL)
				Source = "";

			UInt32 Size = strlen(Source);
			*Dest = new char[Size + 1];

			(*Dest)[Size] = '\0';
			if (Size)
				strcpy_s(*Dest, Size + 1, Source);
		}

		void INISetting::Serialize( char* Dest, UInt32 Size ) const
		{
			switch (Type)
			{
			case kType_Integer:
				sprintf_s(Dest, Size, "%d", Data.i);
				break;
			case kType_Unsigned:
				sprintf_s(Dest, Size, "%08X", Data.u);
				break;
			case kType_Float:
				sprintf_s(Dest, Size, "%f", Data.f);
				break;
			case kType_String:
				sprintf_s(Dest, Size, "%s", Data.s);
				break;
			}
		}

		void INISetting::Deserialize( const char* Source )
		{
			SME_ASSERT(Source);

			switch (Type)
			{
			case kType_Integer:
				Data.i = atoi(Source);
				break;
			case kType_Unsigned:
				sscanf_s(Source, "%08X", &Data.u);
				break;
			case kType_Float:
				Data.f = atof(Source);
				break;
			case kType_String:
				SetCString(&Data.s, Source);
				break;
			}
		}

		bool INISetting::Save(INIManager* Manager)
		{
			if (strlen(Manager->GetPath()) < 2)
				return false;
			
			char Buffer[0x200] = {0};
			Serialize(Buffer, sizeof(Buffer));

			if (!WritePrivateProfileStringA(Section.c_str(), Key.c_str(), Buffer, Manager->GetPath()))
				return false;
			else
				return true;
		}

		void INISetting::Load(INIManager* Manager)
		{
			if (strlen(Manager->GetPath()) < 2)
				return;

			char Buffer[0x200] = {0}, Default[0x200] = {0};
			Serialize(Default, sizeof(Default));

			GetPrivateProfileStringA(Section.c_str(), Key.c_str(), Default, Buffer, sizeof(Buffer), Manager->GetPath());
			Deserialize(Buffer);
		}

		INISetting::INISetting( const char* Key, const char* Section, const char* Description, SInt32 Value ) :
			Key(Key),
			Section(Section),
			Description(Description),
			Type(kType_Integer)
		{
			Data.i = Value;
		}

		INISetting::INISetting( const char* Key, const char* Section, const char* Description, UInt32 Value ) :
			Key(Key),
			Section(Section),
			Description(Description),
			Type(kType_Unsigned)
		{
			Data.u = Value;
		}

		INISetting::INISetting( const char* Key, const char* Section, const char* Description, float Value ) :
			Key(Key),
			Section(Section),
			Description(Description),
			Type(kType_Float)
		{
			Data.f = Value;
		}

		INISetting::INISetting( const char* Key, const char* Section, const char* Description, const char* Value ) :
			Key(Key),
			Section(Section),
			Description(Description),
			Type(kType_String)
		{
			Data.s = NULL;
			SetCString(&Data.s, Value);
		}

		INISetting::~INISetting()
		{
			if (Type == kType_String)
				SAFEDELETE_ARRAY(Data.s);
		}

		bool INISetting::operator<( const INISetting& Second )
		{
			int Result = _stricmp(Section.c_str(), Second.Section.c_str());
	//		if (Result == 0)
	//			Result = _stricmp(Key.c_str(), Second.Key.c_str());

			return Result >= 0;
		}

		const char* INISetting::GetKey( void ) const
		{
			return Key.c_str();
		}

		const char* INISetting::GetSection( void ) const
		{
			return Section.c_str();
		}

		const char* INISetting::GetDescription( void ) const
		{
			return Description.c_str();
		}

		const UInt8 INISetting::GetType( void ) const
		{
			return Type;
		}

		const INISetting::Store& INISetting::GetData( void ) const
		{
			return Data;
		}

		void INISetting::SetInt( SInt32 Value )
		{
			SME_ASSERT(Type == kType_Integer);

			Data.i = Value;
		}

		void INISetting::SetUInt( UInt32 Value )
		{
			SME_ASSERT(Type == kType_Unsigned);

			Data.u = Value;
		}

		void INISetting::SetFloat( float Value )
		{
			SME_ASSERT(Type == kType_Float);

			Data.f = Value;
		}

		void INISetting::SetString( const char* Format, ... )
		{
			SME_ASSERT(Type == kType_String);

			char Buffer[0x200] = {0};

			va_list Args;
			va_start(Args, Format);
			vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
			va_end(Args);

			SetCString(&Data.s, Buffer);
		}

		void INISetting::GetDataAsString( char* OutBuffer, UInt32 Size ) const
		{
			Serialize(OutBuffer, Size);
		}

		void INISetting::SetDataAsString( const char* Source )
		{
			Deserialize(Source);
		}

		bool INISetting::ToggleData( void )
		{
			SME_ASSERT(Type == kType_Integer || Type == kType_Unsigned);

			switch (Type)
			{
			case kType_Integer:
				Data.i = (Data.i == 0);
				return Data.i;
			case kType_Unsigned:
				Data.u = (Data.u == 0);
				return Data.u;
			}

			return false;
		}

		bool INIManager::SortComparator( INISetting* First, INISetting* Second )
		{
			return !(*First < *Second);
		}

		bool INIManager::RegisterSetting( INISetting* Setting, bool AutoLoad /*= true*/, bool Dynamic /*= false*/ )
		{
			SME_ASSERT(Setting);

			if (FetchSetting(Setting->GetKey(), Setting->GetSection()))
				return false;

			if (AutoLoad)
				Setting->Load(this);

			if (Dynamic == false)
			{
				StaticSettings.push_back(Setting);
				StaticSettings.sort(SortComparator);
			}
			else
			{
				DynamicSettings.push_back(Setting);
				DynamicSettings.sort(SortComparator);
			}

			return true;
		}


		INIManager::INIManager() :
			INIFilePath(),
			StaticSettings(),
			DynamicSettings()
		{
			;//
		}

		INIManager::~INIManager()
		{
			// don't save registered settings here, call the save method explicitly when required
			// will cause undefined behaviour if the settings and the manager are statically allocated
			// as the d'tor thunks can get called out of order, i.e., the settings get destroyed before manager

			for (INISettingListT::iterator Itr = DynamicSettings.begin(); Itr != DynamicSettings.end(); Itr++)
				delete *Itr;

			StaticSettings.clear();
			DynamicSettings.clear();
		}

		INISetting* INIManager::FetchSetting( const char* Key, const char* Section, bool Dynamic /*= false*/ )
		{
			if (Dynamic == false)
			{
				for (INISettingListT::const_iterator Itr = StaticSettings.begin(); Itr != StaticSettings.end(); Itr++)
				{
					if (!_stricmp((*Itr)->GetKey(), Key) && !_stricmp((*Itr)->GetSection(), Section))
					{
						return *Itr;
					}
				}
			}
			else
			{
				for (INISettingListT::const_iterator Itr = DynamicSettings.begin(); Itr != DynamicSettings.end(); Itr++)
				{
					if (!_stricmp((*Itr)->GetKey(), Key) && !_stricmp((*Itr)->GetSection(), Section))
					{
						return *Itr;
					}
				}
			}

			return NULL;
		}

		void INIManager::Save(void)
		{
			for (INISettingListT::const_iterator Itr = StaticSettings.begin(); Itr != StaticSettings.end(); Itr++)
			{
				(*Itr)->Save(this);
			}
		}

		void INIManager::Load(void)
		{
			for (INISettingListT::const_iterator Itr = StaticSettings.begin(); Itr != StaticSettings.end(); Itr++)
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

								INISetting* NewSetting = new INISetting(Key.c_str(), Section, "", Value.c_str());

								if (RegisterSetting(NewSetting, false, true) == false)
									delete NewSetting;
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

						INISetting* NewSetting = new INISetting(Key.c_str(), Section, "", Value.c_str());

						if (RegisterSetting(NewSetting, false, true) == false)
							delete NewSetting;
					}
				}

				Result = true;
			}

			return Result;
		}

		void INIManager::SetSettingSection( INISetting* Setting, const char* Section )
		{
			SME_ASSERT(Setting);

			Setting->Section = Section;
		}

		INIManagerIterator::INIManagerIterator( INIManager* Manager, bool DynamicList /*= false*/, const char* Section /*= NULL*/ ) :
			SettingList(NULL),
			Manager(Manager),
			Section(Section)
		{
			SME_ASSERT(Manager);

			SettingList = &Manager->StaticSettings;
			if (DynamicList)
				SettingList = &Manager->DynamicSettings;

			Current = Bookend = SettingList->end();
			Setting = NULL;

			if (SettingList->size())
			{
				Current = SettingList->begin();
				Setting = *Current;
			}
		}

		const INISetting* INIManagerIterator::GetNextSetting()
		{
			Setting = NULL;

			if (Current != SettingList->end())
			{
				while (++Current != SettingList->end())
				{
					INISetting* Now = *Current;

					if (Section == NULL || !_stricmp(Now->GetSection(), Section))
					{
						Setting = Now;
						break;
					}
				}
			}

			return Setting;
		}

		INIManagerIterator::~INIManagerIterator()
		{
			Setting = NULL;
			Manager = NULL;
		}

		bool INIManagerIterator::GetDone() const
		{
			return Setting == NULL;
		}

		const INISetting* INIManagerIterator::GetCurrentSetting() const
		{
			return Setting;
		}

		const INISetting* INIManagerIterator::operator()() const
		{
			return Setting;
		}
	}
}