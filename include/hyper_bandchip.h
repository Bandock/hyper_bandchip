#ifndef _HYPER_BANDCHIP_H_
#define _HYPER_BANDCHIP_H_

#include <vector>
#include <map>
#include <chrono>
#include <filesystem>
#include <SDL.h>
#include <memory>
#include "fonts.h"
#include "menu.h"
#include "machine.h"
#include "cbf.h"

namespace Hyper_BandCHIP
{
	class Renderer;

	enum class MenuKey { Up, Down, Left, Right, Select, Exit };

	enum class CHIP8Key {
		Key_0, Key_1, Key_2, Key_3,
		Key_4, Key_5, Key_6, Key_7,
		Key_8, Key_9, Key_A, Key_B,
		Key_C, Key_D, Key_E, Key_F
	};

	enum class OperationMode { Menu = 0, Emulator = 1 };
	enum class ProgramType { Raw, CHIP8BinaryProgram };
	enum class MenuDisplay {
		Main, LoadProgram, LoadCHIP8BinaryProgram, Configuration, LoadProgramDisplay, LoadCHIP8BinaryProgramDisplay, CPUSettings, Behaviors, PaletteSettings,
		FontSettings, KeyboardRemapping, LoadConfiguration, SaveConfiguration, ErrorDisplay
	};

	enum class LoResFontStyle { VIP, SuperCHIP, KCHIP8 };
	enum class HiResFontStyle { SuperCHIP, Octo, KCHIP8 };

	enum class DirEntryType { Directory, File };

	enum class MainMenuEvent { RunProgram, LoadProgram, LoadCHIP8BinaryProgram, Configuration, Exit };
	enum class LoadProgramMenuEvent { ChangeDirectory, Load };
	enum class LoadCHIP8BinaryProgramMenuEvent { ChangeDirectory, Load };
	enum class ConfigurationMenuEvent {
		ChangeCore, CPUSettings, Behaviors, PaletteSettings, FontSettings, KeyboardRemapping, LoadConfiguration, ReturnToMainMenu
	};
	enum class LoadProgramDisplayMenuEvent { Ok };
	enum class LoadCHIP8BinaryProgramDisplayMenuEvent { Ok };
	enum class CPUSettingsMenuEvent { CommitChanges, ReturnToConfiguration };
	enum class BehaviorMenuEvent { ReturnToConfiguration };
	enum class PaletteSettingsMenuEvent { CommitChanges, ReturnToConfiguration };
	enum class FontSettingsMenuEvent { ChangeLoResFont, ChangeHiResFont, ReturnToConfiguration };
	enum class KeyboardRemappingMenuEvent { ReturnToConfiguration };
	enum class LoadConfigurationMenuEvent { Load };
	enum class SaveConfigurationMenuEvent { };

	struct MainMenuData
	{
		const TextItem Title = { "Hyper BandCHIP V0.15", 220, 20, false };
		const TextItem Author = { "By Joshua Moss", 250, 34, false };
		StatusTextItem CurrentProgram = { "Current Program", 160, 60, "None", false };
		StatusTextItem CurrentMachineStatus = { "Current Machine Status", 120, 74, "Non-Operational", false };
		const ButtonItem RunProgram = { "Run Program", 200, 120, static_cast<unsigned int>(MainMenuEvent::RunProgram), false };
		const ButtonItem LoadProgram = { "Load Program", 200, 134, static_cast<unsigned int>(MainMenuEvent::LoadProgram), false };
		const ButtonItem LoadCHIP8BinaryProgram = { "Load CHIP-8 Binary Program", 200, 148, static_cast<unsigned int>(MainMenuEvent::LoadCHIP8BinaryProgram), false };
		const ButtonItem Configuration = { "Configuration", 200, 162, static_cast<unsigned int>(MainMenuEvent::Configuration), false };
		const ButtonItem Exit = { "Exit Hyper BandCHIP", 200, 176, static_cast<unsigned int>(MainMenuEvent::Exit), false };
		unsigned int CurrentSelectableItemId = 0;
	};

	struct DirectoryEntryData
	{
		ButtonItem Entry;
		DirEntryType Type;
	};

	struct LoadProgramMenuData
	{
		const TextItem Title = { "Load Program", 250, 20, false };
		DirectoryEntryData MenuEntry[16];
		unsigned int CurrentSelectableItemId = 0;
		unsigned int CurrentDirectoryCount = 1;
		unsigned int StartEntry = 0;
	};

	struct LoadCHIP8BinaryProgramMenuData
	{
		const TextItem Title = { "Load CHIP-8 Binary Program", 190, 20, false };
		DirectoryEntryData MenuEntry[16];
		unsigned int CurrentSelectableItemId = 0;
		unsigned int CurrentDirectoryCount = 1;
		unsigned int StartEntry = 0;
	};

	struct ConfigurationMenuData
	{
		const TextItem Title = { "Configuration", 240, 20, false };
		MultiChoiceItem Core = { "Core", 200, 60, static_cast<unsigned int>(ConfigurationMenuEvent::ChangeCore), 0, { "CHIP-8", "SuperCHIP", "XO-CHIP", "HyperCHIP-64", "Pipelined CHIP-8" }, false };
		const ButtonItem CPUSettings = { "CPU Settings", 200, 74, static_cast<unsigned int>(ConfigurationMenuEvent::CPUSettings), false };
		const ButtonItem Behaviors = { "Behaviors", 200, 88, static_cast<unsigned int>(ConfigurationMenuEvent::Behaviors), false };
		const ButtonItem PaletteSettings = { "Palette Settings", 200, 102, static_cast<unsigned int>(ConfigurationMenuEvent::PaletteSettings), false };
		const ButtonItem FontSettings { "Font Settings", 200, 116, static_cast<unsigned int>(ConfigurationMenuEvent::FontSettings), false };
		const ButtonItem KeyboardRemapping = { "Keyboard Remapping", 200, 130, static_cast<unsigned int>(ConfigurationMenuEvent::KeyboardRemapping), false };
		const ButtonItem LoadConfiguration = { "Load Configuration", 200, 144, static_cast<unsigned int>(ConfigurationMenuEvent::LoadConfiguration), false };
		const ButtonItem SaveConfiguration = { "Save Configuration", 200, 158, 0xFFFFFFFF, false };
		const ButtonItem ReturnToMainMenu = { "Return to Main Menu", 200, 172, static_cast<unsigned int>(ConfigurationMenuEvent::ReturnToMainMenu), false };
		unsigned int CurrentSelectableItemId = 0;
	};

	struct LoadProgramDisplayData
	{
		TextItem LoadingProgram = { "Loading program...", 240, 140, true };
		TextItem LoadSuccessful = { "Load Successful!", 240, 140, true };
		TextItem LoadFailed = { "Load Failed.", 250, 140, true };
		ButtonItem Ok = { "Ok", 300, 180, static_cast<unsigned int>(LoadProgramDisplayMenuEvent::Ok), true };
	};

	struct LoadCHIP8BinaryProgramDisplayData
	{
		TextItem LoadingCHIP8BinaryProgram = { "Loading CHIP-8 Binary Program...", 170, 140, true };
		StatusTextItem ProgramName { "Program Name", 20, 10, "", true };
		StatusTextItem ProgramAuthor { "Program Author", 20, 24, "", true };
		StatusTextItem Version { "Format Version", 20, 38, "0", true };
		TextItem NotAValidCHIP8BinaryProgram = { "Not a valid CHIP-8 Binary Program.", 170, 140, true };
		ButtonItem Ok = { "Ok", 300, 180, static_cast<unsigned int>(LoadCHIP8BinaryProgramDisplayMenuEvent::Ok), true };
	};

	struct CPUSettingsMenuData
	{
		const TextItem Title = { "CPU Settings", 266, 20, false };
		StatusTextItem ChangeStatus = { "Change Status", 200, 40, "Unchanged", false };
		AdjustableValueItem CPUCycles = { "CPU Cycles", 200, 60, 0, 60000000, ValueBaseType::Dec, 600, 0, false };
		AdjustableValueItem AdjustmentModifier { "Adjustment Modifier", 200, 74, 60, 600000, ValueBaseType::Dec, 60, 0, false };
		ToggleItem Sync { "Sync", 200, 88, true, false };
		ToggleItem SuperscalarMode { "Superscalar Mode", 200, 102, false, false };
		const ButtonItem CommitChanges = { "Commit Changes", 200, 176, static_cast<unsigned int>(CPUSettingsMenuEvent::CommitChanges), false };
		const ButtonItem ReturnToConfiguration = { "Return to Configuration", 200, 190, static_cast<unsigned int>(CPUSettingsMenuEvent::ReturnToConfiguration), false };
		unsigned int CurrentSelectableItemId = 0;
	};

	struct BehaviorsMenuData
	{
		const TextItem Title = { "Behaviors", 270, 20, false };
		ToggleItem CHIP48_Shift = { "CHIP-48 Shift", 200, 60, false, false };
		ToggleItem CHIP48_LoadStore = { "CHIP-48 Load/Store", 200, 74, false, false };
		ToggleItem VIP_Display_Interrupt = { "VIP Display Interrupt", 200, 88, false, false };
		ToggleItem VIP_Clipping = { "VIP Clipping", 200, 102, false, false };
		ToggleItem VIP_VF_Reset = { "VIP VF Reset", 200, 116, false, false };
		MultiChoiceItem SuperCHIP_Version = { "SuperCHIP Version", 140, 60, 0xFFFFFFFF, 0, { "Fixed SuperCHIP V1.1", "Original SuperCHIP V1.0", "SuperCHIP V1.0", "Original SuperCHIP V1.1" }, false };
		ToggleItem SuperCHIP_Shift = { "SuperCHIP Shift", 200,60, false, false };
		ToggleItem SuperCHIP_LoadStore = { "SuperCHIP Load/Store", 200, 74, false, false };
		ToggleItem Octo_LoResSprite = { "Octo LoRes Sprite Mode", 200, 88, true, false };
		const ButtonItem ReturnToConfiguration = { "Return to Configuration", 200, 190, static_cast<unsigned int>(BehaviorMenuEvent::ReturnToConfiguration), false };
		unsigned int CurrentSelectableItemId = 0;
	};

	struct PaletteSettingsMenuData
	{
		const TextItem Title = { "Palette Settings", 250, 20, false };
		AdjustableValueItem CurrentIndex = { "Current Index", 200, 60, 0, 15, ValueBaseType::Dec, 0, 0, false };
		AdjustableValueItem Red = { "Red", 200, 74, 0, 255, ValueBaseType::Dec, 0, 0, false };
		AdjustableValueItem Green = { "Green", 200, 88, 0, 255, ValueBaseType::Dec, 0, 0, false };
		AdjustableValueItem Blue = { "Blue", 200, 102, 0, 255, ValueBaseType::Dec, 0, 0, false };
		const ButtonItem CommitChanges = { "Commit Changes", 200, 176, static_cast<unsigned int>(PaletteSettingsMenuEvent::CommitChanges), false };
		const ButtonItem ReturnToConfiguration = { "Return to Configuration", 200, 190, static_cast<unsigned int>(PaletteSettingsMenuEvent::ReturnToConfiguration), false };
		unsigned int CurrentSelectableItemId = 0;
	};

	struct FontSettingsMenuData
	{
		const TextItem Title = { "Font Settings", 260, 20, false };
		MultiChoiceItem LoResFontStyle = { "LoRes Font Style", 200, 60, static_cast<unsigned int>(FontSettingsMenuEvent::ChangeLoResFont), 0, { "VIP", "SuperCHIP", "KCHIP8" }, false };
		MultiChoiceItem HiResFontStyle = { "HiRes Font Style", 200, 74, static_cast<unsigned int>(FontSettingsMenuEvent::ChangeHiResFont), 0, { "SuperCHIP", "Octo", "KCHIP8" }, false };
		const ButtonItem ReturnToConfiguration = { "Return to Configuration", 200, 190, static_cast<unsigned int>(FontSettingsMenuEvent::ReturnToConfiguration), false };
		unsigned int CurrentSelectableItemId = 0;
	};

	struct KeyboardRemappingMenuData
	{
		const TextItem Title = { "Keyboard Remapping", 230, 20, false };
		InputItem<2> Keys[16] = {
		{ "0", 250, 74, false, "X" },
		{ "1", 250, 88, false, "1" },
		{ "2", 250, 102, false, "2" },
		{ "3", 250, 116, false, "3" },
		{ "4", 250, 130, false, "Q" },
		{ "5", 250, 144, false, "W" },
		{ "6", 250, 158, false, "E" },
		{ "7", 250, 172, false, "A" },
		{ "8", 350, 74, false, "S" },
		{ "9", 350, 88, false, "D" },
		{ "A", 350, 102, false, "Z" },
		{ "B", 350, 116, false, "C" },
		{ "C", 350, 130, false, "4" },
		{ "D", 350, 144, false, "R" },
		{ "E", 350, 158, false, "F" },
		{ "F", 350, 172, false, "V" }};
		const ButtonItem ReturnToConfiguration = { "Return to Configuration", 200, 230, static_cast<unsigned int>(KeyboardRemappingMenuEvent::ReturnToConfiguration), false };
		unsigned int CurrentSelectableItemId = 0;
		char previous_key = '\0';
		bool input_mode = false;
	};

	struct LoadConfigurationMenuData
	{
		const TextItem Title = { "Load Configuration", 230, 20, false };
		DirectoryEntryData MenuEntry[16];
		unsigned int CurrentSelectableItemId = 0;
		unsigned int CurrentConfigurationFileCount = 0;
		unsigned int StartEntry = 0;
	};

	struct SaveConfigurationMenuData
	{
		const TextItem Title = { "Save Configuration", 230, 20, false };
		InputItem<32> ConfigurationFileName = { "File Name (31 Character Limit)", 160, 40, false, "\0" };
	};

	struct ErrorDisplayData
	{
		StatusTextItem Error = { "Error", 100, 20, "", false };
		ValueItem RegisterValue[0x10] = {
			{ "V0", 100, 60, ValueBaseType::Hex, 0x00, 2, false },
			{ "V1", 100, 74, ValueBaseType::Hex, 0x00, 2, false },
			{ "V2", 100, 88, ValueBaseType::Hex, 0x00, 2, false },
			{ "V3", 100, 102, ValueBaseType::Hex, 0x00, 2, false },
			{ "V4", 100, 116, ValueBaseType::Hex, 0x00, 2, false },
			{ "V5", 100, 130, ValueBaseType::Hex, 0x00, 2, false },
			{ "V6", 100, 144, ValueBaseType::Hex, 0x00, 2, false },
			{ "V7", 100, 158, ValueBaseType::Hex, 0x00, 2, false },
			{ "V8", 100, 172, ValueBaseType::Hex, 0x00, 2, false },
			{ "V9", 100, 186, ValueBaseType::Hex, 0x00, 2, false },
			{ "VA", 100, 200, ValueBaseType::Hex, 0x00, 2, false },
			{ "VB", 100, 214, ValueBaseType::Hex, 0x00, 2, false },
			{ "VC", 100, 228, ValueBaseType::Hex, 0x00, 2, false },
			{ "VD", 100, 242, ValueBaseType::Hex, 0x00, 2, false },
			{ "VE", 100, 256, ValueBaseType::Hex, 0x00, 2, false },
			{ "VF", 100, 270, ValueBaseType::Hex, 0x00, 2, false }};
		ValueItem ProgramCounterValue = { "PC", 300, 60, ValueBaseType::Hex, 0x0000, 4, false };
		ValueItem AddressRegisterValue = { "I", 300, 74, ValueBaseType::Hex, 0x0000, 4, false };
		ValueItem DelayTimerValue = { "DT", 300, 88, ValueBaseType::Dec, 0, false };
		ValueItem SoundTimerValue[0x4] = {
			{ "ST0", 300, 102, ValueBaseType::Dec, 0, false },
			{ "ST1", 300, 116, ValueBaseType::Dec, 0, false },
			{ "ST2", 300, 130, ValueBaseType::Dec, 0, false },
			{ "ST3", 300, 144, ValueBaseType::Dec, 0, false }
		};
	};

	struct WindowDeleter
	{
		void operator()(SDL_Window *Window)
		{
			SDL_DestroyWindow(Window);
		}
	};

	class Application
	{
		public:
			Application();
			~Application();
			
			inline int GetReturnCode() const
			{
				return retcode;
			}
			
			void InitializeKeyMaps();
			void ConstructMenus();
			void ShowMenu(MenuDisplay Menu);
		private:
			void LoadFontStyles();
			void ChangeDirectory(DirectoryEntryData &DirItem);
			bool StartCHIP8BinaryProgram();

			std::unique_ptr<SDL_Window, WindowDeleter> MainWindow;
			std::unique_ptr<Renderer> MainRenderer;
			std::filesystem::path start_path;
			std::map<MenuKey, SDL_Scancode> Menu_KeyMap;
			std::map<CHIP8Key, SDL_Scancode> CHIP8_KeyMap;
			std::map<char, SDL_Scancode> Input_KeyMap;
			OperationMode CurrentOperationMode;
			ProgramType CurrentProgramType;
			MachineCore CurrentMachineCore;
			MenuDisplay CurrentMenu;
			std::vector<Fonts::FontStyle<4, 5>> LoResFontStyleList;
			std::vector<Fonts::FontStyle<8, 10>> HiResFontStyleList;
			LoResFontStyle CurrentLoResFontStyle;
			HiResFontStyle CurrentHiResFontStyle;
			MainMenuData MainMenu;
			LoadProgramMenuData LoadProgramMenu;
			LoadCHIP8BinaryProgramMenuData LoadCHIP8BinaryProgramMenu;
			ConfigurationMenuData ConfigurationMenu;
			LoadProgramDisplayData LoadProgramDisplay;
			LoadCHIP8BinaryProgramDisplayData LoadCHIP8BinaryProgramDisplay;
			CPUSettingsMenuData CPUSettingsMenu;
			BehaviorsMenuData BehaviorsMenu;
			PaletteSettingsMenuData PaletteSettingsMenu;
			FontSettingsMenuData FontSettingsMenu;
			KeyboardRemappingMenuData KeyboardRemappingMenu;
			LoadConfigurationMenuData LoadConfigurationMenu;
			SaveConfigurationMenuData SaveConfigurationMenu;
			ErrorDisplayData ErrorDisplay;
			std::unique_ptr<BandCHIP::CHIP8_Machine> CurrentCHIP8Machine;
			std::unique_ptr<BandCHIP::SuperCHIP_Machine> CurrentSuperCHIPMachine;
			std::unique_ptr<BandCHIP::XOCHIP_Machine> CurrentXOCHIPMachine;
			std::unique_ptr<BandCHIP::HyperCHIP64_Machine> CurrentHyperCHIP64Machine;
			std::unique_ptr<BandCHIP::Superscalar_CHIP8_Machine> CurrentSuperscalarCHIP8Machine;
			std::unique_ptr<CBF::Program> CurrentProgram;
			std::chrono::high_resolution_clock::time_point refresh_tp;
			double refresh_accumulator;
			std::chrono::high_resolution_clock::time_point cpf_tp;
			double cpf_accumulator;
			bool loading_program;
			bool loading_chip8_binary_program;
			bool chip8_binary_program_started;
			int retcode;
	};
}

#endif
