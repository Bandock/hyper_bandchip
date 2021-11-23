#include "../include/machine.h"

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
						const CHIP8_BehaviorData *Behavior = std::get_if<CHIP8_BehaviorData>(&TargetMachine->behavior_data);
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
