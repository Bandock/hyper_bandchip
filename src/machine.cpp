#include "../include/machine.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <bit>

Hyper_BandCHIP::Machine::Machine(MachineCore Core, unsigned int cycles_per_second, unsigned int memory_size, unsigned short display_width, unsigned short display_height) : CurrentMachineCore(Core), CurrentResolutionMode(ResolutionMode::LoRes), cycles_per_second(cycles_per_second), delay_timer(0), sound_timer(0), PC(0), I(0), SP(0), memory(nullptr), display(nullptr), prefix_flags(0), address_nibble_store(0), register_store(0), rng_engine(system_clock::now().time_since_epoch().count()), rng_distrib(0, 255), cycle_accumulator(0.0), dt_accumulator(0.0), st_accumulator(0.0), pause(true), operational(true), error_state(MachineError::NoError)
{
	switch (CurrentMachineCore)
	{
		case MachineCore::BandCHIP_CHIP8:
		{
			behavior_data = CHIP8_BehaviorData{ false, false };
			break;
		}
		case MachineCore::BandCHIP_SuperCHIP:
		{
			behavior_data = SuperCHIP_BehaviorData{ SuperCHIPVersion::Fixed_SuperCHIP11 };
			rpl_user_flags_file.open("RPLUserFlags.bin", std::ios::binary | std::ios::in | std::ios::out);
			if (!rpl_user_flags_file.is_open())
			{
				rpl_user_flags_file.open("RPLUserFlags.bin", std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
				rpl_user_flags_file.pubseekoff(7, std::ios::beg);
				rpl_user_flags_file.sputc(0x00);
			}
			break;
		}
		case MachineCore::BandCHIP_HyperCHIP64:
		{
			rpl_user_flags_file.open("RPLUserFlags.bin", std::ios::binary | std::ios::in | std::ios::out);
			if (!rpl_user_flags_file.is_open())
			{
				rpl_user_flags_file.open("RPLUserFlags.bin", std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
				rpl_user_flags_file.pubseekoff(7, std::ios::beg);
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
	prefix_flags = 0;
	address_nibble_store = 0;
	register_store = 0;
}

void Hyper_BandCHIP::Machine::InitializeTimers()
{
	delay_timer = 0;
	dt_accumulator = 0.0;
	sound_timer = 0;
	st_accumulator = 0.0;
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

void Hyper_BandCHIP::Machine::SetDelayTimer(unsigned char delay_timer)
{
	this->delay_timer = delay_timer;
	if (this->delay_timer > 0)
	{
		dt_tp = current_tp;
		dt_accumulator = cycle_accumulator;
	}
	else
	{
		dt_accumulator = 0.0;
	}
}

void Hyper_BandCHIP::Machine::SetSoundTimer(unsigned char sound_timer)
{
	this->sound_timer = sound_timer;
	if (this->sound_timer > 0)
	{
		st_tp = current_tp;
		st_accumulator = cycle_accumulator;
		switch (audio.GetEnvelopeGeneratorState())
		{
			case EnvelopeGeneratorState::Release:
			case EnvelopeGeneratorState::Off:
			{
				audio.SetEnvelopeGeneratorState(EnvelopeGeneratorState::Attack);
				break;
			}
		}
	}
	else
	{
		st_accumulator = 0.0;
		switch (audio.GetEnvelopeGeneratorState())
		{
			case EnvelopeGeneratorState::Attack:
			case EnvelopeGeneratorState::Decay:
			case EnvelopeGeneratorState::Sustain:
			{
				audio.SetEnvelopeGeneratorState(EnvelopeGeneratorState::Release);
				break;
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
			if (sound_timer > 0)
			{
				st_tp = current_tp;
				if (audio.GetEnvelopeGeneratorState() == EnvelopeGeneratorState::Off)
				{
					audio.SetEnvelopeGeneratorState(EnvelopeGeneratorState::Attack);
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
			if (sound_timer > 0)
			{
				delta_time = current_tp - st_tp;
				if (delta_time.count() > 0.25)
				{
					delta_time = duration<double>(0.25);
				}
				st_accumulator += delta_time.count();
				switch (audio.GetEnvelopeGeneratorState())
				{
					case EnvelopeGeneratorState::Attack:
					case EnvelopeGeneratorState::Decay:
					case EnvelopeGeneratorState::Sustain:
					{
						audio.SetEnvelopeGeneratorState(EnvelopeGeneratorState::Release);
						break;
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
	State.ST = sound_timer;
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
			case MachineCore::BandCHIP_HyperCHIP64:
			{
				InstructionData<MachineCore::BandCHIP_HyperCHIP64> Instruction;
				Instruction.opcode = (memory[PC] >> 4);
				Instruction.operand = ((memory[PC] & 0x0F) << 8) | (memory[static_cast<unsigned short>(PC + 1)] & 0xFFF);
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
	if (sound_timer > 0)
	{
		duration<double> delta_time = current_tp - st_tp;
		if (delta_time.count() > 0.25)
		{
			delta_time = duration<double>(0.25);
		}
		st_accumulator += delta_time.count();
		st_tp = current_tp;
		if (st_accumulator >= 1.0 / 60.0)
		{
			unsigned char tick_count = static_cast<unsigned char>(st_accumulator / (1.0 / 60.0));
			if (sound_timer >= tick_count)
			{
				sound_timer -= tick_count;
			}
			else
			{
				sound_timer = 0;
			}
			if (sound_timer == 0)
			{
				st_accumulator = 0.0;
				audio.SetEnvelopeGeneratorState(EnvelopeGeneratorState::Release);
			}
			else
			{
				st_accumulator -= (static_cast<double>(tick_count) / 60.0);
			}
		}
	}
}

void Hyper_BandCHIP::InstructionData<Hyper_BandCHIP::MachineCore::BandCHIP_CHIP8>::operator()(Machine *TargetMachine)
{
	if (TargetMachine != nullptr)
	{
		switch (opcode)
		{
			case 0x0:
			{
				switch (operand)
				{
					case 0xE0:
					{
						TargetMachine->InitializeVideo();
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					case 0xEE:
					{
						if (TargetMachine->SP > 0)
						{
							TargetMachine->PC = TargetMachine->stack[--TargetMachine->SP];
							TargetMachine->stack[TargetMachine->SP] = 0;
							if (TargetMachine->PC > 0xFFF)
							{
								TargetMachine->PC &= 0xFFF;
							}
						}
						else
						{
							TargetMachine->PauseProgram(true);
							TargetMachine->error_state = MachineError::StackUnderflow;
						}
						break;
					}
					default:
					{
						TargetMachine->PauseProgram(true);
						TargetMachine->error_state = MachineError::MachineInstructionsUnsupported;
						break;
					}
				}
				break;
			}
			case 0x1:
			{
				TargetMachine->PC = (operand & 0xFFF);
				break;
			}
			case 0x2:
			{
				if (TargetMachine->SP < (sizeof(TargetMachine->stack) / sizeof(unsigned short)))
				{
					TargetMachine->stack[TargetMachine->SP++] = TargetMachine->PC + 2;
					TargetMachine->PC = (operand & 0xFFF);
				}
				else
				{
					TargetMachine->PauseProgram(true);
					TargetMachine->error_state = MachineError::StackOverflow;
				}
				break;
			}
			case 0x3:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char value = (operand & 0x0FF);
				TargetMachine->PC += 2;
				if (TargetMachine->V[x] == value)
				{
					TargetMachine->PC += 2;
				}
				if (TargetMachine->PC > 0xFFF)
				{
					TargetMachine->PC &= 0xFFF;
				}
				break;
			}
			case 0x4:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char value = (operand & 0x0FF);
				TargetMachine->PC += 2;
				if (TargetMachine->V[x] != value)
				{
					TargetMachine->PC += 2;
				}
				if (TargetMachine->PC > 0xFFF)
				{
					TargetMachine->PC &= 0xFFF;
				}
				break;
			}
			case 0x5:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char y = ((operand & 0x0F0) >> 4);
				unsigned char z = (operand & 0x00F);
				if (z != 0)
				{
					TargetMachine->pause = true;
					TargetMachine->error_state = MachineError::InvalidInstruction;
					break;
				}
				TargetMachine->PC += 2;
				if (TargetMachine->V[x] == TargetMachine->V[y])
				{
					TargetMachine->PC += 2;
				}
				if (TargetMachine->PC > 0xFFF)
				{
					TargetMachine->PC &= 0xFFF;
				}
				break;
			}
			case 0x6:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char value = (operand & 0x0FF);
				TargetMachine->V[x] = value;
				TargetMachine->PC += 2;
				if (TargetMachine->PC > 0xFFF)
				{
					TargetMachine->PC &= 0xFFF;
				}
				break;
			}
			case 0x7:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char value = (operand & 0x0FF);
				TargetMachine->V[x] += value;
				TargetMachine->PC += 2;
				if (TargetMachine->PC > 0xFFF)
				{
					TargetMachine->PC &= 0xFFF;
				}
				break;
			}
			case 0x8:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char y = ((operand & 0x0F0) >> 4);
				unsigned char z = (operand & 0x00F);
				bool valid = true;
				switch (z)
				{
					case 0x0:
					{
						TargetMachine->V[x] = TargetMachine->V[y];
						break;
					}
					case 0x1:
					{
						TargetMachine->V[x] |= TargetMachine->V[y];
						break;
					}
					case 0x2:
					{
						TargetMachine->V[x] &= TargetMachine->V[y];
						break;
					}
					case 0x3:
					{
						TargetMachine->V[x] ^= TargetMachine->V[y];
						break;
					}
					case 0x4:
					{
						unsigned char tmp = TargetMachine->V[x] + TargetMachine->V[y];
						if (tmp < TargetMachine->V[x])
						{
							TargetMachine->V[0xF] = 0x01;
						}
						else
						{
							TargetMachine->V[0xF] = 0x00;
						}
						TargetMachine->V[x] = tmp;
						break;
					}
					case 0x5:
					{
						unsigned char tmp = TargetMachine->V[x] - TargetMachine->V[y];
						if (tmp > TargetMachine->V[x])
						{
							TargetMachine->V[0xF] = 0x00;
						}
						else
						{
							TargetMachine->V[0xF] = 0x01;
						}
						TargetMachine->V[x] = tmp;
						break;
					}
					case 0x6:
					{
						const CHIP8_BehaviorData *Behavior = std::get_if<CHIP8_BehaviorData>(&TargetMachine->behavior_data);
						TargetMachine->V[0xF] = ((Behavior->CHIP48_Shift ? TargetMachine->V[x] : TargetMachine->V[y]) & 0x01);
						TargetMachine->V[x] = ((Behavior->CHIP48_Shift ? TargetMachine->V[x] : TargetMachine->V[y]) >> 1);
						break;
					}
					case 0x7:
					{
						unsigned char tmp = TargetMachine->V[y] - TargetMachine->V[x];
						if (tmp > TargetMachine->V[y])
						{
							TargetMachine->V[0xF] = 0x00;
						}
						else
						{
							TargetMachine->V[0xF] = 0x01;
						}
						TargetMachine->V[x] = tmp;
						break;
					}
					case 0xE:
					{
						const CHIP8_BehaviorData *Behavior = std::get_if<CHIP8_BehaviorData>(&TargetMachine->behavior_data);
						TargetMachine->V[0xF] = (((Behavior->CHIP48_Shift ? TargetMachine->V[x] : TargetMachine->V[y]) & 0x80) >> 7);
						TargetMachine->V[x] = ((Behavior->CHIP48_Shift ? TargetMachine->V[x] : TargetMachine->V[y]) << 1);
						break;
					}
					default:
					{
						valid = false;
						break;
					}
				}
				if (valid)
				{
					TargetMachine->PC += 2;
					if (TargetMachine->PC > 0xFFF)
					{
						TargetMachine->PC &= 0xFFF;
					}
				}
				else
				{
					TargetMachine->PauseProgram(true);
					TargetMachine->error_state = MachineError::InvalidInstruction;
				}
				break;
			}
			case 0x9:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char y = ((operand & 0x0F0) >> 4);
				unsigned char z = (operand & 0x00F);
				if (z != 0)
				{
					TargetMachine->PauseProgram(true);
					TargetMachine->error_state = MachineError::InvalidInstruction;
					break;
				}
				TargetMachine->PC += 2;
				if (TargetMachine->V[x] != TargetMachine->V[y])
				{
					TargetMachine->PC += 2;
				}
				if (TargetMachine->PC > 0xFFF)
				{
					TargetMachine->PC &= 0xFFF;
				}
				break;
			}
			case 0xA:
			{
				TargetMachine->I = (operand & 0xFFF);
				TargetMachine->PC += 2;
				if (TargetMachine->PC > 0xFFF)
				{
					TargetMachine->PC &= 0xFFF;
				}
				break;
			}
			case 0xB:
			{
				TargetMachine->PC = (operand & 0xFFF) + TargetMachine->V[0];
				if (TargetMachine->PC > 0xFFF)
				{
					TargetMachine->PC &= 0xFFF;
				}
				break;
			}
			case 0xC:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char mask = (operand & 0x0FF);
				TargetMachine->V[x] = (TargetMachine->rng_distrib(TargetMachine->rng_engine) & mask);
				TargetMachine->PC += 2;
				if (TargetMachine->PC > 0xFFF)
				{
					TargetMachine->PC &= 0xFFF;
				}
				break;
			}
			case 0xD:
			{
				unsigned char x = TargetMachine->V[((operand & 0xF00) >> 8)];
				unsigned char y = TargetMachine->V[((operand & 0x0F0) >> 4)];
				unsigned char bytes = (operand & 0x00F);
				unsigned char *sprite = &TargetMachine->memory[TargetMachine->I];
				bool pixels_changed_to_unset = false;
				for (unsigned int py = 0; py < bytes; ++py)
				{
					unsigned char b_offset = 0;
					for (unsigned int px = 0; px < 8; ++px)
					{
						unsigned char pixel = ((sprite[py] & (0x80 >> b_offset++)) >> (7 - px));
						unsigned int display_addr = (((y + py) % 32) * 64) + ((x + px) % 64);
						unsigned char tmp = TargetMachine->display[display_addr];
						TargetMachine->display[display_addr] ^= pixel;
						if (TargetMachine->display[display_addr] != tmp && TargetMachine->display[display_addr] == 0 && !pixels_changed_to_unset)
						{
							pixels_changed_to_unset = true;
						}
					}
				}
				if (pixels_changed_to_unset)
				{
					TargetMachine->V[0xF] = 0x01;
				}
				else
				{
					TargetMachine->V[0xF] = 0x00;
				}
				TargetMachine->PC += 2;
				if (TargetMachine->PC > 0xFFF)
				{
					TargetMachine->PC &= 0xFFF;
				}
				break;
			}
			case 0xE:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char opcode2 = (operand & 0x0FF);
				bool valid = true;
				switch (opcode2)
				{
					case 0x9E:
					{
						TargetMachine->PC += 2;
						if (TargetMachine->key_status[(TargetMachine->V[x] & 0xF)] == 1)
						{
							TargetMachine->PC += 2;
						}
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					case 0xA1:
					{
						TargetMachine->PC += 2;
						if (TargetMachine->key_status[(TargetMachine->V[x] & 0xF)] == 0)
						{
							TargetMachine->PC += 2;
						}
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					default:
					{
						valid = false;
						break;
					}
				}
				if (!valid)
				{
					TargetMachine->PauseProgram(true);
					TargetMachine->error_state = MachineError::InvalidInstruction;
				}
				break;
			}
			case 0xF:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char opcode2 = (operand & 0x0FF);
				bool valid = true;
				switch (opcode2)
				{
					case 0x07:
					{
						TargetMachine->V[x] = TargetMachine->delay_timer;
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					case 0x0A:
					{
						for (unsigned char i = 0x00; i < 0x10; ++i)
						{
							if (TargetMachine->key_status[i] == 1)
							{
								TargetMachine->V[x] = i;
								TargetMachine->PC += 2;
								if (TargetMachine->PC > 0xFFF)
								{
									TargetMachine->PC &= 0xFFF;
								}
								break;
							}
						}
						break;
					}
					case 0x15:
					{
						TargetMachine->SetDelayTimer(TargetMachine->V[x]);
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					case 0x18:
					{
						TargetMachine->SetSoundTimer(TargetMachine->V[x]);
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					case 0x1E:
					{
						TargetMachine->I += TargetMachine->V[x];
						if (TargetMachine->I > 0xFFF)
						{
							TargetMachine->I &= 0xFFF;
						}
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					case 0x29:
					{
						switch (TargetMachine->V[x])
						{
							case 0x0:
							{
								TargetMachine->I = 0x000;
								break;
							}
							case 0x1:
							{
								TargetMachine->I = 0x005;
								break;
							}
							case 0x2:
							{
								TargetMachine->I = 0x00A;
								break;
							}
							case 0x3:
							{
								TargetMachine->I = 0x00F;
								break;
							}
							case 0x4:
							{
								TargetMachine->I = 0x014;
								break;
							}
							case 0x5:
							{
								TargetMachine->I = 0x019;
								break;
							}
							case 0x6:
							{
								TargetMachine->I = 0x01E;
								break;
							}
							case 0x7:
							{
								TargetMachine->I = 0x023;
								break;
							}
							case 0x8:
							{
								TargetMachine->I = 0x028;
								break;
							}
							case 0x9:
							{
								TargetMachine->I = 0x02D;
								break;
							}
							case 0xA:
							{
								TargetMachine->I = 0x032;
								break;
							}
							case 0xB:
							{
								TargetMachine->I = 0x037;
								break;
							}
							case 0xC:
							{
								TargetMachine->I = 0x03C;
								break;
							}
							case 0xD:
							{
								TargetMachine->I = 0x041;
								break;
							}
							case 0xE:
							{
								TargetMachine->I = 0x046;
								break;
							}
							case 0xF:
							{
								TargetMachine->I = 0x04B;
								break;
							}
						}
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					case 0x33:
					{
						unsigned char tmp = TargetMachine->V[x];
						unsigned char digit[3];
						for (unsigned char i = 0; i < 3; ++i)
						{
							digit[i] = tmp % 10;
							tmp /= 10;
						}
						for (unsigned char i = 0; i < 3; ++i)
						{
							TargetMachine->memory[((TargetMachine->I + i) & 0xFFF)] = digit[2 - i];
						}
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					case 0x55:
					{
						const CHIP8_BehaviorData *Behavior = std::get_if<CHIP8_BehaviorData>(&TargetMachine->behavior_data);
						for (unsigned char i = 0; i <= x; ++i)
						{
							TargetMachine->memory[((TargetMachine->I + i) & 0xFFF)] = TargetMachine->V[i];
						}
						TargetMachine->I += x + (Behavior->CHIP48_LoadStore ? 0 : 1);
						if (TargetMachine->I > 0xFFF)
						{
							TargetMachine->I &= 0xFFF;
						}
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					case 0x65:
					{
						const CHIP8_BehaviorData *Behavior = std::get_if<CHIP8_BehaviorData>(&TargetMachine->behavior_data);
						for (unsigned char i = 0; i <= x; ++i)
						{
							TargetMachine->V[i] = TargetMachine->memory[((TargetMachine->I + i) & 0xFFF)];
						}
						TargetMachine->I += x + (Behavior->CHIP48_LoadStore ? 0 : 1);
						if (TargetMachine->I > 0xFFF)
						{
							TargetMachine->I &= 0xFFF;
						}
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					default:
					{
						valid = false;
						break;
					}
				}
				if (!valid)
				{
					TargetMachine->PauseProgram(true);
					TargetMachine->error_state = MachineError::InvalidInstruction;
				}
				break;
			}
		}
	}
}

void Hyper_BandCHIP::InstructionData<Hyper_BandCHIP::MachineCore::BandCHIP_SuperCHIP>::operator()(Machine *TargetMachine)
{
	if (TargetMachine != nullptr)
	{
		switch (opcode)
		{
			case 0x0:
			{
				switch (operand)
				{
					case 0xC0:
					case 0xC1:
					case 0xC2:
					case 0xC3:
					case 0xC4:
					case 0xC5:
					case 0xC6:
					case 0xC7:
					case 0xC8:
					case 0xC9:
					case 0xCA:
					case 0xCB:
					case 0xCC:
					case 0xCD:
					case 0xCE:
					case 0xCF:
					{
						const SuperCHIP_BehaviorData *Behavior = std::get_if<SuperCHIP_BehaviorData>(&TargetMachine->behavior_data);
						switch (Behavior->Version)
						{
							case SuperCHIPVersion::Fixed_SuperCHIP11:
							case SuperCHIPVersion::SuperCHIP11:
							{
								memmove(&TargetMachine->display[TargetMachine->display_width * (operand & 0x0F)], &TargetMachine->display[0], TargetMachine->display_width * (TargetMachine->display_height - (operand & 0x0F)));
								memset(TargetMachine->display, 0x00, TargetMachine->display_width * (operand & 0x0F));
								TargetMachine->PC += 2;
								if (TargetMachine->PC > 0xFFF)
								{
									TargetMachine->PC &= 0xFFF;
								}
								break;
							}
							case SuperCHIPVersion::SuperCHIP10:
							{
								TargetMachine->PauseProgram(true);
								TargetMachine->error_state = MachineError::InvalidInstruction;
								break;
							}
						}
						break;
					}
					case 0xE0:
					{
						TargetMachine->InitializeVideo();
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					case 0xEE:
					{
						if (TargetMachine->SP > 0)
						{
							TargetMachine->PC = TargetMachine->stack[--TargetMachine->SP];
							TargetMachine->stack[TargetMachine->SP] = 0;
							if (TargetMachine->PC > 0xFFF)
							{
								TargetMachine->PC &= 0xFFF;
							}
						}
						else
						{
							TargetMachine->PauseProgram(true);
							TargetMachine->error_state = MachineError::StackUnderflow;
						}
						break;
					}
					case 0xFB:
					{
						const SuperCHIP_BehaviorData *Behavior = std::get_if<SuperCHIP_BehaviorData>(&TargetMachine->behavior_data);
						switch (Behavior->Version)
						{
							case SuperCHIPVersion::Fixed_SuperCHIP11:
							case SuperCHIPVersion::SuperCHIP11:
							{
								for (unsigned short y = 0; y < TargetMachine->display_height; ++y)
								{
									memmove(&TargetMachine->display[(y * TargetMachine->display_width) + 4], &TargetMachine->display[y * TargetMachine->display_width], TargetMachine->display_width - 4);
									memset(&TargetMachine->display[y * TargetMachine->display_width], 0x00, 4);
								}
								TargetMachine->PC += 2;
								if (TargetMachine->PC > 0xFFF)
								{
									TargetMachine->PC &= 0xFFF;
								}
								break;
							}
							case SuperCHIPVersion::SuperCHIP10:
							{
								TargetMachine->PauseProgram(true);
								TargetMachine->error_state = MachineError::InvalidInstruction;
								break;
							}
						}
						break;
					}
					case 0xFC:
					{
						const SuperCHIP_BehaviorData *Behavior = std::get_if<SuperCHIP_BehaviorData>(&TargetMachine->behavior_data);
						switch (Behavior->Version)
						{
							case SuperCHIPVersion::Fixed_SuperCHIP11:
							case SuperCHIPVersion::SuperCHIP11:
							{
								for (unsigned short y = 0; y < TargetMachine->display_height; ++y)
								{
									memmove(&TargetMachine->display[y * TargetMachine->display_width], &TargetMachine->display[(y * TargetMachine->display_width) + 4], TargetMachine->display_width - 4);
									memset(&TargetMachine->display[(y * TargetMachine->display_width) + (TargetMachine->display_width - 4)], 0x00, 4);
								}
								TargetMachine->PC += 2;
								if (TargetMachine->PC > 0xFFF)
								{
									TargetMachine->PC &= 0xFFF;
								}
								break;
							}
							case SuperCHIPVersion::SuperCHIP10:
							{
								TargetMachine->PauseProgram(true);
								TargetMachine->error_state = MachineError::InvalidInstruction;
								break;
							}
						}
						break;
					}
					case 0xFD:
					{
						TargetMachine->InitializeVideo();
						TargetMachine->PauseProgram(true);
						TargetMachine->operational = false;
						break;
					}
					case 0xFE:
					{
						if (TargetMachine->CurrentResolutionMode == ResolutionMode::HiRes)
						{
							TargetMachine->CurrentResolutionMode = ResolutionMode::LoRes;
							TargetMachine->InitializeVideo();
						}
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					case 0xFF:
					{
						if (TargetMachine->CurrentResolutionMode == ResolutionMode::LoRes)
						{
							TargetMachine->CurrentResolutionMode = ResolutionMode::HiRes;
							TargetMachine->InitializeVideo();
						}
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					default:
					{
						TargetMachine->PauseProgram(true);
						TargetMachine->error_state = MachineError::MachineInstructionsUnsupported;
						break;
					}
				}
				break;
			}
			case 0x1:
			{
				TargetMachine->PC = (operand & 0xFFF);
				break;
			}
			case 0x2:
			{
				if (TargetMachine->SP < (sizeof(TargetMachine->stack) / sizeof(unsigned short)))
				{
					TargetMachine->stack[TargetMachine->SP++] = TargetMachine->PC + 2;
					TargetMachine->PC = (operand & 0xFFF);
				}
				else
				{
					TargetMachine->PauseProgram(true);
					TargetMachine->error_state = MachineError::StackOverflow;
				}
				break;
			}
			case 0x3:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char value = (operand & 0x0FF);
				TargetMachine->PC += 2;
				if (TargetMachine->V[x] == value)
				{
					TargetMachine->PC += 2;
				}
				if (TargetMachine->PC > 0xFFF)
				{
					TargetMachine->PC &= 0xFFF;
				}
				break;
			}
			case 0x4:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char value = (operand & 0x0FF);
				TargetMachine->PC += 2;
				if (TargetMachine->V[x] != value)
				{
					TargetMachine->PC += 2;
				}
				if (TargetMachine->PC > 0xFFF)
				{
					TargetMachine->PC &= 0xFFF;
				}
				break;
			}
			case 0x5:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char y = ((operand & 0x0F0) >> 4);
				unsigned char z = (operand & 0x00F);
				if (z != 0)
				{
					TargetMachine->pause = true;
					TargetMachine->error_state = MachineError::InvalidInstruction;
					break;
				}
				TargetMachine->PC += 2;
				if (TargetMachine->V[x] == TargetMachine->V[y])
				{
					TargetMachine->PC += 2;
				}
				if (TargetMachine->PC > 0xFFF)
				{
					TargetMachine->PC &= 0xFFF;
				}
				break;
			}
			case 0x6:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char value = (operand & 0x0FF);
				TargetMachine->V[x] = value;
				TargetMachine->PC += 2;
				if (TargetMachine->PC > 0xFFF)
				{
					TargetMachine->PC &= 0xFFF;
				}
				break;
			}
			case 0x7:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char value = (operand & 0x0FF);
				TargetMachine->V[x] += value;
				TargetMachine->PC += 2;
				if (TargetMachine->PC > 0xFFF)
				{
					TargetMachine->PC &= 0xFFF;
				}
				break;
			}
			case 0x8:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char y = ((operand & 0x0F0) >> 4);
				unsigned char z = (operand & 0x00F);
				bool valid = true;
				switch (z)
				{
					case 0x0:
					{
						TargetMachine->V[x] = TargetMachine->V[y];
						break;
					}
					case 0x1:
					{
						TargetMachine->V[x] |= TargetMachine->V[y];
						break;
					}
					case 0x2:
					{
						TargetMachine->V[x] &= TargetMachine->V[y];
						break;
					}
					case 0x3:
					{
						TargetMachine->V[x] ^= TargetMachine->V[y];
						break;
					}
					case 0x4:
					{
						unsigned char tmp = TargetMachine->V[x] + TargetMachine->V[y];
						if (tmp < TargetMachine->V[x])
						{
							TargetMachine->V[0xF] = 0x01;
						}
						else
						{
							TargetMachine->V[0xF] = 0x00;
						}
						TargetMachine->V[x] = tmp;
						break;
					}
					case 0x5:
					{
						unsigned char tmp = TargetMachine->V[x] - TargetMachine->V[y];
						if (tmp > TargetMachine->V[x])
						{
							TargetMachine->V[0xF] = 0x00;
						}
						else
						{
							TargetMachine->V[0xF] = 0x01;
						}
						TargetMachine->V[x] = tmp;
						break;
					}
					case 0x6:
					{
						const SuperCHIP_BehaviorData *Behavior = std::get_if<SuperCHIP_BehaviorData>(&TargetMachine->behavior_data);
						TargetMachine->V[0xF] = (((Behavior->Version == SuperCHIPVersion::Fixed_SuperCHIP11) ? TargetMachine->V[y] : TargetMachine->V[x]) & 0x01);
						TargetMachine->V[x] = (((Behavior->Version == SuperCHIPVersion::Fixed_SuperCHIP11) ? TargetMachine->V[y] : TargetMachine->V[x]) >> 1);
						break;
					}
					case 0x7:
					{
						unsigned char tmp = TargetMachine->V[y] - TargetMachine->V[x];
						if (tmp > TargetMachine->V[y])
						{
							TargetMachine->V[0xF] = 0x00;
						}
						else
						{
							TargetMachine->V[0xF] = 0x01;
						}
						TargetMachine->V[x] = tmp;
						break;
					}
					case 0xE:
					{
						const SuperCHIP_BehaviorData *Behavior = std::get_if<SuperCHIP_BehaviorData>(&TargetMachine->behavior_data);
						TargetMachine->V[0xF] = ((((Behavior->Version == SuperCHIPVersion::Fixed_SuperCHIP11) ? TargetMachine->V[y] : TargetMachine->V[x]) & 0x80) >> 7);
						TargetMachine->V[x] = (((Behavior->Version == SuperCHIPVersion::Fixed_SuperCHIP11) ? TargetMachine->V[y] : TargetMachine->V[x]) << 1);
						break;
					}
					default:
					{
						valid = false;
						break;
					}
				}
				if (valid)
				{
					TargetMachine->PC += 2;
					if (TargetMachine->PC > 0xFFF)
					{
						TargetMachine->PC &= 0xFFF;
					}
				}
				else
				{
					TargetMachine->PauseProgram(true);
					TargetMachine->error_state = MachineError::InvalidInstruction;
				}
				break;
			}
			case 0x9:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char y = ((operand & 0x0F0) >> 4);
				unsigned char z = (operand & 0x00F);
				if (z != 0)
				{
					TargetMachine->PauseProgram(true);
					TargetMachine->error_state = MachineError::InvalidInstruction;
					break;
				}
				TargetMachine->PC += 2;
				if (TargetMachine->V[x] != TargetMachine->V[y])
				{
					TargetMachine->PC += 2;
				}
				if (TargetMachine->PC > 0xFFF)
				{
					TargetMachine->PC &= 0xFFF;
				}
				break;
			}
			case 0xA:
			{
				TargetMachine->I = (operand & 0xFFF);
				TargetMachine->PC += 2;
				if (TargetMachine->PC > 0xFFF)
				{
					TargetMachine->PC &= 0xFFF;
				}
				break;
			}
			case 0xB:
			{
				TargetMachine->PC = (operand & 0xFFF) + TargetMachine->V[0];
				if (TargetMachine->PC > 0xFFF)
				{
					TargetMachine->PC &= 0xFFF;
				}
				break;
			}
			case 0xC:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char mask = (operand & 0x0FF);
				TargetMachine->V[x] = (TargetMachine->rng_distrib(TargetMachine->rng_engine) & mask);
				TargetMachine->PC += 2;
				if (TargetMachine->PC > 0xFFF)
				{
					TargetMachine->PC &= 0xFFF;
				}
				break;
			}
			case 0xD:
			{
				unsigned char x = TargetMachine->V[((operand & 0xF00) >> 8)];
				unsigned char y = TargetMachine->V[((operand & 0x0F0) >> 4)];
				unsigned char width = 8;
				unsigned char height = (operand & 0x00F);
				unsigned char scale_factor = (TargetMachine->CurrentResolutionMode == ResolutionMode::HiRes) ? 1 : 2;
				if (!height && TargetMachine->CurrentResolutionMode == ResolutionMode::HiRes)
				{
					width = 16;
					height = 16;
				}
				unsigned char *sprite = &TargetMachine->memory[TargetMachine->I];
				bool pixels_changed_to_unset = false;
				for (unsigned int py = 0; py < height; ++py)
				{
					unsigned char b_offset = 0;
					for (unsigned int px = 0; px < width; ++px)
					{
						unsigned int y_offset = py * (width / 8);
						unsigned char pixel = ((sprite[y_offset + (px / 8)] & (0x80 >> b_offset++)) >> (7 - (px % 8)));
						if (b_offset == 8)
						{
							b_offset = 0;
						}
						unsigned int display_addr = (((y + py) * scale_factor % 64) * 128) + ((x + px) * scale_factor % 128);
						switch (TargetMachine->CurrentResolutionMode)
						{
							case ResolutionMode::LoRes:
							{
								for (unsigned char dy = 0; dy < 2; ++dy)
								{
									for (unsigned char dx = 0; dx < 2; ++dx)
									{
										unsigned char tmp = TargetMachine->display[display_addr + (dy * 128) + dx];
										TargetMachine->display[display_addr + (dy * 128) + dx] ^= pixel;
										if (TargetMachine->display[display_addr + (dy * 128) + dx] != tmp && TargetMachine->display[display_addr + (dy * 128) + dx] == 0 && !pixels_changed_to_unset)
										{
											pixels_changed_to_unset = true;
										}
									}
								}
								break;
							}
							case ResolutionMode::HiRes:
							{
								unsigned char tmp = TargetMachine->display[display_addr];
								TargetMachine->display[display_addr] ^= pixel;
								if (TargetMachine->display[display_addr] != tmp && TargetMachine->display[display_addr] == 0 && !pixels_changed_to_unset)
								{
									pixels_changed_to_unset = true;
								}
								break;
							}
						}
					}
				}
				if (pixels_changed_to_unset)
				{
					TargetMachine->V[0xF] = 0x01;
				}
				else
				{
					TargetMachine->V[0xF] = 0x00;
				}
				TargetMachine->PC += 2;
				if (TargetMachine->PC > 0xFFF)
				{
					TargetMachine->PC &= 0xFFF;
				}
				break;
			}
			case 0xE:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char opcode2 = (operand & 0x0FF);
				bool valid = true;
				switch (opcode2)
				{
					case 0x9E:
					{
						TargetMachine->PC += 2;
						if (TargetMachine->key_status[(TargetMachine->V[x] & 0xF)] == 1)
						{
							TargetMachine->PC += 2;
						}
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					case 0xA1:
					{
						TargetMachine->PC += 2;
						if (TargetMachine->key_status[(TargetMachine->V[x] & 0xF)] == 0)
						{
							TargetMachine->PC += 2;
						}
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					default:
					{
						valid = false;
						break;
					}
				}
				if (!valid)
				{
					TargetMachine->PauseProgram(true);
					TargetMachine->error_state = MachineError::InvalidInstruction;
				}
				break;
			}
			case 0xF:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char opcode2 = (operand & 0x0FF);
				bool valid = true;
				switch (opcode2)
				{
					case 0x07:
					{
						TargetMachine->V[x] = TargetMachine->delay_timer;
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					case 0x0A:
					{
						for (unsigned char i = 0x00; i < 0x10; ++i)
						{
							if (TargetMachine->key_status[i] == 1)
							{
								TargetMachine->V[x] = i;
								TargetMachine->PC += 2;
								if (TargetMachine->PC > 0xFFF)
								{
									TargetMachine->PC &= 0xFFF;
								}
								break;
							}
						}
						break;
					}
					case 0x15:
					{
						TargetMachine->SetDelayTimer(TargetMachine->V[x]);
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					case 0x18:
					{
						TargetMachine->SetSoundTimer(TargetMachine->V[x]);
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					case 0x1E:
					{
						TargetMachine->I += TargetMachine->V[x];
						if (TargetMachine->I > 0xFFF)
						{
							TargetMachine->I &= 0xFFF;
						}
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					case 0x29:
					{
						const SuperCHIP_BehaviorData *Behavior = std::get_if<SuperCHIP_BehaviorData>(&TargetMachine->behavior_data);
						switch (TargetMachine->V[x])
						{
							case 0x00:
							{
								TargetMachine->I = 0x000;
								break;
							}
							case 0x01:
							{
								TargetMachine->I = 0x005;
								break;
							}
							case 0x02:
							{
								TargetMachine->I = 0x00A;
								break;
							}
							case 0x03:
							{
								TargetMachine->I = 0x00F;
								break;
							}
							case 0x04:
							{
								TargetMachine->I = 0x014;
								break;
							}
							case 0x05:
							{
								TargetMachine->I = 0x019;
								break;
							}
							case 0x06:
							{
								TargetMachine->I = 0x01E;
								break;
							}
							case 0x07:
							{
								TargetMachine->I = 0x023;
								break;
							}
							case 0x08:
							{
								TargetMachine->I = 0x028;
								break;
							}
							case 0x09:
							{
								TargetMachine->I = 0x02D;
								break;
							}
							case 0x0A:
							{
								TargetMachine->I = 0x032;
								break;
							}
							case 0x0B:
							{
								TargetMachine->I = 0x037;
								break;
							}
							case 0x0C:
							{
								TargetMachine->I = 0x03C;
								break;
							}
							case 0x0D:
							{
								TargetMachine->I = 0x041;
								break;
							}
							case 0x0E:
							{
								TargetMachine->I = 0x046;
								break;
							}
							case 0x0F:
							{
								TargetMachine->I = 0x04B;
								break;
							}
							case 0x10:
							{
								if (Behavior->Version == SuperCHIPVersion::SuperCHIP10)
								{
									TargetMachine->I = 0x050;
								}
								break;
							}
							case 0x11:
							{
								if (Behavior->Version == SuperCHIPVersion::SuperCHIP10)
								{
									TargetMachine->I = 0x05A;
								}
								break;
							}
							case 0x12:
							{
								if (Behavior->Version == SuperCHIPVersion::SuperCHIP10)
								{
									TargetMachine->I = 0x064;
								}
								break;
							}
							case 0x13:
							{
								if (Behavior->Version == SuperCHIPVersion::SuperCHIP10)
								{
									TargetMachine->I = 0x06E;
								}
								break;
							}
							case 0x14:
							{
								if (Behavior->Version == SuperCHIPVersion::SuperCHIP10)
								{
									TargetMachine->I = 0x078;
								}
								break;
							}
							case 0x15:
							{
								if (Behavior->Version == SuperCHIPVersion::SuperCHIP10)
								{
									TargetMachine->I = 0x082;
								}
								break;
							}
							case 0x16:
							{
								if (Behavior->Version == SuperCHIPVersion::SuperCHIP10)
								{
									TargetMachine->I = 0x08C;
								}
								break;
							}
							case 0x17:
							{
								if (Behavior->Version == SuperCHIPVersion::SuperCHIP10)
								{
									TargetMachine->I = 0x096;
								}
								break;
							}
							case 0x18:
							{
								if (Behavior->Version == SuperCHIPVersion::SuperCHIP10)
								{
									TargetMachine->I = 0x0A0;
								}
								break;
							}
							case 0x19:
							{
								if (Behavior->Version == SuperCHIPVersion::SuperCHIP10)
								{
									TargetMachine->I = 0x0AA;
								}
								break;
							}
						}
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine-> PC &= 0xFFF;
						}
						break;
					}
					case 0x30:
					{
						const SuperCHIP_BehaviorData *Behavior = std::get_if<SuperCHIP_BehaviorData>(&TargetMachine->behavior_data);
						switch (Behavior->Version)
						{
							case SuperCHIPVersion::Fixed_SuperCHIP11:
							case SuperCHIPVersion::SuperCHIP11:
							{
								switch (TargetMachine->V[x])
								{
									case 0x0:
									{
										TargetMachine->I = 0x050;
										break;
									}
									case 0x1:
									{
										TargetMachine->I = 0x05A;
										break;
									}
									case 0x2:
									{
										TargetMachine->I = 0x064;
										break;
									}
									case 0x3:
									{
										TargetMachine->I = 0x06E;
										break;
									}
									case 0x4:
									{
										TargetMachine->I = 0x078;
										break;
									}
									case 0x5:
									{
										TargetMachine->I = 0x082;
										break;
									}
									case 0x6:
									{
										TargetMachine->I = 0x08C;
										break;
									}
									case 0x7:
									{
										TargetMachine->I = 0x096;
										break;
									}
									case 0x8:
									{
										TargetMachine->I = 0x0A0;
										break;
									}
									case 0x9:
									{
										TargetMachine->I = 0x0AA;
										break;
									}
								}
								TargetMachine->PC += 2;
								if (TargetMachine->PC > 0xFFF)
								{
									TargetMachine->PC &= 0xFFF;
								}
								break;
							}
							case SuperCHIPVersion::SuperCHIP10:
							{
								valid = false;
								break;
							}
						}
						break;
					}
					case 0x33:
					{
						unsigned char tmp = TargetMachine->V[x];
						unsigned char digit[3];
						for (unsigned char i = 0; i < 3; ++i)
						{
							digit[i] = tmp % 10;
							tmp /= 10;
						}
						for (unsigned char i = 0; i < 3; ++i)
						{
							TargetMachine->memory[((TargetMachine->I + i) & 0xFFF)] = digit[2 - i];
						}
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					case 0x55:
					{
						const SuperCHIP_BehaviorData *Behavior = std::get_if<SuperCHIP_BehaviorData>(&TargetMachine->behavior_data);
						for (unsigned char i = 0; i <= x; ++i)
						{
							TargetMachine->memory[((TargetMachine->I + i) & 0xFFF)] = TargetMachine->V[i];
						}
						switch (Behavior->Version)
						{
							case SuperCHIPVersion::Fixed_SuperCHIP11:
							{
								TargetMachine->I += x + 1;
								break;
							}
							case SuperCHIPVersion::SuperCHIP10:
							{
								TargetMachine->I += x;
								break;
							}
						}
						if (TargetMachine->I > 0xFFF)
						{
							TargetMachine->I &= 0xFFF;
						}
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					case 0x65:
					{
						const SuperCHIP_BehaviorData *Behavior = std::get_if<SuperCHIP_BehaviorData>(&TargetMachine->behavior_data);
						for (unsigned char i = 0; i <= x; ++i)
						{
							TargetMachine->V[i] = TargetMachine->memory[((TargetMachine->I + i) & 0xFFF)];
						}
						switch (Behavior->Version)
						{
							case SuperCHIPVersion::Fixed_SuperCHIP11:
							{
								TargetMachine->I += x + 1;
								break;
							}
							case SuperCHIPVersion::SuperCHIP10:
							{
								TargetMachine->I += x;
								break;
							}
						}
						if (TargetMachine->I > 0xFFF)
						{
							TargetMachine->I &= 0xFFF;
						}
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					case 0x75:
					{
						unsigned char flag_count = (x > 7) ? 7 : x;
						TargetMachine->rpl_user_flags_file.pubseekoff(0, std::ios::beg);
						TargetMachine->rpl_user_flags_file.sputn(reinterpret_cast<const char *>(TargetMachine->V), flag_count + 1);
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					case 0x85:
					{
						unsigned char flag_count = (x > 7) ? 7 : x;
						TargetMachine->rpl_user_flags_file.pubseekoff(0, std::ios::beg);
						TargetMachine->rpl_user_flags_file.sgetn(reinterpret_cast<char *>(TargetMachine->V), flag_count + 1);
						TargetMachine->PC += 2;
						if (TargetMachine->PC > 0xFFF)
						{
							TargetMachine->PC &= 0xFFF;
						}
						break;
					}
					default:
					{
						valid = false;
						break;
					}
				}
				if (!valid)
				{
					TargetMachine->PauseProgram(true);
					TargetMachine->error_state = MachineError::InvalidInstruction;
				}
				break;
			}
		}
	}
}

void Hyper_BandCHIP::InstructionData<Hyper_BandCHIP::MachineCore::BandCHIP_HyperCHIP64>::operator()(Machine *TargetMachine)
{
	if (TargetMachine != nullptr)
	{
		switch (opcode)
		{
			case 0x0:
			{
				switch (operand)
				{
					case 0xC0:
					case 0xC1:
					case 0xC2:
					case 0xC3:
					case 0xC4:
					case 0xC5:
					case 0xC6:
					case 0xC7:
					case 0xC8:
					case 0xC9:
					case 0xCA:
					case 0xCB:
					case 0xCC:
					case 0xCD:
					case 0xCE:
					case 0xCF:
					{
						memmove(&TargetMachine->display[TargetMachine->display_width * (operand & 0x0F)], &TargetMachine->display[0], TargetMachine->display_width * (TargetMachine->display_height - (operand & 0x0F)));
						memset(TargetMachine->display, 0x00, TargetMachine->display_width * (operand & 0x0F));
						TargetMachine->PC += 2;
						break;
					}
					case 0xD0:
					case 0xD1:
					case 0xD2:
					case 0xD3:
					case 0xD4:
					case 0xD5:
					case 0xD6:
					case 0xD7:
					case 0xD8:
					case 0xD9:
					case 0xDA:
					case 0xDB:
					case 0xDC:
					case 0xDD:
					case 0xDE:
					case 0xDF:
					{
						memmove(&TargetMachine->display[0], &TargetMachine->display[TargetMachine->display_width * (operand & 0x0F)], TargetMachine->display_width * (TargetMachine->display_height - (operand & 0x0F)));
						memset(&TargetMachine->display[TargetMachine->display_width * (TargetMachine->display_height - (operand & 0x0F))], 0x00, TargetMachine->display_width * (operand & 0x0F));
						TargetMachine->PC += 2;
						break;
					}
					case 0xE0:
					{
						TargetMachine->InitializeVideo();
						TargetMachine->PC += 2;
						break;
					}
					case 0xEE:
					{
						if (TargetMachine->SP > 0)
						{
							TargetMachine->PC = TargetMachine->stack[--TargetMachine->SP];
							TargetMachine->stack[TargetMachine->SP] = 0;
						}
						else
						{
							TargetMachine->PauseProgram(true);
							TargetMachine->error_state = MachineError::StackUnderflow;
						}
						break;
					}
					case 0xFB:
					{
						for (unsigned short y = 0; y < TargetMachine->display_height; ++y)
						{
							memmove(&TargetMachine->display[(y * TargetMachine->display_width) + 4], &TargetMachine->display[y * TargetMachine->display_width], TargetMachine->display_width - 4);
							memset(&TargetMachine->display[y * TargetMachine->display_width], 0x00, 4);
						}
						TargetMachine->PC += 2;
						break;
					}
					case 0xFC:
					{
						for (unsigned short y = 0; y < TargetMachine->display_height; ++y)
						{
							memmove(&TargetMachine->display[y * TargetMachine->display_width], &TargetMachine->display[(y * TargetMachine->display_width) + 4], TargetMachine->display_width - 4);
							memset(&TargetMachine->display[(y * TargetMachine->display_width) + (TargetMachine->display_width - 4)], 0x00, 4);
						}
						TargetMachine->PC += 2;
						break;
					}
					case 0xFD:
					{
						TargetMachine->InitializeVideo();
						TargetMachine->PauseProgram(true);
						TargetMachine->operational = false;
						break;
					}
					case 0xFE:
					{
						if (TargetMachine->CurrentResolutionMode == ResolutionMode::HiRes)
						{
							TargetMachine->CurrentResolutionMode = ResolutionMode::LoRes;
							TargetMachine->InitializeVideo();
						}
						TargetMachine->PC += 2;
						break;
					}
					case 0xFF:
					{
						if (TargetMachine->CurrentResolutionMode == ResolutionMode::LoRes)
						{
							TargetMachine->CurrentResolutionMode = ResolutionMode::HiRes;
							TargetMachine->InitializeVideo();
						}
						TargetMachine->PC += 2;
						break;
					}
					default:
					{
						TargetMachine->PauseProgram(true);
						TargetMachine->error_state = MachineError::MachineInstructionsUnsupported;
						break;
					}
				}
				break;
			}
			case 0x1:
			{
				TargetMachine->PC = ((TargetMachine->prefix_flags & 0x01) ? (TargetMachine->address_nibble_store << 8) : 0) | (operand & 0xFFF);
				if (TargetMachine->prefix_flags & 0x01)
				{
					TargetMachine->prefix_flags &= ~(0x01);
					TargetMachine->address_nibble_store = 0x00;
				}
				break;
			}
			case 0x2:
			{
				if (TargetMachine->SP < (sizeof(TargetMachine->stack) / sizeof(unsigned short)))
				{
					TargetMachine->stack[TargetMachine->SP++] = TargetMachine->PC + 2;
					TargetMachine->PC = ((TargetMachine->prefix_flags & 0x01) ? (TargetMachine->address_nibble_store << 8) : 0) | (operand & 0xFFF);
					if (TargetMachine->prefix_flags & 0x01)
					{
						TargetMachine->prefix_flags &= ~(0x01);
						TargetMachine->address_nibble_store = 0x00;
					}
					break;
				}
				else
				{
					TargetMachine->PauseProgram(true);
					TargetMachine->error_state = MachineError::StackOverflow;
				}
				break;
			}
			case 0x3:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char value = (operand & 0x0FF);
				TargetMachine->PC += 2;
				if (TargetMachine->V[x] == value)
				{
					unsigned char next_opcode = (TargetMachine->memory[TargetMachine->PC] >> 4);
					unsigned short next_operand = ((TargetMachine->memory[TargetMachine->PC] & 0x0F) << 8) | (TargetMachine->memory[static_cast<unsigned short>(TargetMachine->PC + 1)]);
					while (next_opcode == 0xF && (((next_operand & 0x0F0) >> 4) == 0xB))
					{
						TargetMachine->PC += 2;
						next_opcode = (TargetMachine->memory[TargetMachine->PC] >> 4);
						next_operand = ((TargetMachine->memory[TargetMachine->PC] & 0x0F) << 8) | (TargetMachine->memory[static_cast<unsigned short>(TargetMachine->PC + 1)] & 0xFFF);
					}
					TargetMachine->PC += 2;
				}
				break;
			}
			case 0x4:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char value = (operand & 0x0FF);
				TargetMachine->PC += 2;
				if (TargetMachine->V[x] != value)
				{
					unsigned char next_opcode = (TargetMachine->memory[TargetMachine->PC] >> 4);
					unsigned short next_operand = ((TargetMachine->memory[TargetMachine->PC] & 0x0F) << 8) | (TargetMachine->memory[static_cast<unsigned short>(TargetMachine->PC + 1)] & 0xFFF);
					while (next_opcode == 0xF && (((next_operand & 0x0F0) >> 4) == 0xB))
					{
						TargetMachine->PC += 2;
						next_opcode = (TargetMachine->memory[TargetMachine->PC] >> 4);
						next_operand = ((TargetMachine->memory[TargetMachine->PC] & 0x0F) << 8) | (TargetMachine->memory[static_cast<unsigned short>(TargetMachine->PC + 1)]);
					}
					TargetMachine->PC += 2;
				}
				break;
			}
			case 0x5:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char y = ((operand & 0x0F0) >> 4);
				unsigned char z = (operand & 0x00F);
				bool valid = true;
				switch (z)
				{
					case 0:
					{
						TargetMachine->PC += 2;
						if (TargetMachine->V[x] == TargetMachine->V[y])
						{
							unsigned char next_opcode = (TargetMachine->memory[TargetMachine->PC] >> 4);
							unsigned short next_operand = ((TargetMachine->memory[TargetMachine->PC] & 0x0F) << 8) | (TargetMachine->memory[static_cast<unsigned short>(TargetMachine->PC + 1)]);
							while (next_opcode == 0xF && (((next_operand & 0x0F0) >> 4) == 0xB))
							{
								TargetMachine->PC += 2;
								next_opcode = (TargetMachine->memory[TargetMachine->PC] >> 4);
								next_operand = ((TargetMachine->memory[TargetMachine->PC] & 0x0F) << 8) | (TargetMachine->memory[static_cast<unsigned short>(TargetMachine->PC + 1)] & 0xFFF);
							}
							TargetMachine->PC += 2;
						}	
						break;
					}
					case 2:
					{
						for (unsigned char i = ((x <= y) ? x : y); i <= ((x <= y) ? y : x); ++i)
						{
							TargetMachine->memory[TargetMachine->I + ((x <= y) ? i - x : i - y)] = TargetMachine->V[(x <= y) ? i : x - i];
						}
						TargetMachine->PC += 2;
						break;
					}
					case 3:
					{
						for (unsigned char i = ((x <= y) ? x : y); i <= ((x <= y) ? y : x); ++i)
						{
							TargetMachine->V[(x <= y) ? i : x - i] = TargetMachine->memory[TargetMachine->I + ((x <= y) ? i - x : i - y)];
						}
						TargetMachine->PC += 2;
						break;
					}
					default:
					{
						valid = false;
						break;
					}
				}
				if (!valid)
				{
					TargetMachine->PauseProgram(true);
					TargetMachine->error_state = MachineError::InvalidInstruction;
				}
				break;
			}
			case 0x6:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char value = (operand & 0x0FF);
				TargetMachine->V[x] = value;
				TargetMachine->PC += 2;
				break;
			}
			case 0x7:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char value = (operand & 0x0FF);
				TargetMachine->V[x] += value;
				TargetMachine->PC += 2;
				break;
			}
			case 0x8:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char y = ((operand & 0x0F0) >> 4);
				unsigned char z = (operand & 0x00F);
				bool valid = true;
				switch (z)
				{
					case 0x0:
					{
						TargetMachine->V[x] = TargetMachine->V[y];
						break;
					}
					case 0x1:
					{
						TargetMachine->V[x] |= TargetMachine->V[y];
						break;
					}
					case 0x2:
					{
						TargetMachine->V[x] &= TargetMachine->V[y];
						break;
					}
					case 0x3:
					{
						TargetMachine->V[x] ^= TargetMachine->V[y];
						break;
					}
					case 0x4:
					{
						unsigned char tmp = TargetMachine->V[x] + TargetMachine->V[y];
						if (tmp < TargetMachine->V[x])
						{
							TargetMachine->V[0xF] = 0x01;
						}
						else
						{
							TargetMachine->V[0xF] = 0x00;
						}
						TargetMachine->V[x] = tmp;
						break;
					}
					case 0x5:
					{
						unsigned char tmp = TargetMachine->V[x] - TargetMachine->V[y];
						if (tmp > TargetMachine->V[x])
						{
							TargetMachine->V[0xF] = 0x00;
						}
						else
						{
							TargetMachine->V[0xF] = 0x01;
						}
						TargetMachine->V[x] = tmp;
						break;
					}
					case 0x6:
					{
						TargetMachine->V[0xF] = (TargetMachine->V[y] & 0x01);
						TargetMachine->V[x] = (TargetMachine->V[y] >> 1);
						break;
					}
					case 0x7:
					{
						unsigned char tmp = TargetMachine->V[y] - TargetMachine->V[x];
						if (tmp > TargetMachine->V[y])
						{
							TargetMachine->V[0xF] = 0x00;
						}
						else
						{
							TargetMachine->V[0xF] = 0x01;
						}
						TargetMachine->V[x] = tmp;
						break;
					}
					case 0x8:
					{
						TargetMachine->V[x] = std::rotr(TargetMachine->V[y], 1);
						break;
					}
					case 0x9:
					{
						TargetMachine->V[x] = std::rotl(TargetMachine->V[y], 1);
						break;
					}
					case 0xA:
					{
						TargetMachine->V[0xF] = (TargetMachine->V[x] & TargetMachine->V[y]) ? 1 : 0;
						break;
					}
					case 0xB:
					{
						TargetMachine->V[x] = ~(TargetMachine->V[y]);
						break;
					}
					case 0xE:
					{
						TargetMachine->V[0xF] = ((TargetMachine->V[y] & 0x80) >> 7);
						TargetMachine->V[x] = (TargetMachine->V[y] << 1);
						break;
					}
					default:
					{
						valid = false;
						break;
					}
				}
				if (valid)
				{
					TargetMachine->PC += 2;
				}
				else
				{
					TargetMachine->PauseProgram(true);
					TargetMachine->error_state = MachineError::InvalidInstruction;
				}
				break;
			}
			case 0x9:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char y = ((operand & 0x0F0) >> 4);
				unsigned char z = (operand & 0x00F);
				if (z != 0)
				{
					TargetMachine->PauseProgram(true);
					TargetMachine->error_state = MachineError::InvalidInstruction;
					break;
				}
				TargetMachine->PC += 2;
				if (TargetMachine->V[x] != TargetMachine->V[y])
				{
					unsigned char next_opcode = (TargetMachine->memory[TargetMachine->PC] >> 4);
					unsigned short next_operand = ((TargetMachine->memory[TargetMachine->PC] & 0x0F) << 8) | (TargetMachine->memory[static_cast<unsigned short>(TargetMachine->PC + 1)] & 0xFFF);
					while (next_opcode == 0xF && (((next_operand & 0x0F0) >> 4) == 0xB))
					{
						TargetMachine->PC += 2;
						next_opcode = (TargetMachine->memory[TargetMachine->PC] >> 4);
						next_operand = ((TargetMachine->memory[TargetMachine->PC] & 0x0F) << 8) | (TargetMachine->memory[static_cast<unsigned short>(TargetMachine->PC + 1)]);
					}
					TargetMachine->PC += 2;
				}
				break;
			}
			case 0xA:
			{
				TargetMachine->I = ((TargetMachine->prefix_flags & 0x01) ? (TargetMachine->address_nibble_store << 8) : 0) | (operand & 0xFFF);
				if (TargetMachine->prefix_flags & 0x01)
				{
					TargetMachine->prefix_flags &= ~(0x01);
					TargetMachine->address_nibble_store = 0x00;
				}
				TargetMachine->PC += 2;
				break;
			}
			case 0xB:
			{
				TargetMachine->PC = (((TargetMachine->prefix_flags & 0x01) ? (TargetMachine->address_nibble_store << 8) : 0) | (operand & 0xFFF)) + TargetMachine->V[(TargetMachine->prefix_flags & 0x02) ? TargetMachine->register_store : 0];
				if (TargetMachine->prefix_flags & 0x01)
				{
					TargetMachine->prefix_flags &= ~(0x01);
					TargetMachine->address_nibble_store = 0x00;
				}
				if (TargetMachine->prefix_flags & 0x02)
				{
					TargetMachine->prefix_flags &= ~(0x02);
					TargetMachine->register_store = 0x0;
				}
				break;
			}
			case 0xC:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char mask = (operand & 0x0FF);
				TargetMachine->V[x] = (TargetMachine->rng_distrib(TargetMachine->rng_engine) & mask);
				TargetMachine->PC += 2;
				break;
			}
			case 0xD:
			{
				unsigned char x = TargetMachine->V[((operand & 0xF00) >> 8)];
				unsigned char y = TargetMachine->V[((operand & 0x0F0) >> 4)];
				unsigned char width = 8;
				unsigned char height = (operand & 0x00F);
				unsigned char scale_factor = (TargetMachine->CurrentResolutionMode == ResolutionMode::HiRes) ? 1 : 2;
				if (!height && TargetMachine->CurrentResolutionMode == ResolutionMode::HiRes)
				{
					width = 16;
					height = 16;
				}
				unsigned char *sprite = &TargetMachine->memory[TargetMachine->I];
				bool pixels_changed_to_unset = false;
				for (unsigned int py = 0; py < height; ++py)
				{
					unsigned char b_offset = 0;
					for (unsigned int px = 0; px < width; ++px)
					{
						unsigned int y_offset = py * (width / 8);
						unsigned char pixel = ((sprite[y_offset + (px / 8)] & (0x80 >> b_offset++)) >> (7 - (px % 8)));
						if (b_offset == 8)
						{
							b_offset = 0;
						}
						unsigned int display_addr = (((y + py) * scale_factor % 64) * 128) + ((x + px) * scale_factor % 128);
						switch (TargetMachine->CurrentResolutionMode)
						{
							case ResolutionMode::LoRes:
							{
								for (unsigned char dy = 0; dy < 2; ++dy)
								{
									for (unsigned char dx = 0; dx < 2; ++dx)
									{
										unsigned char tmp = TargetMachine->display[display_addr + (dy * 128) + dx];
										TargetMachine->display[display_addr + (dy * 128) + dx] ^= pixel;
										if (TargetMachine->display[display_addr + (dy * 128) + dx] != tmp && TargetMachine->display[display_addr + (dy * 128) + dx] == 0 && !pixels_changed_to_unset)
										{
											pixels_changed_to_unset = true;
										}
									}
								}
								break;
							}
							case ResolutionMode::HiRes:
							{
								unsigned char tmp = TargetMachine->display[display_addr];
								TargetMachine->display[display_addr] ^= pixel;
								if (TargetMachine->display[display_addr] != tmp && TargetMachine->display[display_addr] == 0 && !pixels_changed_to_unset)
								{
									pixels_changed_to_unset = true;
								}
								break;
							}
						}
					}
				}
				if (pixels_changed_to_unset)
				{
					TargetMachine->V[0xF] = 0x01;
				}
				else
				{
					TargetMachine->V[0xF] = 0x00;
				}
				TargetMachine->PC += 2;
				break;
			}
			case 0xE:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char opcode2 = (operand & 0x0FF);
				bool valid = true;
				switch (opcode2)
				{
					case 0x9E:
					{
						TargetMachine->PC += 2;
						if (TargetMachine->key_status[(TargetMachine->V[x] & 0xF)] == 1)
						{
							unsigned char next_opcode = (TargetMachine->memory[TargetMachine->PC] >> 4);
							unsigned short next_operand = ((TargetMachine->memory[TargetMachine->PC] & 0x0F) << 8) | (TargetMachine->memory[static_cast<unsigned short>(TargetMachine->PC + 1)] & 0xFFF);
							while (next_opcode == 0xF && (((next_operand & 0x0F0) >> 4) == 0xB))
							{
								TargetMachine->PC += 2;
								next_opcode = (TargetMachine->memory[TargetMachine->PC] >> 4);
								next_operand = ((TargetMachine->memory[TargetMachine->PC] & 0x0F) << 8) | (TargetMachine->memory[static_cast<unsigned short>(TargetMachine->PC + 1)]);
							}
							TargetMachine->PC += 2;
						}
						break;
					}
					case 0xA1:
					{
						TargetMachine->PC += 2;
						if (TargetMachine->key_status[(TargetMachine->V[x] & 0xF)] == 0)
						{
							unsigned char next_opcode = (TargetMachine->memory[TargetMachine->PC] >> 4);
							unsigned short next_operand = ((TargetMachine->memory[TargetMachine->PC] & 0x0F) << 8) | (TargetMachine->memory[static_cast<unsigned short>(TargetMachine->PC + 1)] & 0xFFF);
							while (next_opcode == 0xF && (((next_operand & 0x0F0) >> 4) == 0xB))
							{
								TargetMachine->PC += 2;
								next_opcode = (TargetMachine->memory[TargetMachine->PC] >> 4);
								next_operand = ((TargetMachine->memory[TargetMachine->PC] & 0x0F) << 8) | (TargetMachine->memory[static_cast<unsigned short>(TargetMachine->PC + 1)]);
							}
							TargetMachine->PC += 2;
						}
						break;
					}
					default:
					{
						valid = false;
						break;
					}
				}
				if (!valid)
				{
					TargetMachine->PauseProgram(true);
					TargetMachine->error_state = MachineError::InvalidInstruction;
				}
				break;
			}
			case 0xF:
			{
				unsigned char x = ((operand & 0xF00) >> 8);
				unsigned char opcode2 = (operand & 0x0FF);
				bool valid = true;
				switch (opcode2)
				{
					case 0x07:
					{
						TargetMachine->V[x] = TargetMachine->delay_timer;
						TargetMachine->PC += 2;
						break;
					}
					case 0x0A:
					{
						for (unsigned char i = 0x00; i < 0x10; ++i)
						{
							if (TargetMachine->key_status[i] == 1)
							{
								TargetMachine->V[x] = i;
								TargetMachine->PC += 2;
								break;
							}
						}
						break;
					}
					case 0x15:
					{
						TargetMachine->SetDelayTimer(TargetMachine->V[x]);
						TargetMachine->PC += 2;
						break;
					}
					case 0x18:
					{
						TargetMachine->SetSoundTimer(TargetMachine->V[x]);
						TargetMachine->PC += 2;
						break;
					}
					case 0x1E:
					{
						TargetMachine->I += TargetMachine->V[x];
						TargetMachine->PC += 2;
						break;
					}
					case 0x20:
					{
						TargetMachine->PC = ((TargetMachine->memory[static_cast<unsigned short>(TargetMachine->I + TargetMachine->V[x])] << 8) | (TargetMachine->memory[static_cast<unsigned short>(TargetMachine->I + TargetMachine->V[x] + 1)]));
						break;
					}
					case 0x21:
					{
						if (TargetMachine->SP < sizeof(TargetMachine->stack))
						{
							TargetMachine->stack[TargetMachine->SP++] = TargetMachine->PC + 2;
							TargetMachine->PC = ((TargetMachine->memory[static_cast<unsigned short>(TargetMachine->I + TargetMachine->V[x])] << 8) | (TargetMachine->memory[static_cast<unsigned short>(TargetMachine->I + TargetMachine->V[x] + 1)]));
						}
						else
						{
							TargetMachine->PauseProgram(true);
							TargetMachine->error_state = MachineError::StackOverflow;
						}
						break;
					}
					case 0x29:
					{
						switch (TargetMachine->V[x])
						{
							case 0x0:
							{
								TargetMachine->I = 0x0000;
								break;
							}
							case 0x1:
							{
								TargetMachine->I = 0x0005;
								break;
							}
							case 0x2:
							{
								TargetMachine->I = 0x000A;
								break;
							}
							case 0x3:
							{
								TargetMachine->I = 0x000F;
								break;
							}
							case 0x4:
							{
								TargetMachine->I = 0x0014;
								break;
							}
							case 0x5:
							{
								TargetMachine->I = 0x0019;
								break;
							}
							case 0x6:
							{
								TargetMachine->I = 0x001E;
								break;
							}
							case 0x7:
							{
								TargetMachine->I = 0x0023;
								break;
							}
							case 0x8:
							{
								TargetMachine->I = 0x0028;
								break;
							}
							case 0x9:
							{
								TargetMachine->I = 0x002D;
								break;
							}
							case 0xA:
							{
								TargetMachine->I = 0x0032;
								break;
							}
							case 0xB:
							{
								TargetMachine->I = 0x0037;
								break;
							}
							case 0xC:
							{
								TargetMachine->I = 0x003C;
								break;
							}
							case 0xD:
							{
								TargetMachine->I = 0x0041;
								break;
							}
							case 0xE:
							{
								TargetMachine->I = 0x0046;
								break;
							}
							case 0xF:
							{
								TargetMachine->I = 0x004B;
								break;
							}
						}
						TargetMachine->PC += 2;
						break;
					}
					case 0x30:
					{
						switch (TargetMachine->V[x])
						{
							case 0x0:
							{
								TargetMachine->I = 0x0050;
								break;
							}
							case 0x1:
							{
								TargetMachine->I = 0x005A;
								break;
							}
							case 0x2:
							{
								TargetMachine->I = 0x0064;
								break;
							}
							case 0x3:
							{
								TargetMachine->I = 0x006E;
								break;
							}
							case 0x4:
							{
								TargetMachine->I = 0x0078;
								break;
							}
							case 0x5:
							{
								TargetMachine->I = 0x0082;
								break;
							}
							case 0x6:
							{
								TargetMachine->I = 0x008C;
								break;
							}
							case 0x7:
							{
								TargetMachine->I = 0x0096;
								break;
							}
							case 0x8:
							{
								TargetMachine->I = 0x00A0;
								break;
							}
							case 0x9:
							{
								TargetMachine->I = 0x00AA;;
								break;
							}
						}
						TargetMachine->PC += 2;
						break;
					}
					case 0x33:
					{
						unsigned char tmp = TargetMachine->V[x];
						unsigned char digit[3];
						for (unsigned char i = 0; i < 3; ++i)
						{
							digit[i] = tmp % 10;
							tmp /= 10;
						}
						for (unsigned char i = 0; i < 3; ++i)
						{
							TargetMachine->memory[static_cast<unsigned short>(TargetMachine->I + i)] = digit[2 - i];
						}
						TargetMachine->PC += 2;
						break;
					}
					case 0x55:
					{
						for (unsigned char i = 0; i <= x; ++i)
						{
							TargetMachine->memory[static_cast<unsigned short>(TargetMachine->I + i)] = TargetMachine->V[i];
						}
						TargetMachine->I += x + 1;
						TargetMachine->PC += 2;
						break;
					}
					case 0x65:
					{
						for (unsigned char i = 0; i <= x; ++i)
						{
							TargetMachine->V[i] = TargetMachine->memory[static_cast<unsigned short>(TargetMachine->I + i)];
						}
						TargetMachine->I += x + 1;
						TargetMachine->PC += 2;
						break;
					}
					case 0x75:
					{
						unsigned char flag_count = (x > 7) ? 7 : x;
						TargetMachine->rpl_user_flags_file.pubseekoff(0, std::ios::beg);
						TargetMachine->rpl_user_flags_file.sputn(reinterpret_cast<const char *>(TargetMachine->V), flag_count + 1);
						TargetMachine->PC += 2;
						break;
					}
					case 0x85:
					{
						unsigned char flag_count = (x > 7) ? 7 : x;
						TargetMachine->rpl_user_flags_file.pubseekoff(0, std::ios::beg);
						TargetMachine->rpl_user_flags_file.sgetn(reinterpret_cast<char *>(TargetMachine->V), flag_count + 1);
						TargetMachine->PC += 2;
						break;
					}
					case 0xA0:
					{
						TargetMachine->PC += 2;
						break;
					}
					case 0xA1:
					{
						TargetMachine->PC += 2;
						break;
					}
					case 0xA2:
					{
						TargetMachine->I = ((TargetMachine->memory[static_cast<unsigned short>(TargetMachine->I + TargetMachine->V[x])] << 8) | (TargetMachine->memory[static_cast<unsigned short>(TargetMachine->I + TargetMachine->V[x] + 1)]));
						TargetMachine->PC += 2;
						break;
					}
					case 0xB0:
					{
						TargetMachine->prefix_flags |= 0x01;
						TargetMachine->address_nibble_store = x;
						TargetMachine->PC += 2;
						break;
					}
					case 0xB1:
					{
						TargetMachine->prefix_flags |= 0x02;
						TargetMachine->register_store = x;
						TargetMachine->PC += 2;
						break;
					}
					default:
					{
						valid = false;
						break;
					}
				}
				if (!valid)
				{
					TargetMachine->PauseProgram(true);
					TargetMachine->error_state = MachineError::InvalidInstruction;
				}
				break;
			}
		}
	}
}
