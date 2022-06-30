#include "../include/hyper_bandchip.h"
#if defined(RENDERER_OPENGL21)
#include "../include/renderer_opengl21.h"
#elif defined(RENDERER_OPENGLES2)
#include "../include/renderer_opengles2.h"
#elif defined(RENDERER_OPENGLES3)
#include "../include/renderer_opengles3.h"
#endif
#include "../include/menu_fonts.h"
#include "../include/vip_fonts.h"
#include "../include/schip_fonts.h"
#include "../include/octo_fonts.h"
#include "../include/kchip8_fonts.h"
#include <iostream>
#include <fstream>
#include <vector>

using std::cout;
using std::endl;

using std::ifstream;

Hyper_BandCHIP::Application::Application() : MainWindow(nullptr), MainRenderer(nullptr), start_path(std::filesystem::current_path()), CurrentOperationMode(OperationMode::Menu), CurrentMachineCore(MachineCore::BandCHIP_CHIP8), CurrentMenu(MenuDisplay::Main), CurrentLoResFontStyle(LoResFontStyle::VIP), CurrentHiResFontStyle(HiResFontStyle::SuperCHIP), CurrentMachine(nullptr), refresh_accumulator(0.0), loading_program(false), retcode(0)
{
	bool exit = false;
	unsigned int flags = 0x00000000;
	double refresh_time_accumulator = 0.0;
	unsigned int frame_count = 0;
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
#if defined(RENDERER_OPENGL21) || defined(RENDERER_OPENGLES2) || defined(RENDERER_OPENGLES3)
	flags |= SDL_WINDOW_OPENGL;
#if defined(RENDERER_OPENGL21)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#elif defined(RENDERER_OPENGLES2)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(RENDERER_OPENGLES3)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif
#if defined(RENDERER_OPENGLES2) || defined(RENDERER_OPENGLES3)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif
#endif
	MainWindow = std::unique_ptr<SDL_Window, WindowDeleter>(SDL_CreateWindow("Hyper BandCHIP Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 640, flags));
	MainRenderer = std::make_unique<Renderer>(MainWindow.get());
	if (MainRenderer->Fail())
	{
		return;
	}
	LoadFontStyles();
	InitializeKeyMaps();
	ConstructMenus();
	MainRenderer->SetupMenuFonts(reinterpret_cast<const unsigned char *>(Fonts::Menu::Fonts));
	ShowMenu(CurrentMenu);
	refresh_tp = std::chrono::high_resolution_clock::now();
	while (!exit)
	{
		bool idle = true;
		if (loading_program)
		{
			idle = false;
			loading_program = false;
			ifstream ProgramFile(LoadProgramMenu.MenuEntry[LoadProgramMenu.CurrentSelectableItemId].Entry.Text, std::ios::binary);
			ProgramFile.seekg(0, std::ios::end);
			size_t program_size = ProgramFile.tellg();
			ProgramFile.seekg(0, std::ios::beg);
			std::vector<unsigned char> program_data(program_size);
			ProgramFile.read(reinterpret_cast<char *>(program_data.data()), program_size);
			switch (CurrentMachineCore)
			{
				case MachineCore::BandCHIP_CHIP8:
				{
					if (CurrentMachine != nullptr)
					{
						if (CurrentMachine->GetMachineCore() != CurrentMachineCore)
						{
							CurrentMachine = std::make_unique<Machine>(CurrentMachineCore, CPUSettingsMenu.CPUCycles.value, 0x1000, 64, 32, MainRenderer.get());
						}
						else
						{
							CurrentMachine->InitializeRegisters();
							CurrentMachine->InitializeTimers();
							CurrentMachine->InitializeStack();
							CurrentMachine->InitializeMemory();
							CurrentMachine->InitializeVideo();
							CurrentMachine->InitializeKeyStatus();
						}
					}
					else
					{
						CurrentMachine = std::make_unique<Machine>(CurrentMachineCore, CPUSettingsMenu.CPUCycles.value, 0x1000, 64, 32, MainRenderer.get());
					}
					MainRenderer->SetupDisplay(64, 32);
					const unsigned char *lores_fonts = nullptr;
					size_t lores_fonts_size = 0;
					if (LoResFontStyleList.size() == 0)
					{
						switch (CurrentLoResFontStyle)
						{
							case LoResFontStyle::VIP:
							{
								lores_fonts = reinterpret_cast<const unsigned char *>(Fonts::VIP::LoResFonts);
								lores_fonts_size = sizeof(Fonts::VIP::LoResFonts);
								break;
							}
							case LoResFontStyle::SuperCHIP:
							{
								lores_fonts = reinterpret_cast<const unsigned char *>(Fonts::SuperCHIP::LoResFonts);
								lores_fonts_size = sizeof(Fonts::SuperCHIP::LoResFonts);
								break;
							}
							case LoResFontStyle::KCHIP8:
							{
								lores_fonts = reinterpret_cast<const unsigned char *>(Fonts::KCHIP8::LoResFonts);
								lores_fonts_size = sizeof(Fonts::KCHIP8::LoResFonts);
								break;
							}
						}
					}
					else
					{
						Fonts::FontStyle<4, 5> &CurrentLoResFontStyle = LoResFontStyleList[FontSettingsMenu.LoResFontStyle.current_option];
						lores_fonts = CurrentLoResFontStyle.GetFontStyleData();
						lores_fonts_size = CurrentLoResFontStyle.GetFontStyleDataSize();
					}
					CurrentMachine->CopyDataToInterpreterMemory(lores_fonts, 0x000, lores_fonts_size);
					CHIP8_BehaviorData CurrentBehaviorData = { BehaviorsMenu.CHIP48_Shift.toggle, BehaviorsMenu.CHIP48_LoadStore.toggle, BehaviorsMenu.VIP_Display_Interrupt.toggle };
					CurrentMachine->StoreBehaviorData(&CurrentBehaviorData);
					CurrentMachine->SetSync(CPUSettingsMenu.Sync.toggle);
					if (CurrentMachine->LoadProgram(program_data.data(), 0x200, program_size))
					{
						LoadProgramDisplay.LoadingProgram.hidden = true;
						LoadProgramDisplay.LoadSuccessful.hidden = false;
						LoadProgramDisplay.Ok.hidden = false;
						MainMenu.CurrentProgram.Status = LoadProgramMenu.MenuEntry[LoadProgramMenu.CurrentSelectableItemId].Entry.Text;
						MainMenu.CurrentMachineStatus.Status = "Operational";
					}
					else
					{
						LoadProgramDisplay.LoadingProgram.hidden = true;
						LoadProgramDisplay.LoadFailed.hidden = false;
						LoadProgramDisplay.Ok.hidden = false;
					}
					break;
				}
				case MachineCore::BandCHIP_SuperCHIP:
				{
					std::filesystem::path local_dir = std::filesystem::current_path();
					std::filesystem::current_path(start_path);
					if (CurrentMachine != nullptr)
					{
						if (CurrentMachine->GetMachineCore() != CurrentMachineCore)
						{
							CurrentMachine = std::make_unique<Machine>(CurrentMachineCore, CPUSettingsMenu.CPUCycles.value, 0x1000, 128, 64, MainRenderer.get());
						}
						else
						{
							CurrentMachine->SetResolutionMode(ResolutionMode::LoRes);
							CurrentMachine->InitializeRegisters();
							CurrentMachine->InitializeTimers();
							CurrentMachine->InitializeStack();
							CurrentMachine->InitializeMemory();
							CurrentMachine->InitializeVideo();
							CurrentMachine->InitializeKeyStatus();
						}
					}
					else
					{
						CurrentMachine = std::make_unique<Machine>(CurrentMachineCore, CPUSettingsMenu.CPUCycles.value, 0x1000, 128, 64, MainRenderer.get());
					}
					std::filesystem::current_path(local_dir);
					MainRenderer->SetupDisplay(128, 64);
					const unsigned char *lores_fonts = nullptr;
					const unsigned char *hires_fonts = nullptr;
					size_t lores_fonts_size = 0;
					size_t hires_fonts_size = 0;
					if (LoResFontStyleList.size() == 0)
					{
						switch (CurrentLoResFontStyle)
						{
							case LoResFontStyle::VIP:
							{
								lores_fonts = reinterpret_cast<const unsigned char *>(Fonts::VIP::LoResFonts);
								lores_fonts_size = sizeof(Fonts::VIP::LoResFonts);
								break;
							}
							case LoResFontStyle::SuperCHIP:
							{
								lores_fonts = reinterpret_cast<const unsigned char *>(Fonts::SuperCHIP::LoResFonts);
								lores_fonts_size = sizeof(Fonts::SuperCHIP::LoResFonts);
								break;
							}
							case LoResFontStyle::KCHIP8:
							{
								lores_fonts = reinterpret_cast<const unsigned char *>(Fonts::KCHIP8::LoResFonts);
								lores_fonts_size = sizeof(Fonts::KCHIP8::LoResFonts);
								break;
							}
						}
					}
					else
					{
						Fonts::FontStyle<4, 5> &CurrentLoResFontStyle = LoResFontStyleList[FontSettingsMenu.LoResFontStyle.current_option];
						lores_fonts = CurrentLoResFontStyle.GetFontStyleData();
						lores_fonts_size = CurrentLoResFontStyle.GetFontStyleDataSize();
					}
					if (HiResFontStyleList.size() == 0)
					{
						switch (CurrentHiResFontStyle)
						{
							case HiResFontStyle::SuperCHIP:
							{
								hires_fonts = reinterpret_cast<const unsigned char *>(Fonts::SuperCHIP::HiResFonts);
								hires_fonts_size = sizeof(Fonts::SuperCHIP::HiResFonts);
								break;
							}
							case HiResFontStyle::Octo:
							{
								hires_fonts = reinterpret_cast<const unsigned char *>(Fonts::Octo::HiResFonts);
								hires_fonts_size = sizeof(Fonts::Octo::HiResFonts);
								break;
							}
							case HiResFontStyle::KCHIP8:
							{
								hires_fonts = reinterpret_cast<const unsigned char *>(Fonts::KCHIP8::HiResFonts);
								hires_fonts_size = sizeof(Fonts::KCHIP8::HiResFonts);
								break;
							}
						}
					}
					else
					{
						Fonts::FontStyle<8, 10> &CurrentHiResFontStyle = HiResFontStyleList[FontSettingsMenu.HiResFontStyle.current_option];
						hires_fonts = CurrentHiResFontStyle.GetFontStyleData();
						hires_fonts_size = CurrentHiResFontStyle.GetFontStyleDataSize();
					}
					CurrentMachine->CopyDataToInterpreterMemory(lores_fonts, 0x000, lores_fonts_size);
					CurrentMachine->CopyDataToInterpreterMemory(hires_fonts, 0x050, hires_fonts_size);
					SuperCHIP_BehaviorData CurrentBehaviorData;
					switch (BehaviorsMenu.SuperCHIP_Version.current_option)
					{
						case 0:
						{
							CurrentBehaviorData.Version = SuperCHIPVersion::Fixed_SuperCHIP11;
							break;
						}
						case 1:
						{
							CurrentBehaviorData.Version = SuperCHIPVersion::SuperCHIP10;
							break;
						}
						case 2:
						{
							CurrentBehaviorData.Version = SuperCHIPVersion::SuperCHIP11;
							break;
						}
					}
					CurrentMachine->StoreBehaviorData(&CurrentBehaviorData);
					CurrentMachine->SetSync(CPUSettingsMenu.Sync.toggle);
					if (CurrentMachine->LoadProgram(program_data.data(), 0x200, program_size))
					{
						LoadProgramDisplay.LoadingProgram.hidden = true;
						LoadProgramDisplay.LoadSuccessful.hidden = false;
						LoadProgramDisplay.Ok.hidden = false;
						MainMenu.CurrentProgram.Status = LoadProgramMenu.MenuEntry[LoadProgramMenu.CurrentSelectableItemId].Entry.Text;
						MainMenu.CurrentMachineStatus.Status = "Operational";
					}
					else
					{
						LoadProgramDisplay.LoadingProgram.hidden = true;
						LoadProgramDisplay.LoadFailed.hidden = false;
						LoadProgramDisplay.Ok.hidden = false;
					}
					break;
				}
				case MachineCore::BandCHIP_XOCHIP:
				{
					std::filesystem::path local_dir = std::filesystem::current_path();
					std::filesystem::current_path(start_path);
					if (CurrentMachine != nullptr)
					{
						if (CurrentMachine->GetMachineCore() != CurrentMachineCore)
						{
							CurrentMachine = std::make_unique<Machine>(CurrentMachineCore, CPUSettingsMenu.CPUCycles.value, 0x10000, 128, 64, MainRenderer.get());
						}
						else
						{
							CurrentMachine->SetResolutionMode(ResolutionMode::LoRes);
							CurrentMachine->InitializeRegisters();
							CurrentMachine->InitializeTimers();
							CurrentMachine->InitializeStack();
							CurrentMachine->InitializeMemory();
							CurrentMachine->InitializeVideo();
							CurrentMachine->InitializeAudio();
							CurrentMachine->InitializeKeyStatus();
						}
					}
					else
					{
						CurrentMachine = std::make_unique<Machine>(CurrentMachineCore, CPUSettingsMenu.CPUCycles.value, 0x10000, 128, 64, MainRenderer.get());
					}
					std::filesystem::current_path(local_dir);
					MainRenderer->SetupDisplay(128, 64);
					const unsigned char *lores_fonts = nullptr;
					const unsigned char *hires_fonts = nullptr;
					size_t lores_fonts_size = 0;
					size_t hires_fonts_size = 0;
					if (LoResFontStyleList.size() == 0)
					{
						switch (CurrentLoResFontStyle)
						{
							case LoResFontStyle::VIP:
							{
								lores_fonts = reinterpret_cast<const unsigned char *>(Fonts::VIP::LoResFonts);
								lores_fonts_size = sizeof(Fonts::VIP::LoResFonts);
								break;
							}
							case LoResFontStyle::SuperCHIP:
							{
								lores_fonts = reinterpret_cast<const unsigned char *>(Fonts::SuperCHIP::LoResFonts);
								lores_fonts_size = sizeof(Fonts::SuperCHIP::LoResFonts);
								break;
							}
							case LoResFontStyle::KCHIP8:
							{
								lores_fonts = reinterpret_cast<const unsigned char *>(Fonts::KCHIP8::LoResFonts);
								lores_fonts_size = sizeof(Fonts::KCHIP8::LoResFonts);
								break;
							}
						}
					}
					else
					{
						Fonts::FontStyle<4, 5> &CurrentLoResFontStyle = LoResFontStyleList[FontSettingsMenu.LoResFontStyle.current_option];
						lores_fonts = CurrentLoResFontStyle.GetFontStyleData();
						lores_fonts_size = CurrentLoResFontStyle.GetFontStyleDataSize();
					}
					if (HiResFontStyleList.size() == 0)
					{
						switch (CurrentHiResFontStyle)
						{
							case HiResFontStyle::SuperCHIP:
							{
								hires_fonts = reinterpret_cast<const unsigned char *>(Fonts::SuperCHIP::HiResFonts);
								hires_fonts_size = sizeof(Fonts::SuperCHIP::HiResFonts);
								break;
							}
							case HiResFontStyle::Octo:
							{
								hires_fonts = reinterpret_cast<const unsigned char *>(Fonts::Octo::HiResFonts);
								hires_fonts_size = sizeof(Fonts::Octo::HiResFonts);
								break;
							}
							case HiResFontStyle::KCHIP8:
							{
								hires_fonts = reinterpret_cast<const unsigned char *>(Fonts::KCHIP8::HiResFonts);
								hires_fonts_size = sizeof(Fonts::KCHIP8::HiResFonts);
								break;
							}
						}
					}
					else
					{
						Fonts::FontStyle<8, 10> &CurrentHiResFontStyle = HiResFontStyleList[FontSettingsMenu.HiResFontStyle.current_option];
						hires_fonts = CurrentHiResFontStyle.GetFontStyleData();
						hires_fonts_size = CurrentHiResFontStyle.GetFontStyleDataSize();
					}
					CurrentMachine->CopyDataToInterpreterMemory(lores_fonts, 0x000, lores_fonts_size);
					CurrentMachine->CopyDataToInterpreterMemory(hires_fonts, 0x050, hires_fonts_size);
					XOCHIP_BehaviorData CurrentBehaviorData = { BehaviorsMenu.SuperCHIP_Shift.toggle, BehaviorsMenu.SuperCHIP_LoadStore.toggle, BehaviorsMenu.Octo_LoResSprite.toggle };
					CurrentMachine->StoreBehaviorData(&CurrentBehaviorData);
					CurrentMachine->SetSync(CPUSettingsMenu.Sync.toggle);
					if (CurrentMachine->LoadProgram(program_data.data(), 0x200, program_size))
					{
						LoadProgramDisplay.LoadingProgram.hidden = true;
						LoadProgramDisplay.LoadSuccessful.hidden = false;
						LoadProgramDisplay.Ok.hidden = false;
						MainMenu.CurrentProgram.Status = LoadProgramMenu.MenuEntry[LoadProgramMenu.CurrentSelectableItemId].Entry.Text;
						MainMenu.CurrentMachineStatus.Status = "Operational";
					}
					else
					{
						LoadProgramDisplay.LoadingProgram.hidden = true;
						LoadProgramDisplay.LoadFailed.hidden = false;
						LoadProgramDisplay.Ok.hidden = false;
					}
					break;
				}
				case MachineCore::BandCHIP_HyperCHIP64:
				{
					std::filesystem::path local_dir = std::filesystem::current_path();
					std::filesystem::current_path(start_path);
					if (CurrentMachine != nullptr)
					{
						if (CurrentMachine->GetMachineCore() != CurrentMachineCore)
						{
							CurrentMachine = std::make_unique<Machine>(CurrentMachineCore, CPUSettingsMenu.CPUCycles.value, 0x10000, 128, 64, MainRenderer.get());
						}
						else
						{
							CurrentMachine->SetResolutionMode(ResolutionMode::LoRes);
							CurrentMachine->InitializeRegisters();
							CurrentMachine->InitializeTimers();
							CurrentMachine->InitializeStack();
							CurrentMachine->InitializeMemory();
							CurrentMachine->InitializeVideo();
							CurrentMachine->InitializeAudio();
							CurrentMachine->InitializeKeyStatus();
						}
					}
					else
					{
						CurrentMachine = std::make_unique<Machine>(CurrentMachineCore, CPUSettingsMenu.CPUCycles.value, 0x10000, 128, 64, MainRenderer.get());
					}
					std::filesystem::current_path(local_dir);
					MainRenderer->SetupDisplay(128, 64);
					const unsigned char *lores_fonts = nullptr;
					const unsigned char *hires_fonts = nullptr;
					size_t lores_fonts_size = 0;
					size_t hires_fonts_size = 0;
					if (LoResFontStyleList.size() == 0)
					{
						switch (CurrentLoResFontStyle)
						{
							case LoResFontStyle::VIP:
							{
								lores_fonts = reinterpret_cast<const unsigned char *>(Fonts::VIP::LoResFonts);
								lores_fonts_size = sizeof(Fonts::VIP::LoResFonts);
								break;
							}
							case LoResFontStyle::SuperCHIP:
							{
								lores_fonts = reinterpret_cast<const unsigned char *>(Fonts::SuperCHIP::LoResFonts);
								lores_fonts_size = sizeof(Fonts::SuperCHIP::LoResFonts);
								break;
							}
							case LoResFontStyle::KCHIP8:
							{
								lores_fonts = reinterpret_cast<const unsigned char *>(Fonts::KCHIP8::LoResFonts);
								lores_fonts_size = sizeof(Fonts::KCHIP8::LoResFonts);
								break;
							}
						}
					}
					else
					{
						Fonts::FontStyle<4, 5> &CurrentLoResFontStyle = LoResFontStyleList[FontSettingsMenu.LoResFontStyle.current_option];
						lores_fonts = CurrentLoResFontStyle.GetFontStyleData();
						lores_fonts_size = CurrentLoResFontStyle.GetFontStyleDataSize();
					}
					if (HiResFontStyleList.size() == 0)
					{
						switch (CurrentHiResFontStyle)
						{
							case HiResFontStyle::SuperCHIP:
							{
								hires_fonts = reinterpret_cast<const unsigned char *>(Fonts::SuperCHIP::HiResFonts);
								hires_fonts_size = sizeof(Fonts::SuperCHIP::HiResFonts);
								break;
							}
							case HiResFontStyle::Octo:
							{
								hires_fonts = reinterpret_cast<const unsigned char *>(Fonts::Octo::HiResFonts);
								hires_fonts_size = sizeof(Fonts::Octo::HiResFonts);
								break;
							}
							case HiResFontStyle::KCHIP8:
							{
								hires_fonts = reinterpret_cast<const unsigned char *>(Fonts::KCHIP8::HiResFonts);
								hires_fonts_size = sizeof(Fonts::KCHIP8::HiResFonts);
								break;
							}
						}
					}
					else
					{
						Fonts::FontStyle<8, 10> &CurrentHiResFontStyle = HiResFontStyleList[FontSettingsMenu.HiResFontStyle.current_option];
						hires_fonts = CurrentHiResFontStyle.GetFontStyleData();
						hires_fonts_size = CurrentHiResFontStyle.GetFontStyleDataSize();
					}
					CurrentMachine->CopyDataToInterpreterMemory(lores_fonts, 0x000, lores_fonts_size);
					CurrentMachine->CopyDataToInterpreterMemory(hires_fonts, 0x050, hires_fonts_size);
					CurrentMachine->SetSync(CPUSettingsMenu.Sync.toggle);
					if (CurrentMachine->LoadProgram(program_data.data(), 0x200, program_size))
					{
						LoadProgramDisplay.LoadingProgram.hidden = true;
						LoadProgramDisplay.LoadSuccessful.hidden = false;
						LoadProgramDisplay.Ok.hidden = false;
						MainMenu.CurrentProgram.Status = LoadProgramMenu.MenuEntry[LoadProgramMenu.CurrentSelectableItemId].Entry.Text;
						MainMenu.CurrentMachineStatus.Status = "Operational";
					}
					else
					{
						LoadProgramDisplay.LoadingProgram.hidden = true;
						LoadProgramDisplay.LoadFailed.hidden = false;
						LoadProgramDisplay.Ok.hidden = false;
					}
					break;
				}
			}
			ShowMenu(MenuDisplay::LoadProgramDisplay);
		}
		SDL_Event event;
		std::chrono::high_resolution_clock::time_point current_tp = std::chrono::high_resolution_clock::now();
		while (SDL_PollEvent(&event))
		{
			if (idle)
			{
				idle = false;
			}
			switch (event.type)
			{
				case SDL_KEYDOWN:
				{
					SDL_Scancode scancode = event.key.keysym.scancode;
					switch (CurrentOperationMode)
					{
						case OperationMode::Menu:
						{
							bool key_found = false;
							for (auto [key, i] : Menu_KeyMap)
							{
								if (i == scancode)
								{
									switch (key)
									{
										case MenuKey::Up:
										{
											switch (CurrentMenu)
											{
												case MenuDisplay::Main:
												{
													MainMenu.CurrentSelectableItemId = (MainMenu.CurrentSelectableItemId == 0) ? 3 : MainMenu.CurrentSelectableItemId - 1;
													break;
												}
												case MenuDisplay::LoadProgram:
												{
													if (LoadProgramMenu.CurrentSelectableItemId == 0)
													{
														bool change = false;
														if (LoadProgramMenu.StartEntry == 0)
														{
															if (LoadProgramMenu.CurrentDirectoryCount > 16)
															{
																LoadProgramMenu.StartEntry = LoadProgramMenu.CurrentDirectoryCount - 16;
																change = true;
															}
															LoadProgramMenu.CurrentSelectableItemId = (LoadProgramMenu.CurrentDirectoryCount > 16) ? 15 : LoadProgramMenu.CurrentDirectoryCount - 1;
														}
														else
														{
															--LoadProgramMenu.StartEntry;
															change = true;
														}
														if (change)
														{
															std::filesystem::path current_dir(".");
															unsigned int current_entry = 0;
															unsigned int start_entry = LoadProgramMenu.StartEntry;
															unsigned int count = 0;
															if (LoadProgramMenu.StartEntry == 0)
															{
																LoadProgramMenu.MenuEntry[0].Type = DirEntryType::Directory;
																LoadProgramMenu.MenuEntry[0].Entry.Text = "..";
																LoadProgramMenu.MenuEntry[0].Entry.event_id = static_cast<unsigned int>(LoadProgramMenuEvent::ChangeDirectory);
																++count;
															}
															else
															{
																--start_entry;
															}
															unsigned int max_count = LoadProgramMenu.CurrentDirectoryCount;
															if (max_count > 16)
															{
																max_count = 16;
															}
															for (auto i : std::filesystem::directory_iterator(current_dir))
															{
																if (current_entry >= start_entry && count < max_count)
																{
																	std::filesystem::path c_path = i.path();
																	unsigned int current_event_id = 0;
																	if (std::filesystem::is_directory(c_path))
																	{
																		current_event_id = static_cast<unsigned int>(LoadProgramMenuEvent::ChangeDirectory);
																		LoadProgramMenu.MenuEntry[count].Type = DirEntryType::Directory;
																	}
																	else
																	{
																		current_event_id = static_cast<unsigned int>(LoadProgramMenuEvent::Load);
																		LoadProgramMenu.MenuEntry[count].Type = DirEntryType::File;
																	}
																	LoadProgramMenu.MenuEntry[count].Entry.Text = c_path.filename();
																	LoadProgramMenu.MenuEntry[count].Entry.event_id = current_event_id;
																	++count;
																}
																if (count == max_count)
																{
																	break;
																}
																++current_entry;
															}
														}
													}
													else
													{
														--LoadProgramMenu.CurrentSelectableItemId;
													}
													break;
												}
												case MenuDisplay::Configuration:
												{
													ConfigurationMenu.CurrentSelectableItemId = (ConfigurationMenu.CurrentSelectableItemId == 0) ? 8 : ConfigurationMenu.CurrentSelectableItemId - 1;
													break;
												}
												case MenuDisplay::CPUSettings:
												{
													CPUSettingsMenu.CurrentSelectableItemId = (CPUSettingsMenu.CurrentSelectableItemId == 0) ? 4 : CPUSettingsMenu.CurrentSelectableItemId - 1;
													break;
												}
												case MenuDisplay::Behaviors:
												{
													switch (CurrentMachineCore)
													{
														case MachineCore::BandCHIP_CHIP8:
														{
															BehaviorsMenu.CurrentSelectableItemId = (BehaviorsMenu.CurrentSelectableItemId == 0) ? 3 : BehaviorsMenu.CurrentSelectableItemId - 1;
															break;
														}
														case MachineCore::BandCHIP_SuperCHIP:
														{
															BehaviorsMenu.CurrentSelectableItemId = (BehaviorsMenu.CurrentSelectableItemId == 0) ? 1 : BehaviorsMenu.CurrentSelectableItemId - 1;
															break;
														}
														case MachineCore::BandCHIP_XOCHIP:
														{
															BehaviorsMenu.CurrentSelectableItemId = (BehaviorsMenu.CurrentSelectableItemId == 0) ? 3 : BehaviorsMenu.CurrentSelectableItemId - 1;
															break;
														}
													}
													break;
												}
												case MenuDisplay::PaletteSettings:
												{
													PaletteSettingsMenu.CurrentSelectableItemId = (PaletteSettingsMenu.CurrentSelectableItemId == 0) ? 5 : PaletteSettingsMenu.CurrentSelectableItemId - 1;
													break;
												}
												case MenuDisplay::FontSettings:
												{
													FontSettingsMenu.CurrentSelectableItemId = (FontSettingsMenu.CurrentSelectableItemId == 0) ? 2 : FontSettingsMenu.CurrentSelectableItemId - 1;
													break;
												}
												case MenuDisplay::KeyboardRemapping:
												{
													if (!KeyboardRemappingMenu.input_mode)
													{
														if (KeyboardRemappingMenu.CurrentSelectableItemId < 16)
														{
															KeyboardRemappingMenu.CurrentSelectableItemId = (KeyboardRemappingMenu.CurrentSelectableItemId % 8 == 0) ? 16 : KeyboardRemappingMenu.CurrentSelectableItemId - 1;
														}
														else
														{
															KeyboardRemappingMenu.CurrentSelectableItemId = 7;
														}
													}
													break;
												}
											}
											ShowMenu(CurrentMenu);
											break;
										}
										case MenuKey::Down:
										{
											switch (CurrentMenu)
											{
												case MenuDisplay::Main:
												{
													MainMenu.CurrentSelectableItemId = (MainMenu.CurrentSelectableItemId == 3) ? 0 : MainMenu.CurrentSelectableItemId + 1;
													break;
												}
												case MenuDisplay::LoadProgram:
												{
													unsigned int max_count = (LoadProgramMenu.CurrentDirectoryCount > 16) ? 16 : LoadProgramMenu.CurrentDirectoryCount;
													if (LoadProgramMenu.CurrentSelectableItemId == max_count - 1)
													{
														bool change = false;
														if (LoadProgramMenu.CurrentDirectoryCount - LoadProgramMenu.StartEntry > 16)
														{
															++LoadProgramMenu.StartEntry;
															change = true;
														}
														else
														{
															if (LoadProgramMenu.StartEntry != 0)
															{
																LoadProgramMenu.StartEntry = 0;
																change = true;
															}
															LoadProgramMenu.CurrentSelectableItemId = 0;
														}
														if (change)
														{
															std::filesystem::path current_dir(".");
															unsigned int current_entry = 0;
															unsigned int start_entry = LoadProgramMenu.StartEntry;
															unsigned int count = 0;
															if (LoadProgramMenu.StartEntry == 0)
															{
																LoadProgramMenu.MenuEntry[0].Type = DirEntryType::Directory;
																LoadProgramMenu.MenuEntry[0].Entry.Text = "..";
																LoadProgramMenu.MenuEntry[0].Entry.event_id = static_cast<unsigned int>(LoadProgramMenuEvent::ChangeDirectory);
																++count;
															}
															else
															{
																--start_entry;
															}
															for (auto i : std::filesystem::directory_iterator(current_dir))
															{
																if (current_entry >= start_entry && count < max_count)
																{
																	std::filesystem::path c_path = i.path();
																	unsigned int current_event_id = 0;
																	if (std::filesystem::is_directory(c_path))
																	{
																		current_event_id = static_cast<unsigned int>(LoadProgramMenuEvent::ChangeDirectory);
																		LoadProgramMenu.MenuEntry[count].Type = DirEntryType::Directory;
																	}
																	else
																	{
																		current_event_id = static_cast<unsigned int>(LoadProgramMenuEvent::Load);
																		LoadProgramMenu.MenuEntry[count].Type = DirEntryType::File;
																	}
																	LoadProgramMenu.MenuEntry[count].Entry.Text = c_path.filename();
																	LoadProgramMenu.MenuEntry[count].Entry.event_id = current_event_id;
																	++count;
																}
																if (count == max_count)
																{
																	break;
																}
																++current_entry;
															}
														}
													}
													else
													{
														++LoadProgramMenu.CurrentSelectableItemId;
													}
													break;
												}
												case MenuDisplay::Configuration:
												{
													ConfigurationMenu.CurrentSelectableItemId = (ConfigurationMenu.CurrentSelectableItemId == 8) ? 0 : ConfigurationMenu.CurrentSelectableItemId + 1;
													break;
												}
												case MenuDisplay::CPUSettings:
												{
													CPUSettingsMenu.CurrentSelectableItemId = (CPUSettingsMenu.CurrentSelectableItemId == 4) ? 0 : CPUSettingsMenu.CurrentSelectableItemId + 1;
													break;
												}
												case MenuDisplay::Behaviors:
												{
													switch (CurrentMachineCore)
													{
														case MachineCore::BandCHIP_CHIP8:
														{
															BehaviorsMenu.CurrentSelectableItemId = (BehaviorsMenu.CurrentSelectableItemId == 3) ? 0 : BehaviorsMenu.CurrentSelectableItemId + 1;
															break;
														}
														case MachineCore::BandCHIP_SuperCHIP:
														{
															BehaviorsMenu.CurrentSelectableItemId = (BehaviorsMenu.CurrentSelectableItemId == 1) ? 0 : BehaviorsMenu.CurrentSelectableItemId + 1;
															break;
														}
														case MachineCore::BandCHIP_XOCHIP:
														{
															BehaviorsMenu.CurrentSelectableItemId = (BehaviorsMenu.CurrentSelectableItemId == 3) ? 0 : BehaviorsMenu.CurrentSelectableItemId + 1;
															break;
														}
													}
													break;
												}
												case MenuDisplay::PaletteSettings:
												{
													PaletteSettingsMenu.CurrentSelectableItemId = (PaletteSettingsMenu.CurrentSelectableItemId == 5) ? 0 : PaletteSettingsMenu.CurrentSelectableItemId + 1;
													break;
												}
												case MenuDisplay::FontSettings:
												{
													FontSettingsMenu.CurrentSelectableItemId = (FontSettingsMenu.CurrentSelectableItemId == 2) ? 0 : FontSettingsMenu.CurrentSelectableItemId + 1;
													break;
												}
												case MenuDisplay::KeyboardRemapping:
												{
													if (!KeyboardRemappingMenu.input_mode)
													{
														if (KeyboardRemappingMenu.CurrentSelectableItemId < 16)
														{
															KeyboardRemappingMenu.CurrentSelectableItemId = (KeyboardRemappingMenu.CurrentSelectableItemId % 8 == 7) ? 16 : KeyboardRemappingMenu.CurrentSelectableItemId + 1;
														}
														else
														{
															KeyboardRemappingMenu.CurrentSelectableItemId = 0;
														}
													}
													break;
												}
											}
											ShowMenu(CurrentMenu);
											break;
										}
										case MenuKey::Left:
										{
											switch (CurrentMenu)
											{
												case MenuDisplay::Configuration:
												{
													unsigned int event_id = 0xFFFFFFFF;
													switch (ConfigurationMenu.CurrentSelectableItemId)
													{
														case 0:
														{
															event_id = ConfigurationMenu.Core.event_id;
															break;
														}
														case 1:
														{
															event_id = ConfigurationMenu.CPUSettings.event_id;
															break;
														}
														case 2:
														{
															event_id = ConfigurationMenu.Behaviors.event_id;
															break;
														}
														case 3:
														{
															event_id = ConfigurationMenu.PaletteSettings.event_id;
															break;
														}
														case 4:
														{
															event_id = ConfigurationMenu.FontSettings.event_id;
															break;
														}
														case 5:
														{
															event_id = ConfigurationMenu.KeyboardRemapping.event_id;
															break;
														}
														case 6:
														{
															event_id = ConfigurationMenu.LoadConfiguration.event_id;
															break;
														}
														case 7:
														{
															event_id = ConfigurationMenu.SaveConfiguration.event_id;
															break;
														}
														case 8:
														{
															event_id = ConfigurationMenu.ReturnToMainMenu.event_id;
															break;
														}
													}
													switch (static_cast<ConfigurationMenuEvent>(event_id))
													{
														case ConfigurationMenuEvent::ChangeCore:
														{
															switch (CurrentMachineCore)
															{
																case MachineCore::BandCHIP_CHIP8:
																{
																	CurrentMachineCore = MachineCore::BandCHIP_HyperCHIP64;
																	ConfigurationMenu.Core.current_option = 3;
																	break;
																}
																case MachineCore::BandCHIP_SuperCHIP:
																{
																	CurrentMachineCore = MachineCore::BandCHIP_CHIP8;
																	ConfigurationMenu.Core.current_option = 0;
																	BehaviorsMenu.CHIP48_Shift.toggle = false;
																	BehaviorsMenu.CHIP48_LoadStore.toggle = false;
																	BehaviorsMenu.VIP_Display_Interrupt.toggle = false;
																	break;
																}
																case MachineCore::BandCHIP_XOCHIP:
																{
																	CurrentMachineCore = MachineCore::BandCHIP_SuperCHIP;
																	ConfigurationMenu.Core.current_option = 1;
																	BehaviorsMenu.SuperCHIP_Version.current_option = 0;
																	break;
																}
																case MachineCore::BandCHIP_HyperCHIP64:
																{
																	CurrentMachineCore = MachineCore::BandCHIP_XOCHIP;
																	ConfigurationMenu.Core.current_option = 2;
																	BehaviorsMenu.SuperCHIP_LoadStore.toggle = false;
																	BehaviorsMenu.Octo_LoResSprite.toggle = true;
																	break;
																}
															}
															BehaviorsMenu.CurrentSelectableItemId = 0;
															ShowMenu(CurrentMenu);
															break;
														}
													}
													break;
												}
												case MenuDisplay::CPUSettings:
												{
													switch (CPUSettingsMenu.CurrentSelectableItemId)
													{
														case 0:
														{
															unsigned int cycles = CPUSettingsMenu.CPUCycles.value - CPUSettingsMenu.AdjustmentModifier.value;
															bool underflow = (cycles > CPUSettingsMenu.CPUCycles.value);
															if (underflow || cycles < CPUSettingsMenu.CPUCycles.min)
															{
																cycles = CPUSettingsMenu.CPUCycles.min;
															}
															CPUSettingsMenu.CPUCycles.value = cycles;
															if (CurrentMachine != nullptr)
															{
																CPUSettingsMenu.ChangeStatus.Status = "Changed";
															}
															ShowMenu(CurrentMenu);
															break;
														}
														case 1:
														{
															if (CPUSettingsMenu.AdjustmentModifier.value > CPUSettingsMenu.AdjustmentModifier.min)
															{
																CPUSettingsMenu.AdjustmentModifier.value /= 10;
															}
															ShowMenu(CurrentMenu);
															break;
														}
														case 2:
														{
															CPUSettingsMenu.Sync.toggle = (CPUSettingsMenu.Sync.toggle) ? false : true;
															ShowMenu(CurrentMenu);
															break;
														}
													}
													break;
												}
												case MenuDisplay::Behaviors:
												{
													switch (CurrentMachineCore)
													{
														case MachineCore::BandCHIP_CHIP8:
														{
															switch (BehaviorsMenu.CurrentSelectableItemId)
															{
																case 0:
																{
																	BehaviorsMenu.CHIP48_Shift.toggle = (BehaviorsMenu.CHIP48_Shift.toggle) ? false : true;
																	break;
																}
																case 1:
																{
																	BehaviorsMenu.CHIP48_LoadStore.toggle = (BehaviorsMenu.CHIP48_LoadStore.toggle) ? false : true;
																	break;
																}
																case 2:
																{
																	BehaviorsMenu.VIP_Display_Interrupt.toggle = (BehaviorsMenu.VIP_Display_Interrupt.toggle) ? false : true;
																	break;
																}
															}
															ShowMenu(CurrentMenu);
															break;
														}
														case MachineCore::BandCHIP_SuperCHIP:
														{
															switch (BehaviorsMenu.CurrentSelectableItemId)
															{
																case 0:
																{
																	BehaviorsMenu.SuperCHIP_Version.current_option = (BehaviorsMenu.SuperCHIP_Version.current_option == 0) ? 2 : BehaviorsMenu.SuperCHIP_Version.current_option - 1;
																	break;
																}
															}
															ShowMenu(CurrentMenu);
															break;
														}
														case MachineCore::BandCHIP_XOCHIP:
														{
															switch (BehaviorsMenu.CurrentSelectableItemId)
															{
																case 0:
																{
																	BehaviorsMenu.SuperCHIP_Shift.toggle = (BehaviorsMenu.SuperCHIP_Shift.toggle) ? false : true;
																	break;
																}
																case 1:
																{
																	BehaviorsMenu.SuperCHIP_LoadStore.toggle = (BehaviorsMenu.SuperCHIP_LoadStore.toggle) ? false : true;
																	break;
																}
																case 2:
																{
																	BehaviorsMenu.Octo_LoResSprite.toggle = (BehaviorsMenu.Octo_LoResSprite.toggle) ? false : true;
																	break;
																}
															}
															ShowMenu(CurrentMenu);
															break;
														}
													}
													break;
												}
												case MenuDisplay::PaletteSettings:
												{
													switch (PaletteSettingsMenu.CurrentSelectableItemId)
													{
														case 0:
														{
															PaletteSettingsMenu.CurrentIndex.value = (PaletteSettingsMenu.CurrentIndex.value == 0) ? PaletteSettingsMenu.CurrentIndex.max : PaletteSettingsMenu.CurrentIndex.value - 1;
															RGBColorData data = MainRenderer->GetPaletteIndex(PaletteSettingsMenu.CurrentIndex.value);
															PaletteSettingsMenu.Red.value = data.r;
															PaletteSettingsMenu.Green.value = data.g;
															PaletteSettingsMenu.Blue.value = data.b;
															ShowMenu(CurrentMenu);
															break;
														}
														case 1:
														{
															if (PaletteSettingsMenu.Red.value > PaletteSettingsMenu.Red.min)
															{
																--PaletteSettingsMenu.Red.value;
																ShowMenu(CurrentMenu);
															}
															break;
														}
														case 2:
														{
															if (PaletteSettingsMenu.Green.value > PaletteSettingsMenu.Green.min)
															{
																--PaletteSettingsMenu.Green.value;
																ShowMenu(CurrentMenu);
															}
															break;
														}
														case 3:
														{
															if (PaletteSettingsMenu.Blue.value > PaletteSettingsMenu.Blue.min)
															{
																--PaletteSettingsMenu.Blue.value;
																ShowMenu(CurrentMenu);
															}
															break;
														}
													}
													break;
												}
												case MenuDisplay::FontSettings:
												{
													unsigned int event_id = 0xFFFFFFFF;
													switch (FontSettingsMenu.CurrentSelectableItemId)
													{
														case 0:
														{
															event_id = FontSettingsMenu.LoResFontStyle.event_id;
															break;
														}
														case 1:
														{
															event_id = FontSettingsMenu.HiResFontStyle.event_id;
															break;
														}
														case 2:
														{
															event_id = FontSettingsMenu.ReturnToConfiguration.event_id;
															break;
														}
													}
													switch (static_cast<FontSettingsMenuEvent>(event_id))
													{
														case FontSettingsMenuEvent::ChangeLoResFont:
														{
															if (LoResFontStyleList.size() == 0)
															{
																switch (CurrentLoResFontStyle)
																{
																	case LoResFontStyle::VIP:
																	{
																		CurrentLoResFontStyle = LoResFontStyle::KCHIP8;
																		FontSettingsMenu.LoResFontStyle.current_option = 2;
																		break;
																	}
																	case LoResFontStyle::SuperCHIP:
																	{
																		CurrentLoResFontStyle = LoResFontStyle::VIP;
																		FontSettingsMenu.LoResFontStyle.current_option = 0;
																		break;
																	}
																	case LoResFontStyle::KCHIP8:
																	{
																		CurrentLoResFontStyle = LoResFontStyle::SuperCHIP;
																		FontSettingsMenu.LoResFontStyle.current_option = 1;
																		break;
																	}
																}
															}
															else
															{
																FontSettingsMenu.LoResFontStyle.current_option = (FontSettingsMenu.LoResFontStyle.current_option == 0) ? LoResFontStyleList.size() - 1 : FontSettingsMenu.LoResFontStyle.current_option - 1;
															}
															ShowMenu(CurrentMenu);
															break;
														}
														case FontSettingsMenuEvent::ChangeHiResFont:
														{
															if (HiResFontStyleList.size() == 0)
															{
																switch (CurrentHiResFontStyle)
																{
																	case HiResFontStyle::SuperCHIP:
																	{
																		CurrentHiResFontStyle = HiResFontStyle::KCHIP8;
																		FontSettingsMenu.HiResFontStyle.current_option = 2;
																		break;
																	}
																	case HiResFontStyle::Octo:
																	{
																		CurrentHiResFontStyle = HiResFontStyle::SuperCHIP;
																		FontSettingsMenu.HiResFontStyle.current_option = 0;
																		break;
																	}
																	case HiResFontStyle::KCHIP8:
																	{
																		CurrentHiResFontStyle = HiResFontStyle::Octo;
																		FontSettingsMenu.HiResFontStyle.current_option = 1;
																		break;
																	}
																}
															}
															else
															{
																FontSettingsMenu.HiResFontStyle.current_option = (FontSettingsMenu.HiResFontStyle.current_option == 0) ? HiResFontStyleList.size() - 1 : FontSettingsMenu.HiResFontStyle.current_option - 1;
															}
															ShowMenu(CurrentMenu);
															break;
														}
													}
													break;
												}
												case MenuDisplay::KeyboardRemapping:
												{
													if (!KeyboardRemappingMenu.input_mode)
													{
														if (KeyboardRemappingMenu.CurrentSelectableItemId < 16)
														{
															KeyboardRemappingMenu.CurrentSelectableItemId = (KeyboardRemappingMenu.CurrentSelectableItemId >= 8) ? KeyboardRemappingMenu.CurrentSelectableItemId - 8 : KeyboardRemappingMenu.CurrentSelectableItemId + 8;
															ShowMenu(CurrentMenu);
														}
													}
													break;
												}
											}
											break;
										}
										case MenuKey::Right:
										{
											switch (CurrentMenu)
											{
												case MenuDisplay::Configuration:
												{
													unsigned int event_id = 0xFFFFFFFF;
													switch (ConfigurationMenu.CurrentSelectableItemId)
													{
														case 0:
														{
															event_id = ConfigurationMenu.Core.event_id;
															break;
														}
														case 1:
														{
															event_id = ConfigurationMenu.CPUSettings.event_id;
															break;
														}
														case 2:
														{
															event_id = ConfigurationMenu.Behaviors.event_id;
															break;
														}
														case 3:
														{
															event_id = ConfigurationMenu.PaletteSettings.event_id;
															break;
														}
														case 4:
														{
															event_id = ConfigurationMenu.FontSettings.event_id;
															break;
														}
														case 5:
														{
															event_id = ConfigurationMenu.KeyboardRemapping.event_id;
															break;
														}
														case 6:
														{
															event_id = ConfigurationMenu.LoadConfiguration.event_id;
															break;
														}
														case 7:
														{
															event_id = ConfigurationMenu.SaveConfiguration.event_id;
															break;
														}
														case 8:
														{
															event_id = ConfigurationMenu.ReturnToMainMenu.event_id;
															break;
														}
													}
													switch (static_cast<ConfigurationMenuEvent>(event_id))
													{
														case ConfigurationMenuEvent::ChangeCore:
														{
															switch (CurrentMachineCore)
															{
																case MachineCore::BandCHIP_CHIP8:
																{
																	CurrentMachineCore = MachineCore::BandCHIP_SuperCHIP;
																	ConfigurationMenu.Core.current_option = 1;
																	BehaviorsMenu.SuperCHIP_Version.current_option = 0;
																	break;
																}
																case MachineCore::BandCHIP_SuperCHIP:
																{
																	CurrentMachineCore = MachineCore::BandCHIP_XOCHIP;
																	ConfigurationMenu.Core.current_option = 2;
																	BehaviorsMenu.SuperCHIP_LoadStore.toggle = false;
																	BehaviorsMenu.Octo_LoResSprite.toggle = true;
																	break;
																}
																case MachineCore::BandCHIP_XOCHIP:
																{
																	CurrentMachineCore = MachineCore::BandCHIP_HyperCHIP64;
																	ConfigurationMenu.Core.current_option = 3;
																	break;
																}
																case MachineCore::BandCHIP_HyperCHIP64:
																{
																	CurrentMachineCore = MachineCore::BandCHIP_CHIP8;
																	ConfigurationMenu.Core.current_option = 0;
																	BehaviorsMenu.CHIP48_Shift.toggle = false;
																	BehaviorsMenu.CHIP48_LoadStore.toggle = false;
																	BehaviorsMenu.VIP_Display_Interrupt.toggle = false;
																	break;
																}
															}
															BehaviorsMenu.CurrentSelectableItemId = 0;
															ShowMenu(CurrentMenu);
															break;
														}
													}
													break;
												}
												case MenuDisplay::CPUSettings:
												{
													switch(CPUSettingsMenu.CurrentSelectableItemId)
													{
														case 0:
														{
															unsigned int cycles = CPUSettingsMenu.CPUCycles.value + CPUSettingsMenu.AdjustmentModifier.value;
															if (cycles > CPUSettingsMenu.CPUCycles.max)
															{
																cycles = CPUSettingsMenu.CPUCycles.max;
															}
															CPUSettingsMenu.CPUCycles.value = cycles;
															if (CurrentMachine != nullptr)
															{
																CPUSettingsMenu.ChangeStatus.Status = "Changed";
															}
															ShowMenu(CurrentMenu);
															break;
														}
														case 1:
														{
															if (CPUSettingsMenu.AdjustmentModifier.value < CPUSettingsMenu.AdjustmentModifier.max)
															{
																CPUSettingsMenu.AdjustmentModifier.value *= 10;
															}
															ShowMenu(CurrentMenu);
															break;
														}
														case 2:
														{
															CPUSettingsMenu.Sync.toggle = (CPUSettingsMenu.Sync.toggle) ? false : true;
															ShowMenu(CurrentMenu);
															break;
														}
													}
													break;
												}
												case MenuDisplay::Behaviors:
												{
													switch (CurrentMachineCore)
													{
														case MachineCore::BandCHIP_CHIP8:
														{
															switch (BehaviorsMenu.CurrentSelectableItemId)
															{
																case 0:
																{
																	BehaviorsMenu.CHIP48_Shift.toggle = (BehaviorsMenu.CHIP48_Shift.toggle) ? false : true;
																	break;
																}
																case 1:
																{
																	BehaviorsMenu.CHIP48_LoadStore.toggle = (BehaviorsMenu.CHIP48_LoadStore.toggle) ? false : true;
																	break;
																}
																case 2:
																{
																	BehaviorsMenu.VIP_Display_Interrupt.toggle = (BehaviorsMenu.VIP_Display_Interrupt.toggle) ? false : true;
																	break;
																}
															}
															ShowMenu(CurrentMenu);
															break;
														}
														case MachineCore::BandCHIP_SuperCHIP:
														{
															switch (BehaviorsMenu.CurrentSelectableItemId)
															{
																case 0:
																{
																	BehaviorsMenu.SuperCHIP_Version.current_option = (BehaviorsMenu.SuperCHIP_Version.current_option == 2) ? 0 : BehaviorsMenu.SuperCHIP_Version.current_option + 1;
																	break;
																}
															}
															ShowMenu(CurrentMenu);
															break;
														}
														case MachineCore::BandCHIP_XOCHIP:
														{
															switch (BehaviorsMenu.CurrentSelectableItemId)
															{
																case 0:
																{
																	BehaviorsMenu.SuperCHIP_Shift.toggle = (BehaviorsMenu.SuperCHIP_Shift.toggle) ? false : true;
																	break;
																}
																case 1:
																{
																	BehaviorsMenu.SuperCHIP_LoadStore.toggle = (BehaviorsMenu.SuperCHIP_LoadStore.toggle) ? false : true;
																	break;
																}
																case 2:
																{
																	BehaviorsMenu.Octo_LoResSprite.toggle = (BehaviorsMenu.Octo_LoResSprite.toggle) ? false : true;
																	break;
																}
															}
															ShowMenu(CurrentMenu);
															break;
														}
													}
													break;
												}
												case MenuDisplay::PaletteSettings:
												{
													switch (PaletteSettingsMenu.CurrentSelectableItemId)
													{
														case 0:
														{
															PaletteSettingsMenu.CurrentIndex.value = (PaletteSettingsMenu.CurrentIndex.value == PaletteSettingsMenu.CurrentIndex.max) ? 0 : PaletteSettingsMenu.CurrentIndex.value + 1;
															RGBColorData data = MainRenderer->GetPaletteIndex(PaletteSettingsMenu.CurrentIndex.value);
															PaletteSettingsMenu.Red.value = data.r;
															PaletteSettingsMenu.Green.value = data.g;
															PaletteSettingsMenu.Blue.value = data.b;
															ShowMenu(CurrentMenu);
															break;
														}
														case 1:
														{
															if (PaletteSettingsMenu.Red.value < PaletteSettingsMenu.Red.max)
															{
																++PaletteSettingsMenu.Red.value;
																ShowMenu(CurrentMenu);
															}
															break;
														}
														case 2:
														{
															if (PaletteSettingsMenu.Green.value < PaletteSettingsMenu.Green.max)
															{
																++PaletteSettingsMenu.Green.value;
																ShowMenu(CurrentMenu);
															}
															break;
														}
														case 3:
														{
															if (PaletteSettingsMenu.Blue.value < PaletteSettingsMenu.Blue.max)
															{
																++PaletteSettingsMenu.Blue.value;
																ShowMenu(CurrentMenu);
															}
															break;
														}
													}
													break;
												}
												case MenuDisplay::FontSettings:
												{
													unsigned int event_id = 0xFFFFFFFF;
													switch (FontSettingsMenu.CurrentSelectableItemId)
													{
														case 0:
														{
															event_id = FontSettingsMenu.LoResFontStyle.event_id;
															break;
														}
														case 1:
														{
															event_id = FontSettingsMenu.HiResFontStyle.event_id;
															break;
														}
														case 2:
														{
															event_id = FontSettingsMenu.ReturnToConfiguration.event_id;
															break;
														}
													}
													switch (static_cast<FontSettingsMenuEvent>(event_id))
													{
														case FontSettingsMenuEvent::ChangeLoResFont:
														{
															if (LoResFontStyleList.size() == 0)
															{
																switch (CurrentLoResFontStyle)
																{
																	case LoResFontStyle::VIP:
																	{
																		CurrentLoResFontStyle = LoResFontStyle::SuperCHIP;
																		FontSettingsMenu.LoResFontStyle.current_option = 1;
																		break;
																	}
																	case LoResFontStyle::SuperCHIP:
																	{
																		CurrentLoResFontStyle = LoResFontStyle::KCHIP8;
																		FontSettingsMenu.LoResFontStyle.current_option = 2;
																		break;
																	}
																	case LoResFontStyle::KCHIP8:
																	{
																		CurrentLoResFontStyle = LoResFontStyle::VIP;
																		FontSettingsMenu.LoResFontStyle.current_option = 0;
																		break;
																	}
																}
															}
															else
															{
																FontSettingsMenu.LoResFontStyle.current_option = (FontSettingsMenu.LoResFontStyle.current_option == LoResFontStyleList.size() - 1) ? 0 : FontSettingsMenu.LoResFontStyle.current_option + 1;
															}
															ShowMenu(CurrentMenu);
															break;
														}
														case FontSettingsMenuEvent::ChangeHiResFont:
														{
															if (HiResFontStyleList.size() == 0)
															{
																switch (CurrentHiResFontStyle)
																{
																	case HiResFontStyle::SuperCHIP:
																	{
																		CurrentHiResFontStyle = HiResFontStyle::Octo;
																		FontSettingsMenu.HiResFontStyle.current_option = 1;
																		break;
																	}
																	case HiResFontStyle::Octo:
																	{
																		CurrentHiResFontStyle = HiResFontStyle::KCHIP8;
																		FontSettingsMenu.HiResFontStyle.current_option = 2;
																		break;
																	}
																	case HiResFontStyle::KCHIP8:
																	{
																		CurrentHiResFontStyle = HiResFontStyle::SuperCHIP;
																		FontSettingsMenu.HiResFontStyle.current_option = 0;
																		break;
																	}
																}
															}
															else
															{
																FontSettingsMenu.HiResFontStyle.current_option = (FontSettingsMenu.HiResFontStyle.current_option == HiResFontStyleList.size() - 1) ? 0 : FontSettingsMenu.HiResFontStyle.current_option + 1;
															}
															ShowMenu(CurrentMenu);
															break;
														}
													}
													break;
												}
												case MenuDisplay::KeyboardRemapping:
												{
													if (!KeyboardRemappingMenu.input_mode)
													{
														if (KeyboardRemappingMenu.CurrentSelectableItemId != 16)
														{
															KeyboardRemappingMenu.CurrentSelectableItemId = (KeyboardRemappingMenu.CurrentSelectableItemId < 8) ? KeyboardRemappingMenu.CurrentSelectableItemId + 8 : KeyboardRemappingMenu.CurrentSelectableItemId - 8;
															ShowMenu(CurrentMenu);
														}
													}
													break;
												}
											}
											break;
										}
										case MenuKey::Select:
										{
											switch (CurrentMenu)
											{
												case MenuDisplay::Main:
												{
													unsigned int event_id = 0;
													switch (MainMenu.CurrentSelectableItemId)
													{
														case 0:
														{
															event_id = MainMenu.RunProgram.event_id;
															break;
														}
														case 1:
														{
															event_id = MainMenu.LoadProgram.event_id;
															break;
														}
														case 2:
														{
															event_id = MainMenu.Configuration.event_id;
															break;
														}
														case 3:
														{
															event_id = MainMenu.Exit.event_id;
															break;
														}
													}
													switch (static_cast<MainMenuEvent>(event_id))
													{
														case MainMenuEvent::RunProgram:
														{
															if (CurrentMachine != nullptr)
															{
																if (CurrentMachine->IsOperational())
																{
																	CurrentOperationMode = OperationMode::Emulator;
																	MainRenderer->SetDisplayMode(DisplayMode::Emulator);
																	CurrentMachine->SetCurrentTime(current_tp);
																	CurrentMachine->PauseProgram(false);
																}
															}
															break;
														}
														case MainMenuEvent::LoadProgram:
														{
															CurrentMenu = MenuDisplay::LoadProgram;
															ShowMenu(CurrentMenu);
															break;
														}
														case MainMenuEvent::Configuration:
														{
															CurrentMenu = MenuDisplay::Configuration;
															ShowMenu(CurrentMenu);
															break;
														}
														case MainMenuEvent::Exit:
														{
															exit = true;
															break;
														}
													}
													break;
												}
												case MenuDisplay::LoadProgram:
												{
													DirectoryEntryData &CurrentItem = LoadProgramMenu.MenuEntry[LoadProgramMenu.CurrentSelectableItemId];
													switch (static_cast<LoadProgramMenuEvent>(CurrentItem.Entry.event_id))
													{
														case LoadProgramMenuEvent::ChangeDirectory:
														{
															std::filesystem::path current_dir(CurrentItem.Entry.Text);
															std::filesystem::current_path(current_dir);
															LoadProgramMenu.MenuEntry[0].Type = DirEntryType::Directory;
															LoadProgramMenu.MenuEntry[0].Entry.Text = "..";
															LoadProgramMenu.MenuEntry[0].Entry.event_id = static_cast<unsigned int>(LoadProgramMenuEvent::ChangeDirectory);
															LoadProgramMenu.CurrentSelectableItemId = 0;
															LoadProgramMenu.StartEntry = 0;
															LoadProgramMenu.CurrentDirectoryCount = 1;
															for (unsigned int i = 0; i < 15; ++i)
															{
																LoadProgramMenu.MenuEntry[1 + i].Entry.hidden = true;
															}
															unsigned int count = 0;
															for (auto i : std::filesystem::directory_iterator("."))
															{
																if (count < 15)
																{
																	std::filesystem::path c_path = i.path();
																	unsigned int current_event_id = 0;
																	if (std::filesystem::is_directory(c_path))
																	{
																		current_event_id = static_cast<unsigned int>(LoadProgramMenuEvent::ChangeDirectory);
																		LoadProgramMenu.MenuEntry[1 + count].Type = DirEntryType::Directory;
																	}
																	else
																	{
																		current_event_id = static_cast<unsigned int>(LoadProgramMenuEvent::Load);
																		LoadProgramMenu.MenuEntry[1 + count].Type = DirEntryType::File;
																	}
																	LoadProgramMenu.MenuEntry[1 + count].Entry.Text = c_path.filename();
																	LoadProgramMenu.MenuEntry[1 + count].Entry.event_id = current_event_id;
																	LoadProgramMenu.MenuEntry[1 + count].Entry.hidden = false;
																}
																++count;
															}
															LoadProgramMenu.CurrentDirectoryCount += count;
															ShowMenu(MenuDisplay::LoadProgram);
															break;
														};
														case LoadProgramMenuEvent::Load:
														{
															LoadProgramDisplay.LoadingProgram.hidden = false;
															loading_program = true;
															CurrentMenu = MenuDisplay::LoadProgramDisplay;
															ShowMenu(MenuDisplay::LoadProgramDisplay);
															break;
														}
													}
													break;
												}
												case MenuDisplay::Configuration:
												{
													unsigned int event_id = 0;
													switch (ConfigurationMenu.CurrentSelectableItemId)
													{
														case 0:
														{
															event_id = ConfigurationMenu.Core.event_id;
															break;
														}
														case 1:
														{
															event_id = ConfigurationMenu.CPUSettings.event_id;
															break;
														}
														case 2:
														{
															event_id = ConfigurationMenu.Behaviors.event_id;
															break;
														}
														case 3:
														{
															event_id = ConfigurationMenu.PaletteSettings.event_id;
															break;
														}
														case 4:
														{
															event_id = ConfigurationMenu.FontSettings.event_id;
															break;
														}
														case 5:
														{
															event_id = ConfigurationMenu.KeyboardRemapping.event_id;
															break;
														}
														case 6:
														{
															event_id = ConfigurationMenu.LoadConfiguration.event_id;
															break;
														}
														case 7:
														{
															event_id = ConfigurationMenu.SaveConfiguration.event_id;
															break;
														}
														case 8:
														{
															event_id = ConfigurationMenu.ReturnToMainMenu.event_id;
															break;
														}
													}
													switch (static_cast<ConfigurationMenuEvent>(event_id))
													{
														case ConfigurationMenuEvent::ChangeCore:
														{
															switch (CurrentMachineCore)
															{
																case MachineCore::BandCHIP_CHIP8:
																{
																	CurrentMachineCore = MachineCore::BandCHIP_SuperCHIP;
																	ConfigurationMenu.Core.current_option = 1;
																	BehaviorsMenu.SuperCHIP_Version.current_option = 0;
																	break;
																}
																case MachineCore::BandCHIP_SuperCHIP:
																{
																	CurrentMachineCore = MachineCore::BandCHIP_XOCHIP;
																	ConfigurationMenu.Core.current_option = 2;
																	break;
																}
																case MachineCore::BandCHIP_XOCHIP:
																{
																	CurrentMachineCore = MachineCore::BandCHIP_HyperCHIP64;
																	ConfigurationMenu.Core.current_option = 3;
																	break;
																}
																case MachineCore::BandCHIP_HyperCHIP64:
																{
																	CurrentMachineCore = MachineCore::BandCHIP_CHIP8;
																	ConfigurationMenu.Core.current_option = 0;
																	BehaviorsMenu.CHIP48_Shift.toggle = false;
																	BehaviorsMenu.CHIP48_LoadStore.toggle = false;
																	BehaviorsMenu.VIP_Display_Interrupt.toggle = false;
																	break;
																}
															}
															BehaviorsMenu.CurrentSelectableItemId = 0;
															ShowMenu(CurrentMenu);
															break;
														}
														case ConfigurationMenuEvent::CPUSettings:
														{
															CurrentMenu = MenuDisplay::CPUSettings;
															ShowMenu(CurrentMenu);
															break;
														}
														case ConfigurationMenuEvent::Behaviors:
														{
															CurrentMenu = MenuDisplay::Behaviors;
															ShowMenu(CurrentMenu);
															break;
														}
														case ConfigurationMenuEvent::PaletteSettings:
														{
															CurrentMenu = MenuDisplay::PaletteSettings;
															ShowMenu(CurrentMenu);
															break;
														}
														case ConfigurationMenuEvent::FontSettings:
														{
															CurrentMenu = MenuDisplay::FontSettings;
															ShowMenu(CurrentMenu);
															break;
														}
														case ConfigurationMenuEvent::KeyboardRemapping:
														{
															CurrentMenu = MenuDisplay::KeyboardRemapping;
															ShowMenu(CurrentMenu);
															break;
														}
														case ConfigurationMenuEvent::LoadConfiguration:
														{
															CurrentMenu = MenuDisplay::LoadConfiguration;
															ShowMenu(CurrentMenu);
															break;
														}
														case ConfigurationMenuEvent::ReturnToMainMenu:
														{
															CurrentMenu = MenuDisplay::Main;
															ShowMenu(CurrentMenu);
															break;
														}
													}
													break;
												}
												case MenuDisplay::LoadProgramDisplay:
												{
													if (!LoadProgramDisplay.Ok.hidden)
													{
														LoadProgramDisplay.LoadSuccessful.hidden = true;
														LoadProgramDisplay.LoadFailed.hidden = true;
														LoadProgramDisplay.Ok.hidden = true;
														CurrentMenu = MenuDisplay::Main;
														ShowMenu(CurrentMenu);
													}
													break;
												}
												case MenuDisplay::CPUSettings:
												{
													unsigned int event_id = 0xFFFFFFFF;
													switch (CPUSettingsMenu.CurrentSelectableItemId)
													{
														case 2:
														{
															CPUSettingsMenu.Sync.toggle = (CPUSettingsMenu.Sync.toggle) ? false : true;
															ShowMenu(CurrentMenu);
															break;
														}
														case 3:
														{
															event_id = CPUSettingsMenu.CommitChanges.event_id;
															break;
														}
														case 4:
														{
															event_id = CPUSettingsMenu.ReturnToConfiguration.event_id;
															break;
														}
													}
													switch (static_cast<CPUSettingsMenuEvent>(event_id))
													{
														case CPUSettingsMenuEvent::CommitChanges:
														{
															if (CurrentMachine != nullptr && CPUSettingsMenu.ChangeStatus.Status == "Changed")
															{
																CurrentMachine->SetCyclesPerSecond(CPUSettingsMenu.CPUCycles.value);
																CPUSettingsMenu.ChangeStatus.Status = "Unchanged";
																ShowMenu(CurrentMenu);
															}
															break;
														}
														case CPUSettingsMenuEvent::ReturnToConfiguration:
														{
															CurrentMenu = MenuDisplay::Configuration;
															ShowMenu(CurrentMenu);
															break;
														}
													}
													break;
												}
												case MenuDisplay::Behaviors:
												{
													switch (CurrentMachineCore)
													{
														case MachineCore::BandCHIP_CHIP8:
														{
															switch (BehaviorsMenu.CurrentSelectableItemId)
															{
																case 0:
																{
																	BehaviorsMenu.CHIP48_Shift.toggle = (BehaviorsMenu.CHIP48_Shift.toggle) ? false : true;
																	break;
																}
																case 1:
																{
																	BehaviorsMenu.CHIP48_LoadStore.toggle = (BehaviorsMenu.CHIP48_LoadStore.toggle) ? false : true;
																	break;
																}
																case 2:
																{
																	BehaviorsMenu.VIP_Display_Interrupt.toggle = (BehaviorsMenu.VIP_Display_Interrupt.toggle) ? false : true;
																	break;
																}
																case 3:
																{
																	CurrentMenu = MenuDisplay::Configuration;
																	break;
																}
															}
															ShowMenu(CurrentMenu);
															break;
														}
														case MachineCore::BandCHIP_SuperCHIP:
														{
															switch (BehaviorsMenu.CurrentSelectableItemId)
															{
																case 0:
																{
																	BehaviorsMenu.SuperCHIP_Version.current_option = (BehaviorsMenu.SuperCHIP_Version.current_option == 2) ? 0 : BehaviorsMenu.SuperCHIP_Version.current_option + 1;
																	break;
																}
																case 1:
																{
																	CurrentMenu = MenuDisplay::Configuration;
																	break;
																}
															}
															ShowMenu(CurrentMenu);
															break;
														}
														case MachineCore::BandCHIP_XOCHIP:
														{
															switch (BehaviorsMenu.CurrentSelectableItemId)
															{
																case 0:
																{
																	BehaviorsMenu.SuperCHIP_Shift.toggle = (BehaviorsMenu.SuperCHIP_Shift.toggle) ? false : true;
																	break;
																}
																case 1:
																{
																	BehaviorsMenu.SuperCHIP_LoadStore.toggle = (BehaviorsMenu.SuperCHIP_LoadStore.toggle) ? false : true;
																	break;
																}
																case 2:
																{
																	BehaviorsMenu.Octo_LoResSprite.toggle = (BehaviorsMenu.Octo_LoResSprite.toggle) ? false : true;
																	break;
																}
																case 3:
																{
																	CurrentMenu = MenuDisplay::Configuration;
																	break;
																}
															}
															ShowMenu(CurrentMenu);
															break;
														}
														default:
														{
															CurrentMenu = MenuDisplay::Configuration;
															ShowMenu(CurrentMenu);
															break;
														}
													}
													break;
												}
												case MenuDisplay::PaletteSettings:
												{
													unsigned int event_id = 0xFFFFFFFF;
													switch (PaletteSettingsMenu.CurrentSelectableItemId)
													{
														case 4:
														{
															event_id = PaletteSettingsMenu.CommitChanges.event_id;
															break;
														}
														case 5:
														{
															event_id = PaletteSettingsMenu.ReturnToConfiguration.event_id;
															break;
														}
													}
													switch (static_cast<PaletteSettingsMenuEvent>(event_id))
													{
														case PaletteSettingsMenuEvent::CommitChanges:
														{
															RGBColorData data = { static_cast<unsigned char>(PaletteSettingsMenu.Red.value), static_cast<unsigned char>(PaletteSettingsMenu.Green.value), static_cast<unsigned char>(PaletteSettingsMenu.Blue.value) };
															MainRenderer->SetPaletteIndex(data, static_cast<unsigned char>(PaletteSettingsMenu.CurrentIndex.value));
															ShowMenu(CurrentMenu);
															break;
														}
														case PaletteSettingsMenuEvent::ReturnToConfiguration:
														{
															CurrentMenu = MenuDisplay::Configuration;
															ShowMenu(CurrentMenu);
															break;
														}
													}
													break;
												}
												case MenuDisplay::FontSettings:
												{
													unsigned int event_id = 0xFFFFFFFF;
													switch (FontSettingsMenu.CurrentSelectableItemId)
													{
														case 0:
														{
															event_id = FontSettingsMenu.LoResFontStyle.event_id;
															break;
														}
														case 1:
														{
															event_id = FontSettingsMenu.HiResFontStyle.event_id;
															break;
														}
														case 2:
														{
															event_id = FontSettingsMenu.ReturnToConfiguration.event_id;
															break;
														}
													}
													switch (static_cast<FontSettingsMenuEvent>(event_id))
													{
														case FontSettingsMenuEvent::ChangeLoResFont:
														{
															if (LoResFontStyleList.size() == 0)
															{
																switch (CurrentLoResFontStyle)
																{
																	case LoResFontStyle::VIP:
																	{
																		CurrentLoResFontStyle = LoResFontStyle::SuperCHIP;
																		FontSettingsMenu.LoResFontStyle.current_option = 1;
																		break;
																	}
																	case LoResFontStyle::SuperCHIP:
																	{
																		CurrentLoResFontStyle = LoResFontStyle::KCHIP8;
																		FontSettingsMenu.LoResFontStyle.current_option = 2;
																		break;
																	}
																	case LoResFontStyle::KCHIP8:
																	{
																		CurrentLoResFontStyle = LoResFontStyle::VIP;
																		FontSettingsMenu.LoResFontStyle.current_option = 0;
																		break;
																	}
																}
															}
															else
															{
																FontSettingsMenu.LoResFontStyle.current_option = (FontSettingsMenu.LoResFontStyle.current_option == LoResFontStyleList.size() - 1) ? 0 : FontSettingsMenu.LoResFontStyle.current_option + 1;
															}
															ShowMenu(CurrentMenu);
															break;
														}
														case FontSettingsMenuEvent::ChangeHiResFont:
														{
															if (HiResFontStyleList.size() == 0)
															{
																switch (CurrentHiResFontStyle)
																{
																	case HiResFontStyle::SuperCHIP:
																	{
																		CurrentHiResFontStyle = HiResFontStyle::Octo;
																		FontSettingsMenu.HiResFontStyle.current_option = 1;
																		break;
																	}
																	case HiResFontStyle::Octo:
																	{
																		CurrentHiResFontStyle = HiResFontStyle::KCHIP8;
																		FontSettingsMenu.HiResFontStyle.current_option = 2;
																		break;
																	}
																	case HiResFontStyle::KCHIP8:
																	{
																		CurrentHiResFontStyle = HiResFontStyle::SuperCHIP;
																		FontSettingsMenu.HiResFontStyle.current_option = 0;
																		break;
																	}
																}
															}
															else
															{
																FontSettingsMenu.HiResFontStyle.current_option = (FontSettingsMenu.HiResFontStyle.current_option == HiResFontStyleList.size() - 1) ? 0 : FontSettingsMenu.HiResFontStyle.current_option + 1;
															}
															ShowMenu(CurrentMenu);
															break;
														}
														case FontSettingsMenuEvent::ReturnToConfiguration:
														{
															CurrentMenu = MenuDisplay::Configuration;
															ShowMenu(CurrentMenu);
															break;
														}
													}
													break;
												}
												case MenuDisplay::KeyboardRemapping:
												{
													if (!KeyboardRemappingMenu.input_mode)
													{
														if (KeyboardRemappingMenu.CurrentSelectableItemId < 16)
														{
															KeyboardRemappingMenu.input_mode = true;
															KeyboardRemappingMenu.previous_key = KeyboardRemappingMenu.Keys[KeyboardRemappingMenu.CurrentSelectableItemId].input_data[0];
															KeyboardRemappingMenu.Keys[KeyboardRemappingMenu.CurrentSelectableItemId].input_data[0] = '\0';
															ShowMenu(CurrentMenu);
														}
														else
														{
															unsigned int event_id = KeyboardRemappingMenu.ReturnToConfiguration.event_id;
															switch(static_cast<KeyboardRemappingMenuEvent>(event_id))
															{
																case KeyboardRemappingMenuEvent::ReturnToConfiguration:
																{
																	CurrentMenu = MenuDisplay::Configuration;
																	ShowMenu(CurrentMenu);
																	break;
																}
															}
														}
													}
													break;
												}
											}
											break;
										}
										case MenuKey::Exit:
										{
											switch (CurrentMenu)
											{
												case MenuDisplay::Main:
												{
													exit = true;
													break;
												}
												case MenuDisplay::LoadProgram:
												{
													CurrentMenu = MenuDisplay::Main;
													ShowMenu(CurrentMenu);
													break;
												}
												case MenuDisplay::Configuration:
												{
													CurrentMenu = MenuDisplay::Main;
													ShowMenu(CurrentMenu);
													break;
												}
												case MenuDisplay::CPUSettings:
												{
													CurrentMenu = MenuDisplay::Configuration;
													ShowMenu(CurrentMenu);
													break;
												}
												case MenuDisplay::Behaviors:
												{
													CurrentMenu = MenuDisplay::Configuration;
													ShowMenu(CurrentMenu);
													break;
												}
												case MenuDisplay::PaletteSettings:
												{
													CurrentMenu = MenuDisplay::Configuration;
													ShowMenu(CurrentMenu);
													break;
												}
												case MenuDisplay::FontSettings:
												{
													CurrentMenu = MenuDisplay::Configuration;
													ShowMenu(CurrentMenu);
													break;
												}
												case MenuDisplay::KeyboardRemapping:
												{
													if (!KeyboardRemappingMenu.input_mode)
													{
														CurrentMenu = MenuDisplay::Configuration;
													}
													else
													{
														KeyboardRemappingMenu.Keys[KeyboardRemappingMenu.CurrentSelectableItemId].input_data[0] = KeyboardRemappingMenu.previous_key;
														KeyboardRemappingMenu.previous_key = '\0';
														KeyboardRemappingMenu.input_mode = false;
													}
													ShowMenu(CurrentMenu);
													break;
												}
												case MenuDisplay::LoadConfiguration:
												{
													CurrentMenu = MenuDisplay::Configuration;
													ShowMenu(CurrentMenu);
													break;
												}
												case MenuDisplay::ErrorDisplay:
												{
													CurrentMenu = MenuDisplay::Main;
													ShowMenu(CurrentMenu);
													break;
												}
											}
											break;
										}
									}
									key_found = true;
									break;
								}
							}
							if (!key_found)
							{
								switch (CurrentMenu)
								{
									case MenuDisplay::KeyboardRemapping:
									{
										if (KeyboardRemappingMenu.input_mode)
										{
											for (auto [key, i] : Input_KeyMap)
											{
												if (i == scancode)
												{
													for (unsigned char i2 = 0x00; i2 < 0x10; ++i2)
													{
														if (i2 == KeyboardRemappingMenu.CurrentSelectableItemId)
														{
															continue;
														}
														if (KeyboardRemappingMenu.Keys[i2].input_data[0] == key)
														{
															KeyboardRemappingMenu.Keys[i2].input_data[0] = KeyboardRemappingMenu.previous_key;
															CHIP8_KeyMap[static_cast<CHIP8Key>(i2)] = Input_KeyMap[KeyboardRemappingMenu.Keys[i2].input_data[0]];
															break;
														}
													}
													KeyboardRemappingMenu.Keys[KeyboardRemappingMenu.CurrentSelectableItemId].input_data[0] = key;
													CHIP8_KeyMap[static_cast<CHIP8Key>(KeyboardRemappingMenu.CurrentSelectableItemId)] = Input_KeyMap[KeyboardRemappingMenu.Keys[KeyboardRemappingMenu.CurrentSelectableItemId].input_data[0]];
													KeyboardRemappingMenu.previous_key = '\0';
													KeyboardRemappingMenu.input_mode = false;
													ShowMenu(CurrentMenu);
													break;	
												}
											}
										}
										break;
									}
								}
							}
							break;
						}
						case OperationMode::Emulator:
						{
							bool key_found = false;
							for (auto [key, i] : CHIP8_KeyMap)
							{
								if (i == scancode)
								{
									CurrentMachine->SetKeyStatus(static_cast<unsigned char>(key), true);
									key_found = true;
									break;
								}
							}
							if (!key_found)
							{
								if (scancode == SDL_SCANCODE_ESCAPE)
								{
									CurrentMachine->SetCurrentTime(current_tp);
									CurrentMachine->PauseProgram(true);
									CurrentOperationMode = OperationMode::Menu;
									MainRenderer->SetDisplayMode(DisplayMode::Menu);
								}
							}
							break;
						}
					}
					break;
				}
				case SDL_KEYUP:
				{
					SDL_Scancode scancode = event.key.keysym.scancode;
					switch(CurrentOperationMode)
					{
						case OperationMode::Menu:
						{
							break;
						}
						case OperationMode::Emulator:
						{
							for (unsigned char i = 0x00; i < 0x10; ++i)
							{
								if (CHIP8_KeyMap[static_cast<CHIP8Key>(i)] == scancode)
								{
									CurrentMachine->SetKeyStatus(i, false);
								}
							}
							break;
						}
					}
					break;
				}
				case SDL_QUIT:
				{
					exit = true;
					break;
				}
			}
		}
		bool sync = false;
		if (CurrentMachine != nullptr)
		{
			sync = CurrentMachine->GetSync();
			if (!CurrentMachine->IsPaused())
			{
				if (idle)
				{
					idle = false;
				}
				CurrentMachine->SetCurrentTime(current_tp);
				if (!sync)
				{
					CurrentMachine->RunDelayTimer();
					CurrentMachine->RunSoundTimer();
				}
				CurrentMachine->ExecuteInstructions();
				if (!sync)
				{
					unsigned char *display = nullptr;
					unsigned short display_width = 0;
					unsigned short display_height = 0;
					CurrentMachine->GetDisplay(&display, &display_width, &display_height);
					MainRenderer->WriteToDisplay(display, display_width, display_height);
				}
				MachineError Error = CurrentMachine->GetErrorState();
				if (Error != MachineError::NoError)
				{
					MachineState State = CurrentMachine->GetMachineState();
					switch (Error)
					{
						case MachineError::InvalidInstruction:
						{
							ErrorDisplay.Error.Status = "Invalid Instruction";
							break;
						}
						case MachineError::StackOverflow:
						{
							ErrorDisplay.Error.Status = "Stack Overflow";
							break;
						}
						case MachineError::StackUnderflow:
						{
							ErrorDisplay.Error.Status = "Stack Underflow";
							break;
						}
						case MachineError::MachineInstructionsUnsupported:
						{
							ErrorDisplay.Error.Status = "Machine Instructions Unsupported";
							break;
						}
					}
					for (unsigned char i = 0x00; i < 0x10; ++i)
					{
						ErrorDisplay.RegisterValue[i].value = State.V[i];
					}
					ErrorDisplay.ProgramCounterValue.value = State.PC;
					ErrorDisplay.AddressRegisterValue.value = State.I;
					ErrorDisplay.DelayTimerValue.value = State.DT;
					for (unsigned char i = 0; i < 4; ++i)
					{
						ErrorDisplay.SoundTimerValue[i].value = State.ST[i];
					}
					CurrentOperationMode = OperationMode::Menu;
					CurrentMenu = MenuDisplay::ErrorDisplay;
					MainRenderer->SetDisplayMode(DisplayMode::Menu);
					ShowMenu(CurrentMenu);
				}
				else if (!CurrentMachine->IsOperational())
				{
					CurrentOperationMode = OperationMode::Menu;
					CurrentMenu = MenuDisplay::Main;
					MainRenderer->SetDisplayMode(DisplayMode::Menu);
					MainMenu.CurrentMachineStatus.Status = "Non-Operational";
					ShowMenu(CurrentMenu);
				}
			}
		}
		if (!sync || CurrentOperationMode == OperationMode::Menu)
		{
			std::chrono::duration<double> delta_time = current_tp - refresh_tp;
			if (delta_time.count() > 0.25)
			{
				delta_time = std::chrono::duration<double>(0.25);
			}
			refresh_accumulator += delta_time.count();
			// refresh_time_accumulator += delta_time.count();
			refresh_tp = current_tp;
			if (refresh_accumulator >= 1.0 / 60.0)
			{
				refresh_accumulator = 0.0;
				MainRenderer->Render();
				/*
				++frame_count;
				if (refresh_time_accumulator >= 1.0)
				{
					std::ostringstream window_title;
					window_title << "Hyper BandCHIP Emulator (FPS: " << frame_count << ')';
					SDL_SetWindowTitle(MainWindow.get(), window_title.str().c_str());
					frame_count = 0;
					refresh_time_accumulator -= 1.0;
				}
				*/
				// SDL_Delay(10);
			}
		}
		if (idle)
		{
			SDL_Delay(1);
		}
	}
}

Hyper_BandCHIP::Application::~Application()
{
}

/*
int Hyper_BandCHIP::Application::GetReturnCode() const
{
	return retcode;
}
*/

void Hyper_BandCHIP::Application::InitializeKeyMaps()
{
	Menu_KeyMap[MenuKey::Up] = SDL_SCANCODE_UP;
	Menu_KeyMap[MenuKey::Down] = SDL_SCANCODE_DOWN;
	Menu_KeyMap[MenuKey::Left] = SDL_SCANCODE_LEFT;
	Menu_KeyMap[MenuKey::Right] = SDL_SCANCODE_RIGHT;
	Menu_KeyMap[MenuKey::Select] = SDL_SCANCODE_RETURN;
	Menu_KeyMap[MenuKey::Exit] = SDL_SCANCODE_ESCAPE;

	CHIP8_KeyMap[CHIP8Key::Key_1] = SDL_SCANCODE_1;
	CHIP8_KeyMap[CHIP8Key::Key_2] = SDL_SCANCODE_2;
	CHIP8_KeyMap[CHIP8Key::Key_3] = SDL_SCANCODE_3;
	CHIP8_KeyMap[CHIP8Key::Key_C] = SDL_SCANCODE_4;
	CHIP8_KeyMap[CHIP8Key::Key_4] = SDL_SCANCODE_Q;
	CHIP8_KeyMap[CHIP8Key::Key_5] = SDL_SCANCODE_W;
	CHIP8_KeyMap[CHIP8Key::Key_6] = SDL_SCANCODE_E;
	CHIP8_KeyMap[CHIP8Key::Key_D] = SDL_SCANCODE_R;
	CHIP8_KeyMap[CHIP8Key::Key_7] = SDL_SCANCODE_A;
	CHIP8_KeyMap[CHIP8Key::Key_8] = SDL_SCANCODE_S;
	CHIP8_KeyMap[CHIP8Key::Key_9] = SDL_SCANCODE_D;
	CHIP8_KeyMap[CHIP8Key::Key_E] = SDL_SCANCODE_F;
	CHIP8_KeyMap[CHIP8Key::Key_A] = SDL_SCANCODE_Z;
	CHIP8_KeyMap[CHIP8Key::Key_0] = SDL_SCANCODE_X;
	CHIP8_KeyMap[CHIP8Key::Key_B] = SDL_SCANCODE_C;
	CHIP8_KeyMap[CHIP8Key::Key_F] = SDL_SCANCODE_V;

	Input_KeyMap[' '] = SDL_SCANCODE_SPACE;
	Input_KeyMap['0'] = SDL_SCANCODE_0;
	Input_KeyMap['1'] = SDL_SCANCODE_1;
	Input_KeyMap['2'] = SDL_SCANCODE_2;
	Input_KeyMap['3'] = SDL_SCANCODE_3;
	Input_KeyMap['4'] = SDL_SCANCODE_4;
	Input_KeyMap['5'] = SDL_SCANCODE_5;
	Input_KeyMap['6'] = SDL_SCANCODE_6;
	Input_KeyMap['7'] = SDL_SCANCODE_7;
	Input_KeyMap['8'] = SDL_SCANCODE_8;
	Input_KeyMap['9'] = SDL_SCANCODE_9;
	Input_KeyMap['A'] = SDL_SCANCODE_A;
	Input_KeyMap['B'] = SDL_SCANCODE_B;
	Input_KeyMap['C'] = SDL_SCANCODE_C;
	Input_KeyMap['D'] = SDL_SCANCODE_D;
	Input_KeyMap['E'] = SDL_SCANCODE_E;
	Input_KeyMap['F'] = SDL_SCANCODE_F;
	Input_KeyMap['G'] = SDL_SCANCODE_G;
	Input_KeyMap['H'] = SDL_SCANCODE_H;
	Input_KeyMap['I'] = SDL_SCANCODE_I;
	Input_KeyMap['J'] = SDL_SCANCODE_J;
	Input_KeyMap['K'] = SDL_SCANCODE_K;
	Input_KeyMap['L'] = SDL_SCANCODE_L;
	Input_KeyMap['M'] = SDL_SCANCODE_M;
	Input_KeyMap['N'] = SDL_SCANCODE_N;
	Input_KeyMap['O'] = SDL_SCANCODE_O;
	Input_KeyMap['P'] = SDL_SCANCODE_P;
	Input_KeyMap['Q'] = SDL_SCANCODE_Q;
	Input_KeyMap['R'] = SDL_SCANCODE_R;
	Input_KeyMap['S'] = SDL_SCANCODE_S;
	Input_KeyMap['T'] = SDL_SCANCODE_T;
	Input_KeyMap['U'] = SDL_SCANCODE_U;
	Input_KeyMap['V'] = SDL_SCANCODE_V;
	Input_KeyMap['W'] = SDL_SCANCODE_W;
	Input_KeyMap['X'] = SDL_SCANCODE_X;
	Input_KeyMap['Y'] = SDL_SCANCODE_Y;
	Input_KeyMap['Z'] = SDL_SCANCODE_Z;
}

void Hyper_BandCHIP::Application::ConstructMenus()
{
	std::filesystem::path current_dir(".");
	unsigned short start_y = 54;
	unsigned short count = 0;
	LoadProgramMenu.MenuEntry[0].Entry = { "..", 100, 40, static_cast<unsigned int>(LoadProgramMenuEvent::ChangeDirectory), false };
	LoadProgramMenu.MenuEntry[0].Type = DirEntryType::Directory;
	for (unsigned int i = 0; i < 15; ++i)
	{
		LoadProgramMenu.MenuEntry[1 + i].Entry = { "", 100, static_cast<unsigned short>(start_y + (i * 14)), 0xFFFFFFFF, true };
	}
	for (auto i : std::filesystem::directory_iterator(current_dir))
	{
		if (count < 15)
		{
			std::filesystem::path c_path = i.path();
			unsigned int current_event_id = 0;
			if (std::filesystem::is_directory(c_path))
			{
				current_event_id = static_cast<unsigned int>(LoadProgramMenuEvent::ChangeDirectory);
				LoadProgramMenu.MenuEntry[1 + count].Type = DirEntryType::Directory;
			}
			else
			{
				current_event_id = static_cast<unsigned int>(LoadProgramMenuEvent::Load);
				LoadProgramMenu.MenuEntry[1 + count].Type = DirEntryType::File;
			}
			LoadProgramMenu.MenuEntry[1 + count].Entry.Text = c_path.filename();
			LoadProgramMenu.MenuEntry[1 + count].Entry.event_id = current_event_id;
			LoadProgramMenu.MenuEntry[1 + count].Entry.hidden = false;
		}
		++count;
	}
	LoadProgramMenu.CurrentDirectoryCount += count;
	start_y = 40;
	count = 0;
	for (unsigned char i = 0; i < 16; ++i)
	{
		LoadConfigurationMenu.MenuEntry[i].Entry = { "", 100, static_cast<unsigned short>(start_y + (i * 14)), 0xFFFFFFFF, true };
	}
	for (auto i : std::filesystem::directory_iterator(current_dir))
	{
		std::filesystem::path c_path = i.path();
		if (std::filesystem::is_regular_file(c_path))
		{
			if (count < 16)
			{
				LoadConfigurationMenu.MenuEntry[count].Type = DirEntryType::File;
				LoadConfigurationMenu.MenuEntry[count].Entry.Text = c_path.filename();
				LoadConfigurationMenu.MenuEntry[count].Entry.event_id = static_cast<unsigned int>(LoadConfigurationMenuEvent::Load);
				LoadConfigurationMenu.MenuEntry[count].Entry.hidden = false;
			}
			++count;
		}
	}
	LoadConfigurationMenu.CurrentConfigurationFileCount = count;
}

void Hyper_BandCHIP::Application::ShowMenu(Hyper_BandCHIP::MenuDisplay Menu)
{
	MainRenderer->ClearMenu();
	switch (Menu)
	{
		case MenuDisplay::Main:
		{
			DisplayItem(*MainRenderer, MainMenu.Title, 1);
			DisplayItem(*MainRenderer, MainMenu.Author, 1);
			DisplayItem(*MainRenderer, MainMenu.CurrentProgram, 1);
			DisplayItem(*MainRenderer, MainMenu.CurrentMachineStatus, 1);
			DisplayItem(*MainRenderer, MainMenu.RunProgram, (MainMenu.CurrentSelectableItemId == 0) ? 2 : 1);
			DisplayItem(*MainRenderer, MainMenu.LoadProgram, (MainMenu.CurrentSelectableItemId == 1) ? 2 : 1);
			DisplayItem(*MainRenderer, MainMenu.Configuration, (MainMenu.CurrentSelectableItemId == 2) ? 2 : 1);
			DisplayItem(*MainRenderer, MainMenu.Exit, (MainMenu.CurrentSelectableItemId == 3) ? 2 : 1);
			break;
		}
		case MenuDisplay::LoadProgram:
		{
			DisplayItem(*MainRenderer, LoadProgramMenu.Title, 1);
			for (unsigned int i = 0; i < 16; i++)
			{
				DirectoryEntryData &CurrentMenuEntry = LoadProgramMenu.MenuEntry[i];
				if (!CurrentMenuEntry.Entry.hidden)
				{
					unsigned char unselected_color = (CurrentMenuEntry.Type == DirEntryType::File) ? 3 : 1;
					DisplayItem(*MainRenderer, CurrentMenuEntry.Entry, (LoadProgramMenu.CurrentSelectableItemId == i) ? 2 : unselected_color);
				}
			}
			break;
		}
		case MenuDisplay::Configuration:
		{
			DisplayItem(*MainRenderer, ConfigurationMenu.Title, 1);
			DisplayItem(*MainRenderer, ConfigurationMenu.Core, (ConfigurationMenu.CurrentSelectableItemId == 0) ? 2 : 1);
			DisplayItem(*MainRenderer, ConfigurationMenu.CPUSettings, (ConfigurationMenu.CurrentSelectableItemId == 1) ? 2 : 1);
			DisplayItem(*MainRenderer, ConfigurationMenu.Behaviors, (ConfigurationMenu.CurrentSelectableItemId == 2) ? 2 : 1);
			DisplayItem(*MainRenderer, ConfigurationMenu.PaletteSettings, (ConfigurationMenu.CurrentSelectableItemId == 3) ? 2 : 1);
			DisplayItem(*MainRenderer, ConfigurationMenu.FontSettings, (ConfigurationMenu.CurrentSelectableItemId == 4) ? 2 : 1);
			DisplayItem(*MainRenderer, ConfigurationMenu.KeyboardRemapping, (ConfigurationMenu.CurrentSelectableItemId == 5) ? 2 : 1);
			DisplayItem(*MainRenderer, ConfigurationMenu.LoadConfiguration, (ConfigurationMenu.CurrentSelectableItemId == 6) ? 2 : 1);
			DisplayItem(*MainRenderer, ConfigurationMenu.SaveConfiguration, (ConfigurationMenu.CurrentSelectableItemId == 7) ? 2 : 1);
			DisplayItem(*MainRenderer, ConfigurationMenu.ReturnToMainMenu, (ConfigurationMenu.CurrentSelectableItemId == 8) ? 2 : 1);
			break;
		}
		case MenuDisplay::LoadProgramDisplay:
		{
			if (!LoadProgramDisplay.LoadingProgram.hidden)
			{
				DisplayItem(*MainRenderer, LoadProgramDisplay.LoadingProgram, 1);
			}
			if (!LoadProgramDisplay.LoadSuccessful.hidden)
			{
				DisplayItem(*MainRenderer, LoadProgramDisplay.LoadSuccessful, 1);
			}
			if (!LoadProgramDisplay.LoadFailed.hidden)
			{
				DisplayItem(*MainRenderer, LoadProgramDisplay.LoadFailed, 1);
			}
			if (!LoadProgramDisplay.Ok.hidden)
			{
				DisplayItem(*MainRenderer, LoadProgramDisplay.Ok, 2);
			}
			break;
		}
		case MenuDisplay::CPUSettings:
		{
			DisplayItem(*MainRenderer, CPUSettingsMenu.Title, 1);
			DisplayItem(*MainRenderer, CPUSettingsMenu.ChangeStatus, 1);
			DisplayItem(*MainRenderer, CPUSettingsMenu.CPUCycles, (CPUSettingsMenu.CurrentSelectableItemId == 0) ? 2 : 1);
			DisplayItem(*MainRenderer, CPUSettingsMenu.AdjustmentModifier, (CPUSettingsMenu.CurrentSelectableItemId == 1) ? 2 : 1);
			DisplayItem(*MainRenderer, CPUSettingsMenu.Sync, (CPUSettingsMenu.CurrentSelectableItemId == 2) ? 2 : 1);
			DisplayItem(*MainRenderer, CPUSettingsMenu.CommitChanges, (CPUSettingsMenu.CurrentSelectableItemId == 3) ? 2 : 1);
			DisplayItem(*MainRenderer, CPUSettingsMenu.ReturnToConfiguration, (CPUSettingsMenu.CurrentSelectableItemId == 4) ? 2 : 1);
			break;
		}
		case MenuDisplay::Behaviors:
		{
			DisplayItem(*MainRenderer, BehaviorsMenu.Title, 1);
			switch (CurrentMachineCore)
			{
				case MachineCore::BandCHIP_CHIP8:
				{
					DisplayItem(*MainRenderer, BehaviorsMenu.CHIP48_Shift, (BehaviorsMenu.CurrentSelectableItemId == 0) ? 2 : 1);
					DisplayItem(*MainRenderer, BehaviorsMenu.CHIP48_LoadStore, (BehaviorsMenu.CurrentSelectableItemId == 1) ? 2 : 1);
					DisplayItem(*MainRenderer, BehaviorsMenu.VIP_Display_Interrupt, (BehaviorsMenu.CurrentSelectableItemId == 2) ? 2 : 1);
					DisplayItem(*MainRenderer, BehaviorsMenu.ReturnToConfiguration, (BehaviorsMenu.CurrentSelectableItemId == 3) ? 2 : 1);
					break;
				}
				case MachineCore::BandCHIP_SuperCHIP:
				{
					DisplayItem(*MainRenderer, BehaviorsMenu.SuperCHIP_Version, (BehaviorsMenu.CurrentSelectableItemId == 0) ? 2 : 1);
					DisplayItem(*MainRenderer, BehaviorsMenu.ReturnToConfiguration, (BehaviorsMenu.CurrentSelectableItemId == 1) ? 2 : 1);
					break;
				}
				case MachineCore::BandCHIP_XOCHIP:
				{
					DisplayItem(*MainRenderer, BehaviorsMenu.SuperCHIP_Shift, (BehaviorsMenu.CurrentSelectableItemId == 0) ? 2 : 1);
					DisplayItem(*MainRenderer, BehaviorsMenu.SuperCHIP_LoadStore, (BehaviorsMenu.CurrentSelectableItemId == 1) ? 2 : 1);
					DisplayItem(*MainRenderer, BehaviorsMenu.Octo_LoResSprite, (BehaviorsMenu.CurrentSelectableItemId == 2) ? 2 : 1 );
					DisplayItem(*MainRenderer, BehaviorsMenu.ReturnToConfiguration, (BehaviorsMenu.CurrentSelectableItemId == 3) ? 2 : 1);
					break;
				}
				default:
				{
					DisplayItem(*MainRenderer, BehaviorsMenu.ReturnToConfiguration, 2);
					break;
				}
			}
			break;
		}
		case MenuDisplay::PaletteSettings:
		{
			DisplayItem(*MainRenderer, PaletteSettingsMenu.Title, 1);
			DisplayItem(*MainRenderer, PaletteSettingsMenu.CurrentIndex, (PaletteSettingsMenu.CurrentSelectableItemId == 0) ? 2 : 1);
			DisplayItem(*MainRenderer, PaletteSettingsMenu.Red, (PaletteSettingsMenu.CurrentSelectableItemId == 1) ? 2 : 1);
			DisplayItem(*MainRenderer, PaletteSettingsMenu.Green, (PaletteSettingsMenu.CurrentSelectableItemId == 2) ? 2 : 1);
			DisplayItem(*MainRenderer, PaletteSettingsMenu.Blue, (PaletteSettingsMenu.CurrentSelectableItemId == 3) ? 2 : 1);
			DisplayItem(*MainRenderer, PaletteSettingsMenu.CommitChanges, (PaletteSettingsMenu.CurrentSelectableItemId == 4) ? 2 : 1);
			DisplayItem(*MainRenderer, PaletteSettingsMenu.ReturnToConfiguration, (PaletteSettingsMenu.CurrentSelectableItemId == 5) ? 2 : 1);
			break;
		}
		case MenuDisplay::FontSettings:
		{
			DisplayItem(*MainRenderer, FontSettingsMenu.Title, 1);
			DisplayItem(*MainRenderer, FontSettingsMenu.LoResFontStyle, (FontSettingsMenu.CurrentSelectableItemId == 0) ? 2 : 1);
			DisplayItem(*MainRenderer, FontSettingsMenu.HiResFontStyle, (FontSettingsMenu.CurrentSelectableItemId == 1) ? 2 : 1);
			DisplayItem(*MainRenderer, FontSettingsMenu.ReturnToConfiguration, (FontSettingsMenu.CurrentSelectableItemId == 2) ? 2 : 1);
			break;
		}
		case MenuDisplay::KeyboardRemapping:
		{
			DisplayItem(*MainRenderer, KeyboardRemappingMenu.Title, 1);
			for (unsigned char i = 0x00; i < 0x10; ++i)
			{
				DisplayItem(*MainRenderer, KeyboardRemappingMenu.Keys[i], (KeyboardRemappingMenu.CurrentSelectableItemId == i) ? 2 : 1);
			}
			DisplayItem(*MainRenderer, KeyboardRemappingMenu.ReturnToConfiguration, (KeyboardRemappingMenu.CurrentSelectableItemId == 16) ? 2 : 1);
			break;
		}
		case MenuDisplay::LoadConfiguration:
		{
			DisplayItem(*MainRenderer, LoadConfigurationMenu.Title, 1);
			for (unsigned char i = 0; i < 16; ++i)
			{
				DirectoryEntryData &CurrentMenuEntry = LoadConfigurationMenu.MenuEntry[i];
				if (!CurrentMenuEntry.Entry.hidden)
				{
					DisplayItem(*MainRenderer, CurrentMenuEntry.Entry, (LoadConfigurationMenu.CurrentSelectableItemId == i) ? 3 : 1);
				}
			}
			break;
		}
		case MenuDisplay::ErrorDisplay:
		{
			DisplayItem(*MainRenderer, ErrorDisplay.Error, 1);
			for (unsigned char i = 0x00; i < 0x10; ++i)
			{
				DisplayItem(*MainRenderer, ErrorDisplay.RegisterValue[i], 1);
			}
			DisplayItem(*MainRenderer, ErrorDisplay.ProgramCounterValue, 1);
			DisplayItem(*MainRenderer, ErrorDisplay.AddressRegisterValue, 1);
			DisplayItem(*MainRenderer, ErrorDisplay.DelayTimerValue, 1);
			for (unsigned char i = 0; i < 4; ++i)
			{
				DisplayItem(*MainRenderer, ErrorDisplay.SoundTimerValue[i], 1);
			}
			break;
		}
	}
}

void Hyper_BandCHIP::Application::LoadFontStyles()
{
	std::filesystem::path lores_dir("fonts/lores");
	std::filesystem::path hires_dir("fonts/hires");
	for (auto i : std::filesystem::directory_iterator(lores_dir))
	{
		std::filesystem::path c_path = i.path();
		if (std::filesystem::is_regular_file(c_path))
		{
			if (c_path.extension() == ".mbft")
			{
				Fonts::FontStyle<4, 5> CurrentLoResFontStyle(c_path.string());
				if (CurrentLoResFontStyle.IsLoaded())
				{
					LoResFontStyleList.push_back(CurrentLoResFontStyle);
				}
			}
		}
	}
	if (LoResFontStyleList.size() > 0)
	{
		FontSettingsMenu.LoResFontStyle.Options = {};
		for (auto i : LoResFontStyleList)
		{
			FontSettingsMenu.LoResFontStyle.Options.push_back(i.GetName());
		}
	}
	for (auto i : std::filesystem::directory_iterator(hires_dir))
	{
		std::filesystem::path c_path = i.path();
		if (std::filesystem::is_regular_file(c_path))
		{
			if (c_path.extension() == ".mbft")
			{
				Fonts::FontStyle<8, 10> CurrentHiResFontStyle(c_path.string());
				if (CurrentHiResFontStyle.IsLoaded())
				{
					HiResFontStyleList.push_back(CurrentHiResFontStyle);
				}
			}
		}
	}
	if (HiResFontStyleList.size() > 0)
	{
		FontSettingsMenu.HiResFontStyle.Options = {};
		for (auto i : HiResFontStyleList)
		{
			FontSettingsMenu.HiResFontStyle.Options.push_back(i.GetName());
		}
	}
}

int main(int argc, char *argv[])
{
	Hyper_BandCHIP::Application App;
	return App.GetReturnCode();
}
