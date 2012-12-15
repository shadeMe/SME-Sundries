#pragma once

// SME::INI - A simple INI managing class that allows INI settings to be registered to a database.

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

			const char*					GetKey(void) const;
			const char*					GetSection(void) const;
			const char*					GetDescription(void) const;
			const UInt8					GetType(void) const;

			const Store&				GetData(void) const;
			
			void						SetInt(SInt32 Value);
			void						SetUInt(UInt32 Value);
			void						SetFloat(float Value);
			void						SetString(const char* Format, ...);

			void						GetDataAsString(char* OutBuffer, UInt32 Size) const;
			void						SetDataAsString(const char* Source);

			bool						ToggleData(void);
		};

		typedef std::list<INISetting*>							INISettingListT;

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
	}
}