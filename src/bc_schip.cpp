#include "../include/machine.h"
#include <cstring>

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
						const SuperCHIP_BehaviorData *Behavior = std::get_if<SuperCHIP_BehaviorData>(&TargetMachine->behavior_data);
						unsigned char tmp = (((Behavior->Version == SuperCHIPVersion::Fixed_SuperCHIP11) ? TargetMachine->V[y] : TargetMachine->V[x]) & 0x01);
						TargetMachine->V[x] = (((Behavior->Version == SuperCHIPVersion::Fixed_SuperCHIP11) ? TargetMachine->V[y] : TargetMachine->V[x]) >> 1);
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
						const SuperCHIP_BehaviorData *Behavior = std::get_if<SuperCHIP_BehaviorData>(&TargetMachine->behavior_data);
						unsigned char tmp = ((((Behavior->Version == SuperCHIPVersion::Fixed_SuperCHIP11) ? TargetMachine->V[y] : TargetMachine->V[x]) & 0x80) >> 7);
						TargetMachine->V[x] = (((Behavior->Version == SuperCHIPVersion::Fixed_SuperCHIP11) ? TargetMachine->V[y] : TargetMachine->V[x]) << 1);
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
				unsigned char width = 8;
				unsigned char height = (operand & 0x00F);
				unsigned char scale_factor = (TargetMachine->CurrentResolutionMode == ResolutionMode::HiRes) ? 1 : 2;
				if (!height)
				{
					if (TargetMachine->CurrentResolutionMode == ResolutionMode::HiRes)
					{
						width = 16;
					}
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
