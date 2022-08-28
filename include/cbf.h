#ifndef _CBF_H_
#define _CBF_H_

#include <array>
#include <vector>
#include <string>
#include <variant>

namespace Hyper_BandCHIP
{
	namespace CBF
	{
		enum class PlatformType : unsigned char
		{
			CHIP8 = 0x01,
			CHIP8_12 = 0x02,
			CHIP8_I = 0x03,
			CHIP8_II = 0x04,
			CHIP8_III = 0x05,
			CHIP8_2PageDisplay = 0x06,
			CHIP8_C = 0x07,
			CHIP10 = 0x08,
			CHIP8_SaveRestoreVariableMod = 0x09,
			Improved_CHIP8_SaveRestoreVariableMod = 0x0A,
			CHIP8_RelativeBranchingMod = 0x0B,
			Another_CHIP8_RelativeBranchingMod = 0x0C,
			CHIP8_FastSingleDotDrawMod = 0x0D,
			CHIP8_IOPortDriver = 0x0E,
			CHIP8_8BitMultiplyAndDivide = 0x0F,
			HIRES_CHIP8 = 0x10,
			HIRES_CHIP8_IOPortDriver = 0x11,
			HIRES_CHIP8_PageSwitching = 0x12,
			CHIP8E = 0x13,
			CHIP8_Improved_BNNN = 0x14,
			CHIP8_Scrolling_Routine = 0x15,
			CHIP8X = 0x16,
			CHIP8X_TwoPageDisplay = 0x17,
			HiRes_CHIP8x = 0x18,
			CHIP8Y = 0x19,
			CHIP8_CopyToScreen = 0x1A,
			CHIP_BETA = 0x1B,
			CHIP8M = 0x1C,
			MultipleNimInterpreter = 0x1D,
			DoubleArrayModification = 0x1E,
			CHIP8_DREAM6800 = 0x1F,
			CHIP8_DREAM6800_LogicOps = 0x20,
			CHIP8_DREAM6800_Joystick = 0x21,
			CHIPOS2K_DREAM6800 = 0x22,
			CHIP8_ETI660 = 0x23,
			CHIP8_ETI660_Color = 0x24,
			CHIP8_ETI660_HiRes = 0x25,
			CHIP8_COSMAC_ELF = 0x26,
			CHIP8_VDU = 0x27,
			CHIP8_AE = 0x28,
			Dreamcards_Extended_CHIP8_V20 = 0x29, 
			CHIP8_Amiga = 0x2A,
			CHIP48 = 0x2B,
			SuperCHIP10 = 0x2C,
			SuperCHIP11 = 0x2D,
			GCHIP = 0x2E,
			SCHIP_GCHIP_Compatibility = 0x2F,
			VIP2K_CHIP8 = 0x30,
			SCHIP_ScrollUp = 0x31,
			chip8run = 0x32,
			Mega_Chip = 0x33,
			XO_CHIP = 0x34,
			Octo = 0x35,
			CHIP8_Classic_Color =  0x36
		};

		enum class PropertyType : unsigned char
		{
			DesiredExecutionSpeed = 0x01,
			ProgramName = 0x02,
			ProgramDescription = 0x03,
			ProgramAuthor = 0x04,
			ProgramURL = 0x05,
			ProgramReleaseDate = 0x06,
			CoverArt = 0x07,
			KeyInputConfiguration = 0x08,
			ColorConfiguration = 0x09,
			ScreenOrientation = 0x0A,
			FontData = 0x0B,
			ToolVanity = 0x0C,
			LicenseInformation = 0x0D
		};

		enum class ScreenOrientation : unsigned char
		{
			TopUp = 0x00, LeftUp = 0x01, RightUp = 0x02, BottomUp = 0x03
		};

		struct CoverArtData
		{
			unsigned char planes;
			unsigned char width;
			unsigned char height;
			std::vector<unsigned char> data;
		};

		struct KeyInputData
		{
			unsigned char host_key;
			unsigned char chip8_key;
		};

		struct ColorData
		{
			unsigned char r;
			unsigned char g;
			unsigned char b;
		};

		struct FontData
		{
			unsigned short address;
			std::vector<unsigned char> data;
		};

		struct Header
		{
			std::array<unsigned char, 3> magic_number;
			unsigned char version;
			unsigned char bytecode_table_ptr;
			unsigned char properties_table_ptr;
		};

		struct BytecodeTableEntry
		{
			PlatformType platform;
			std::vector<unsigned char> code;
		};

		struct PropertiesTableEntry
		{
			PropertyType type;
			std::variant<unsigned int, std::string, CoverArtData, std::vector<KeyInputData>, std::vector<ColorData>, ScreenOrientation, FontData> data;
		};

		class Program
		{
			public:
				Program(std::vector<unsigned char> &&program_data);
				~Program();

				unsigned char GetVersion() const;
				std::vector<unsigned char> *GetBytecodeData(PlatformType platform);
				bool HasPropertyType(PropertyType type) const;
				unsigned int GetDesiredExecutionSpeed() const;
				std::string GetProgramName() const;
				std::string GetProgramDescription() const;
				std::vector<std::string> GetProgramAuthors() const;
				std::vector<std::string> GetProgramURLs() const;
				unsigned int GetProgramReleaseDate() const;
				std::vector<ColorData> *GetColorConfiguration();
				FontData *GetFontData();
				std::string GetToolVanity() const;
				std::string GetLicenseInformation() const;
				
				inline bool IsValid() const
				{
					return valid;
				}
			private:
				Header header;
				std::vector<BytecodeTableEntry> bytecode_table;
				std::vector<PropertiesTableEntry> properties_table;
				bool valid;
		};
	}
}

#endif
