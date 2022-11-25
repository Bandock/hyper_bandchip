#include "machine.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <bit>
#if defined(RENDERER_OPENGL21)
#include "renderer_opengl21.h"
#elif defined(RENDERER_OPENGL30)
#include "renderer_opengl30.h"
#elif defined(RENDERER_OPENGLES2)
#include "renderer_opengles2.h"
#elif defined(RENDERER_OPENGLES3)
#include "renderer_opengles3.h"
#endif

Hyper_BandCHIP::Machine::Machine(MachineCore Core, unsigned int cycles_per_second, unsigned int memory_size, unsigned short display_width, unsigned short display_height, Renderer *DisplayRenderer) : CurrentMachineCore(Core), CurrentResolutionMode(ResolutionMode::LoRes), DisplayRenderer(DisplayRenderer), sync(false), superscalar_mode(false), display_interrupt(false), cycles_per_second(cycles_per_second), delay_timer(0), sound_timer{ 0, 0, 0, 0 }, PC(0), I(0), SP(0), memory(nullptr), display(nullptr), key_pressed(0), plane(0x01), voice(0), rng_engine(system_clock::now().time_since_epoch().count()), rng_distrib(0, 255), cycle_accumulator(0.0), dt_accumulator(0.0), st_accumulator{ 0.0, 0.0, 0.0, 0.0 } , pause(true), operational(true), wait_for_key_release(false), error_state(MachineError::NoError)
{
	switch (CurrentMachineCore)
	{
		case MachineCore::BandCHIP_CHIP8:
		{
			CurrentMachineAudioModel = MachineAudioModel::Synthesizer;
			audio_system.emplace<Audio>();
			break;
		}
		case MachineCore::BandCHIP_SuperCHIP:
		{
			CurrentMachineAudioModel = MachineAudioModel::Synthesizer;
			audio_system.emplace<Audio>();
			rpl_user_flags_file.open("RPLUserFlags.bin", std::ios::binary | std::ios::in | std::ios::out);
			if (!rpl_user_flags_file.is_open())
			{
				rpl_user_flags_file.open("RPLUserFlags.bin", std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
				rpl_user_flags_file.pubseekoff(15, std::ios::beg);
				rpl_user_flags_file.sputc(0x00);
			}
			break;
		}
		case MachineCore::BandCHIP_XOCHIP:
		{
			CurrentMachineAudioModel = MachineAudioModel::Sampled_XOCHIP;
			audio_system.emplace<XOCHIP_Audio>();
			rpl_user_flags_file.open("RPLUserFlags.bin", std::ios::binary | std::ios::in | std::ios::out);
			if (!rpl_user_flags_file.is_open())
			{
				rpl_user_flags_file.open("RPLUserFlags.bin", std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
				rpl_user_flags_file.pubseekoff(15, std::ios::beg);
				rpl_user_flags_file.sputc(0x00);
			}
			break;
		}
		case MachineCore::BandCHIP_HyperCHIP64:
		{
			CurrentMachineAudioModel = MachineAudioModel::Sampled_HyperCHIP64;
			audio_system.emplace<HyperCHIP64_Audio>();
			rpl_user_flags_file.open("RPLUserFlags.bin", std::ios::binary | std::ios::in | std::ios::out);
			if (!rpl_user_flags_file.is_open())
			{
				rpl_user_flags_file.open("RPLUserFlags.bin", std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
				rpl_user_flags_file.pubseekoff(15, std::ios::beg);
				rpl_user_flags_file.sputc(0x00);
			}
			break;
		}
	}
	memset(&V, 0, sizeof(V));
	memset(stack, 0, sizeof(stack));
	this->memory_size = (memory_size < 4096) ? 4096 : memory_size;
	memory = new unsigned char[this->memory_size];
	memset(memory, 0, this->memory_size);
	this->display_width = (display_width < 64) ? 64 : display_width;
	this->display_height = (display_height < 32) ? 32 : display_height;
	display = new unsigned char[this->display_width * this->display_height];
	memset(display, 0x00, this->display_width * this->display_height);
	memset(key_status, 0x00, 0x10);
	cycle_rate = (cycles_per_second > 0) ? 1.0 / static_cast<double>(cycles_per_second) : 0.0;
}

Hyper_BandCHIP::Machine::~Machine()
{
	if (memory != nullptr)
	{
		delete [] memory;
	}
	if (display != nullptr)
	{
		delete [] display;
	}
}

void Hyper_BandCHIP::Machine::InitializeMemory()
{
	memset(memory, 0x00, memory_size);
}

void Hyper_BandCHIP::Machine::InitializeVideo()
{
	memset(display, 0x00, display_width * display_height);
	plane = 0x01;
}

void Hyper_BandCHIP::Machine::SetSoundTimer(unsigned char sound_timer)
{
	unsigned char voice_count = 0;
	switch (CurrentMachineCore)
	{
		case MachineCore::BandCHIP_CHIP8:
		case MachineCore::BandCHIP_SuperCHIP:
		case MachineCore::BandCHIP_XOCHIP:
		{
			voice_count = 1;
			break;
		}
		case MachineCore::BandCHIP_HyperCHIP64:
		{
			voice_count = 4;
			break;
		}
	}
	if (voice < voice_count)
	{
		this->sound_timer[voice] = sound_timer;
		if (!sync)
		{
			st_accumulator[voice] = 0.0;
		}
		else
		{
			SoundTimerSync[voice].cycle_counter = 0;
		}
		if (this->sound_timer[voice] > 0)
		{
			if (!sync)
			{
				st_tp[voice] = current_tp;
			}
			switch (CurrentMachineAudioModel)
			{
				case MachineAudioModel::Synthesizer:
				{
					Audio *audio = std::get_if<Audio>(&audio_system);
					switch (audio->GetEnvelopeGeneratorState())
					{
						case EnvelopeGeneratorState::Release:
						case EnvelopeGeneratorState::Off:
						{
							audio->SetEnvelopeGeneratorState(EnvelopeGeneratorState::Attack);
							break;
						}
					}
					break;
				}
				case MachineAudioModel::Sampled_XOCHIP:
				{
					XOCHIP_Audio *audio = std::get_if<XOCHIP_Audio>(&audio_system);
					if (audio->IsPaused())
					{
						audio->PauseAudio(false);
					}
					break;
				}
				case MachineAudioModel::Sampled_HyperCHIP64:
				{
					HyperCHIP64_Audio *audio = std::get_if<HyperCHIP64_Audio>(&audio_system);
					if (audio->IsPaused(voice))
					{
						audio->PauseAudio(false, voice);
					}
					break;
				}
			}
		}
		else
		{
			switch (CurrentMachineAudioModel)
			{
				case MachineAudioModel::Synthesizer:
				{
					Audio *audio = std::get_if<Audio>(&audio_system);
					switch (audio->GetEnvelopeGeneratorState())
					{
						case EnvelopeGeneratorState::Attack:
						case EnvelopeGeneratorState::Decay:
						case EnvelopeGeneratorState::Sustain:
						{
							audio->SetEnvelopeGeneratorState(EnvelopeGeneratorState::Release);
							break;
						}
					}
					break;
				}
				case MachineAudioModel::Sampled_XOCHIP:
				{
					XOCHIP_Audio *audio = std::get_if<XOCHIP_Audio>(&audio_system);
					if (!audio->IsPaused())
					{
						audio->PauseAudio(true);
						audio->Reset();
					}
					break;
				}
				case MachineAudioModel::Sampled_HyperCHIP64:
				{
					HyperCHIP64_Audio *audio = std::get_if<HyperCHIP64_Audio>(&audio_system);
					if (!audio->IsPaused(voice))
					{
						audio->PauseAudio(true, voice);
						audio->Reset(voice);
					}
					break;
				}
			}
		}
	}
}

/*
void Hyper_BandCHIP::Machine::SetKeyStatus(unsigned char key, bool pressed)
{
	if (key <= 0xF)
	{
		key_status[key] = pressed;
	}
}
*/

/*
bool Hyper_BandCHIP::Machine::LoadProgram(const unsigned char *source, unsigned short start_address, unsigned int size)
{
	if (source != nullptr)
	{
		if (size > 0 && size <= memory_size - 0x200)
		{
			this->start_address = start_address;
			PC = this->start_address;
			memcpy(&memory[this->start_address], source, size);
			cycle_accumulator = 0.0;
			if (error_state != MachineError::NoError)
			{
				error_state = MachineError::NoError;
			}
			if (!operational)
			{
				operational = true;
			}
			return true;
		}
	}
	return false;
}
*/

void Hyper_BandCHIP::Machine::PauseProgram(bool pause)
{
	if (this->pause != pause)
	{
		if (this->pause)
		{
			cycle_tp = current_tp;
			if (delay_timer > 0 && !sync)
			{
				dt_tp = current_tp;
			}
			unsigned char voice_count = 0;
			switch (CurrentMachineCore)
			{
				case MachineCore::BandCHIP_CHIP8:
				case MachineCore::BandCHIP_SuperCHIP:
				case MachineCore::BandCHIP_XOCHIP:
				{
					voice_count = 1;
					break;
				}
				case MachineCore::BandCHIP_HyperCHIP64:
				{
					voice_count = 4;
					break;
				}
			}
			for (unsigned char v = 0; v < voice_count; ++v)
			{
				if (sound_timer[v] > 0)
				{
					if (!sync)
					{
						st_tp[v] = current_tp;
					}
					switch (CurrentMachineAudioModel)
					{
						case MachineAudioModel::Synthesizer:
						{
							Audio *audio = std::get_if<Audio>(&audio_system);
							if (audio->GetEnvelopeGeneratorState() == EnvelopeGeneratorState::Off)
							{
								audio->SetEnvelopeGeneratorState(EnvelopeGeneratorState::Attack);
							}
							break;
						}
						case MachineAudioModel::Sampled_XOCHIP:
						{
							XOCHIP_Audio *audio = std::get_if<XOCHIP_Audio>(&audio_system);
							if (audio->IsPaused())
							{
								audio->PauseAudio(false);
							}
							break;
						}
						case MachineAudioModel::Sampled_HyperCHIP64:
						{
							HyperCHIP64_Audio *audio = std::get_if<HyperCHIP64_Audio>(&audio_system);
							if (audio->IsPaused(v))
							{
								audio->PauseAudio(false, v);
							}
							break;
						}
					}
				}
			}
		}
		else
		{
			duration<double> delta_time = current_tp - cycle_tp;
			if (delta_time.count() > 0.25)
			{
				delta_time = duration<double>(0.25);
			}
			cycle_accumulator += delta_time.count();
			if (delay_timer > 0 && !sync)
			{
				delta_time = current_tp - dt_tp;
				if (delta_time.count() > 0.25)
				{
					delta_time = duration<double>(0.25);
				}
				dt_accumulator += delta_time.count();
			}
			unsigned char voice_count = 0;
			switch (CurrentMachineCore)
			{
				case MachineCore::BandCHIP_CHIP8:
				case MachineCore::BandCHIP_SuperCHIP:
				case MachineCore::BandCHIP_XOCHIP:
				{
					voice_count = 1;
					break;
				}
				case MachineCore::BandCHIP_HyperCHIP64:
				{
					voice_count = 4;
					break;
				}
			}
			for (unsigned char v = 0; v < voice_count; ++v)
			{
				if (sound_timer[v] > 0)
				{
					if (!sync)
					{
						delta_time = current_tp - st_tp[v];
						if (delta_time.count() > 0.25)
						{
							delta_time = duration<double>(0.25);
						}
						st_accumulator[v] += delta_time.count();
					}
					switch (CurrentMachineAudioModel)
					{
						case MachineAudioModel::Synthesizer:
						{
							Audio *audio = std::get_if<Audio>(&audio_system);
							switch (audio->GetEnvelopeGeneratorState())
							{
								case EnvelopeGeneratorState::Attack:
								case EnvelopeGeneratorState::Decay:
								case EnvelopeGeneratorState::Sustain:
								{
									audio->SetEnvelopeGeneratorState(EnvelopeGeneratorState::Release);
									break;
								}
							}
							break;
						}
						case MachineAudioModel::Sampled_XOCHIP:
						{
							XOCHIP_Audio *audio = std::get_if<XOCHIP_Audio>(&audio_system);
							if (!audio->IsPaused())
							{
								audio->PauseAudio(true);
							}
							break;
						}
						case MachineAudioModel::Sampled_HyperCHIP64:
						{
							HyperCHIP64_Audio *audio = std::get_if<HyperCHIP64_Audio>(&audio_system);
							if (!audio->IsPaused(v))
							{
								audio->PauseAudio(true, v);
							}
							break;
						}
					}
				}
			}
		}
	}
	this->pause = pause;
}

/*
void Hyper_BandCHIP::Machine::RunSoundTimer()
{
	unsigned char voice_count = 0;
	switch (CurrentMachineCore)
	{
		case MachineCore::BandCHIP_CHIP8:
		case MachineCore::BandCHIP_SuperCHIP:
		case MachineCore::BandCHIP_XOCHIP:
		{
			voice_count = 1;
			break;
		}
		case MachineCore::BandCHIP_HyperCHIP64:
		{
			voice_count = 4;
			break;
		}
	}
	for (unsigned char v = 0; v < voice_count; ++v)
	{
		if (sound_timer[v] > 0)
		{
			if (!sync)
			{
				duration<double> delta_time = current_tp - st_tp[v];
				if (delta_time.count() > 0.25)
				{
					delta_time = duration<double>(0.25);
				}
				st_accumulator[v] += delta_time.count();
				st_tp[v] = current_tp;
				constexpr double st_rate = 1.0 / 60.0;
				if (st_accumulator[v] >= st_rate)
				{
					unsigned char tick_count = static_cast<unsigned char>(st_accumulator[v] / st_rate);
					if (sound_timer[v] >= tick_count)
					{
						sound_timer[v] -= tick_count;
					}
					else
					{
						sound_timer[v] = 0;
					}
					if (sound_timer[v] == 0)
					{
						st_accumulator[v] = 0.0;
						switch (CurrentMachineAudioModel)
						{
							case MachineAudioModel::Synthesizer:
							{
								Audio *audio = std::get_if<Audio>(&audio_system);
								audio->SetEnvelopeGeneratorState(EnvelopeGeneratorState::Release);
								break;
							}
							case MachineAudioModel::Sampled_XOCHIP:
							{
								XOCHIP_Audio *audio = std::get_if<XOCHIP_Audio>(&audio_system);
								audio->PauseAudio(true);
								audio->Reset();
								break;
							}
							case MachineAudioModel::Sampled_HyperCHIP64:
							{
								HyperCHIP64_Audio *audio = std::get_if<HyperCHIP64_Audio>(&audio_system);
								audio->PauseAudio(true, v);
								audio->Reset(v);
								break;
							}
						}
					}
					else
					{
						st_accumulator[v] -= (static_cast<double>(tick_count) / 60.0);
					}
				}
			}
			else
			{
				++SoundTimerSync[v].cycle_counter;
				if (SoundTimerSync[v].cycle_counter >= SoundTimerSync[v].cycles_per_frame)
				{
					SoundTimerSync[v].cycle_counter = 0;
					--sound_timer[v];
					if (sound_timer[v] == 0)
					{
						switch (CurrentMachineAudioModel)
						{
							case MachineAudioModel::Synthesizer:
							{
								Audio *audio = std::get_if<Audio>(&audio_system);
								audio->SetEnvelopeGeneratorState(EnvelopeGeneratorState::Release);
								break;
							}
							case MachineAudioModel::Sampled_XOCHIP:
							{
								XOCHIP_Audio *audio = std::get_if<XOCHIP_Audio>(&audio_system);
								audio->PauseAudio(true);
								audio->Reset();
								break;
							}
							case MachineAudioModel::Sampled_HyperCHIP64:
							{
								HyperCHIP64_Audio *audio = std::get_if<HyperCHIP64_Audio>(&audio_system);
								audio->PauseAudio(true, v);
								audio->Reset(v);
								break;
							}
						}
					}
				}
			}
		}
	}
}
*/

void Hyper_BandCHIP::Machine::SyncToCycle()
{
	RunDelayTimer();
	RunSoundTimer<0>();
	RunSoundTimer<1>();
	RunSoundTimer<2>();
	RunSoundTimer<3>();
	++RefreshSync.cycle_counter;
	if (RefreshSync.cycle_counter >= RefreshSync.cycles_per_frame)
	{
		RefreshSync.cycle_counter = 0;
		if (CurrentMachineCore == MachineCore::BandCHIP_CHIP8)
		{
			const CHIP8_BehaviorData *Behavior = std::get_if<CHIP8_BehaviorData>(&behavior_data);
			if (Behavior->VIP_Display_Interrupt)
			{
				display_interrupt = true;
				return;
			}
		}
		DisplayRenderer->WriteToDisplay(display, display_width, display_height);
		DisplayRenderer->Render();
	}
}

void Hyper_BandCHIP::Machine::DisplayRender()
{
	DisplayRenderer->WriteToDisplay(display, display_width, display_height);
	DisplayRenderer->Render();
}
