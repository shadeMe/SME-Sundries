#pragma once

// SME::INI - A simple INI managing class that allows INI settings to be registered to a database.

namespace SME
{
	namespace INI
	{
		class INISetting;
		class INIManagerIterator;

		class INIManager
		{
		protected:
			static bool											SortComparator(INISetting* First, INISetting* Second);

			typedef std::list<INISetting*>						INISettingListT;

			std::string											INIFilePath;
			INISettingListT										SettingList;

			virtual bool										RegisterSetting(const char* Key, const char* Section, const char* DefaultValue, const char* Description);

			friend class										INIManagerIterator;
		public:
			INIManager();
			virtual ~INIManager();

			virtual void										Initialize(const char* INIPath, void* Paramenter) = 0;
			virtual INISetting*									FetchSetting(const char* Key, const char* Section);

			void												Save(void);
			void												Load(void);
			const char*											GetPath() const;

			virtual int											DirectRead(const char* Setting, const char* Section, const char* Default, char* OutBuffer, UInt32 Size);
			virtual int											DirectRead(const char* Section, char* OutBuffer, UInt32 Size);
			virtual bool										DirectWrite(const char* Setting, const char* Section, const char* Value);
			virtual bool										DirectWrite(const char* Section, const char* Value);
		};

		#define GetINI(key, section)							FetchSetting(key, section)
		#define GetINIStr(key, section)							FetchSetting(key, section)->GetValueAsString()
		#define GetINIInt(key, section)							FetchSetting(key, section)->GetValueAsInteger()
		#define GetINIFlt(key, section)							FetchSetting(key, section)->GetValueAsFloat()
		#define SetINI(key, section, value)						FetchSetting(key, section)->SetValue(value)

		class INISetting
		{
		protected:
			std::string											Value;
			std::string											Key;
			std::string											Section;
			std::string											DefaultValue;
			std::string											Description;

			bool												Save(INIManager* Manager);
			void												Load(INIManager* Manager);

			friend class										INIManager;
		public:
			INISetting(INIManager* Manager, const char* Key, const char* Section, const char* DefaultValue, const char* Description);
			virtual ~INISetting();

			virtual bool										operator<(const INISetting& Second);

			const char*											GetDescription(void) const;
			const char*											GetKey(void) const;
			const char*											GetSection(void) const;
			const char*											GetValueAsString(void) const;
			int													GetValueAsInteger(void) const;
			float												GetValueAsFloat(void) const;
			void												SetValue(const char* Format, ...);
		};

		class INIManagerIterator
		{
			UInt32												INIListSize;
			INIManager::INISettingListT::const_iterator			Bookend;
			INIManager::INISettingListT::const_iterator			Iterator;

			INISetting*											CurrentSetting;
			INIManager*											Manager;
		public:
			INIManagerIterator(INIManager* Manager);
			virtual ~INIManagerIterator();

			bool												GetDone() const;
			const INISetting*									GetNextSetting();
			const INISetting*									GetCurrentSetting() const;
		};
	}
}