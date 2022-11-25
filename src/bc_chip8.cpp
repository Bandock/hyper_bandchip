#include "machine.h"
#if defined(RENDERER_OPENGL21)
#include "renderer_opengl21.h"
#elif defined(RENDERER_OPENGL30)
#include "renderer_opengl30.h"
#elif defined(RENDERER_OPENGLES2)
#include "renderer_opengles2.h"
#elif defined(RENDERER_OPENGLES3)
#include "renderer_opengles3.h"
#endif
#include <iostream>

/*
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
				const CHIP8_BehaviorData *Behavior = std::get_if<CHIP8_BehaviorData>(&TargetMachine->behavior_data);
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
						if (Behavior->VIP_VF_Reset)
						{
							TargetMachine->V[0xF] = 0x00;
						}
						break;
					}
					case 0x2:
					{
						TargetMachine->V[x] &= TargetMachine->V[y];
						if (Behavior->VIP_VF_Reset)
						{
							TargetMachine->V[0xF] = 0x00;
						}
						break;
					}
					case 0x3:
					{
						TargetMachine->V[x] ^= TargetMachine->V[y];
						if (Behavior->VIP_VF_Reset)
						{
							TargetMachine->V[0xF] = 0x00;
						}
						break;
					}
					case 0x4:
					{
						unsigned char tmp = TargetMachine->V[x] + TargetMachine->V[y];
						bool overflow = (tmp < TargetMachine->V[x]);
						TargetMachine->V[x] = tmp;
						if (overflow)
						{
							TargetMachine->V[0xF] = 0x01;
						}
						else
						{
							TargetMachine->V[0xF] = 0x00;
						}
						break;
					}
					case 0x5:
					{
						unsigned char tmp = TargetMachine->V[x] - TargetMachine->V[y];
						bool underflow = (tmp > TargetMachine->V[x]);
						TargetMachine->V[x] = tmp;
						if (underflow)
						{
							TargetMachine->V[0xF] = 0x00;
						}
						else
						{
							TargetMachine->V[0xF] = 0x01;
						}
						break;
					}
					case 0x6:
					{
						unsigned char tmp = ((Behavior->CHIP48_Shift ? TargetMachine->V[x] : TargetMachine->V[y]) & 0x01);
						TargetMachine->V[x] = ((Behavior->CHIP48_Shift ? TargetMachine->V[x] : TargetMachine->V[y]) >> 1);
						TargetMachine->V[0xF] = tmp;
						break;
					}
					case 0x7:
					{
						unsigned char tmp = TargetMachine->V[y] - TargetMachine->V[x];
						bool underflow = (tmp > TargetMachine->V[y]);
						TargetMachine->V[x] = tmp;
						if (underflow)
						{
							TargetMachine->V[0xF] = 0x00;
						}
						else
						{
							TargetMachine->V[0xF] = 0x01;
						}
						break;
					}
					case 0xE:
					{
						const CHIP8_BehaviorData *Behavior = std::get_if<CHIP8_BehaviorData>(&TargetMachine->behavior_data);
						unsigned char tmp = (((Behavior->CHIP48_Shift ? TargetMachine->V[x] : TargetMachine->V[y]) & 0x80) >> 7);
						TargetMachine->V[x] = ((Behavior->CHIP48_Shift ? TargetMachine->V[x] : TargetMachine->V[y]) << 1);
						TargetMachine->V[0xF] = tmp;
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
				const CHIP8_BehaviorData *Behavior = std::get_if<CHIP8_BehaviorData>(&TargetMachine->behavior_data);
				if (Behavior->VIP_Display_Interrupt && !TargetMachine->display_interrupt && TargetMachine->sync)
				{
					break;
				}
				unsigned char x = (TargetMachine->V[((operand & 0xF00) >> 8)] & 0x3F);
				unsigned char y = (TargetMachine->V[((operand & 0x0F0) >> 4)] & 0x1F);
				unsigned char bytes = (operand & 0x00F);
				unsigned char *sprite = &TargetMachine->memory[TargetMachine->I];
				bool pixels_changed_to_unset = false;
				for (unsigned int py = 0; py < bytes; ++py)
				{
					if (Behavior->VIP_Clipping && y + py >= 32)
					{
						break;
					}
					unsigned char b_offset = 0;
					for (unsigned int px = 0; px < 8; ++px)
					{
						if (Behavior->VIP_Clipping && x + px >= 64)
						{
							break;
						}
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
				if (Behavior->VIP_Display_Interrupt && TargetMachine->display_interrupt && TargetMachine->sync)
				{
					TargetMachine->display_interrupt = false;
					TargetMachine->DisplayRenderer->WriteToDisplay(TargetMachine->display, TargetMachine->display_width, TargetMachine->display_height);
					TargetMachine->DisplayRenderer->Render();
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
						if (!TargetMachine->wait_for_key_release)
						{
							for (unsigned char i = 0x00; i < 0x10; ++i)
							{
								if (TargetMachine->key_status[i] == 1)
								{
									TargetMachine->wait_for_key_release = true;
									TargetMachine->key_pressed = i;
									break;
								}
							}
						}
						else
						{
							if (TargetMachine->key_status[TargetMachine->key_pressed] == 0)
							{
								TargetMachine->wait_for_key_release = false;
								TargetMachine->V[x] = TargetMachine->key_pressed;
								TargetMachine->PC += 2;
								if (TargetMachine->PC > 0xFFF)
								{
									TargetMachine->PC &= 0xFFF;
								}
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
						switch (TargetMachine->V[x] & 0xF)
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
*/

Hyper_BandCHIP::BandCHIP::CHIP8_Machine::CHIP8_Machine(unsigned int cycles_per_second, Renderer *DisplayRenderer) : DisplayRenderer(DisplayRenderer), display_interrupt(false), delay_timer(0), sound_timer(0), PC(0), I(0), SP(0), key_pressed(0), rng_engine(system_clock::now().time_since_epoch().count()), rng_distrib(0, 255), cycle_accumulator(0.0), pause(true), operational(true), wait_for_key_release(false), error_state(MachineError::NoError)
{
	memset(V.data(), 0, V.size());
	memset(stack.data(), 0, stack.size());
	// memory.resize(4096);
	memset(memory.data(), 0, memory.size());
	memset(display.data(), 0, display.size());
	memset(key_status, 0x00, 0x10);
	SetCyclesPerSecond(*this, cycles_per_second);
	cycle_rate = (cycles_per_second > 0) ? 1.0 / static_cast<double>(cycles_per_second) : 0.0;
}

Hyper_BandCHIP::BandCHIP::CHIP8_Machine::~CHIP8_Machine()
{
}

void Hyper_BandCHIP::BandCHIP::CHIP8_Machine::ExecuteInstructions()
{
	duration<double> delta_time = current_tp - cycle_tp;
	if (delta_time.count() > 0.25)
	{
		delta_time = duration<double>(0.25);
	}
	cycle_accumulator += delta_time.count();
	cycle_tp = current_tp;
	if (cycles_per_second == 0)
	{
		if (cycle_accumulator == 0.0)
		{
			return;
		}
		uint32_t new_cycles_per_second_u32 = 30000.0 / cycle_accumulator;
		if (new_cycles_per_second_u32 >= 60)
		{
			uint32_t excess_cycles = new_cycles_per_second_u32 % 60;
			if (excess_cycles)
			{
				new_cycles_per_second_u32 -= excess_cycles;
			}
			cycle_rate = 1.0 / new_cycles_per_second_u32;
			uint32_t previous_cycles_per_frame = RefreshSync.cycles_per_frame;
			RefreshSync.cycles_per_frame = new_cycles_per_second_u32 / 60;
			RefreshSync.cycle_counter = static_cast<double>(RefreshSync.cycle_counter) * (static_cast<double>(RefreshSync.cycles_per_frame) / static_cast<double>(previous_cycles_per_frame));
			previous_cycles_per_frame = DelayTimerSync.cycles_per_frame;
			DelayTimerSync.cycles_per_frame = new_cycles_per_second_u32 / 60;
			DelayTimerSync.cycle_counter = static_cast<double>(DelayTimerSync.cycle_counter) * (static_cast<double>(DelayTimerSync.cycles_per_frame) / static_cast<double>(previous_cycles_per_frame));
			previous_cycles_per_frame = SoundTimerSync.cycles_per_frame;
			SoundTimerSync.cycles_per_frame = new_cycles_per_second_u32 / 60;
			SoundTimerSync.cycle_counter = static_cast<double>(SoundTimerSync.cycle_counter) * (static_cast<double>(SoundTimerSync.cycles_per_frame) / static_cast<double>(previous_cycles_per_frame));
		}
		else
		{
			return;
		}
	}
	uint32_t cycles = (cycle_accumulator / cycle_rate);
	cycle_accumulator -= static_cast<double>(cycles) * cycle_rate;
	for ( ; cycles && !pause; --cycles)
	{
		SyncToCycle(*this);
		uint8_t opcode = 0x0;
		uint16_t operand = 0x000;
		if (!(PC % 2))
		{
			uint16_t instruction = *(reinterpret_cast<uint16_t *>(&memory[PC]));
			opcode = std::endian::native != std::endian::big ? ((instruction & 0xF0) >> 4) : (instruction >> 12);
			operand = std::endian::native != std::endian::big ? (((instruction & 0x0F) << 8) | (instruction >> 8)) : (instruction & 0xFFF);
		}
		else
		{
			opcode = (memory[PC] >> 4);
			operand = ((memory[PC] & 0xF) << 8) | (memory[((PC + 1) & 0xFFF)]);
		}
		PC += 2;
		if (PC > 0xFFF)
		{
			PC &= 0xFFF;
		}
		switch (opcode)
		{
			case 0x0:
			{
				switch (operand)
				{
					case 0xE0:
					{
						InitializeVideo(*this);
						break;
					}
					case 0xEE:
					{
						if (SP > 0)
						{
							PC = stack[--SP];
							stack[SP] = 0;
						}
						else
						{
							PC -= 2;
							if (PC > 0xFFF)
							{
								PC &= 0xFFF;
							}
							PauseProgram(*this, true);
							error_state = MachineError::StackUnderflow;
						}
						break;
					}
					default:
					{
						PC -= 2;
						if (PC > 0xFFF)
						{
							PC &= 0xFFF;
						}
						PauseProgram(*this, true);
						error_state = MachineError::MachineInstructionsUnsupported;
						break;
					}
				}
				break;
			}
			case 0x1:
			{
				PC = operand;
				break;
			}
			case 0x2:
			{
				if (SP < (sizeof(stack) / sizeof(uint16_t)))
				{
					stack[SP++] = PC;
					PC = operand;
				}
				else
				{
					PC -= 2;
					if (PC > 0xFFF)
					{
						PC &= 0xFFF;
					}
					PauseProgram(*this, true);
					error_state = MachineError::StackOverflow;
				}
				break;
			}
			case 0x3:
			{
				uint8_t x = ((operand & 0xF00) >> 8);
				uint8_t value = (operand & 0x0FF);
				if (V[x] == value)
				{
					PC += 2;
					if (PC > 0xFFF)
					{
						PC &= 0xFFF;
					}
				}
				break;
			}
			case 0x4:
			{
				uint8_t x = ((operand & 0xF00) >> 8);
				uint8_t value = (operand & 0x0FF);
				if (V[x] != value)
				{
					PC += 2;
					if (PC > 0xFFF)
					{
						PC &= 0xFFF;
					}
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
					PC -= 2;
					if (PC > 0xFFF)
					{
						PC &= 0xFFF;
					}
					PauseProgram(*this, true);
					error_state = MachineError::InvalidInstruction;
					break;
				}
				if (V[x] == V[y])
				{
					PC += 2;
					if (PC > 0xFFF)
					{
						PC &= 0xFFF;
					}
				}
				break;
			}
			case 0x6:
			{
				uint8_t x = ((operand & 0xF00) >> 8);
				uint8_t value = (operand & 0x0FF);
				V[x] = value;
				break;
			}
			case 0x7:
			{
				uint8_t x = ((operand & 0xF00) >> 8);
				uint8_t value = (operand & 0x0FF);
				V[x] += value;
				break;
			}
			case 0x8:
			{
				uint8_t x = ((operand & 0xF00) >> 8);
				uint8_t y = ((operand & 0x0F0) >> 4);
				uint8_t z = (operand & 0x00F);
				switch (z)
				{
					case 0x0:
					{
						V[x] = V[y];
						break;
					}
					case 0x1:
					{
						V[x] |= V[y];
						if (behavior_data.VIP_VF_Reset)
						{
							V[0xF] = 0x00;
						}
						break;
					}
					case 0x2:
					{
						V[x] &= V[y];
						if (behavior_data.VIP_VF_Reset)
						{
							V[0xF] = 0x00;
						}
						break;
					}
					case 0x3:
					{
						V[x] ^= V[y];
						if (behavior_data.VIP_VF_Reset)
						{
							V[0xF] = 0x00;
						}
						break;
					}
					case 0x4:
					{
						uint8_t tmp = V[x] + V[y];
						bool overflow = (tmp < V[x]);
						V[x] = tmp;
						V[0xF] = overflow ? 0x01 : 0x00;
						break;
					}
					case 0x5:
					{
						uint8_t tmp = V[x] - V[y];
						bool underflow = (tmp > V[x]);
						V[x] = tmp;
						V[0xF] = underflow ? 0x00 : 0x01;
						break;
					}
					case 0x6:
					{
						uint8_t tmp = ((behavior_data.CHIP48_Shift ? V[x] : V[y]) & 0x01);
						V[x] = ((behavior_data.CHIP48_Shift ? V[x] : V[y]) >> 1);
						V[0xF] = tmp;
						break;
					}
					case 0x7:
					{
						uint8_t tmp = V[y] - V[x];
						bool underflow = (tmp > V[y]);
						V[x] = tmp;
						V[0xF] = underflow ? 0x00 : 0x01;
						break;
					}
					case 0xE:
					{
						uint8_t tmp = (((behavior_data.CHIP48_Shift ? V[x] : V[y]) & 0x80) >> 7);
						V[x] = ((behavior_data.CHIP48_Shift ? V[x] : V[y]) << 1);
						V[0xF] = tmp;
						break;
					}
					default:
					{
						PC -= 2;
						if (PC > 0xFFF)
						{
							PC &= 0xFFF;
						}
						PauseProgram(*this, true);
						error_state = MachineError::InvalidInstruction;
						break;
					}
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
					PC -= 2;
					if (PC > 0xFFF)
					{
						PC &= 0xFFF;
					}
					PauseProgram(*this, true);
					error_state = MachineError::InvalidInstruction;
					break;
				}
				if (V[x] != V[y])
				{
					PC += 2;
					if (PC > 0xFFF)
					{
						PC &= 0xFFF;
					}
				}
				break;
			}
			case 0xA:
			{
				I = operand;
				break;
			}
			case 0xB:
			{
				PC = operand + V[0];
				if (PC > 0xFFF)
				{
					PC &= 0xFFF;
				}
				break;
			}
			case 0xC:
			{
				uint8_t x = ((operand & 0xF00) >> 8);
				uint8_t mask = (operand & 0x0FF);
				V[x] = (rng_distrib(rng_engine) & mask);
				break;
			}
			case 0xD:
			{
				if (behavior_data.VIP_Display_Interrupt && !display_interrupt)
				{
					PC -= 2;
					break;
				}
				uint8_t x = (V[((operand & 0xF00) >> 8)] & 0x3F);
				uint8_t y = (V[((operand & 0x0F0) >> 4)] & 0x1F);
				uint8_t bytes = (operand & 0x00F);
				uint8_t *sprite = &memory[I];
				bool pixels_changed_to_unset = false;
				for (uint32_t py = 0; py < bytes; ++py)
				{
					if (behavior_data.VIP_Clipping && y + py >= 32)
					{
						break;
					}
					uint8_t b_offset = 0;
					for (uint32_t px = 0; px < 8; ++px)
					{
						if (behavior_data.VIP_Clipping && x + px >= 64)
						{
							break;
						}
						uint8_t pixel = ((sprite[py] & (0x80 >> b_offset++)) >> (7 - px));
						uint32_t display_addr = (((y + py) % 32) * 64) + ((x + px) % 64);
						uint8_t tmp = display[display_addr];
						display[display_addr] ^= pixel;
						if (display[display_addr] != tmp && display[display_addr] == 0 && !pixels_changed_to_unset)
						{
							pixels_changed_to_unset = true;
						}
					}
				}
				V[0xF] = pixels_changed_to_unset ? 0x01 : 0x00;
				if (behavior_data.VIP_Display_Interrupt && display_interrupt)
				{
					display_interrupt = false;
					DisplayRenderer->WriteToDisplay(display.data(), display_width, display_height);
					DisplayRenderer->Render();
				}
				break;
			}
			case 0xE:
			{
				uint8_t x = ((operand & 0xF00) >> 8);
				uint8_t opcode2 = (operand & 0x0FF);
				switch (opcode2)
				{
					case 0x9E:
					{
						if (key_status[(V[x] & 0xF)] == 1)
						{
							PC += 2;
							if (PC > 0xFFF)
							{
								PC &= 0xFFF;
							}
						}
						break;
					}
					case 0xA1:
					{
						if (key_status[(V[x] & 0xF)] == 0)
						{
							PC += 2;
							if (PC > 0xFFF)
							{
								PC &= 0xFFF;
							}
						}
						break;
					}
					default:
					{
						PC -= 2;
						if (PC > 0xFFF)
						{
							PC &= 0xFFF;
						}
						PauseProgram(*this, true);
						error_state = MachineError::InvalidInstruction;
						break;
					}
				}
				break;
			}
			case 0xF:
			{
				uint8_t x = ((operand & 0xF00) >> 8);
				uint8_t opcode2 = (operand & 0x0FF);
				switch (opcode2)
				{
					case 0x07:
					{
						V[x] = delay_timer;
						break;
					}
					case 0x0A:
					{
						if (!wait_for_key_release)
						{
							PC -= 2;
							if (PC > 0xFFF)
							{
								PC &= 0xFFF;
							}
							for (uint8_t i = 0x00; i < 0x10; ++i)
							{
								if (key_status[i] == 1)
								{
									wait_for_key_release = true;
									key_pressed = i;
									break;
								}
							}
						}
						else
						{
							if (key_status[key_pressed] == 0)
							{
								wait_for_key_release = false;
								V[x] = key_pressed;
							}
							else
							{
								PC -= 2;
								if (PC > 0xFFF)
								{
									PC &= 0xFFF;
								}
							}
						}
						break;
					}
					case 0x15:
					{
						SetDelayTimer(*this, V[x]);
						break;
					}
					case 0x18:
					{
						SetSoundTimer(*this, V[x]);
						break;
					}
					case 0x1E:
					{
						I += V[x];
						break;
					}
					case 0x29:
					{
						switch (V[x] & 0xF)
						{
							case 0x0:
							{
								I = 0x000;
								break;
							}
							case 0x1:
							{
								I = 0x005;
								break;
							}
							case 0x2:
							{
								I = 0x00A;
								break;
							}
							case 0x3:
							{
								I = 0x00F;
								break;
							}
							case 0x4:
							{
								I = 0x014;
								break;
							}
							case 0x5:
							{
								I = 0x019;
								break;
							}
							case 0x6:
							{
								I = 0x01E;
								break;
							}
							case 0x7:
							{
								I = 0x023;
								break;
							}
							case 0x8:
							{
								I = 0x028;
								break;
							}
							case 0x9:
							{
								I = 0x02D;
								break;
							}
							case 0xA:
							{
								I = 0x032;
								break;
							}
							case 0xB:
							{
								I = 0x037;
								break;
							}
							case 0xC:
							{
								I = 0x03C;
								break;
							}
							case 0xD:
							{
								I = 0x041;
								break;
							}
							case 0xE:
							{
								I = 0x046;
								break;
							}
							case 0xF:
							{
								I = 0x04B;
								break;
							}
						}
						break;
					}
					case 0x33:
					{
						uint8_t tmp = V[x];
						uint8_t digit[3];
						for (uint8_t i = 0; i < 3; ++i)
						{
							digit[i] = tmp % 10;
							tmp /= 10;
						}
						for (uint8_t i = 0; i < 3; ++i)
						{
							memory[((I + i) & 0xFFF)] = digit[2 - i];
						}
						break;
					}
					case 0x55:
					{
						for (uint8_t i = 0; i <= x; ++i)
						{
							memory[((I + i) & 0xFFF)] = V[i];
						}
						I += x + (behavior_data.CHIP48_LoadStore ? 0 : 1);
						break;
					}
					case 0x65:
					{
						for (uint8_t i = 0; i <= x; ++i)
						{
							V[i] = memory[((I + i) & 0xFFF)];
						}
						I += x + (behavior_data.CHIP48_LoadStore ? 0 : 1);
						break;
					}
				}
				break;
			}
		}
		if (behavior_data.VIP_Display_Interrupt)
		{
			if (display_interrupt)
			{
				display_interrupt = false;
				DisplayRender(*this);
			}
		}
	}
}

Hyper_BandCHIP::BandCHIP::Superscalar_CHIP8_Machine::Superscalar_CHIP8_Machine(unsigned int cycles_per_second, Renderer *DisplayRenderer) : DisplayRenderer(DisplayRenderer), display_interrupt(false), cycles_per_second(cycles_per_second), delay_timer(0), sound_timer(0), PC(0), I(0), SP(0), key_pressed(0), rng_engine(system_clock::now().time_since_epoch().count()), rng_distrib(0, 255), cycle_accumulator(0.0), pause(true), operational(true), wait_for_key_release(false), instruction_fetch(false), instructions_to_execute(0), error_state(MachineError::NoError)
{
	memset(V.data(), 0, V.size());
	memset(stack.data(), 0, stack.size());
	memory.resize(4096);
	memset(memory.data(), 0, memory.size());
	memset(display.data(), 0, display.size());
	memset(key_status, 0x00, 0x10);
	cycle_rate = (cycles_per_second > 0) ? 1.0 / static_cast<double>(cycles_per_second) : 0.0;
	for (size_t i = 0; i < execution_unit.size(); ++i)
	{
		execution_unit[i].cycles = 0;
		execution_unit[i].running = true;
		execution_unit[i].ExecutionThread = std::thread(InstructionExecution, this, &execution_unit[i]);
	}
}

Hyper_BandCHIP::BandCHIP::Superscalar_CHIP8_Machine::~Superscalar_CHIP8_Machine()
{
	for (size_t i = 0; i < execution_unit.size(); ++i)
	{
		if (execution_unit[i].running)
		{
			execution_unit[i].running = false;
			execution_unit[i].ExecutionThread.join();
		}
	}
}

void Hyper_BandCHIP::BandCHIP::Superscalar_CHIP8_Machine::ExecuteInstructions()
{
	duration<double> delta_time = current_tp - cycle_tp;
	if (delta_time.count() > 0.25)
	{
		delta_time = duration<double>(0.25);
	}
	cycle_accumulator += delta_time.count();
	cycle_tp = current_tp;
	if (cycles_per_second == 0)
	{
		if (cycle_accumulator == 0.0)
		{
			return;
		}
		uint32_t new_cycles_per_second_u32 = 30000.0 / cycle_accumulator;
		if (new_cycles_per_second_u32 >= 60)
		{
			uint32_t excess_cycles = new_cycles_per_second_u32 % 60;
			if (excess_cycles)
			{
				new_cycles_per_second_u32 -= excess_cycles;
			}
			cycle_rate = 1.0 / new_cycles_per_second_u32;
			uint32_t previous_cycles_per_frame = RefreshSync.cycles_per_frame;
			RefreshSync.cycles_per_frame = new_cycles_per_second_u32 / 60;
			RefreshSync.cycle_counter = static_cast<double>(RefreshSync.cycle_counter) * (static_cast<double>(RefreshSync.cycles_per_frame) / static_cast<double>(previous_cycles_per_frame));
			previous_cycles_per_frame = DelayTimerSync.cycles_per_frame;
			DelayTimerSync.cycles_per_frame = new_cycles_per_second_u32 / 60;
			DelayTimerSync.cycle_counter = static_cast<double>(DelayTimerSync.cycle_counter) * (static_cast<double>(DelayTimerSync.cycles_per_frame) / static_cast<double>(previous_cycles_per_frame));
			previous_cycles_per_frame = SoundTimerSync.cycles_per_frame;
			SoundTimerSync.cycles_per_frame = new_cycles_per_second_u32 / 60;
			SoundTimerSync.cycle_counter = static_cast<double>(SoundTimerSync.cycle_counter) * (static_cast<double>(SoundTimerSync.cycles_per_frame) / static_cast<double>(previous_cycles_per_frame));
		}
		else
		{
			return;
		}
	}
	uint32_t cycles = (cycle_accumulator / cycle_rate);
	cycle_accumulator -= static_cast<double>(cycles) * cycle_rate;
	execution_unit[0].cycles = cycles;
	for ( ; cycles && !pause; --cycles)
	{
		SyncToCycle(*this);
		// ++instructions_to_execute;
		// instruction_fetch = true;
		/*
		while (instructions_to_execute)
		{
			if (render_display)
			{
				render_display = false;
				DisplayRenderer->WriteToDisplay(display.data(), display_width, display_height);
				DisplayRenderer->Render();
			}
			else
			{
				std::this_thread::sleep_for(100ns);
			}
		}
		*/
		if (error_state != MachineError::NoError)
		{
			break;
		}
		if (behavior_data.VIP_Display_Interrupt)
		{
			if (display_interrupt)
			{
				display_interrupt = false;
				DisplayRender(*this);
			}
		}
	}
}

Hyper_BandCHIP::BandCHIP::Pipelined_CHIP8_Machine::Pipelined_CHIP8_Machine(unsigned int cycles_per_second, Renderer *DisplayRenderer)
{
}

Hyper_BandCHIP::BandCHIP::Pipelined_CHIP8_Machine::~Pipelined_CHIP8_Machine()
{
}

void Hyper_BandCHIP::BandCHIP::Superscalar_CHIP8_Machine::InstructionExecution(Superscalar_CHIP8_Machine *machine, ExecutionUnitData *current_execution_unit)
{
	while (current_execution_unit->running)
	{
		if (current_execution_unit->cycles)
		{
			uint32_t cycles = current_execution_unit->cycles;
			for ( ; cycles && !machine->pause; --cycles)
			{
				uint8_t opcode = 0x0;
				uint16_t operand = 0x000;
				if (!(machine->PC % 2))
				{
					uint16_t instruction = *(reinterpret_cast<uint16_t *>(&machine->memory[machine->PC]));
					opcode = std::endian::native != std::endian::big ? ((instruction & 0xF0) >> 4) : (instruction >> 12);
					operand = std::endian::native != std::endian::big ? (((instruction & 0x0F) << 8) | (instruction >> 8)) : (instruction & 0xFFF);
				}
				else
				{
					opcode = (machine->memory[machine->PC] >> 4);
					operand = ((machine->memory[machine->PC] & 0xF) << 8) | (machine->memory[((machine->PC + 1) & 0xFFF)]);
				}
				machine->PC += 2;
				if (machine->PC > 0xFFF)
				{
					machine->PC &= 0xFFF;
				}
				machine->instruction_fetch = false;
				switch (opcode)
				{
					case 0x0:
					{
						switch (operand)
						{
							case 0xE0:
							{
								InitializeVideo(*machine);
								break;
							}
							case 0xEE:
							{

								if (machine->SP > 0)
								{
									machine->PC = machine->stack[--machine->SP];
									machine->stack[machine->SP] = 0;
								}
								else
								{
									machine->PC -= 2;
									if (machine->PC > 0xFFF)
									{
										machine->PC &= 0xFFF;
									}
									PauseProgram(*machine, true);
									machine->error_state = MachineError::StackUnderflow;
								}
								break;
							}
							default:
							{
								machine->PC -= 2;
								if (machine->PC > 0xFFF)
								{
									machine->PC &= 0xFFF;
								}
								PauseProgram(*machine, true);
								machine->error_state = MachineError::MachineInstructionsUnsupported;
								break;
							}
						}
						break;
					}
					case 0x1:
					{
						machine->PC = operand;
						break;
					}
					case 0x2:
					{
						if (machine->SP < (sizeof(machine->stack) / sizeof(uint16_t)))
						{
							machine->stack[machine->SP++] = machine->PC;
							machine->PC = operand;
						}
						else
						{
							machine->PC -= 2;
							if (machine->PC > 0xFFF)
							{
								machine->PC &= 0xFFF;
							}
							PauseProgram(*machine, true);
							machine->error_state = MachineError::StackOverflow;
						}
						break;
					}
					case 0x3:
					{
						uint8_t x = ((operand & 0xF00) >> 8);
						uint8_t value =(operand & 0x0FF);
						if (machine->V[x] == value)
						{
							machine->PC += 2;
							if (machine->PC > 0xFFF)
							{
								machine->PC &= 0xFFF;
							}
						}
						break;
					}
					case 0x4:
					{
						uint8_t x = ((operand & 0xF00) >> 8);
						uint8_t value = (operand & 0x0FF);
						if (machine->V[x] != value)
						{
							machine->PC += 2;
							if (machine->PC > 0xFFF)
							{
								machine->PC &= 0xFFF;
							}
						}
						break;
					}
					case 0x5:
					{
						uint8_t x = ((operand & 0xF00) >> 8);
						uint8_t y = ((operand & 0x0F0) >> 4);
						uint8_t z = (operand & 0x00F);
						if (z != 0)
						{
							machine->PC -= 2;
							if (machine->PC > 0xFFF)
							{
								machine->PC &= 0xFFF;
							}
							PauseProgram(*machine, true);
							machine->error_state = MachineError::InvalidInstruction;
							break;
						}
						if (machine->V[x] == machine->V[y])
						{
							machine->PC += 2;
							if (machine->PC > 0xFFF)
							{
								machine->PC &= 0xFFF;
							}
						}
						break;
					}
					case 0x6:
					{
						uint8_t x = ((operand & 0xF00) >> 8);
						uint8_t value = (operand & 0x0FF);
						machine->V[x] = value;
						break;
					}
					case 0x7:
					{
						uint8_t x = ((operand & 0xF00) >> 8);
						uint8_t value = (operand & 0x0FF);
						machine->V[x] += value;
						break;
					}
					case 0x8:
					{
						uint8_t x = ((operand & 0xF00) >> 8);
						uint8_t y = ((operand & 0x0F0) >> 4);
						uint8_t z = (operand & 0x00F);
						switch (z)
						{
							case 0x0:
							{
								machine->V[x] = machine->V[y];
								break;
							}
							case 0x1:
							{
								machine->V[x] |= machine->V[y];
								if (machine->behavior_data.VIP_VF_Reset)
								{
									machine->V[0xF] = 0x00;
								}
								break;
							}
							case 0x2:
							{
								machine->V[x] &= machine->V[y];
								if (machine->behavior_data.VIP_VF_Reset)
								{
									machine->V[0xF] = 0x00;
								}
								break;
							}
							case 0x3:
							{
								machine->V[x] ^= machine->V[y];
								if (machine->behavior_data.VIP_VF_Reset)
								{
									machine->V[0xF] = 0x00;
								}
								break;
							}
							case 0x4:
							{
								uint8_t tmp = machine->V[x] + machine->V[y];
								bool overflow = (tmp < machine->V[x]);
								machine->V[x] = tmp;
								machine->V[0xF] = overflow ? 0x01 : 0x00;
								break;
							}
							case 0x5:
							{
								uint8_t tmp = machine->V[x] - machine->V[y];
								bool underflow = (tmp > machine->V[x]);
								machine->V[x] = tmp;
								machine->V[0xF] = underflow ? 0x00 : 0x01;
								break;
							}
							case 0x6:
							{
								uint8_t tmp = ((machine->behavior_data.CHIP48_Shift ? machine->V[x] : machine->V[y]) & 0x01);
								machine->V[x] = ((machine->behavior_data.CHIP48_Shift ? machine->V[x] : machine->V[y]) >> 1);
								machine->V[0xF] = tmp;
								break;
							}
							case 0x7:
							{
								uint8_t tmp = machine->V[y] - machine->V[x];
								bool underflow = (tmp > machine->V[y]);
								machine->V[x] = tmp;
								machine->V[0xF] = underflow ? 0x00 : 0x01;
								break;
							}
							case 0xE:
							{
								uint8_t tmp = (((machine->behavior_data.CHIP48_Shift ? machine->V[x] : machine->V[y]) & 0x80) >> 7);
								machine->V[x] = ((machine->behavior_data.CHIP48_Shift ? machine->V[x] : machine->V[y]) << 1);
								machine->V[0xF] = tmp;
								break;
							}
							default:
							{
								machine->PC -= 2;
								if (machine->PC > 0xFFF)
								{
									machine->PC &= 0xFFF;
								}
								PauseProgram(*machine, true);
								machine->error_state = MachineError::InvalidInstruction;
								break;
							}
						}
						break;
					}
					case 0x9:
					{
						uint8_t x = ((operand & 0xF00) >> 8);
						uint8_t y = ((operand & 0x0F0) >> 4);
						uint8_t z = (operand & 0x00F);
						if (z != 0)
						{
							machine->PC -= 2;
							if (machine->PC > 0xFFF)
							{
								machine->PC &= 0xFFF;
							}
							PauseProgram(*machine, true);
							machine->error_state = MachineError::InvalidInstruction;
							break;
						}
						if (machine->V[x] != machine->V[y])
						{
							machine->PC += 2;
							if (machine->PC > 0xFFF)
							{
								machine->PC &= 0xFFF;
							}
						}
						break;
					}
					case 0xA:
					{
						machine->I = operand;
						break;
					}
					case 0xB:
					{
						machine->PC = operand + machine->V[0];
						if (machine->PC > 0xFFF)
						{
							machine->PC &= 0xFFF;
						}
						break;
					}
					case 0xC:
					{
						uint8_t x = ((operand & 0xF00) >> 8);
						uint8_t mask = (operand & 0x0FF);
						machine->V[x] = (machine->rng_distrib(machine->rng_engine) & mask);
						break;
					}
					case 0xD:
					{
						if (machine->behavior_data.VIP_Display_Interrupt && !machine->display_interrupt)
						{
							break;
						}
						uint8_t x = (machine->V[((operand & 0xF00) >> 8)] & 0x3F);
						uint8_t y = (machine->V[((operand & 0x0F0) >> 4)] & 0x1F);
						uint8_t bytes = (operand & 0x00F);
						uint8_t *sprite = &machine->memory[machine->I];
						bool pixels_changed_to_unset = false;
						for (uint32_t py = 0; py < bytes; ++py)
						{
							if (machine->behavior_data.VIP_Clipping && y + py >= 32)
							{
								break;
							}
							uint8_t b_offset = 0;
							for (uint32_t px = 0; px < 8; ++px)
							{
								if (machine->behavior_data.VIP_Clipping && x + px >= 64)
								{
									break;
								}
								uint8_t pixel = ((sprite[py] & (0x80 >> b_offset++)) >> (7 - px));
								uint32_t display_addr = (((y + py) % 32) * 64) + ((x + px) % 64);
								uint8_t tmp = machine->display[display_addr];
								machine->display[display_addr] ^= pixel;
								if (machine->display[display_addr] != tmp && machine->display[display_addr] == 0 && !pixels_changed_to_unset)
								{
									pixels_changed_to_unset = true;
								}
							}
						}
						machine->V[0xF] = pixels_changed_to_unset ? 0x01 : 0x00;
						if (machine->behavior_data.VIP_Display_Interrupt && machine->display_interrupt)
						{
							machine->display_interrupt = false;
							machine->render_display = true;
						}
						break;
					}
					case 0xE:
					{
						uint8_t x = ((operand & 0xF00) >> 8);
						uint8_t opcode2 = (operand & 0x0FF);
						switch (opcode2)
						{
							case 0x9E:
							{
								if (machine->key_status[(machine->V[x] & 0xF)] == 1)
								{
									machine->PC += 2;
									if (machine->PC > 0xFFF)
									{
										machine->PC &= 0xFFF;
									}
								}
								break;
							}
							case 0xA1:
							{
								if (machine->key_status[(machine->V[x] & 0xF)] == 0)
								{
									machine->PC += 2;
									if (machine->PC > 0xFFF)
									{
										machine->PC &= 0xFFF;
									}
								}
								break;
							}
							default:
							{
								machine->PC -= 2;
								if (machine->PC > 0xFFF)
								{
									machine->PC &= 0xFFF;
								}
								PauseProgram(*machine, true);
								machine->error_state = MachineError::InvalidInstruction;
								break;
							}
						}
						break;
					}
					case 0xF:
					{
						uint8_t x = ((operand & 0xF00) >> 8);
						uint8_t opcode2 = (operand & 0x0FF);
						switch (opcode2)
						{
							case 0x07:
							{
								machine->V[x] = machine->delay_timer;
								break;
							}
							case 0x0A:
							{
								if (!machine->wait_for_key_release)
								{
									machine->PC -= 2;
									if (machine->PC > 0xFFF)
									{
										machine->PC &= 0xFFF;
									}
									for (uint8_t i = 0x00; i < 0x10; ++i)
									{
										if (machine->key_status[i] == 1)
										{
											machine->wait_for_key_release = true;
											machine->key_pressed = i;
											break;
										}
									}
								}
								else
								{
									if (machine->key_status[machine->key_pressed] == 0)
									{
										machine->wait_for_key_release = false;
										machine->V[x] = machine->key_pressed;
									}
									else
									{
										machine->PC -= 2;
										if (machine->PC > 0xFFF)
										{
											machine->PC &= 0xFFF;
										}
									}
								}
								break;
							}
							case 0x15:
							{
								SetDelayTimer(*machine, machine->V[x]);
								break;
							}
							case 0x18:
							{
								SetSoundTimer(*machine, machine->V[x]);
								break;
							}
							case 0x1E:
							{
								machine->I += machine->V[x];
								break;
							}
							case 0x29:
							{
								switch (machine->V[x] & 0xF)
								{
									case 0x0:
									{
										machine->I = 0x000;
										break;
									}
									case 0x1:
									{
										machine->I = 0x005;
										break;
									}
									case 0x2:
									{
										machine->I = 0x00A;
										break;
									}
									case 0x3:
									{
										machine->I = 0x00F;
										break;
									}
									case 0x4:
									{
										machine->I = 0x014;
										break;
									}
									case 0x5:
									{
										machine->I = 0x019;
										break;
									}
									case 0x6:
									{
										machine->I = 0x01E;
										break;
									}
									case 0x7:
									{
										machine->I = 0x023;
										break;
									}
									case 0x8:
									{
										machine->I = 0x028;
										break;
									}
									case 0x9:
									{
										machine->I = 0x02D;
										break;
									}
									case 0xA:
									{
										machine->I = 0x032;
										break;
									}
									case 0xB:
									{
										machine->I = 0x037;
										break;
									}
									case 0xC:
									{
										machine->I = 0x03C;
										break;
									}
									case 0xD:
									{
										machine->I = 0x041;
										break;
									}
									case 0xE:
									{
										machine->I = 0x046;
										break;
									}
									case 0xF:
									{
										machine->I = 0x04B;
										break;
									}
								}
								break;
							}
							case 0x33:
							{
								uint8_t tmp = machine->V[x];
								uint8_t digit[3];
								for (uint8_t i = 0; i < 3; ++i)
								{
									digit[i] = tmp % 10;
									tmp /= 10;
								}
								for (uint8_t i = 0; i < 3; ++i)
								{
									machine->memory[((machine->I + i) & 0xFFF)] = digit[2 - i];
								}
								break;
							}
							case 0x55:
							{
								for (uint8_t i = 0; i <= x; ++i)
								{
									machine->memory[((machine->I + i) & 0xFFF)] = machine->V[i];
								}
								machine->I += x + (machine->behavior_data.CHIP48_LoadStore ? 0 : 1);
								break;
							}
							case 0x65:
							{
								for (uint8_t i = 0; i <= x; ++i)
								{
									machine->V[i] = machine->memory[((machine->I + i) & 0xFFF)];
								}
								machine->I += x + (machine->behavior_data.CHIP48_LoadStore ? 0 : 1);
								break;
							}
						}
						break;
					}
				}
				--machine->instructions_to_execute;
			}
			current_execution_unit->cycles = 0;
		}
		else
		{
			std::this_thread::sleep_for(100ns);
		}
	}
}
