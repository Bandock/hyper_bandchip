#include "../include/machine.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <bit>

Hyper_BandCHIP::Machine::Machine(MachineCore Core, unsigned int cycles_per_second, unsigned int memory_size, unsigned short display_width, unsigned short display_height) : CurrentMachineCore(Core), CurrentResolutionMode(ResolutionMode::LoRes), cycles_per_second(cycles_per_second), delay_timer(0), sound_timer{ 0, 0, 0, 0 }, PC(0), I(0), SP(0), memory(nullptr), display(nullptr), plane(0x01), voice(0), rng_engine(system_clock::now().time_since_epoch().count()), rng_distrib(0, 255), cycle_accumulator(0.0), dt_accumulator(0.0), st_accumulator{ 0.0, 0.0, 0.0, 0.0 } , pause(true), operational(true), error_state(MachineError::NoError)
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
	memset(V, 0, sizeof(V));
	memset(stack, 0, sizeof(stack));
	this->memory_size = (memory_size < 4096) ? 4096 : memory_size;
	memory = new unsigned char[this->memory_size];
	memset(memory, 0, this->memory_size);
	this->display_width = (display_width < 64) ? 64 : display_width;
	this->display_height = (display_height < 32) ? 32 : display_height;
	display = new unsigned char[this->display_width * this->display_height];
	memset(display, 0x00, this->display_width * this->display_height);
	memset(key_status, 0x00, 0x10);
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

void Hyper_BandCHIP::Machine::SetResolutionMode(Hyper_BandCHIP::ResolutionMode Mode)
{
	switch (CurrentMachineCore)
	{
		case MachineCore::BandCHIP_SuperCHIP:
		case MachineCore::BandCHIP_XOCHIP:
		case MachineCore::BandCHIP_HyperCHIP64:
		{
			CurrentResolutionMode = Mode;
			break;
		}
	}
}

void Hyper_BandCHIP::Machine::InitializeRegisters()
{
	memset(V, 0, sizeof(V));
	I = 0;
}

void Hyper_BandCHIP::Machine::InitializeTimers()
{
	delay_timer = 0;
	dt_accumulator = 0.0;
	sound_timer = { 0, 0, 0, 0 };
	st_accumulator = { 0.0, 0.0, 0.0, 0.0 };
}

void Hyper_BandCHIP::Machine::InitializeStack()
{
	SP = 0;
	memset(stack, 0, sizeof(stack));
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

void Hyper_BandCHIP::Machine::InitializeAudio()
{
	switch (CurrentMachineAudioModel)
	{
		case MachineAudioModel::Sampled_XOCHIP:
		{
			XOCHIP_Audio *audio = std::get_if<XOCHIP_Audio>(&audio_system);
			audio->InitializeAudioBuffer();
			audio->Reset();
			audio->SetPlaybackRate(64);
			break;
		}
		case MachineAudioModel::Sampled_HyperCHIP64:
		{
			HyperCHIP64_Audio *audio = std::get_if<HyperCHIP64_Audio>(&audio_system);
			voice = 0;
			for (unsigned char v = 0; v < 4; ++v)
			{
				audio->InitializeAudioBuffer(v);
				audio->Reset(v);
				audio->SetChannelOutput(0x03, v);
				audio->SetPlaybackRate(64, v);
				break;
			}
		}
	}
}

void Hyper_BandCHIP::Machine::InitializeKeyStatus()
{
	memset(key_status, 0x00, 0x10);
}

void Hyper_BandCHIP::Machine::CopyDataToInterpreterMemory(const unsigned char *source, unsigned short address, unsigned int size)
{
	if (source != nullptr)
	{
		if (size > 0 && size <= 0x200 && address < 0x200)
		{
			if (address + size - 1 < 0x200)
			{
				memcpy(&memory[address], source, size);
			}
		}
	}
}

void Hyper_BandCHIP::Machine::GetDisplay(unsigned char **display, unsigned short *display_width, unsigned short *display_height)
{
	if (display != nullptr)
	{
		*display = this->display;
	}
	if (display_width != nullptr)
	{
		*display_width = this->display_width;
	}
	if (display_height != nullptr)
	{
		*display_height = this->display_height;
	}
}

void Hyper_BandCHIP::Machine::SetCyclesPerSecond(unsigned int cycles_per_second)
{
	this->cycles_per_second = cycles_per_second;
}

void Hyper_BandCHIP::Machine::SetDelayTimer(unsigned char delay_timer)
{
	this->delay_timer = delay_timer;
	dt_accumulator = 0.0;
	if (this->delay_timer > 0)
	{
		dt_tp = current_tp;
	}
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
		st_accumulator[voice] = 0.0;
		if (this->sound_timer[voice] > 0)
		{
			st_tp[voice] = current_tp;
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

void Hyper_BandCHIP::Machine::SetKeyStatus(unsigned char key, bool pressed)
{
	if (key <= 0xF)
	{
		key_status[key] = pressed;
	}
}

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

void Hyper_BandCHIP::Machine::PauseProgram(bool pause)
{
	if (this->pause != pause)
	{
		if (this->pause)
		{
			cycle_tp = current_tp;
			if (delay_timer > 0)
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
					st_tp[v] = current_tp;
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
			if (delay_timer > 0)
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
					delta_time = current_tp - st_tp[v];
					if (delta_time.count() > 0.25)
					{
						delta_time = duration<double>(0.25);
					}
					st_accumulator[v] += delta_time.count();
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

bool Hyper_BandCHIP::Machine::IsPaused() const
{
	return pause;
}

bool Hyper_BandCHIP::Machine::IsOperational() const
{
	return operational;
}

Hyper_BandCHIP::MachineCore Hyper_BandCHIP::Machine::GetMachineCore() const
{
	return CurrentMachineCore;
}

Hyper_BandCHIP::MachineError Hyper_BandCHIP::Machine::GetErrorState() const
{
	return error_state;
}

Hyper_BandCHIP::MachineState Hyper_BandCHIP::Machine::GetMachineState() const
{
	MachineState State;
	memcpy (State.V, V, 0x10);
	State.PC = PC;
	State.I = I;
	State.DT = delay_timer;
	for (unsigned char i = 0; i < 4; ++i)
	{
		State.ST[i] = sound_timer[i];
	}
	return State;
}

void Hyper_BandCHIP::Machine::SetCurrentTime(const high_resolution_clock::time_point current_tp)
{
	this->current_tp = current_tp;
}

void Hyper_BandCHIP::Machine::ExecuteInstructions()
{
	duration<double> delta_time = current_tp - cycle_tp;
	if (delta_time.count() > 0.25)
	{
		delta_time = duration<double>(0.25);
	}
	cycle_accumulator += delta_time.count();
	cycle_tp = current_tp;
	while (cycle_accumulator >= 1.0 / static_cast<double>(cycles_per_second) && !pause)
	{
		cycle_accumulator -= 1.0 / static_cast<double>(cycles_per_second);
		switch (CurrentMachineCore)
		{
			case MachineCore::BandCHIP_CHIP8:
			{
				InstructionData<MachineCore::BandCHIP_CHIP8> Instruction;
				Instruction.opcode = (memory[PC] >> 4);
				Instruction.operand = ((memory[PC] & 0x0F) << 8) | (memory[((PC + 1) & 0xFFF)]);
				Instruction(this);
				break;
			}
			case MachineCore::BandCHIP_SuperCHIP:
			{
				InstructionData<MachineCore::BandCHIP_SuperCHIP> Instruction;
				Instruction.opcode = (memory[PC] >> 4);
				Instruction.operand = ((memory[PC] & 0x0F) << 8) | (memory[((PC + 1) & 0xFFF)]);
				Instruction(this);
				break;
			}
			case MachineCore::BandCHIP_XOCHIP:
			{
				InstructionData<MachineCore::BandCHIP_XOCHIP> Instruction;
				Instruction.opcode = (memory[PC] >> 4);
				Instruction.operand = ((memory[PC] & 0x0F) << 8) | (memory[static_cast<unsigned short>(PC + 1)]);
				Instruction(this);
				break;
			}
			case MachineCore::BandCHIP_HyperCHIP64:
			{
				InstructionData<MachineCore::BandCHIP_HyperCHIP64> Instruction;
				Instruction.opcode = (memory[PC] >> 4);
				Instruction.operand = ((memory[PC] & 0x0F) << 8) | (memory[static_cast<unsigned short>(PC + 1)]);
				Instruction(this);
				break;
			}
		}
	}
}

void Hyper_BandCHIP::Machine::RunDelayTimer()
{
	if (delay_timer > 0)
	{
		duration<double> delta_time = current_tp - dt_tp;
		if (delta_time.count() > 0.25)
		{
			delta_time = duration<double>(0.25);
		}
		dt_accumulator += delta_time.count();
		dt_tp = current_tp;
		if (dt_accumulator >= 1.0 / 60.0)
		{
			unsigned char tick_count = static_cast<unsigned char>(dt_accumulator / (1.0 / 60.0));
			if (delay_timer >= tick_count)
			{
				delay_timer -= tick_count;
			}
			else
			{
				delay_timer = 0;
			}
			if (delay_timer == 0)
			{
				dt_accumulator = 0.0;
			}
			else
			{
				dt_accumulator -= (static_cast<double>(tick_count) / 60.0);
			}
		}
	}
}

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
			duration<double> delta_time = current_tp - st_tp[v];
			if (delta_time.count() > 0.25)
			{
				delta_time = duration<double>(0.25);
			}
			st_accumulator[v] += delta_time.count();
			st_tp[v] = current_tp;
			if (st_accumulator[v] >= 1.0 / 60.0)
			{
				unsigned char tick_count = static_cast<unsigned char>(st_accumulator[v] / (1.0 / 60.0));
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
	}
}
