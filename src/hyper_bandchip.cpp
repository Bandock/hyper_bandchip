#include "../include/hyper_bandchip.h"
#ifdef RENDERER_OPENGLES3
#include "../include/renderer_opengles3.h"
#endif
#include "../include/fonts.h"
#include <iostream>
#include <fstream>

using std::cout;
using std::endl;

using std::ifstream;

Hyper_BandCHIP::Application::Application() : MainWindow(nullptr), MainRenderer(nullptr), start_path(std::filesystem::current_path()), CurrentOperationMode(OperationMode::Menu), CurrentMachineCore(MachineCore::BandCHIP_CHIP8), CurrentMenu(MenuDisplay::Main), CHIP8_Fonts(nullptr), SuperCHIP_Fonts(nullptr), CurrentMachine(nullptr), refresh_accumulator(0.0), loading_program(false), retcode(0)
{
	bool exit = false;
	unsigned int flags = 0x00000000;
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
#ifdef RENDERER_OPENGLES3
	flags |= SDL_WINDOW_OPENGL;
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif
	MainWindow = SDL_CreateWindow("Hyper BandCHIP Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 640, flags);
	MainRenderer = new Renderer(MainWindow);
	if (MainRenderer->Fail())
	{
		return;
	}
	InitializeKeyMaps();
	ConstructMenus();
	ConstructFonts();
	ShowMenu(CurrentMenu);
	refresh_tp = std::chrono::high_resolution_clock::now();
	while (!exit)
	{
		if (loading_program)
		{
			loading_program = false;
			ifstream ProgramFile(LoadProgramMenu.MenuEntry[LoadProgramMenu.CurrentSelectableItemId].Entry.Text, std::ios::binary);
			ProgramFile.seekg(0, std::ios::end);
			size_t program_size = ProgramFile.tellg();
			ProgramFile.seekg(0, std::ios::beg);
			unsigned char *program_data = new unsigned char[program_size];
			ProgramFile.read(reinterpret_cast<char *>(program_data), program_size);
			switch (CurrentMachineCore)
			{
				case MachineCore::BandCHIP_CHIP8:
				{
					if (CurrentMachine != nullptr)
					{
						if (CurrentMachine->GetMachineCore() != CurrentMachineCore)
						{
							delete CurrentMachine;
							CurrentMachine = new Machine();
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
						CurrentMachine = new Machine();
					}
					MainRenderer->SetupDisplay(64, 32);
					CurrentMachine->CopyDataToInterpreterMemory(reinterpret_cast<const unsigned char *>(CHIP8_Fonts), 0, sizeof(Font<5, 16>));
					CHIP8_BehaviorData CurrentBehaviorData = { BehaviorsMenu.CHIP48_Shift.toggle, BehaviorsMenu.CHIP48_LoadStore.toggle };
					CurrentMachine->StoreBehaviorData(&CurrentBehaviorData);
					if (CurrentMachine->LoadProgram(program_data, 0x200, program_size))
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
							delete CurrentMachine;
							CurrentMachine = new Machine(CurrentMachineCore, 1800, 0x1000, 128, 64);
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
						CurrentMachine = new Machine(CurrentMachineCore, 1800, 0x1000, 128, 64);
					}
					std::filesystem::current_path(local_dir);
					MainRenderer->SetupDisplay(128, 64);
					CurrentMachine->CopyDataToInterpreterMemory(reinterpret_cast<const unsigned char *>(CHIP8_Fonts), 0, sizeof(Font<5, 16>));
					CurrentMachine->CopyDataToInterpreterMemory(reinterpret_cast<const unsigned char *>(SuperCHIP_Fonts), sizeof(Font<5, 16>), sizeof(Font<10, 10>));
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
					if (CurrentMachine->LoadProgram(program_data, 0x200, program_size))
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
							delete CurrentMachine;
							CurrentMachine = new Machine(CurrentMachineCore, 1800, 0x10000, 128, 64);
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
						CurrentMachine = new Machine(CurrentMachineCore, 1800, 0x10000, 128,  64);
					}
					std::filesystem::current_path(local_dir);
					MainRenderer->SetupDisplay(128, 64);
					CurrentMachine->CopyDataToInterpreterMemory(reinterpret_cast<const unsigned char *>(CHIP8_Fonts), 0, sizeof(Font<5, 16>));
					CurrentMachine->CopyDataToInterpreterMemory(reinterpret_cast<const unsigned char *>(SuperCHIP_Fonts), sizeof(Font<5, 16>), sizeof(Font<10, 10>));
					if (CurrentMachine->LoadProgram(program_data, 0x200, program_size))
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
			delete [] program_data;
		}
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
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
													ConfigurationMenu.CurrentSelectableItemId = (ConfigurationMenu.CurrentSelectableItemId == 0) ? 6 : ConfigurationMenu.CurrentSelectableItemId - 1;
													break;
												}
												case MenuDisplay::Behaviors:
												{
													switch (CurrentMachineCore)
													{
														case MachineCore::BandCHIP_CHIP8:
														{
															BehaviorsMenu.CurrentSelectableItemId = (BehaviorsMenu.CurrentSelectableItemId == 0) ? 2 : BehaviorsMenu.CurrentSelectableItemId - 1;
															break;
														}
														case MachineCore::BandCHIP_SuperCHIP:
														{
															BehaviorsMenu.CurrentSelectableItemId = (BehaviorsMenu.CurrentSelectableItemId == 0) ? 1 : BehaviorsMenu.CurrentSelectableItemId - 1;
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
													ConfigurationMenu.CurrentSelectableItemId = (ConfigurationMenu.CurrentSelectableItemId == 6) ? 0 : ConfigurationMenu.CurrentSelectableItemId + 1;
													break;
												}
												case MenuDisplay::Behaviors:
												{
													switch (CurrentMachineCore)
													{
														case MachineCore::BandCHIP_CHIP8:
														{
															BehaviorsMenu.CurrentSelectableItemId = (BehaviorsMenu.CurrentSelectableItemId == 2) ? 0 : BehaviorsMenu.CurrentSelectableItemId + 1;
															break;
														}
														case MachineCore::BandCHIP_SuperCHIP:
														{
															BehaviorsMenu.CurrentSelectableItemId = (BehaviorsMenu.CurrentSelectableItemId == 1) ? 0 : BehaviorsMenu.CurrentSelectableItemId + 1;
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
															event_id = ConfigurationMenu.Behaviors.event_id;
															break;
														}
														case 2:
														{
															event_id = ConfigurationMenu.PaletteSettings.event_id;
															break;
														}
														case 3:
														{
															event_id = ConfigurationMenu.KeyboardRemapping.event_id;
															break;
														}
														case 4:
														{
															event_id = ConfigurationMenu.LoadConfiguration.event_id;
															break;
														}
														case 5:
														{
															event_id = ConfigurationMenu.SaveConfiguration.event_id;
															break;
														}
														case 6:
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
																	ConfigurationMenu.Core.current_option = 2;
																	break;
																}
																case MachineCore::BandCHIP_SuperCHIP:
																{
																	CurrentMachineCore = MachineCore::BandCHIP_CHIP8;
																	ConfigurationMenu.Core.current_option = 0;
																	BehaviorsMenu.CHIP48_Shift.toggle = false;
																	BehaviorsMenu.CHIP48_LoadStore.toggle = false;
																	break;
																}
																case MachineCore::BandCHIP_HyperCHIP64:
																{
																	CurrentMachineCore = MachineCore::BandCHIP_SuperCHIP;
																	ConfigurationMenu.Core.current_option = 1;
																	BehaviorsMenu.SuperCHIP_Version.current_option = 0;
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
													}
													break;
												}
												case MenuDisplay::PaletteSettings:
												{
													switch (PaletteSettingsMenu.CurrentSelectableItemId)
													{
														case 0:
														{
															PaletteSettingsMenu.CurrentIndex.value = (PaletteSettingsMenu.CurrentIndex.value == 0) ? 3 : PaletteSettingsMenu.CurrentIndex.value - 1;
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
															event_id = ConfigurationMenu.Behaviors.event_id;
															break;
														}
														case 2:
														{
															event_id = ConfigurationMenu.PaletteSettings.event_id;
															break;
														}
														case 3:
														{
															event_id = ConfigurationMenu.KeyboardRemapping.event_id;
															break;
														}
														case 4:
														{
															event_id = ConfigurationMenu.LoadConfiguration.event_id;
															break;
														}
														case 5:
														{
															event_id = ConfigurationMenu.SaveConfiguration.event_id;
															break;
														}
														case 6:
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
																	CurrentMachineCore = MachineCore::BandCHIP_HyperCHIP64;
																	ConfigurationMenu.Core.current_option = 2;
																	break;
																}
																case MachineCore::BandCHIP_HyperCHIP64:
																{
																	CurrentMachineCore = MachineCore::BandCHIP_CHIP8;
																	ConfigurationMenu.Core.current_option = 0;
																	BehaviorsMenu.CHIP48_Shift.toggle = false;
																	BehaviorsMenu.CHIP48_LoadStore.toggle = false;
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
															event_id = ConfigurationMenu.Behaviors.event_id;
															break;
														}
														case 2:
														{
															event_id = ConfigurationMenu.PaletteSettings.event_id;
															break;
														}
														case 3:
														{
															event_id = ConfigurationMenu.KeyboardRemapping.event_id;
															break;
														}
														case 4:
														{
															event_id = ConfigurationMenu.LoadConfiguration.event_id;
															break;
														}
														case 5:
														{
															event_id = ConfigurationMenu.SaveConfiguration.event_id;
															break;
														}
														case 6:
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
																	CurrentMachineCore = MachineCore::BandCHIP_HyperCHIP64;
																	ConfigurationMenu.Core.current_option = 2;
																	break;
																}
																case MachineCore::BandCHIP_HyperCHIP64:
																{
																	CurrentMachineCore = MachineCore::BandCHIP_CHIP8;
																	ConfigurationMenu.Core.current_option = 0;
																	BehaviorsMenu.CHIP48_Shift.toggle = false;
																	BehaviorsMenu.CHIP48_LoadStore.toggle = false;
																	break;
																}
															}
															BehaviorsMenu.CurrentSelectableItemId = 0;
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
														case ConfigurationMenuEvent::KeyboardRemapping:
														{
															CurrentMenu = MenuDisplay::KeyboardRemapping;
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
		std::chrono::high_resolution_clock::time_point current_tp = std::chrono::high_resolution_clock::now();
		if (CurrentMachine != nullptr)
		{
			if (!CurrentMachine->IsPaused())
			{
				CurrentMachine->SetCurrentTime(current_tp);
				CurrentMachine->RunDelayTimer();
				CurrentMachine->RunSoundTimer();
				CurrentMachine->ExecuteInstructions();
				unsigned char *display = nullptr;
				unsigned short display_width = 0;
				unsigned short display_height = 0;
				CurrentMachine->GetDisplay(&display, &display_width, &display_height);
				MainRenderer->WriteToDisplay(display, display_width, display_height);
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
					ErrorDisplay.SoundTimerValue.value = State.ST;
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
		std::chrono::duration<double> delta_time = current_tp - refresh_tp;
		if (delta_time.count() > 0.25)
		{
			delta_time = std::chrono::duration<double>(0.25);
		}
		refresh_accumulator += delta_time.count();
		refresh_tp = current_tp;
		if (refresh_accumulator >= 1.0 / 60.0)
		{
			refresh_accumulator = 0.0;
			MainRenderer->Render();
			SDL_Delay(10);
		}
	}
}

Hyper_BandCHIP::Application::~Application()
{
	if (CHIP8_Fonts != nullptr)
	{
		delete CHIP8_Fonts;
	}
	if (SuperCHIP_Fonts != nullptr)
	{
		delete SuperCHIP_Fonts;
	}
	delete MainRenderer;
	SDL_DestroyWindow(MainWindow);
}

int Hyper_BandCHIP::Application::GetReturnCode() const
{
	return retcode;
}

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
}

void Hyper_BandCHIP::Application::ConstructFonts()
{
	const FontData<10> Menu_FontData[] = {
		{ // Space
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000
		},
		{ // !
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00000000,
			0b00010000,
			0b00010000,
		},
		{ // "
			0b00100100,
			0b00100100,
			0b00100100,
			0b00100100,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000
		},
		{ // #
			0b01000100,
			0b01000100,
			0b01000100,
			0b11111110,
			0b01000100,
			0b01000100,
			0b11111110,
			0b01000100,
			0b01000100,
			0b01000100
		},
		{ // $
			0b00010000,
			0b01111100,
			0b10010010,
			0b10010000,
			0b01111100,
			0b00010010,
			0b00010010,
			0b10010010,
			0b01111100,
			0b00010000
		},
		{ // %
			0b00000000,
			0b01000010,
			0b00000100,
			0b00000100,
			0b00001000,
			0b00010000,
			0b00100000,
			0b01000000,
			0b01000000,
			0b10000100
		},
		{ // &
			0b01100000,
			0b10010000,
			0b10010000,
			0b01100000,
			0b01100000,
			0b10010010,
			0b10001010,
			0b10000100,
			0b10000100,
			0b01111010
		},
		{ // '
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000
		},
		{ // (
			0b00001000,
			0b00010000,
			0b00100000,
			0b00100000,
			0b00100000,
			0b00100000,
			0b00100000,
			0b00100000,
			0b00010000,
			0b00001000
		},
		{ // )
			0b00100000,
			0b00010000,
			0b00001000,
			0b00001000,
			0b00001000,
			0b00001000,
			0b00001000,
			0b00001000,
			0b00010000,
			0b00100000
		},
		{ // *
			0b00010000,
			0b00111000,
			0b00010000,
			0b00101000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000
		},
		{ // +
			0b00000000,
			0b00000000,
			0b00000000,
			0b00010000,
			0b00010000,
			0b01111100,
			0b00010000,
			0b00010000,
			0b00000000,
			0b00000000
		},
		{ // ,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00011000,
			0b00110000,
			0b00100000
		},
		{ // -
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111110,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000
		},
		{ // .
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00011000,
			0b00011000
		},
		{ // /
			0b00000010,
			0b00000010,
			0b00000100,
			0b00001000,
			0b00010000,
			0b00010000,
			0b00100000,
			0b01000000,
			0b10000000,
			0b10000000
		},
		{ // 0
			0b01111110,
			0b10000011,
			0b10000101,
			0b10000101,
			0b10011001,
			0b10011001,
			0b10100001,
			0b10100001,
			0b11000001,
			0b01111110
		},
		{ // 1
			0b00011000,
			0b00101000,
			0b01001000,
			0b00001000,
			0b00001000,
			0b00001000,
			0b00001000,
			0b00001000,
			0b00001000,
			0b01111110
		},
		{ // 2
			0b01111100,
			0b10000010,
			0b00000010,
			0b00000100,
			0b00001000,
			0b00010000,
			0b00100000,
			0b01000000,
			0b10000000,
			0b11111110
		},
		{ // 3
			0b01111100,
			0b10000010,
			0b00000010,
			0b00000010,
			0b00111100,
			0b00000010,
			0b00000010,
			0b00000010,
			0b10000010,
			0b01111100
		},
		{ // 4
			0b00000100,
			0b00001100,
			0b00010100,
			0b00100100,
			0b01000100,
			0b10000100,
			0b11111110,
			0b00000100,
			0b00000100,
			0b00000100
		},
		{ // 5
			0b11111110,
			0b10000000,
			0b10000000,
			0b10000000,
			0b11111100,
			0b00000010,
			0b00000010,
			0b00000010,
			0b10000010,
			0b01111100
		},
		{ // 6
			0b01111100,
			0b10000010,
			0b10000010,
			0b10000000,
			0b10111100,
			0b11000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b01111100
		},
		{ // 7
			0b11111110,
			0b00000010,
			0b00000100,
			0b00000100,
			0b00001000,
			0b00001000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000
		},
		{ // 8
			0b01111100,
			0b10000010,
			0b10000010,
			0b10000010,
			0b01111100,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b01111100
		},
		{ // 9
			0b01111100,
			0b10000010,
			0b10000010,
			0b10000010,
			0b01111110,
			0b00000010,
			0b00000010,
			0b00000010,
			0b10000010,
			0b01111100
		},
		{ // :
			0b00000000,
			0b00000000,
			0b00011000,
			0b00011000,
			0b00000000,
			0b00000000,
			0b00011000,
			0b00011000,
			0b00000000,
			0b00000000
		},
		{ // ;
			0b00000000,
			0b00000000,
			0b00011000,
			0b00011000,
			0b00000000,
			0b00000000,
			0b00011000,
			0b00001000,
			0b00010000,
			0b00000000
		},
		{ // <
			0b00000000,
			0b00000000,
			0b00000000,
			0b00001100,
			0b00110000,
			0b11000000,
			0b00110000,
			0b00001100,
			0b00000000,
			0b00000000
		},
		{ // =
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111110,
			0b00000000,
			0b00000000,
			0b01111110,
			0b00000000,
			0b00000000,
			0b00000000
		},
		{ // >
			0b00000000,
			0b00000000,
			0b00000000,
			0b11000000,
			0b00110000,
			0b00001100,
			0b00110000,
			0b11000000,
			0b00000000,
			0b00000000
		},
		{ // ?
			0b00111000,
			0b01000100,
			0b10000010,
			0b00000100,
			0b00001000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00000000,
			0b00010000
		},
		{ // @
			0b01111100,
			0b10000010,
			0b10000010,
			0b10011010,
			0b10101010,
			0b10101010,
			0b10110110,
			0b10000000,
			0b10000010,
			0b01111100,
		},
		{ // A
			0b00010000,
			0b00111000,
			0b01000100,
			0b10000010,
			0b10000010,
			0b11111110,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010
		},
		{ // B
			0b11111100,
			0b10000010,
			0b10000010,
			0b10000010,
			0b11111100,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b11111100
		},
		{ // C
			0b01111100,
			0b10000010,
			0b10000000,
			0b10000000,
			0b10000000,
			0b10000000,
			0b10000000,
			0b10000000,
			0b10000010,
			0b01111100
		},
		{ // D
			0b11111100,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b11111100
		},
		{ // E
			0b11111110,
			0b10000000,
			0b10000000,
			0b10000000,
			0b11111110,
			0b10000000,
			0b10000000,
			0b10000000,
			0b10000000,
			0b11111111
		},
		{ // F
			0b11111110,
			0b10000000,
			0b10000000,
			0b10000000,
			0b11111110,
			0b10000000,
			0b10000000,
			0b10000000,
			0b10000000,
			0b10000000
		},
		{ // G
			0b01111100,
			0b10000010,
			0b10000010,
			0b10000000,
			0b10000000,
			0b10011110,
			0b10000010,
			0b10000010,
			0b10000010,
			0b01111100
		},
		{ // H
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b11111110,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010
		},
		{ // I
			0b11111110,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b11111111
		},
		{ // J
			0b11111110,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b10010000,
			0b10010000,
			0b01100000
		},
		{ // K
			0b10000100,
			0b10001000,
			0b10010000,
			0b10100000,
			0b11000000,
			0b10100000,
			0b10010000,
			0b10001000,
			0b10000100,
			0b10000100
		},
		{ // L
			0b10000000,
			0b10000000,
			0b10000000,
			0b10000000,
			0b10000000,
			0b10000000,
			0b10000000,
			0b10000000,
			0b10000000,
			0b11111110
		},
		{ // M
			0b10000010,
			0b11000110,
			0b11000110,
			0b10101010,
			0b10101010,
			0b10010010,
			0b10010010,
			0b10000010,
			0b10000010,
			0b10000010
		},
		{ // N
			0b10000010,
			0b11000010,
			0b11000010,
			0b10100010,
			0b10100010,
			0b10010010,
			0b10010010,
			0b10001010,
			0b10001010,
			0b10000110
		},
		{ // O
			0b01111100,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b01111100
		},
		{ // P
			0b11111100,
			0b10000010,
			0b10000010,
			0b10000010,
			0b11111100,
			0b10000000,
			0b10000000,
			0b10000000,
			0b10000000,
			0b10000000
		},
		{ // Q
			0b01111000,
			0b10000100,
			0b10000100,
			0b10000100,
			0b10000100,
			0b10000100,
			0b10000100,
			0b01101100,
			0b00011000,
			0b00000110
		},
		{ // R
			0b11111100,
			0b10000010,
			0b10000010,
			0b10000010,
			0b11111100,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010
		},
		{ // S
			0b01111100,
			0b10000010,
			0b10000000,
			0b10000000,
			0b01100000,
			0b00011100,
			0b00000010,
			0b00000010,
			0b10000010,
			0b01111100
		},
		{ // T
			0b11111110,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000
		},
		{ // U
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b01000100,
			0b00111000
		},
		{ // V
			0b10000010,
			0b10000010,
			0b10000010,
			0b10000010,
			0b01000100,
			0b01000100,
			0b01000100,
			0b00101000,
			0b00101000,
			0b00010000
		},
		{ // W
			0b10000010,
			0b10000010,
			0b10000010,
			0b10010010,
			0b01010100,
			0b01010100,
			0b01111100,
			0b00101000,
			0b00101000
		},
		{ // X
			0b10000010,
			0b10000010,
			0b01000100,
			0b00101000,
			0b00010000,
			0b00010000,
			0b00101000,
			0b01000100,
			0b10000010,
			0b10000010
		},
		{ // Y
			0b10000010,
			0b10000010,
			0b01000100,
			0b01000100,
			0b00101000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000
		},
		{ // Z
			0b11111110,
			0b00000010,
			0b00000100,
			0b00001000,
			0b00010000,
			0b00100000,
			0b01000000,
			0b01000000,
			0b10000000,
			0b11111110
		},
		{ // [
			0b00111000,
			0b00100000,
			0b00100000,
			0b00100000,
			0b00100000,
			0b00100000,
			0b00100000,
			0b00100000,
			0b00100000,
			0b00111000
		},
		{ // \
			0b10000000,
			0b10000000,
			0b01000000,
			0b00100000,
			0b00010000,
			0b00010000,
			0b00001000,
			0b00000100,
			0b00000010,
			0b00000010
		},
		{ // ]
			0b00111000,
			0b00001000,
			0b00001000,
			0b00001000,
			0b00001000,
			0b00001000,
			0b00001000,
			0b00001000,
			0b00001000,
			0b00111000
		},
		{ // ^
			0b00010000,
			0b00101000,
			0b01000100,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000
		},
		{ // _
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b11111111
		},
		{ // `
			0b00110000,
			0b00011000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000
		},
		{ // a
			0b00000000,
			0b00000000,
			0b00000000,
			0b00111000,
			0b01000100,
			0b00000100,
			0b00111100,
			0b01000100,
			0b01000100,
			0b00111010
		},
		{ // b
			0b10000000,
			0b10000000,
			0b10000000,
			0b10000000,
			0b10111000,
			0b11000100,
			0b10000100,
			0b10000100,
			0b11000100,
			0b10111000
		},
		{ // c
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111000,
			0b10000100,
			0b10000000,
			0b10000000,
			0b10000100,
			0b01111000
		},
		{ // d
			0b00000100,
			0b00000100,
			0b00000100,
			0b00000100,
			0b01110100,
			0b10001100,
			0b10000100,
			0b10000100,
			0b10001100,
			0b01110100
		},
		{ // e
			0b00000000,
			0b00000000,
			0b00000000,
			0b01111000,
			0b10000100,
			0b10000100,
			0b11111100,
			0b10000000,
			0b10000100,
			0b01111000
		},
		{ // f
			0b00001100,
			0b00010000,
			0b00010000,
			0b00010000,
			0b01111100,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000
		},
		{ // g
			0b00000000,
			0b00000000,
			0b00000000,
			0b01110100,
			0b10001100,
			0b10000100,
			0b01111100,
			0b00000100,
			0b10000100,
			0b01111000
		},	
		{ // h
			0b10000000,
			0b10000000,
			0b10000000,
			0b10000000,
			0b10111000,
			0b11000100,
			0b10000100,
			0b10000100,
			0b10000100,
			0b10000100
		},
		{ // i
			0b00010000,
			0b00000000,
			0b00000000,
			0b00110000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b01111100
		},
		{ // j
			0b00010000,
			0b00000000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b01100000
		},
		{ // k
			0b10000000,
			0b10000000,
			0b10001000,
			0b10010000,
			0b10100000,
			0b11000000,
			0b10100000,
			0b10010000,
			0b10001000,
			0b10000100
		},
		{ // l
			0b01110000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b01111100
		},
		{ // m
			0b00000000,
			0b00000000,
			0b00000000,
			0b10000010,
			0b11000110,
			0b10101010,
			0b10010010,
			0b10010010,
			0b10010010,
			0b10010010
		},
		{ // n
			0b00000000,
			0b00000000,
			0b00000000,
			0b10111000,
			0b11000100,
			0b10000100,
			0b10000100,
			0b10000100,
			0b10000100,
			0b10000100
		},
		{ // o
			0b00000000,
			0b00000000,
			0b00000000,
			0b00111000,
			0b01000100,
			0b01000100,
			0b01000100,
			0b01000100,
			0b01000100,
			0b00111000
		},
		{ // p
			0b00000000,
			0b00000000,
			0b00000000,
			0b10111000,
			0b11000100,
			0b10000100,
			0b11000100,
			0b10111000,
			0b10000000,
			0b10000000
		},
		{ // q
			0b00000000,
			0b00000000,
			0b00000000,
			0b01110100,
			0b10001100,
			0b10000100,
			0b10001100,
			0b01110100,
			0b00000100,
			0b00000100
		},
		{ // r
			0b00000000,
			0b00000000,
			0b00000000,
			0b10111000,
			0b11000100,
			0b10000000,
			0b10000000,
			0b10000000,
			0b10000000,
			0b10000000
		},
		{ // s
			0b00000000,
			0b00000000,
			0b00000000,
			0b00111000,
			0b01000100,
			0b01000000,
			0b00111000,
			0b00000100,
			0b01000100,
			0b00111000
		},
		{ // t
			0b00000000,
			0b00000000,
			0b00010000,
			0b01111100,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00001100
		},
		{ // u
			0b00000000,
			0b00000000,
			0b00000000,
			0b10001000,
			0b10001000,
			0b10001000,
			0b10001000,
			0b10001000,
			0b10001000,
			0b01110100
		},
		{ // v
			0b00000000,
			0b00000000,
			0b00000000,
			0b10000100,
			0b10000100,
			0b10000100,
			0b01001000,
			0b01001000,
			0b01001000,
			0b00110000
		},
		{ // w
			0b00000000,
			0b00000000,
			0b00000000,
			0b10010010,
			0b10010010,
			0b10010010,
			0b01010100,
			0b01010100,
			0b01010100,
			0b00101000
		},
		{ // x
			0b00000000,
			0b00000000,
			0b00000000,
			0b10000100,
			0b10000100,
			0b01001000,
			0b00110000,
			0b01001000,
			0b10000100,
			0b10000100
		},
		{ // y
			0b00000000,
			0b00000000,
			0b00000000,
			0b10000100,
			0b10000100,
			0b01001000,
			0b01001000,
			0b00110000,
			0b00010000,
			0b01100000
		},
		{ // z
			0b00000000,
			0b00000000,
			0b00000000,
			0b11111100,
			0b00001000,
			0b00010000,
			0b00100000,
			0b01000000,
			0b10000000,
			0b11111100
		},
		{ // {
			0b00111000,
			0b01000000,
			0b01000000,
			0b01000000,
			0b10000000,
			0b10000000,
			0b01000000,
			0b01000000,
			0b01000000,
			0b00111000
		},
		{ // }
			0b11100000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00001000,
			0b00001000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b11100000
		},
		{ // |
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00010000
		},
		{ // ~
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00110010,
			0b01001100,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000
		}
	};
	MainRenderer->SetupMenuFonts(reinterpret_cast<const unsigned char *>(Menu_FontData));
	const FontData<5> CHIP8_FontData[] = {
		{ // 0
			0b11110000,
			0b10010000,
			0b10010000,
			0b10010000,
			0b11110000
	       	},
		{ // 1
			0b00100000,
			0b01100000,
			0b00100000,
			0b00100000,
			0b01110000
		},
		{ // 2
			0b11110000,
			0b00010000,
			0b11110000,
			0b10000000,
			0b11110000
	       	},
		{ // 3
			0b11110000,
			0b00010000,
			0b11110000,
			0b00010000,
			0b11110000
		},
		{ // 4
			0b10010000,
			0b10010000,
			0b11110000,
			0b00010000,
			0b00010000
	       	},
		{ // 5
			0b11110000,
			0b10000000,
			0b11110000,
			0b00010000,
			0b11110000
		},
		{ // 6
			0b11110000,
			0b10000000,
			0b11110000,
			0b10010000,
			0b11110000
		},
		{ // 7
			0b11110000,
			0b00010000,
			0b00100000,
			0b01000000,
			0b01000000
		},
		{ // 8
			0b11110000,
			0b10010000,
			0b11110000,
			0b10010000,
			0b11110000
		},
		{ // 9
			0b11110000,
			0b10010000,
			0b11110000,
			0b00010000,
			0b11110000
		},
		{ // A
			0b11110000,
			0b10010000,
			0b11110000,
			0b10010000,
			0b10010000
		},
		{ // B
			0b11100000,
			0b10010000,
			0b11100000,
			0b10010000,
			0b11100000
		},
		{ // C
			0b11110000,
			0b10000000,
			0b10000000,
			0b10000000,
			0b11110000
		},
		{ // D
			0b11100000,
			0b10010000,
			0b10010000,
			0b10010000,
			0b11100000
		},
		{ // E
			0b11110000,
			0b10000000,
			0b11110000,
			0b10000000,
			0b11110000
		},
		{ // F
			0b11110000,
			0b10000000,
			0b11110000,
			0b10000000,
			0b10000000
		}
	};
	CHIP8_Fonts = new Font<5, 16>(CHIP8_FontData);
	const FontData<10> SuperCHIP_FontData[] = {
		{ // 0
			0b00111100,
			0b01111110,
			0b11100111,
			0b11000011,
			0b11000011,
			0b11000011,
			0b11000011,
			0b11100111,
			0b01111110,
			0b00111100
		},
		{ // 1
			0b00011000,
			0b00111000,
			0b01011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00011000,
			0b00111100
		},
		{ // 2
			0b00111110,
			0b01111111,
			0b11000011,
			0b00000110,
			0b00001100,
			0b00011000,
			0b00110000,
			0b01100000,
			0b11111111,
			0b11111111
		},
		{ // 3
			0b00111100,
			0b01111110,
			0b11000011,
			0b00000011,
			0b00001110,
			0b00001110,
			0b00000011,
			0b11000011,
			0b01111110,
			0b00111100
		},
		{ // 4
			0b00000110,
			0b00001110,
			0b00011110,
			0b00110110,
			0b01100110,
			0b11000110,
			0b11111111,
			0b11111111,
			0b00000110,
			0b00000110,
		},
		{ // 5
			0b11111111,
			0b11111111,
			0b11000000,
			0b11000000,
			0b11111100,
			0b11111110,
			0b00000011,
			0b11000011,
			0b01111110,
			0b00111100
		},
		{ // 6
			0b00111111,
			0b01111110,
			0b11000000,
			0b11000000,
			0b11111100,
			0b11111110,
			0b11000011,
			0b11000011,
			0b01111110,
			0b00111100
		},
		{ // 7
			0b11111111,
			0b11111111,
			0b00000011,
			0b00000110,
			0b00001100,
			0b00011000,
			0b00110000,
			0b01100000,
			0b01100000,
			0b01100000
		},
		{ // 8
			0b00111100,
			0b01111110,
			0b11000011,
			0b11000011,
			0b01111110,
			0b01111110,
			0b11000011,
			0b11000011,
			0b01111110,
			0b00111100
		},
		{ // 9
			0b00111100,
			0b01111110,
			0b11000011,
			0b11000011,
			0b01111111,
			0b00111111,
			0b00000011,
			0b00000011,
			0b01111110,
			0b11111100
		}
	};
	SuperCHIP_Fonts = new Font<10, 10>(SuperCHIP_FontData);
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
			DisplayItem(*MainRenderer, ConfigurationMenu.Behaviors, (ConfigurationMenu.CurrentSelectableItemId == 1) ? 2 : 1);
			DisplayItem(*MainRenderer, ConfigurationMenu.PaletteSettings, (ConfigurationMenu.CurrentSelectableItemId == 2) ? 2 : 1);
			DisplayItem(*MainRenderer, ConfigurationMenu.KeyboardRemapping, (ConfigurationMenu.CurrentSelectableItemId == 3) ? 2 : 1);
			DisplayItem(*MainRenderer, ConfigurationMenu.LoadConfiguration, (ConfigurationMenu.CurrentSelectableItemId == 4) ? 2 : 1);
			DisplayItem(*MainRenderer, ConfigurationMenu.SaveConfiguration, (ConfigurationMenu.CurrentSelectableItemId == 5) ? 2 : 1);
			DisplayItem(*MainRenderer, ConfigurationMenu.ReturnToMainMenu, (ConfigurationMenu.CurrentSelectableItemId == 6) ? 2 : 1);
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
		case MenuDisplay::Behaviors:
		{
			DisplayItem(*MainRenderer, BehaviorsMenu.Title, 1);
			switch (CurrentMachineCore)
			{
				case MachineCore::BandCHIP_CHIP8:
				{
					DisplayItem(*MainRenderer, BehaviorsMenu.CHIP48_Shift, (BehaviorsMenu.CurrentSelectableItemId == 0) ? 2 : 1);
					DisplayItem(*MainRenderer, BehaviorsMenu.CHIP48_LoadStore, (BehaviorsMenu.CurrentSelectableItemId == 1) ? 2 : 1);
					DisplayItem(*MainRenderer, BehaviorsMenu.ReturnToConfiguration, (BehaviorsMenu.CurrentSelectableItemId == 2) ? 2 : 1);
					break;
				}
				case MachineCore::BandCHIP_SuperCHIP:
				{
					DisplayItem(*MainRenderer, BehaviorsMenu.SuperCHIP_Version, (BehaviorsMenu.CurrentSelectableItemId == 0) ? 2 : 1);
					DisplayItem(*MainRenderer, BehaviorsMenu.ReturnToConfiguration, (BehaviorsMenu.CurrentSelectableItemId == 1) ? 2 : 1);
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
			DisplayItem(*MainRenderer, ErrorDisplay.SoundTimerValue, 1);
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	Hyper_BandCHIP::Application App;
	return App.GetReturnCode();
}
