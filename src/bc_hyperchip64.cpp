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
#include <iostream>

/*
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
				unsigned char x = (TargetMachine->V[((operand & 0xF00) >> 8)] & 0x7F);
				unsigned char y = (TargetMachine->V[((operand & 0x0F0) >> 4)] & 0x3F);
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
					unsigned int y_offset = py * (width / 8);
					for (unsigned int px = 0; px < width; ++px)
					{
						unsigned char current_plane = 0;
						unsigned int display_addr = (((y + py) * scale_factor % 64) * 128) + ((x + px) * scale_factor % 128);
						unsigned char *display = &TargetMachine->display[display_addr];
						unsigned char pixel = 0;
						for (unsigned char p = 0; p < plane_count; ++p)
						{
							for (; current_plane < 4; ++current_plane)
							{
								if ((0x01 << current_plane) & TargetMachine->plane)
								{
									pixel |= (((sprite[((width / 8) * height * p) + y_offset + (px / 8)] & (0x80 >> b_offset)) >> (7 - (px % 8))) << current_plane);
									++current_plane;
									break;
								}
							}
						}
						switch (TargetMachine->CurrentResolutionMode)
						{
							case ResolutionMode::LoRes:
							{
								for (unsigned char dy = 0; dy < 2; ++dy)
								{
									for (unsigned char dx = 0; dx < 2; ++dx)
									{
										unsigned char *upscaled_display = &display[(dy * 128) + dx];
										unsigned char tmp = *upscaled_display;
										if (pixel)
										{
											*upscaled_display ^= pixel;
										}
										if (*upscaled_display != tmp && !pixels_changed_to_unset)
										{
											for (unsigned char cp = 0; cp < 4; ++cp)
											{
												if ((0x01 << cp) & TargetMachine->plane)
												{
													pixels_changed_to_unset = !(*upscaled_display & (0x01 << cp));
													break;
												}
											}
										}
									}
								}
								break;
							}
							case ResolutionMode::HiRes:
							{
								unsigned char tmp = *display;
								if (pixel)
								{
									*display ^= pixel;
								}
								if (*display != tmp && !pixels_changed_to_unset)
								{
									for (unsigned char cp = 0; cp < 4; ++cp)
									{
										if ((0x01 << cp) & TargetMachine->plane)
										{
											pixels_changed_to_unset = !(*display & (0x01 << cp));
											break;
										}
									}
								}
								break;
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
						switch (TargetMachine->V[x] & 0xF)
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
						switch (TargetMachine->V[x] & 0xF)
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
						TargetMachine->rpl_user_flags_file.sputn(reinterpret_cast<const char *>(&TargetMachine->V), flag_count + 1);
						TargetMachine->PC += 2;
						break;
					}
					case 0x85:
					{
						unsigned char flag_count = (x > 15) ? 15 : x;
						TargetMachine->rpl_user_flags_file.pubseekoff(0, std::ios::beg);
						TargetMachine->rpl_user_flags_file.sgetn(reinterpret_cast<char *>(&TargetMachine->V), flag_count + 1);
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
*/

Hyper_BandCHIP::BandCHIP::HyperCHIP64_Machine::HyperCHIP64_Machine(unsigned int cycles_per_second, Renderer *DisplayRenderer) : CurrentResolutionMode(ResolutionMode::LoRes), DisplayRenderer(DisplayRenderer), delay_timer(0), sound_timer { 0, 0, 0, 0 }, PC(0), I(0), SP(0), draw_buffer(128 * 64), key_pressed(0), plane(0x01), voice(0), rng_engine(system_clock::now().time_since_epoch().count()), rng_distrib(0, 255), cycle_accumulator(0.0), pause(true), operational(true), wait_for_key_release(false), error_state(MachineError::NoError)
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
	memset(memory.data(), 0, memory.size());
	memset(display.data(), 0, display.size());
	memset(key_status, 0x00, 0x10);
	SetCyclesPerSecond<HyperCHIP64_Machine, HyperCHIP64_Audio>(*this, cycles_per_second);
	cycle_rate = (cycles_per_second > 0) ? 1.0 / static_cast<double>(cycles_per_second) : 0.0;
}

Hyper_BandCHIP::BandCHIP::HyperCHIP64_Machine::~HyperCHIP64_Machine()
{
}

void Hyper_BandCHIP::BandCHIP::HyperCHIP64_Machine::ExecuteInstructions()
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
			for (size_t v = 0; v < 4; ++v)
			{
				previous_cycles_per_frame = SoundTimerSync[v].cycles_per_frame;
				SoundTimerSync[v].cycles_per_frame = new_cycles_per_second_u32 / 60;
				SoundTimerSync[v].cycle_counter = static_cast<double>(SoundTimerSync[v].cycle_counter) * (static_cast<double>(SoundTimerSync[v].cycles_per_frame) / static_cast<double>(previous_cycles_per_frame));
			}
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
		SyncToCycle<HyperCHIP64_Machine, HyperCHIP64_Audio>(*this);
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
			operand = ((memory[PC] & 0xF) << 8) | (memory[static_cast<uint16_t>(PC + 1)]);
		}
		PC += 2;
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
						if (plane && (operand & 0x0F))
						{
							memcpy(draw_buffer.data(), &display[0], (display_height - (operand & 0x0F)) * display_width);
							for (uint16_t y = (operand & 0xF); y < display_height; ++y)
							{
								for (uint16_t x = 0; x < display_width; ++x)
								{
									uint8_t tmp = (draw_buffer[((y - (operand & 0x0F)) * display_width) + x] & plane);
									display[(y * display_width) + x] &= ~(plane);
									display[(y * display_width) + x] |= tmp;
								}
							}
							for (uint16_t y = 0; y < (operand & 0x0F); ++y)
							{
								for (uint16_t x = 0; x < display_width; ++x)
								{
									display[(y * display_width) + x] &= ~(plane);
								}
							}
						}
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
						if (plane && (operand & 0x0F))
						{
							memcpy(draw_buffer.data(), &display[(operand & 0x0F) * display_width], (display_height - (operand & 0x0F)) * display_width);
							for (uint16_t y = 0; y < display_height - (operand & 0x0F); ++y)
							{
								for (uint16_t x = 0; x < display_width; ++x)
								{
									uint8_t tmp = (draw_buffer[(y * display_width) + x] & plane);
									display[(y * display_width) + x] &= ~(plane);
									display[(y * display_width) + x] |= tmp;
								}
							}
							for (uint16_t y = display_height - (operand & 0x0F); y < display_height; ++y)
							{
								for (uint16_t x = 0; x < display_width; ++x)
								{
									display[(y * display_width) + x] &= ~(plane);
								}
							}
						}
						break;
					}
					case 0xE0:
					{
						if (plane)
						{
							for (uint32_t i = 0; i < display_width * display_height; ++i)
							{
								display[i] &= ~(plane);
							}
						}
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
							PauseProgram<HyperCHIP64_Machine, HyperCHIP64_Audio>(*this, true);
							error_state = MachineError::StackUnderflow;
						}
						break;
					}
					case 0xFB:
					{
						if (plane)
						{
							for (uint16_t y = 0; y < display_height; ++y)
							{
								memcpy(draw_buffer.data(), &display[y * display_width], display_width - 4);
								for (uint16_t x = 0; x < display_width - 4; ++x)
								{
									uint8_t tmp = (draw_buffer[x] & plane);
									display[(y * display_width) + 4 + x] &= ~(plane);
									display[(y * display_width) + 4 + x] |= tmp;
								}
								for (uint16_t x = 0; x < 4; ++x)
								{
									display[(y * display_width) + x] &= ~(plane);
								}
							}
						}
						break;
					}
					case 0xFC:
					{
						if (plane)
						{
							for (uint16_t y = 0; y < display_height; ++y)
							{
								memcpy(draw_buffer.data(), &display[(y * display_width) + 4], display_width - 4);
								for (uint16_t x = 0; x < display_width - 4; ++x)
								{
									uint8_t tmp = (draw_buffer[x] & plane);
									display[(y * display_width) + x] &= ~(plane);
									display[(y * display_width) + x] |= tmp;
								}
								for (uint16_t x = 0; x < 4; ++x)
								{
									display[(y * display_width) + 4 + x] &= ~(plane);
								}
							}
						}
						break;
					}
					case 0xFD:
					{
						InitializeVideo(*this);
						PauseProgram<HyperCHIP64_Machine, HyperCHIP64_Audio>(*this, true);
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
						PauseProgram<HyperCHIP64_Machine, HyperCHIP64_Audio>(*this, true);
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
					PauseProgram<HyperCHIP64_Machine, HyperCHIP64_Audio>(*this, true);
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
					if (!(PC % 2))
					{
						uint16_t instruction = *(reinterpret_cast<uint16_t *>(&memory[PC]));
						opcode = std::endian::native != std::endian::big ? ((instruction & 0xF0) >> 4) : (instruction >> 12);
						operand = std::endian::native != std::endian::big ? (((instruction & 0x0F) << 8) | (instruction >> 8)) : (instruction & 0xFFF);
					}
					else
					{
						opcode = (memory[PC] >> 4);
						operand = ((memory[PC] & 0x0F) << 8) | (memory[static_cast<uint16_t>(PC + 1)]);
					}
					PC += (opcode == 0xF && operand == 0x000) ? 4 : 2;
				}
				break;
			}
			case 0x4:
			{
				uint8_t x = ((operand & 0xF00) >> 8);
				uint8_t value = (operand & 0x0FF);
				if (V[x] != value)
				{
					if (!(PC % 2))
					{
						uint16_t instruction = *(reinterpret_cast<uint16_t *>(&memory[PC]));
						opcode = std::endian::native != std::endian::big ? ((instruction & 0xF0) >> 4) : (instruction >> 12);
						operand = std::endian::native != std::endian::big ? (((instruction & 0x0F) << 8) | (instruction >> 8)) : (instruction & 0xFFF);
					}
					else
					{
						opcode = (memory[PC] >> 4);
						operand = ((memory[PC] & 0x0F) << 8) | (memory[static_cast<uint16_t>(PC + 1)]);
					}
					PC += (opcode == 0xF && operand == 0x000) ? 4 : 2;
				}
				break;
			}
			case 0x5:
			{
				uint8_t x = ((operand & 0xF00) >> 8);
				uint8_t y = ((operand & 0x0F0) >> 4);
				uint8_t z = (operand & 0x00F);
				switch (z)
				{
					case 0:
					{
						if (V[x] == V[y])
						{
							if (!(PC % 2))
							{
								uint16_t instruction = *(reinterpret_cast<uint16_t *>(&memory[PC]));
								opcode = std::endian::native != std::endian::big ? ((instruction & 0xF0) >> 4) : (instruction >> 12);
								operand = std::endian::native != std::endian::big ? (((instruction & 0x0F) << 8) | (instruction >> 8)) : (instruction & 0xFFF);
							}
							else
							{
								opcode = (memory[PC] >> 4);
								operand = ((memory[PC] & 0x0F) << 8) | (memory[static_cast<uint16_t>(PC + 1)]);
							}
							PC += (opcode == 0xF && operand == 0x000) ? 4 : 2;
						}
						break;
					}
					case 2:
					{
						for (uint8_t i = ((x <= y) ? x : y); i <= ((x <= y) ? y : x); ++i)
						{
							memory[I + ((x <= y) ? i - x : i - y)] = V[(x <= y) ? i : x - i];
						}
						break;
					}
					case 3:
					{
						for (uint8_t i = ((x <= y) ? x : y); i <= ((x <= y) ? y : x); ++i)
						{
							V[(x <= y) ? i : x - i] = memory[I + ((x <= y) ? i - x : i - y)];
						}
						break;
					}
					default:
					{
						PC -= 2;
						PauseProgram<HyperCHIP64_Machine, HyperCHIP64_Audio>(*this, true);
						error_state = MachineError::InvalidInstruction;
						break;
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
						uint8_t tmp = (V[y] & 0x01);
						V[x] = (V[y] >> 1);
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
					case 0x8:
					{
						V[x] = std::rotr(V[y], 1);
						break;
					}
					case 0x9:
					{
						V[x] = std::rotl(V[y], 1);
						break;
					}
					case 0xA:
					{
						V[0xF] = (V[x] & V[y]) ? 1 : 0;
						break;
					}
					case 0xB:
					{
						V[x] = ~(V[y]);
						break;
					}
					case 0xE:
					{
						uint8_t tmp = ((V[y] & 0x80) >> 7);
						V[x] = (V[y] << 1);
						V[0xF] = tmp;
						break;
					}
					default:
					{
						PC -= 2;
						PauseProgram<HyperCHIP64_Machine, HyperCHIP64_Audio>(*this, true);
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
					PauseProgram<HyperCHIP64_Machine, HyperCHIP64_Audio>(*this, true);
					error_state = MachineError::InvalidInstruction;
					break;
				}
				if (V[x] != V[y])
				{
					if (!(PC % 2))
					{
						uint16_t instruction = *(reinterpret_cast<uint16_t *>(&memory[PC]));
						opcode = std::endian::native != std::endian::big ? ((instruction & 0xF0) >> 4) : (instruction >> 12);
						operand = std::endian::native != std::endian::big ? (((instruction & 0x0F) << 8) | (instruction >> 8)) : (instruction & 0xFFF);
					}
					else
					{
						opcode = (memory[PC] >> 4);
						operand = ((memory[PC] & 0x0F) << 8) | (memory[static_cast<uint16_t>(PC + 1)]);
					}
					PC += (opcode == 0xF && operand == 0x000) ? 4 : 2;
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
				uint8_t plane_count = 0;
				if (!height)
				{
					width = 16;
					height = 16;
				}
				uint8_t *sprite = &memory[I];
				bool pixels_changed_to_unset = false;
				for (uint8_t p = 0; p < 4; ++p)
				{
					if (plane & (0x01 << p))
					{
						++plane_count;
					}
				}
				for (uint32_t py = 0; py < height; ++py)
				{
					uint8_t b_offset = 0;
					uint32_t y_offset = py * (width / 8);
					for (uint32_t px = 0; px < width; ++px)
					{
						uint8_t current_plane = 0;
						uint32_t display_addr = (((y + py) * scale_factor % 64) * 128) + ((x + px) * scale_factor % 128);
						uint8_t *display = &this->display[display_addr];
						uint8_t pixel = 0;
						for (uint8_t p = 0; p < plane_count; ++p)
						{
							for (; current_plane < 4; ++current_plane)
							{
								if ((0x01 << current_plane) & plane)
								{
									pixel |= (((sprite[((width / 8) * height * p) + y_offset + (px / 8)] & (0x80 >> b_offset)) >> (7 - (px % 8))) << current_plane);
									++current_plane;
									break;
								}
							}
						}
						switch (CurrentResolutionMode)
						{
							case ResolutionMode::LoRes:
							{
								for (uint8_t dy = 0; dy < 2; ++dy)
								{
									for (uint8_t dx = 0; dx < 2; ++dx)
									{
										uint8_t *upscaled_display = &display[(dy * 128) + dx];
										uint8_t tmp = *upscaled_display;
										if (pixel)
										{
											*upscaled_display ^= pixel;
										}
										if (*upscaled_display != tmp && !pixels_changed_to_unset)
										{
											for (uint8_t cp = 0; cp < 4; ++cp)
											{
												if ((0x01 << cp) & plane)
												{
													pixels_changed_to_unset = !(*upscaled_display & (0x01 << cp));
													break;
												}
											}
										}
									}
								}
								break;
							}
							case ResolutionMode::HiRes:
							{
								uint8_t tmp = *display;
								if (pixel)
								{
									*display ^= pixel;
								}
								if (*display != tmp && !pixels_changed_to_unset)
								{
									for (uint8_t cp = 0; cp < 4; ++cp)
									{
										if ((0x01 << cp) & plane)
										{
											pixels_changed_to_unset = !(*display & (0x01 << cp));
											break;
										}
									}
								}
								break;
							}
						}
						++b_offset;
						if (b_offset == 8)
						{
							b_offset = 0;
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
							if (!(PC % 2))
							{
								uint16_t instruction = *(reinterpret_cast<uint16_t *>(&memory[PC]));
								opcode = std::endian::native != std::endian::big ? ((instruction & 0xF0) >> 4) : (instruction >> 12);
								operand = std::endian::native != std::endian::big ? (((instruction & 0x0F) << 8) | (instruction >> 8)) : (instruction & 0xFFF);
							}
							else
							{
								opcode = (memory[PC] >> 4);
								operand = ((memory[PC] & 0x0F) << 8) | (memory[static_cast<uint16_t>(PC + 1)]);
							}
							PC += (opcode == 0xF && operand == 0x000) ? 4 : 2;
						}
						break;
					}
					case 0xA1:
					{
						if (key_status[(V[x] & 0xF)] == 0)
						{
							if (!(PC % 2))
							{
								uint16_t instruction = *(reinterpret_cast<uint16_t *>(&memory[PC]));
								opcode = std::endian::native != std::endian::big ? ((instruction & 0xF0) >> 4) : (instruction >> 12);
								operand = std::endian::native != std::endian::big ? (((instruction & 0x0F) << 8) | (instruction >> 8)) : (instruction & 0xFFF);
							}
							else
							{
								opcode = (memory[PC] >> 4);
								operand = ((memory[PC] & 0x0F) << 8) | (memory[static_cast<uint16_t>(PC + 1)]);
							}
							PC += (opcode == 0xF && operand == 0x000) ? 4 : 2;
						}
						break;
					}
					default:
					{
						PC -= 2;
						PauseProgram<HyperCHIP64_Machine, HyperCHIP64_Audio>(*this, true);
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
					case 0x00:
					{
						I = ((memory[PC] << 8) | (memory[static_cast<uint16_t>(PC + 1)]));
						PC += 2;
						break;
					}
					case 0x01:
					{
						plane = x;
						break;
					}
					case 0x02:
					{
						audio_system.CopyToAudioBuffer(memory.data(), I, voice);
						break;
					}
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
						SetSoundTimer<HyperCHIP64_Machine, HyperCHIP64_Audio>(*this, V[x]);
						break;
					}
					case 0x1E:
					{
						I += V[x];
						break;
					}
					case 0x20:
					{
						PC = ((memory[static_cast<uint16_t>(I + V[x])] << 8) | (memory[static_cast<uint16_t>(I + V[x] + 1)]));
						break;
					}
					case 0x21:
					{
						if (SP < (sizeof(stack) / sizeof(uint16_t)))
						{
							stack[SP++] = PC;
							PC = ((memory[static_cast<uint16_t>(I + V[x])] << 8) | (memory[static_cast<uint16_t>(I + V[x] + 1)]));
						}
						else
						{
							PC -= 2;
							PauseProgram<HyperCHIP64_Machine, HyperCHIP64_Audio>(*this, true);
							error_state = MachineError::StackOverflow;
						}
						break;
					}
					case 0x29:
					{
						switch (V[x] & 0xF)
						{
							case 0x0:
							{
								I = 0x0000;
								break;
							}
							case 0x1:
							{
								I = 0x0005;
								break;
							}
							case 0x2:
							{
								I = 0x000A;
								break;
							}
							case 0x3:
							{
								I = 0x000F;
								break;
							}
							case 0x4:
							{
								I = 0x0014;
								break;
							}
							case 0x5:
							{
								I = 0x0019;
								break;
							}
							case 0x6:
							{
								I = 0x001E;
								break;
							}
							case 0x7:
							{
								I = 0x0023;
								break;
							}
							case 0x8:
							{
								I = 0x0028;
								break;
							}
							case 0x9:
							{
								I = 0x002D;
								break;
							}
							case 0xA:
							{
								I = 0x0032;
								break;
							}
							case 0xB:
							{
								I = 0x0037;
								break;
							}
							case 0xC:
							{
								I = 0x003C;
								break;
							}
							case 0xD:
							{
								I = 0x0041;
								break;
							}
							case 0xE:
							{
								I = 0x0046;
								break;
							}
							case 0xF:
							{
								I = 0x004B;
								break;
							}
						}
						break;
					}
					case 0x30:
					{
						switch (V[x] & 0xF)
						{
							case 0x0:
							{
								I = 0x0050;
								break;
							}
							case 0x1:
							{
								I = 0x005A;
								break;
							}
							case 0x2:
							{
								I = 0x0064;
								break;
							}
							case 0x3:
							{
								I = 0x006E;
								break;
							}
							case 0x4:
							{
								I = 0x0078;
								break;
							}
							case 0x5:
							{
								I = 0x0082;
								break;
							}
							case 0x6:
							{
								I = 0x008C;
								break;
							}
							case 0x7:
							{
								I = 0x0096;
								break;
							}
							case 0x8:
							{
								I = 0x00A0;
								break;
							}
							case 0x9:
							{
								I = 0x00AA;
								break;
							}
							case 0xA:
							{
								I = 0x00B4;
								break;
							}
							case 0xB:
							{
								I = 0x00BE;
								break;
							}
							case 0xC:
							{
								I = 0x00C8;
								break;
							}
							case 0xD:
							{
								I = 0x00D2;
								break;
							}
							case 0xE:
							{
								I = 0x00DC;
								break;
							}
							case 0xF:
							{
								I = 0x00E6;
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
							memory[static_cast<uint16_t>(I + i)] = digit[2 - i];
						}
						break;
					}
					case 0x3A:
					{
						uint8_t pitch = V[x];
						audio_system.SetPlaybackRate(pitch, voice);
						break;
					}
					case 0x3B:
					{
						uint8_t volume = V[x];
						audio_system.SetVolume(volume, voice);
						break;
					}
					case 0x3C:
					{
						voice = x;
						break;
					}
					case 0x3D:
					{
						audio_system.SetChannelOutput(x, voice);
						break;
					}
					case 0x55:
					{
						if (I + x <= 0xFFFF)
						{
							memcpy(&memory[I], &V[0], x + 1);
						}
						else
						{
							for (uint8_t i = 0; i <= x; ++i)
							{
								memory[static_cast<uint16_t>(I + i)] = V[i];
							}
						}
						I += x + 1;
						break;
					}
					case 0x65:
					{
						if (I + x <= 0xFFFF)
						{
							memcpy(&V[0], &memory[I], x + 1);
						}
						else
						{
							for (uint8_t i = 0; i <= x; ++i)
							{
								V[i] = memory[static_cast<uint16_t>(I + i)];
							}
						}
						I += x + 1;
						break;
					}
					case 0x75:
					{
						uint8_t flag_count = (x > 15) ? 15 : x;
						rpl_user_flags_file.pubseekoff(0, std::ios::beg);
						rpl_user_flags_file.sputn(reinterpret_cast<const char *>(&V), flag_count + 1);
						break;
					}
					case 0x85:
					{
						uint8_t flag_count = (x > 15) ? 15 : x;
						rpl_user_flags_file.pubseekoff(0, std::ios::beg);
						rpl_user_flags_file.sgetn(reinterpret_cast<char *>(&V), flag_count + 1);
						break;
					}
					case 0xA0:
					case 0xA1:
					{
						break;
					}
					case 0xA2:
					{
						I = ((memory[static_cast<uint16_t>(I + V[x])] << 8) | (memory[static_cast<uint16_t>(I + V[x] + 1)]));
						break;
					}
					default:
					{
						PC -= 2;
						PauseProgram<HyperCHIP64_Machine, HyperCHIP64_Audio>(*this, true);
						error_state = MachineError::InvalidInstruction;
						break;
					}
				}
				break;
			}
		}
	}
}
