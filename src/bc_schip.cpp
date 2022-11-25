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
#include <cstring>

/*
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
							case SuperCHIPVersion::Original_SuperCHIP10:
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
							case SuperCHIPVersion::Original_SuperCHIP10:
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
							case SuperCHIPVersion::Original_SuperCHIP10:
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
				unsigned char x = (TargetMachine->V[((operand & 0xF00) >> 8)] & 0x7F);
				unsigned char y = (TargetMachine->V[((operand & 0x0F0) >> 4)] & 0x3F);
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
						const SuperCHIP_BehaviorData *Behavior = std::get_if<SuperCHIP_BehaviorData>(&TargetMachine->behavior_data);
						unsigned char font_digit = (TargetMachine->V[x] & ((Behavior->Version == SuperCHIPVersion::Original_SuperCHIP10) ? 0x1F : 0x0F));
						switch (font_digit)
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
								TargetMachine->I = 0x050;
								break;
							}
							case 0x11:
							{
								TargetMachine->I = 0x05A;
								break;
							}
							case 0x12:
							{
								TargetMachine->I = 0x064;
								break;
							}
							case 0x13:
							{
								TargetMachine->I = 0x06E;
								break;
							}
							case 0x14:
							{
								TargetMachine->I = 0x078;
								break;
							}
							case 0x15:
							{
								TargetMachine->I = 0x082;
								break;
							}
							case 0x16:
							{
								TargetMachine->I = 0x08C;
								break;
							}
							case 0x17:
							{
								TargetMachine->I = 0x096;
								break;
							}
							case 0x18:
							{
								TargetMachine->I = 0x0A0;
								break;
							}
							case 0x19:
							{
								TargetMachine->I = 0x0AA;
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
							case SuperCHIPVersion::SuperCHIP10:
							case SuperCHIPVersion::Fixed_SuperCHIP11:
							case SuperCHIPVersion::SuperCHIP11:
							{
								switch (TargetMachine->V[x] & 0xF)
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
									case 0xA:
									{
										TargetMachine->I = 0x0B4;
										break;
									}
									case 0xB:
									{
										TargetMachine->I = 0x0BE;
										break;
									}
									case 0xC:
									{
										TargetMachine->I = 0x0C8;
										break;
									}
									case 0xD:
									{
										TargetMachine->I = 0x0D2;
										break;
									}
									case 0xE:
									{
										TargetMachine->I = 0x0DC;
										break;
									}
									case 0xF:
									{
										TargetMachine->I = 0x0E6;
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
							case SuperCHIPVersion::Original_SuperCHIP10:
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
							case SuperCHIPVersion::Original_SuperCHIP10:
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
							case SuperCHIPVersion::Original_SuperCHIP10:
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
						TargetMachine->rpl_user_flags_file.sputn(reinterpret_cast<const char *>(&TargetMachine->V), flag_count + 1);
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
						TargetMachine->rpl_user_flags_file.sgetn(reinterpret_cast<char *>(&TargetMachine->V), flag_count + 1);
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

Hyper_BandCHIP::BandCHIP::SuperCHIP_Machine::SuperCHIP_Machine(unsigned int cycles_per_second, Renderer *DisplayRenderer) : CurrentResolutionMode(ResolutionMode::LoRes), DisplayRenderer(DisplayRenderer), delay_timer(0), sound_timer(0), PC(0), I(0), SP(0), key_pressed(0), rng_engine(system_clock::now().time_since_epoch().count()), rng_distrib(0, 255), cycle_accumulator(0.0), pause(true), operational(true), wait_for_key_release(false), error_state(MachineError::NoError)
{
	rpl_user_flags_file.open("RPLUserFlags.bin", std::ios::binary | std::ios::in | std::ios::out);
	if (!rpl_user_flags_file.is_open())
	{
		rpl_user_flags_file.open("RPLUserFlags.bin", std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
		rpl_user_flags_file.pubseekoff(15, std::ios::beg);
		rpl_user_flags_file.sputc(0x00);
	}
	memset(V.data(), 0, V.size());
	memset(stack.data(), 0, stack.size());
	// memory.resize(4096);
	memset(memory.data(), 0, memory.size());
	memset(display.data(), 0, display.size());
	memset(key_status, 0x00, 0x10);
	SetCyclesPerSecond(*this, cycles_per_second);
	cycle_rate = (cycles_per_second > 0) ? 1.0 / static_cast<double>(cycles_per_second) : 0.0;
}

Hyper_BandCHIP::BandCHIP::SuperCHIP_Machine::~SuperCHIP_Machine()
{
}

void Hyper_BandCHIP::BandCHIP::SuperCHIP_Machine::ExecuteInstructions()
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
						switch (behavior_data.Version)
						{
							case SuperCHIPVersion::Fixed_SuperCHIP11:
							case SuperCHIPVersion::SuperCHIP11:
							{
								memmove(&display[display_width * (operand & 0x0F)], &display[0], display_width * (display_height - (operand & 0x0F)));
								memset(display.data(), 0x00, display_width * (operand & 0x0F));
								break;
							}
							case SuperCHIPVersion::Original_SuperCHIP10:
							case SuperCHIPVersion::SuperCHIP10:
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
							if (PC > 0xFFF)
							{
								PC &= 0xFFF;
							}
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
					case 0xFB:
					{
						switch (behavior_data.Version)
						{
							case SuperCHIPVersion::Fixed_SuperCHIP11:
							case SuperCHIPVersion::SuperCHIP11:
							{
								for (uint16_t y = 0; y < display_height; ++y)
								{
									memmove(&display[(y * display_width) + 4], &display[y * display_width], display_width - 4);
									memset(&display[y * display_width], 0x00, 4);
								}
								break;
							}
							case SuperCHIPVersion::Original_SuperCHIP10:
							case SuperCHIPVersion::SuperCHIP10:
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
					case 0xFC:
					{
						switch (behavior_data.Version)
						{
							case SuperCHIPVersion::Fixed_SuperCHIP11:
							case SuperCHIPVersion::SuperCHIP11:
							{
								for (uint16_t y = 0; y < display_height; ++y)
								{
									memmove(&display[y * display_width], &display[(y * display_width) + 4], display_width - 4);
									memset(&display[(y * display_width) + (display_width - 4)], 0x00, 4);
								}
								break;
							}
							case SuperCHIPVersion::Original_SuperCHIP10:
							case SuperCHIPVersion::SuperCHIP10:
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
					case 0xFD:
					{
						PC -= 2;
						if (PC > 0xFFF)
						{
							PC &= 0xFFF;
						}
						InitializeVideo(*this);
						PauseProgram(*this, true);
						operational = false;
						break;
					}
					case 0xFE:
					{
						if (CurrentResolutionMode == ResolutionMode::HiRes)
						{
							CurrentResolutionMode = ResolutionMode::LoRes;
							InitializeVideo(*this);
						}
						break;
					}
					case 0xFF:
					{
						if (CurrentResolutionMode == ResolutionMode::LoRes)
						{
							CurrentResolutionMode = ResolutionMode::HiRes;
							InitializeVideo(*this);
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
				uint8_t x = ((operand & 0xF00) >> 8);
				uint8_t y = ((operand & 0x0F0) >> 4);
				uint8_t z = (operand & 0x00F);
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
						break;
					}
					case 0x2:
					{
						V[x] &= V[y];
						break;
					}
					case 0x3:
					{
						V[x] ^= V[y];
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
						uint8_t tmp = (((behavior_data.Version == SuperCHIPVersion::Fixed_SuperCHIP11) ? V[y] : V[x]) & 0x01);
						V[x] = (((behavior_data.Version == SuperCHIPVersion::Fixed_SuperCHIP11) ? V[y] : V[x]) >> 1);
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
						uint8_t tmp = ((((behavior_data.Version == SuperCHIPVersion::Fixed_SuperCHIP11) ? V[y] : V[x]) & 0x80) >> 7);
						V[x] = (((behavior_data.Version == SuperCHIPVersion::Fixed_SuperCHIP11) ? V[y] : V[x]) << 1);
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
				uint8_t x = ((operand & 0xF00) >> 8);
				uint8_t y = ((operand & 0x0F0) >> 4);
				uint8_t z = (operand & 0x00F);
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
				uint8_t x = (V[((operand & 0xF00) >> 8)] & 0x7F);
				uint8_t y = (V[((operand & 0x0F0) >> 4)] & 0x3F);
				uint8_t width = 8;
				uint8_t height = (operand & 0x00F);
				uint8_t scale_factor = (CurrentResolutionMode == ResolutionMode::HiRes) ? 1 : 2;
				if (!height)
				{
					if (CurrentResolutionMode == ResolutionMode::HiRes)
					{
						width = 16;
					}
					height = 16;
				}
				uint8_t *sprite = &memory[I];
				bool pixels_changed_to_unset = false;
				for (uint32_t py = 0; py < height; ++py)
				{
					uint8_t b_offset = 0;
					for (uint32_t px = 0; px < width; ++px)
					{
						uint32_t y_offset = py * (width / 8);
						uint8_t pixel = ((sprite[y_offset + (px / 8)] & (0x80 >> b_offset++)) >> (7 - (px % 8)));
						if (b_offset == 8)
						{
							b_offset = 0;
						}
						uint32_t display_addr = (((y + py) * scale_factor % 64) * 128) + ((x + px) * scale_factor % 128);
						switch (CurrentResolutionMode)
						{
							case ResolutionMode::LoRes:
							{
								for (uint8_t dy = 0; dy < 2; ++dy)
								{
									for (uint8_t dx = 0; dx < 2; ++dx)
									{
										uint8_t tmp = display[display_addr + (dy * 128) + dx];
										display[display_addr + (dy * 128) + dx] ^= pixel;
										if (display[display_addr + (dy * 128) + dx] != tmp && display[display_addr + (dy * 128) + dx] == 0 && !pixels_changed_to_unset)
										{
											pixels_changed_to_unset = true;
										}
									}
								}
								break;
							}
							case ResolutionMode::HiRes:
							{
								uint8_t tmp = display[display_addr];
								display[display_addr] ^= pixel;
								if (display[display_addr] != tmp && display[display_addr] == 0 && !pixels_changed_to_unset)
								{
									pixels_changed_to_unset = true;
								}
								break;
							}
						}
					}
				}
				V[0xF] = pixels_changed_to_unset ? 0x01 : 0x00;
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
						uint8_t font_digit = (V[x] & ((behavior_data.Version == SuperCHIPVersion::Original_SuperCHIP10) ? 0x1F : 0x0F));
						switch (font_digit)
						{
							case 0x00:
							{
								I = 0x000;
								break;
							}
							case 0x01:
							{
								I = 0x005;
								break;
							}
							case 0x02:
							{
								I = 0x00A;
								break;
							}
							case 0x03:
							{
								I = 0x00F;
								break;
							}
							case 0x04:
							{
								I = 0x014;
								break;
							}
							case 0x05:
							{
								I = 0x019;
								break;
							}
							case 0x06:
							{
								I = 0x01E;
								break;
							}
							case 0x07:
							{
								I = 0x023;
								break;
							}
							case 0x08:
							{
								I = 0x028;
								break;
							}
							case 0x09:
							{
								I = 0x02D;
								break;
							}
							case 0x0A:
							{
								I = 0x032;
								break;
							}
							case 0x0B:
							{
								I = 0x037;
								break;
							}
							case 0x0C:
							{
								I = 0x03C;
								break;
							}
							case 0x0D:
							{
								I = 0x041;
								break;
							}
							case 0x0E:
							{
								I = 0x046;
								break;
							}
							case 0x0F:
							{
								I = 0x04B;
								break;
							}
							case 0x10:
							{
								I = 0x050;
								break;
							}
							case 0x11:
							{
								I = 0x05A;
								break;
							}
							case 0x12:
							{
								I = 0x064;
								break;
							}
							case 0x13:
							{
								I = 0x06E;
								break;
							}
							case 0x14:
							{
								I = 0x078;
								break;
							}
							case 0x15:
							{
								I = 0x082;
								break;
							}
							case 0x16:
							{
								I = 0x08C;
								break;
							}
							case 0x17:
							{
								I = 0x096;
								break;
							}
							case 0x18:
							{
								I = 0x0A0;
								break;
							}
							case 0x19:
							{
								I = 0x0AA;
								break;
							}
						}
						break;
					}
					case 0x30:
					{
						switch (behavior_data.Version)
						{
							case SuperCHIPVersion::SuperCHIP10:
							case SuperCHIPVersion::Fixed_SuperCHIP11:
							case SuperCHIPVersion::SuperCHIP11:
							{
								switch (V[x] & 0xF)
								{
									case 0x0:
									{
										I = 0x050;
										break;
									}
									case 0x1:
									{
										I = 0x05A;
										break;
									}
									case 0x2:
									{
										I = 0x064;
										break;
									}
									case 0x3:
									{
										I = 0x06E;
										break;
									}
									case 0x4:
									{
										I = 0x078;
										break;
									}
									case 0x5:
									{
										I = 0x082;
										break;
									}
									case 0x6:
									{
										I = 0x08C;
										break;
									}
									case 0x7:
									{
										I = 0x096;
										break;
									}
									case 0x8:
									{
										I = 0x0A0;
										break;
									}
									case 0x9:
									{
										I = 0x0AA;
										break;
									}
									case 0xA:
									{
										I = 0x0B4;
										break;
									}
									case 0xB:
									{
										I = 0x0BE;
										break;
									}
									case 0xC:
									{
										I = 0x0C8;
										break;
									}
									case 0xD:
									{
										I = 0x0D2;
										break;
									}
									case 0xE:
									{
										I = 0x0DC;
										break;
									}
									case 0xF:
									{
										I = 0x0E6;
										break;
									}
								}
								break;
							}
							case SuperCHIPVersion::Original_SuperCHIP10:
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
						switch (behavior_data.Version)
						{
							case SuperCHIPVersion::Fixed_SuperCHIP11:
							{
								I += x + 1;
								break;
							}
							case SuperCHIPVersion::Original_SuperCHIP10:
							case SuperCHIPVersion::SuperCHIP10:
							{
								I += x;
								break;
							}
						}
						break;
					}
					case 0x65:
					{
						for (uint8_t i = 0; i <= x; ++i)
						{
							V[i] = memory[((I + i) & 0xFFF)];
						}
						switch (behavior_data.Version)
						{
							case SuperCHIPVersion::Fixed_SuperCHIP11:
							{
								I += x + 1;
								break;
							}
							case SuperCHIPVersion::Original_SuperCHIP10:
							case SuperCHIPVersion::SuperCHIP10:
							{
								I += x;
								break;
							}
						}
						break;
					}
					case 0x75:
					{
						uint8_t flag_count = (x > 7) ? 7 : x;
						rpl_user_flags_file.pubseekoff(0, std::ios::beg);
						rpl_user_flags_file.sputn(reinterpret_cast<const char *>(&V), flag_count + 1);
						break;
					}
					case 0x85:
					{
						uint8_t flag_count = (x > 7) ? 7 : x;
						rpl_user_flags_file.pubseekoff(0, std::ios::beg);
						rpl_user_flags_file.sgetn(reinterpret_cast<char *>(&V), flag_count + 1);
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
		}
	}
}
