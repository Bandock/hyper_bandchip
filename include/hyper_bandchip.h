#ifndef _HYPER_BANDCHIP_H_
#define _HYPER_BANDCHIP_H_

#include <map>
#include <chrono>
#include <filesystem>
#include <SDL.h>
#include "menu.h"
#include "machine.h"

namespace Hyper_BandCHIP
{
	class Renderer;
	template <int size, int character_count>
	class Font;

	enum class MenuKey { Up, Down, Left, Right, Select, Exit };

	enum class CHIP8Key {
		Key_0, Key_1, Key_2, Key_3,
		Key_4, Key_5, Key_6, Key_7,
		Key_8, Key_9, Key_A, Key_B,
		Key_C, Key_D, Key_E, Key_F
	};

	enum class OperationMode { Menu = 0, Emulator = 1 };
	enum class MenuDisplay { Main, LoadProgram, Configuration, LoadProgramDisplay, Behaviors, PaletteSettings, KeyboardRemapping, ErrorDisplay };

	enum class DirEntryType { Directory, File };

	enum class MainMenuEvent { RunProgram, LoadProgram, Configuration, Exit };
	enum class LoadProgramMenuEvent { ChangeDirectory, Load };
	enum class ConfigurationMenuEvent { ChangeCore, Behaviors, PaletteSettings, KeyboardRemapping, ReturnToMainMenu };
	enum class LoadProgramDisplayMenuEvent { Ok };
	enum class BehaviorMenuEvent { ReturnToConfiguration };
	enum class PaletteSettingsMenuEvent { CommitChanges, ReturnToConfiguration };
	enum class KeyboardRemappingMenuEvent { ReturnToConfiguration };

	struct MainMenuData
	{
		const TextItem Title = { "Hyper BandCHIP V0.7", 220, 20, false };
		const TextItem Author = { "By Joshua Moss", 250, 34, false };
		StatusTextItem CurrentProgram = { "Current Program", 160, 60, "None", false };
		StatusTextItem CurrentMachineStatus = { "Current Machine Status", 120, 74, "Non-Operational", false };
		const ButtonItem RunProgram = { "Run Program", 200, 120, static_cast<unsigned int>(MainMenuEvent::RunProgram), false };
		const ButtonItem LoadProgram = { "Load Program", 200, 134, static_cast<unsigned int>(MainMenuEvent::LoadProgram), false };
		const ButtonItem Configuration = { "Configuration", 200, 148, static_cast<unsigned int>(MainMenuEvent::Configuration), false };
		const ButtonItem Exit = { "Exit Hyper BandCHIP", 200, 162, static_cast<unsigned int>(MainMenuEvent::Exit), false };
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

	struct ConfigurationMenuData
	{
		const TextItem Title = { "Configuration", 240, 20, false };
		MultiChoiceItem Core = { "Core", 200, 60, static_cast<unsigned int>(ConfigurationMenuEvent::ChangeCore), 0, { "CHIP-8", "SuperCHIP", "HyperCHIP-64" }, false };
		const ButtonItem Behaviors = { "Behaviors", 200, 74, static_cast<unsigned int>(ConfigurationMenuEvent::Behaviors), false };
		const ButtonItem PaletteSettings = { "Palette Settings", 200, 88, static_cast<unsigned int>(ConfigurationMenuEvent::PaletteSettings), false };
		const ButtonItem KeyboardRemapping = { "Keyboard Remapping", 200, 102, static_cast<unsigned int>(ConfigurationMenuEvent::KeyboardRemapping), false };
		const ButtonItem LoadConfiguration = { "Load Configuration", 200, 116, 0xFFFFFFFF, false };
		const ButtonItem SaveConfiguration = { "Save Configuration", 200, 130, 0xFFFFFFFF, false };
		const ButtonItem ReturnToMainMenu = { "Return to Main Menu", 200, 144, static_cast<unsigned int>(ConfigurationMenuEvent::ReturnToMainMenu), false };
		unsigned int CurrentSelectableItemId = 0;
	};

	struct LoadProgramDisplayData
	{
		TextItem LoadingProgram = { "Loading program...", 240, 140, true };
		TextItem LoadSuccessful = { "Load Successful!", 240, 140, true };
		TextItem LoadFailed = { "Load Failed.", 250, 140, true };
		ButtonItem Ok = { "Ok", 300, 180, static_cast<unsigned int>(LoadProgramDisplayMenuEvent::Ok), true };
	};

	struct BehaviorsMenuData
	{
		const TextItem Title = { "Behaviors", 270, 20, false };
		ToggleItem CHIP48_Shift = { "CHIP-48 Shift", 200, 60, false, false };
		ToggleItem CHIP48_LoadStore = { "CHIP-48 Load/Store", 200, 74, false, false };
		MultiChoiceItem SuperCHIP_Version = { "SuperCHIP Version", 140, 60, 0xFFFFFFFF, 0, { "Fixed SuperCHIP V1.1", "SuperCHIP V1.0", "Original SuperCHIP V1.1" }, false };
		const ButtonItem ReturnToConfiguration = { "Return to Configuration", 200, 190, static_cast<unsigned int>(BehaviorMenuEvent::ReturnToConfiguration), false };
		unsigned int CurrentSelectableItemId = 0;
	};

	struct PaletteSettingsMenuData
	{
		const TextItem Title = { "Palette Settings", 250, 20, false };
		AdjustableValueItem CurrentIndex = { "Current Index", 200, 60, 0, 3, ValueBaseType::Dec, 0, 0, false };
		AdjustableValueItem Red = { "Red", 200, 74, 0, 255, ValueBaseType::Dec, 0, 0, false };
		AdjustableValueItem Green = { "Green", 200, 88, 0, 255, ValueBaseType::Dec, 0, 0, false };
		AdjustableValueItem Blue = { "Blue", 200, 102, 0, 255, ValueBaseType::Dec, 0, 0, false };
		const ButtonItem CommitChanges = { "Commit Changes", 200, 176, static_cast<unsigned int>(PaletteSettingsMenuEvent::CommitChanges), false };
		const ButtonItem ReturnToConfiguration = { "Return to Configuration", 200, 190, static_cast<unsigned int>(PaletteSettingsMenuEvent::ReturnToConfiguration), false };
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
		ValueItem SoundTimerValue = { "ST", 300, 102, ValueBaseType::Dec, 0, false };
	};

	class Application
	{
		public:
			Application();
			~Application();
			int GetReturnCode() const;
			void InitializeKeyMaps();
			void ConstructMenus();
			void ConstructFonts();
			void ShowMenu(MenuDisplay Menu);
		private:
			SDL_Window *MainWindow;
			Renderer *MainRenderer;
			std::filesystem::path start_path;
			std::map<MenuKey, SDL_Scancode> Menu_KeyMap;
			std::map<CHIP8Key, SDL_Scancode> CHIP8_KeyMap;
			std::map<char, SDL_Scancode> Input_KeyMap;
			OperationMode CurrentOperationMode;
			MachineCore CurrentMachineCore;
			MenuDisplay CurrentMenu;
			MainMenuData MainMenu;
			LoadProgramMenuData LoadProgramMenu;
			ConfigurationMenuData ConfigurationMenu;
			LoadProgramDisplayData LoadProgramDisplay;
			BehaviorsMenuData BehaviorsMenu;
			PaletteSettingsMenuData PaletteSettingsMenu;
			KeyboardRemappingMenuData KeyboardRemappingMenu;
			ErrorDisplayData ErrorDisplay;
			Font<5, 16> *CHIP8_Fonts;
			Font<10, 10> *SuperCHIP_Fonts;
			Machine *CurrentMachine;
			std::chrono::high_resolution_clock::time_point refresh_tp;
			double refresh_accumulator;
			bool loading_program;
			int retcode;
	};
}

#endif
