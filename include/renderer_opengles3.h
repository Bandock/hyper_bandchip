#ifndef _RENDERER_OPENGLES3_H_
#define _RENDERER_OPENGLES3_H_

#include <SDL.h>
#include <GL/glew.h>
#include <array>
#include <vector>
#include <string>

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

	using glTexStorage2D_Func = void (GLAPIENTRY *)(GLenum, GLsizei, GLenum, GLsizei, GLsizei);

	struct RGBColorData
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
	};

	class Renderer
	{
		public:
			Renderer(SDL_Window &Window);
			~Renderer();
			glTexStorage2D_Func glTexStorage2D;
			void SetupMenuFonts(const unsigned char *src);
			void SetupDisplay(unsigned short width, unsigned short height);
			void WriteToDisplay(const unsigned char *src, unsigned short width, unsigned short height);
			void ClearDisplay();
			void ClearMenu();
			void OutputStringToMenu(std::string str, unsigned short x, unsigned short y, unsigned char color);
			void SetDisplayMode(DisplayMode mode);
			void SetPaletteIndex(RGBColorData data, unsigned char index);
			RGBColorData GetPaletteIndex(unsigned char index) const;
			void Render(SDL_Window &Window);
			bool Fail() const;
		private:
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
