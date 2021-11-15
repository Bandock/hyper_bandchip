#include "../include/machine.h"
#include <cstring>

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
						if (TargetMachine->plane && (operand & 0x0F))
						{
							unsigned char *buffer = new unsigned char[(TargetMachine->display_height - (operand & 0x0F)) * TargetMachine->display_width];
							memcpy(buffer, &TargetMachine->display[0], (TargetMachine->display_height - (operand & 0x0F)) * TargetMachine->display_width);
							for (unsigned short y = (operand & 0xF); y < TargetMachine->display_height; ++y)
							{
								for (unsigned short x = 0; x < TargetMachine->display_width; ++x)
								{
									unsigned char tmp = (buffer[((y - (operand & 0x0F)) * TargetMachine->display_width) + x] & TargetMachine->plane);
									TargetMachine->display[(y * TargetMachine->display_width) + x] &= ~(TargetMachine->plane);
									TargetMachine->display[(y * TargetMachine->display_width) + x] |= tmp;
								}
							}
							for (unsigned short y = 0; y < (operand & 0x0F); ++y)
							{
								for (unsigned short x = 0; x < TargetMachine->display_width; ++x)
								{
									TargetMachine->display[(y * TargetMachine->display_width) + x] &= ~(TargetMachine->plane);
								}
							}
							delete [] buffer;
						}
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
						if (TargetMachine->plane && (operand & 0x0F))
						{
							unsigned char *buffer = new unsigned char[(TargetMachine->display_height - (operand & 0x0F)) * TargetMachine->display_width];
							memcpy(buffer, &TargetMachine->display[(operand & 0x0F) * TargetMachine->display_width], (TargetMachine->display_height - (operand & 0x0F)) * TargetMachine->display_width);
							for (unsigned short y = 0; y < TargetMachine->display_height - (operand & 0x0F); ++y)
							{
								for (unsigned short x = 0; x < TargetMachine->display_width; ++x)
								{
									unsigned char tmp = (buffer[(y * TargetMachine->display_width) + x] & TargetMachine->plane);
									TargetMachine->display[(y * TargetMachine->display_width) + x] &= ~(TargetMachine->plane);
									TargetMachine->display[(y * TargetMachine->display_width) + x] |= tmp;
								}
							}
							for (unsigned short y = TargetMachine->display_height - (operand & 0x0F); y < TargetMachine->display_height; ++y)
							{
								for (unsigned short x = 0; x < TargetMachine->display_width; ++x)
								{
									TargetMachine->display[(y * TargetMachine->display_width) + x] &= ~(TargetMachine->plane);
								}
							}
							delete [] buffer;
						}
						TargetMachine->PC += 2;
						break;
					}
					case 0xE0:
					{
						if (TargetMachine->plane)
						{
							for (unsigned int i = 0; i < TargetMachine->display_width * TargetMachine->display_height; ++i)
							{
								TargetMachine->display[i] &= ~(TargetMachine->plane);
							}
						}
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
						if (TargetMachine->plane)
						{
							unsigned char *buffer = new unsigned char[TargetMachine->display_width - 4];
							for (unsigned short y = 0; y < TargetMachine->display_height; ++y)
							{
								memcpy(buffer, &TargetMachine->display[y * TargetMachine->display_width], TargetMachine->display_width - 4);
								for (unsigned short x = 0; x < TargetMachine->display_width - 4; ++x)
								{
									unsigned char tmp = (buffer[x] & TargetMachine->plane);
									TargetMachine->display[(y * TargetMachine->display_width) + 4 + x] &= ~(TargetMachine->plane);
									TargetMachine->display[(y * TargetMachine->display_width) + 4 + x] |= tmp;
								}
								for (unsigned short x = 0; x < 4; ++x)
								{
									TargetMachine->display[(y * TargetMachine->display_width) + x] &= ~(TargetMachine->plane);
								}
							}
							delete [] buffer;
						}
						TargetMachine->PC += 2;
						break;
					}
					case 0xFC:
					{
						if (TargetMachine->plane)
						{
							unsigned char *buffer = new unsigned char[TargetMachine->display_width - 4];
							for (unsigned short y = 0; y < TargetMachine->display_height; ++y)
							{
								memcpy(buffer, &TargetMachine->display[(y * TargetMachine->display_width) + 4], TargetMachine->display_width - 4);
								for (unsigned short x = 0; x < TargetMachine->display_width - 4; ++x)
								{
									unsigned char tmp = (buffer[x] & TargetMachine->plane);
									TargetMachine->display[(y * TargetMachine->display_width) + x] &= ~(TargetMachine->plane);
									TargetMachine->display[(y * TargetMachine->display_width) + x] |= tmp;
								}
								for (unsigned short x = 0; x < 4; ++x)
								{
									TargetMachine->display[(y * TargetMachine->display_width) + 4 + x] &= ~(TargetMachine->plane);
								}
							}
							delete [] buffer;
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
				TargetMachine->PC = (operand & 0xFFF);
				break;
			}
			case 0x2:
			{
				if (TargetMachine->SP < (sizeof(TargetMachine->stack) / sizeof(unsigned short)))
				{
					TargetMachine->stack[TargetMachine->SP++] = TargetMachine->PC + 2;
					TargetMachine->PC = (operand & 0xFFF);
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
					if (next_opcode == 0xF && (next_operand & 0xFFF) == 0x000)
					{
						TargetMachine->PC += 2;
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
					unsigned short next_operand = ((TargetMachine->memory[TargetMachine->PC] & 0x0F) << 8) | (TargetMachine->memory[static_cast<unsigned short>(TargetMachine->PC + 1)]);
					if (next_opcode == 0xF && (next_operand & 0xFFF) == 0x000)
					{
						TargetMachine->PC += 2;
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
							if (next_opcode == 0xF && (next_operand & 0xFFF) == 0x000)
							{
								TargetMachine->PC += 2;
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
						unsigned char tmp = (TargetMachine->V[y] & 0x01);
						TargetMachine->V[x] = (TargetMachine->V[y] >> 1);
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
						unsigned char tmp = ((TargetMachine->V[y] & 0x80) >> 7);
						TargetMachine->V[x] = (TargetMachine->V[y] << 1);
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
					unsigned short next_operand = ((TargetMachine->memory[TargetMachine->PC] & 0x0F) << 8) | (TargetMachine->memory[static_cast<unsigned short>(TargetMachine->PC + 1)]);
					if (next_opcode == 0xF && (next_operand & 0xFFF) == 0x000)
					{
						TargetMachine->PC += 2;
					}
					TargetMachine->PC += 2;
				}
				break;
			}
			case 0xA:
			{
				TargetMachine->I = (operand & 0xFFF);
				TargetMachine->PC += 2;
				break;
			}
			case 0xB:
			{
				TargetMachine->PC = (operand & 0xFFF) + TargetMachine->V[0];
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
				unsigned char plane_count = 0;
				if (!height)
				{
					width = 16;
					height = 16;
				}
				unsigned char *sprite = &TargetMachine->memory[TargetMachine->I];
				bool pixels_changed_to_unset = false;
				for (unsigned char p = 0; p < 4; ++p)
				{
					if (TargetMachine->plane & (0x01 << p))
					{
						++plane_count;
					}
				}
				for (unsigned int py = 0; py < height; ++py)
				{
					unsigned char b_offset = 0;
					for (unsigned int px = 0; px < width; ++px)
					{
						unsigned int y_offset = py * (width / 8);
						unsigned char current_plane = 0;
						for (unsigned char p = 0; p < plane_count; ++p)
						{
							unsigned char current_mask = 0x00;
							unsigned char pixel_color_data = 0;
							for (; current_plane < 4; ++current_plane)
							{
								if ((0x01 << current_plane) & TargetMachine->plane)
								{
									current_mask |= (0x01 << current_plane);
									++current_plane;
									break;
								}
							}
							unsigned char pixel = ((sprite[((width / 8) * height * p) + y_offset + (px / 8)] & (0x80 >> b_offset)) >> (7 - (px % 8)));
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
											if (pixel)
											{
												pixel_color_data |= current_mask;
											}
											TargetMachine->display[display_addr + (dy * 128) + dx] ^= pixel_color_data;
											if (TargetMachine->display[display_addr + (dy * 128) + dx] != tmp && !(TargetMachine->display[display_addr + (dy * 128) + dx] & current_mask) && !pixels_changed_to_unset)
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
									if (pixel)
									{
										pixel_color_data |= current_mask;
									}
									TargetMachine->display[display_addr] ^= pixel_color_data;
									if (TargetMachine->display[display_addr] != tmp && !(TargetMachine->display[display_addr] & current_mask) && !pixels_changed_to_unset)
									{
										pixels_changed_to_unset = true;
									}
									break;
								}
							}
						}
						++b_offset;
						if (b_offset == 8)
						{
							b_offset = 0;
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
							unsigned short next_operand = ((TargetMachine->memory[TargetMachine->PC] & 0x0F) << 8) | (TargetMachine->memory[static_cast<unsigned short>(TargetMachine->PC + 1)]);
							if (next_opcode == 0xF && (next_operand & 0xFFF) == 0x000)
							{
								TargetMachine->PC += 2;
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
							unsigned short next_operand = ((TargetMachine->memory[TargetMachine->PC] & 0x0F) << 8) | (TargetMachine->memory[static_cast<unsigned short>(TargetMachine->PC + 1)]);
							if (next_opcode == 0xF && (next_operand & 0xFFF) == 0x000)
							{
								TargetMachine->PC += 2;
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
					case 0x00:
					{
						TargetMachine->PC += 2;
						TargetMachine->I = ((TargetMachine->memory[TargetMachine->PC] << 8) | (TargetMachine->memory[static_cast<unsigned short>(TargetMachine->PC + 1)]));
						TargetMachine->PC += 2;
						break;
					}
					case 0x01:
					{
						TargetMachine->plane = x;
						TargetMachine->PC += 2;
						break;
					}
					case 0x02:
					{
						HyperCHIP64_Audio *audio = std::get_if<HyperCHIP64_Audio>(&TargetMachine->audio_system);
						audio->CopyToAudioBuffer(TargetMachine->memory, TargetMachine->I, TargetMachine->voice);
						TargetMachine->PC += 2;
						break;
					}
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
								TargetMachine->I = 0x00AA;
								break;
							}
							case 0xA:
							{
								TargetMachine->I = 0x00B4;
								break;
							}
							case 0xB:
							{
								TargetMachine->I = 0x00BE;
								break;
							}
							case 0xC:
							{
								TargetMachine->I = 0x00C8;
								break;
							}
							case 0xD:
							{
								TargetMachine->I = 0x00D2;
								break;
							}
							case 0xE:
							{
								TargetMachine->I = 0x00DC;
								break;
							}
							case 0xF:
							{
								TargetMachine->I = 0x00E6;
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
					case 0x3A:
					{
						HyperCHIP64_Audio *audio = std::get_if<HyperCHIP64_Audio>(&TargetMachine->audio_system);
						unsigned char pitch = TargetMachine->V[x];
						audio->SetPlaybackRate(pitch, TargetMachine->voice);
						TargetMachine->PC += 2;
						break;
					}
					case 0x3B:
					{
						HyperCHIP64_Audio *audio = std::get_if<HyperCHIP64_Audio>(&TargetMachine->audio_system);
						unsigned char volume = TargetMachine->V[x];
						audio->SetVolume(volume, TargetMachine->voice);
						TargetMachine->PC += 2;
						break;
					}
					case 0x3C:
					{
						TargetMachine->voice = x;
						TargetMachine->PC += 2;
						break;
					}
					case 0x3D:
					{
						HyperCHIP64_Audio *audio = std::get_if<HyperCHIP64_Audio>(&TargetMachine->audio_system);
						unsigned char channel_mask = x;
						audio->SetChannelOutput(channel_mask, TargetMachine->voice);
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
						unsigned char flag_count = (x > 15) ? 15 : x;
						TargetMachine->rpl_user_flags_file.pubseekoff(0, std::ios::beg);
						TargetMachine->rpl_user_flags_file.sputn(reinterpret_cast<const char *>(TargetMachine->V), flag_count + 1);
						TargetMachine->PC += 2;
						break;
					}
					case 0x85:
					{
						unsigned char flag_count = (x > 15) ? 15 : x;
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
