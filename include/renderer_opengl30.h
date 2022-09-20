#ifndef _RENDERER_OPENGL30_H_
#define _RENDERER_OPENGL30_H_

#include <SDL.h>
#include <GL/glew.h>
#include <array>
#include <vector>
#include <string>
#include <iostream>

namespace Hyper_BandCHIP
{
	enum class DisplayMode { Emulator, Menu };

	struct Vertex
	{
		float pos[4];
		float tex[2];
	};

	struct ColorData
	{
		std::array<float, 4> Color;
	};

	struct DisplayControl
	{
		std::array<ColorData, 16> Palette;
	};

	struct FontControl
	{
		unsigned int FontColor;
	};

	struct RGBColorData
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
	};

	class Renderer
	{
		public:
			Renderer(SDL_Window *Window);
			~Renderer();

			inline void SetupMenuFonts(const unsigned char *src)
			{
				if (src != nullptr)
				{
					if (CurrentBoundTextureId != MenuFontTextureId)
					{
						CurrentBoundTextureId = MenuFontTextureId;
						glBindTexture(GL_TEXTURE_2D, MenuFontTextureId);
					}
					unsigned char *font_data = new unsigned char[128 * 64];
					unsigned int s_offset = (59 % 10) + ((59 / 10) * (10 * 16));
					unsigned char s_bit_offset = 0;
					memset(font_data, 0x00, 128 * 64);
					for (unsigned int y = 0; y < 10 * 6; ++y)
					{
						unsigned char current_char = ' ' + (((59 - y) / 10) * 16);
						for (unsigned int x = 0; x < 8 * 16; ++x)
						{
							font_data[((y + 4) * (8 * 16)) + x] = ((src[s_offset] & (0x80 >> s_bit_offset)) >> (7 - s_bit_offset));
							++s_bit_offset;
							if (s_bit_offset > 7)
							{
								s_bit_offset = 0;
								s_offset += 10;
								++current_char;
							}
							if (current_char >= 0x7F)
							{
								break;
							}
						}
						s_offset = ((59 - y - 1) % 10) + (((59 - y - 1) / 10) * (10 * 16));
					}
					glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8UI, 128, 64);
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 64, GL_RED_INTEGER, GL_UNSIGNED_BYTE, font_data);
					delete [] font_data;
				}
			}

			inline void SetupDisplay(unsigned short width, unsigned short height)
			{
				if (display_width != width && display_height != height)
				{
					display_width = width;
					display_height = height;
					display.resize(display_width * display_height);
				}
				memset(display.data(), 0, display_width * display_height);
				if (CurrentBoundTextureId != DisplayTextureId)
				{
					CurrentBoundTextureId = DisplayTextureId;
					glBindTexture(GL_TEXTURE_2D, DisplayTextureId);
				}
				glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, display_width, display_height, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, display.data());
			}

			inline void WriteToDisplay(const unsigned char *src, unsigned short width, unsigned short height)
			{
				if (src != nullptr && width <= this->display_width && height <= this->display_height)
				{
					if (CurrentBoundTextureId != DisplayTextureId)
					{
						CurrentBoundTextureId = DisplayTextureId;
						glBindTexture(GL_TEXTURE_2D, DisplayTextureId);
					}
					for (unsigned short y = 0; y < height; ++y)
					{
						memcpy(&display[((height - y - 1) * width)], &src[y * width], width);
					}
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_BYTE, display.data());
				}
			}

			inline void ClearDisplay()
			{
				memset(display.data(), 0, display_width * display_height);
				if (CurrentBoundTextureId != DisplayTextureId)
				{
					CurrentBoundTextureId = DisplayTextureId;
					glBindTexture(GL_TEXTURE_2D, DisplayTextureId);
				}
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, display_width, display_height, GL_RED_INTEGER, GL_UNSIGNED_BYTE, display.data());
			}

			inline void ClearMenu()
			{
				if (CurrentFramebuffer != MenuFBOId)
				{
					CurrentFramebuffer = MenuFBOId;
					glBindFramebuffer(GL_DRAW_FRAMEBUFFER, MenuFBOId);
				}
				glClear(GL_COLOR_BUFFER_BIT);
			}

			void OutputStringToMenu(std::string str, unsigned short x, unsigned short y, unsigned char color);

			inline void SetDisplayMode(DisplayMode mode)
			{
				CurrentDisplayMode = mode;
			}

			inline void SetPaletteIndex(RGBColorData data, unsigned char index)
			{
				if (index < 16)
				{
					disp_ctrl.Palette[index] = { static_cast<float>(data.r) / 255.0f, static_cast<float>(data.g) / 255.0f, static_cast<float>(data.b) / 255.0f, 1.0f };
					if (CurrentProgramId == MainProgramId)
					{
						glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(disp_ctrl), &disp_ctrl);
					}
					else
					{
						glBindBuffer(GL_UNIFORM_BUFFER, DisplayControlUBOId);
						glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(disp_ctrl), &disp_ctrl);
						glBindBuffer(GL_UNIFORM_BUFFER, FontControlUBOId);
					}
				}
			}

			inline RGBColorData GetPaletteIndex(unsigned char index) const
			{
				if (index < 16)
				{
					return { static_cast<unsigned char>(disp_ctrl.Palette[index].Color[0] * 255.0f), static_cast<unsigned char>(disp_ctrl.Palette[index].Color[1] * 255.0f), static_cast<unsigned char>(disp_ctrl.Palette[index].Color[2] * 255.0f) };
				}
				else
				{
					return { 0, 0, 0 };
				}
			}

			inline void Render()
			{
				if (CurrentFramebuffer != 0)
				{
					CurrentFramebuffer = 0;
					glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				}
				if (CurrentProgramId != MainProgramId)
				{
					glUseProgram(MainProgramId);
					CurrentProgramId = MainProgramId;
					glBindBuffer(GL_UNIFORM_BUFFER, DisplayControlUBOId);
				}
				switch (CurrentDisplayMode)
				{
					case DisplayMode::Emulator:
					{
						if (CurrentBoundTextureId != DisplayTextureId)
						{
							CurrentBoundTextureId = DisplayTextureId;
							glBindTexture(GL_TEXTURE_2D, DisplayTextureId);
						}
						break;
					}
					case DisplayMode::Menu:
					{
						if (CurrentBoundTextureId != MenuTextureId)
						{
							CurrentBoundTextureId = MenuTextureId;
							glBindTexture(GL_TEXTURE_2D, MenuTextureId);
						}
						break;
					}
				}
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, reinterpret_cast<void *>(0));
				SDL_GL_SwapWindow(Window);
			}

			inline bool Fail() const
			{
				return fail;
			}
		private:
			SDL_Window *Window;
			SDL_GLContext GLContext;
			GLuint VertexShaderId, FragmentShaderId, MenuFragmentShaderId,
			       MainProgramId, MenuProgramId, VAOId, VBOId, IBOId,
			       DisplayControlUBOId, FontControlUBOId, DisplayTextureId,
			       MenuFBOId, MenuTextureId, MenuFontTextureId,
			       CurrentBoundTextureId, CurrentFramebuffer,
			       CurrentProgramId;
			Vertex vertices[4];
			unsigned char indices[4];
			DisplayControl disp_ctrl;
			FontControl font_ctrl;
			std::vector<unsigned char> display;
			unsigned short display_width;
			unsigned short display_height;
			DisplayMode CurrentDisplayMode;
			bool fail;
	};
}

#endif
