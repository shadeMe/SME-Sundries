// SME::INI - INI managing classes and associated bollocks

#ifndef __SME_SUNDRIES_INIMANAGER_H__
#define __SME_SUNDRIES_INIMANAGER_H__

#include "SME_Prefix.h"

#pragma warning(push)
#pragma warning(disable: 4800)

namespace SME
{
	namespace INI
	{
		class INIManager;
		class INIManagerIterator;

		class INISetting
		{
		public:
			enum
			{
				kType_Integer	= 0,
				kType_Unsigned,			// interpreted as hexadecimals
				kType_Float,
				kType_String
			};
		private:
			void						SetCString(char** Dest, const char* Source);		// dest must be a dynamically allocated array

			void						Serialize(char* Dest, UInt32 Size) const;
			void						Deserialize(const char* Source);

			union Store
			{
				SInt32					i;
				UInt32					u;
				float					f;
				char*					s;
			};

			const UInt8					Type;
			Store						Data;

			std::string					Key;
			std::string					Section;
			std::string					Description;

			bool						Save(INIManager* Manager);
			void						Load(INIManager* Manager);

			friend class INIManager;
		public:
			INISetting(const char* Key, const char* Section, const char* Description, SInt32 Value);
			INISetting(const char* Key, const char* Section, const char* Description, UInt32 Value);
			INISetting(const char* Key, const char* Section, const char* Description, float Value);
			INISetting(const char* Key, const char* Section, const char* Description, const char* Value);
			~INISetting();

			bool						operator<(const INISetting& Second);
			const Store&				operator()(void) const;

			const Store&				GetData(void) const;
			const char*					GetKey(void) const;
			const char*					GetSection(void) const;
			const char*					GetDescription(void) const;
			const UInt8					GetType(void) const;

			void						SetInt(SInt32 Value);
			void						SetUInt(UInt32 Value);
			void						SetFloat(float Value);
			void						SetString(const char* Format, ...);

			void						GetDataAsString(char* OutBuffer, UInt32 Size) const;
			void						SetDataAsString(const char* Source);

			bool						ToggleData(void);
		};

		typedef std::vector<INISetting*>						INISettingListT;

		class INIManager
		{
		protected:
			static bool											SortComparator(INISetting* First, INISetting* Second);
			static void											SetSettingSection(INISetting* Setting, const char* Section);

			std::string											INIFilePath;
			INISettingListT										StaticSettings;		// registered with RegisterSetting()
			INISettingListT										DynamicSettings;	// dynamically created by PopulateXXX(), destroyed with the manager

			// caller retains ownership of pointer for non-dynamic settings
			virtual bool										RegisterSetting(INISetting* Setting, bool AutoLoad = true, bool Dynamic = false);

			friend class										INIManagerIterator;
		public:
			INIManager();
			virtual ~INIManager();

			virtual void										Initialize(const char* INIPath, void* Parameter) = 0;
			virtual INISetting*									FetchSetting(const char* Key, const char* Section, bool Dynamic = false);

			void												Save(void);			// only saves static settings
			void												Load(void);
			const char*											GetPath() const;

			virtual int											DirectRead(const char* Setting, const char* Section, const char* Default, char* OutBuffer, UInt32 Size);
			virtual int											DirectRead(const char* Section, char* OutBuffer, UInt32 Size);
			virtual bool										DirectWrite(const char* Setting, const char* Section, const char* Value);
			virtual bool										DirectWrite(const char* Section, const char* Value);

			// new instances are created as string settings
			virtual bool										PopulateFromSection(const char* Section);
			virtual bool										PopulateFromINI(void);
		};

		class INIManagerIterator
		{
			INISettingListT*									SettingList;
			INISettingListT::const_iterator						Bookend;
			INISettingListT::const_iterator						Current;
			const char*											Section;

			INISetting*											Setting;
			INIManager*											Manager;
		public:
			INIManagerIterator(INIManager* Manager, bool DynamicList = false, const char* Section = NULL);
			~INIManagerIterator();

			bool												GetDone() const;
			const INISetting*									GetNextSetting();
			const INISetting*									GetCurrentSetting() const;

			const INISetting*									operator()() const;
		};

		inline void INISetting::SetCString( char** Dest, const char* Source )
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

		inline void INISetting::Serialize( char* Dest, UInt32 Size ) const
		{
			switch (Type)
			{
			case kType_Integer:
				_snprintf_s(Dest, Size, _TRUNCATE, "%d", Data.i);
				break;
			case kType_Unsigned:
				_snprintf_s(Dest, Size, _TRUNCATE, "%08X", Data.u);
				break;
			case kType_Float:
				_snprintf_s(Dest, Size, _TRUNCATE, "%f", Data.f);
				break;
			case kType_String:
				_snprintf_s(Dest, Size, _TRUNCATE, "%s", Data.s);
				break;
			}
		}

		inline void INISetting::Deserialize( const char* Source )
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

		inline bool INISetting::Save(INIManager* Manager)
		{
			if (strlen(Manager->GetPath()) < 2)
				return false;

			char Buffer[0x4000] = {0};
			Serialize(Buffer, sizeof(Buffer));

			if (!WritePrivateProfileStringA(Section.c_str(), Key.c_str(), Buffer, Manager->GetPath()))
				return false;
			else
				return true;
		}

		inline void INISetting::Load(INIManager* Manager)
		{
			if (strlen(Manager->GetPath()) < 2)
				return;

			char Buffer[0x4000] = {0}, Default[0x4000] = {0};
			Serialize(Default, sizeof(Default));

			GetPrivateProfileStringA(Section.c_str(), Key.c_str(), Default, Buffer, sizeof(Buffer), Manager->GetPath());
			Deserialize(Buffer);
		}

		inline INISetting::INISetting( const char* Key, const char* Section, const char* Description, SInt32 Value ) :
			Key(Key),
			Section(Section),
			Description(Description),
			Type(kType_Integer)
		{
			Data.i = Value;
		}

		inline INISetting::INISetting( const char* Key, const char* Section, const char* Description, UInt32 Value ) :
			Key(Key),
			Section(Section),
			Description(Description),
			Type(kType_Unsigned)
		{
			Data.u = Value;
		}

		inline INISetting::INISetting( const char* Key, const char* Section, const char* Description, float Value ) :
			Key(Key),
			Section(Section),
			Description(Description),
			Type(kType_Float)
		{
			Data.f = Value;
		}

		inline INISetting::INISetting( const char* Key, const char* Section, const char* Description, const char* Value ) :
			Key(Key),
			Section(Section),
			Description(Description),
			Type(kType_String)
		{
			Data.s = NULL;
			SetCString(&Data.s, Value);
		}

		inline INISetting::~INISetting()
		{
			if (Type == kType_String)
				SAFEDELETE_ARRAY(Data.s);
		}

		inline bool INISetting::operator<( const INISetting& Second )
		{
			int Result = _stricmp(Section.c_str(), Second.Section.c_str());
			//		if (Result == 0)
			//			Result = _stricmp(Key.c_str(), Second.Key.c_str());

			return Result >= 0;
		}

		inline const char* INISetting::GetKey( void ) const
		{
			return Key.c_str();
		}

		inline const char* INISetting::GetSection( void ) const
		{
			return Section.c_str();
		}

		inline const char* INISetting::GetDescription( void ) const
		{
			return Description.c_str();
		}

		inline const UInt8 INISetting::GetType( void ) const
		{
			return Type;
		}

		inline void INISetting::SetInt( SInt32 Value )
		{
			SME_ASSERT(Type == kType_Integer);

			Data.i = Value;
		}

		inline void INISetting::SetUInt( UInt32 Value )
		{
			SME_ASSERT(Type == kType_Unsigned);

			Data.u = Value;
		}

		inline void INISetting::SetFloat( float Value )
		{
			SME_ASSERT(Type == kType_Float);

			Data.f = Value;
		}

		inline void INISetting::SetString( const char* Format, ... )
		{
			SME_ASSERT(Type == kType_String);

			char Buffer[0x200] = {0};

			va_list Args;
			va_start(Args, Format);
			vsnprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, Format, Args);
			va_end(Args);

			SetCString(&Data.s, Buffer);
		}

		inline void INISetting::GetDataAsString( char* OutBuffer, UInt32 Size ) const
		{
			Serialize(OutBuffer, Size);
		}

		inline void INISetting::SetDataAsString( const char* Source )
		{
			Deserialize(Source);
		}

		inline bool INISetting::ToggleData( void )
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

		inline const INISetting::Store& INISetting::operator()( void ) const
		{
			return Data;
		}

		inline const INISetting::Store& INISetting::GetData( void ) const
		{
			return Data;
		}

		inline bool INIManager::SortComparator( INISetting* First, INISetting* Second )
		{
			return !(*First < *Second);
		}

		inline bool INIManager::RegisterSetting( INISetting* Setting, bool AutoLoad /*= true*/, bool Dynamic /*= false*/ )
		{
			SME_ASSERT(Setting);

			if (FetchSetting(Setting->GetKey(), Setting->GetSection()))
				return false;

			if (AutoLoad)
				Setting->Load(this);

			if (Dynamic == false)
			{
				StaticSettings.push_back(Setting);
				std::sort(StaticSettings.begin(), StaticSettings.end(), SortComparator);
			}
			else
			{
				DynamicSettings.push_back(Setting);
				std::sort(DynamicSettings.begin(), DynamicSettings.end(), SortComparator);
			}

			return true;
		}


		inline INIManager::INIManager() :
			INIFilePath(),
			StaticSettings(),
			DynamicSettings()
		{
			;//
		}

		inline INIManager::~INIManager()
		{
			// don't save registered settings here, call the save method explicitly when required
			// will cause undefined behaviour if the settings and the manager are statically allocated
			// as the d'tor thunks can get called out of order, i.e., the settings get destroyed before manager

			for (INISettingListT::iterator Itr = DynamicSettings.begin(); Itr != DynamicSettings.end(); Itr++)
				delete *Itr;

			StaticSettings.clear();
			DynamicSettings.clear();
		}

		inline INISetting* INIManager::FetchSetting( const char* Key, const char* Section, bool Dynamic /*= false*/ )
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

		inline void INIManager::Save(void)
		{
			for (INISettingListT::const_iterator Itr = StaticSettings.begin(); Itr != StaticSettings.end(); Itr++)
			{
				(*Itr)->Save(this);
			}
		}

		inline void INIManager::Load(void)
		{
			for (INISettingListT::const_iterator Itr = StaticSettings.begin(); Itr != StaticSettings.end(); Itr++)
			{
				(*Itr)->Load(this);
			}
		}

		inline const char* INIManager::GetPath() const
		{
			return INIFilePath.c_str();
		}

		inline int INIManager::DirectRead( const char* Setting, const char* Section, const char* Default, char* OutBuffer, UInt32 Size )
		{
			return GetPrivateProfileString(Section, Setting, Default, OutBuffer, Size, GetPath());
		}

		inline int INIManager::DirectRead( const char* Section, char* OutBuffer, UInt32 Size )
		{
			return GetPrivateProfileSection(Section, OutBuffer, Size, GetPath());
		}

		inline bool INIManager::DirectWrite( const char* Setting, const char* Section, const char* Value )
		{
			return WritePrivateProfileString(Section, Setting, Value, GetPath());
		}

		inline bool INIManager::DirectWrite( const char* Section, const char* Value )
		{
			return WritePrivateProfileSection(Section, Value, GetPath());
		}

		inline bool INIManager::PopulateFromINI( void )
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

		inline bool INIManager::PopulateFromSection( const char* Section )
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

		inline void INIManager::SetSettingSection( INISetting* Setting, const char* Section )
		{
			SME_ASSERT(Setting);

			Setting->Section = Section;
		}

		inline INIManagerIterator::INIManagerIterator( INIManager* Manager, bool DynamicList /*= false*/, const char* Section /*= NULL*/ ) :
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

		inline const INISetting* INIManagerIterator::GetNextSetting()
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

		inline INIManagerIterator::~INIManagerIterator()
		{
			Setting = NULL;
			Manager = NULL;
		}

		inline bool INIManagerIterator::GetDone() const
		{
			return Setting == NULL;
		}

		inline const INISetting* INIManagerIterator::GetCurrentSetting() const
		{
			return Setting;
		}

		inline const INISetting* INIManagerIterator::operator()() const
		{
			return Setting;
		}
	}
}

#pragma warning(pop)
#endif