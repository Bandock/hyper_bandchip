#include "../../include/renderer_opengles3.h"
#include <iostream>

using std::cout;
using std::endl;

Hyper_BandCHIP::Renderer::Renderer(SDL_Window *Window) : Window(Window), VertexShaderId(0), FragmentShaderId(0), MenuFragmentShaderId(0), MainProgramId(0), MenuProgramId(0), VAOId(0), VBOId(0), IBOId(0), DisplayControlUBOId(0), FontControlUBOId(0), DisplayTextureId(0), MenuFBOId(0), MenuTextureId(0), MenuFontTextureId(0), CurrentBoundTextureId(0), CurrentFramebuffer(0), CurrentProgramId(0), display(nullptr), display_width(0), display_height(0), CurrentDisplayMode(DisplayMode::Menu), fail(false)
{
	const char *VertexShaderCode = R"(#version 300 es

layout(location = 0) in vec4 pos;
layout(location = 1) in vec2 tex;

out vec2 outTex;

void main()
{
	gl_Position = pos;
	outTex = tex;
})";
	const char *FragmentShaderCode = R"(#version 300 es

precision highp float;
precision highp int;
precision highp usampler2D;

layout(std140) uniform DisplayControl
{
	vec4 Palette[4];
};

in vec2 outTex;
uniform usampler2D CurrentTexture;

out vec4 outColor;

void main()
{
	ivec2 texDim = textureSize(CurrentTexture, 0);
	uvec4 color_data = texelFetch(CurrentTexture, ivec2(int(outTex.x * float(texDim.x)), int(outTex.y * float(texDim.y))), 0);
	outColor = Palette[color_data.x];
})";
	const char *MenuFragmentShaderCode = R"(#version 300 es

precision highp int;
precision highp float;
precision highp usampler2D;

layout(std140) uniform FontControl
{
	uint FontColor;
};

in vec2 outTex;
uniform usampler2D CurrentTexture;

layout(location = 0) out uint outColorIndex;

void main()
{
	ivec2 texDim = textureSize(CurrentTexture, 0);
	uvec4 color_data = texelFetch(CurrentTexture, ivec2(int(outTex.x * float(texDim.x)), int((outTex.y) * float(texDim.y))), 0);
	outColorIndex = (color_data.x == uint(1)) ? FontColor : uint(0);
})";
	vertices[0] = {{ -1.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }};
	vertices[1] = {{ 1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }};
	vertices[2] = {{ -1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }};
	vertices[3] = {{ 1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }};
	indices[0] = 0;
	indices[1] = 2;
	indices[2] = 1;
	indices[3] = 3;
	disp_ctrl.Palette[0] = { 0.0f, 0.0f, 0.0f, 1.0f };
	disp_ctrl.Palette[1] = { 1.0f, 1.0f, 1.0f, 1.0f };
	disp_ctrl.Palette[2] = { 1.0f, 0.0f, 0.0f, 1.0f };
	disp_ctrl.Palette[3] = { 1.0f, 1.0f, 0.0f, 1.0f };
	font_ctrl.FontColor = 1;
	GLContext = SDL_GL_CreateContext(Window);
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		cout << "GLEW Initialization failed." << endl;
		fail = true;
		return;
	}
	VertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	MenuFragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(VertexShaderId, 1, &VertexShaderCode, nullptr);
	glShaderSource(FragmentShaderId, 1, &FragmentShaderCode, nullptr);
	glShaderSource(MenuFragmentShaderId, 1, &MenuFragmentShaderCode, nullptr);
	GLint compile_status = 0, info_log_len = 0;
	glCompileShader(VertexShaderId);
	glGetShaderiv(VertexShaderId, GL_COMPILE_STATUS, &compile_status);
	if (compile_status == GL_FALSE)
	{
		cout << "Vertex Shader compilation failed." << endl;
		glGetShaderiv(VertexShaderId, GL_INFO_LOG_LENGTH, &info_log_len);
		char *info_log = new char[info_log_len];
		glGetShaderInfoLog(VertexShaderId, info_log_len, nullptr, info_log);
		cout << info_log << endl;
		delete [] info_log;
		fail = true;
		return;
	}
	glCompileShader(FragmentShaderId);
	glGetShaderiv(FragmentShaderId, GL_COMPILE_STATUS, &compile_status);
	if (compile_status == GL_FALSE)
	{
		cout << "Fragment Shader compilation failed." << endl;
		glGetShaderiv(FragmentShaderId, GL_INFO_LOG_LENGTH, &info_log_len);
		char *info_log = new char[info_log_len];
		glGetShaderInfoLog(FragmentShaderId, info_log_len, nullptr, info_log);
		cout << info_log << endl;
		delete [] info_log;
		fail = true;
		return;
	}
	glCompileShader(MenuFragmentShaderId);
	glGetShaderiv(MenuFragmentShaderId, GL_COMPILE_STATUS, &compile_status);
	if (compile_status == GL_FALSE)
	{
		cout << "Menu Fragment Shader compilation failed." << endl;
		glGetShaderiv(MenuFragmentShaderId, GL_INFO_LOG_LENGTH, &info_log_len);
		char *info_log = new char[info_log_len];
		glGetShaderInfoLog(MenuFragmentShaderId, info_log_len, nullptr, info_log);
		cout << info_log << endl;
		delete [] info_log;
		fail = true;
		return;
	}
	MainProgramId = glCreateProgram();
	glAttachShader(MainProgramId, VertexShaderId);
	glAttachShader(MainProgramId, FragmentShaderId);
	glLinkProgram(MainProgramId);
	int link_status = 0;
	glGetProgramiv(MainProgramId, GL_LINK_STATUS, &link_status);
	if (link_status == GL_FALSE)
	{
		cout << "Main Program linkage failed." << endl;
		glGetProgramiv(MainProgramId, GL_INFO_LOG_LENGTH, &info_log_len);
		char *info_log = new char[info_log_len];
		glGetProgramInfoLog(MainProgramId, info_log_len, nullptr, info_log);
		cout << info_log << endl;
		delete [] info_log;
		return;
	}
	MenuProgramId = glCreateProgram();
	glAttachShader(MenuProgramId, VertexShaderId);
	glAttachShader(MenuProgramId, MenuFragmentShaderId);
	glLinkProgram(MenuProgramId);
	glGetProgramiv(MenuProgramId, GL_LINK_STATUS, &link_status);
	if (link_status == GL_FALSE)
	{
		cout << "Menu Program linkage failed." << endl;
		glGetProgramiv(MenuProgramId, GL_INFO_LOG_LENGTH, &info_log_len);
		char *info_log = new char[info_log_len];
		glGetProgramInfoLog(MenuProgramId, info_log_len, nullptr, info_log);
		cout << info_log << endl;
		delete [] info_log;
		return;
	}
	glGenVertexArrays(1, &VAOId);
	glBindVertexArray(VAOId);
	glGenBuffers(1, &VBOId);
	glGenBuffers(1, &IBOId);
	glGenBuffers(1, &DisplayControlUBOId);
	glGenBuffers(1, &FontControlUBOId);
	glBindBuffer(GL_ARRAY_BUFFER, VBOId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBOId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, FontControlUBOId);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(font_ctrl), &font_ctrl, GL_STATIC_DRAW);
	glUniformBlockBinding(MainProgramId, 0, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, DisplayControlUBOId, 0, sizeof(disp_ctrl));
	glBufferData(GL_UNIFORM_BUFFER, sizeof(disp_ctrl), &disp_ctrl, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(sizeof(float) * 4));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glGenTextures(1, &DisplayTextureId);
	glGenTextures(1, &MenuFontTextureId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, MenuFontTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, DisplayTextureId);
	CurrentBoundTextureId = DisplayTextureId;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SetupDisplay(64, 32);
	glGenFramebuffers(1, &MenuFBOId);
	glGenTextures(1, &MenuTextureId);
	glBindTexture(GL_TEXTURE_2D, MenuTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, 640, 320, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, MenuFBOId);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, MenuTextureId, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, DisplayTextureId);
	glUseProgram(MainProgramId);
	CurrentProgramId = MainProgramId;
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, 1280, 640);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	SDL_GL_SetSwapInterval(0);
}

Hyper_BandCHIP::Renderer::~Renderer()
{
	glUseProgram(0);
	if (CurrentFramebuffer != 0)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}
	glDeleteFramebuffers(1, &MenuFBOId);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &MenuTextureId);
	glDeleteTextures(1, &MenuFontTextureId);
	glDeleteTextures(1, &DisplayTextureId);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBOId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &IBOId);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, 0, 0, 0);
	glUniformBlockBinding(MainProgramId, 0, 0);
	glDeleteBuffers(1, &DisplayControlUBOId);
	glDeleteBuffers(1, &FontControlUBOId);
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VAOId);
	glDetachShader(MenuProgramId, VertexShaderId);
	glDetachShader(MenuProgramId, MenuFragmentShaderId);
	if (MenuProgramId != 0)
	{
		glDeleteProgram(MenuProgramId);
	}
	glDetachShader(MainProgramId, VertexShaderId);
	glDetachShader(MainProgramId, FragmentShaderId);
	if (MainProgramId != 0)
	{
		glDeleteProgram(MainProgramId);
	}
	if (VertexShaderId != 0)
	{
		glDeleteShader(VertexShaderId);
	}
	if (FragmentShaderId != 0)
	{
		glDeleteShader(FragmentShaderId);
	}
	if (MenuFragmentShaderId != 0)
	{
		glDeleteShader(MenuFragmentShaderId);
	}
	SDL_GL_DeleteContext(GLContext);
	if (display != nullptr)
	{
		delete [] display;
	}
}

void Hyper_BandCHIP::Renderer::SetupMenuFonts(const unsigned char *src)
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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, 128, 64, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, font_data);
		delete [] font_data;
	}
}

void Hyper_BandCHIP::Renderer::SetupDisplay(unsigned short width, unsigned short height)
{
	if (display_width != width && display_height != height)
	{
		if (display != nullptr)
		{
			delete [] display;
		}
		display_width = width;
		display_height = height;
		display = new unsigned char[display_width * display_height];
	}
	memset(display, 0, display_width * display_height);
	if (CurrentBoundTextureId != DisplayTextureId)
	{
		CurrentBoundTextureId = DisplayTextureId;
		glBindTexture(GL_TEXTURE_2D, DisplayTextureId);
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, display_width, display_height, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, display);
}

void Hyper_BandCHIP::Renderer::WriteToDisplay(const unsigned char *src, unsigned short width, unsigned short height)
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
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_BYTE, display);
	}
}

void Hyper_BandCHIP::Renderer::ClearDisplay()
{
	memset(display, 0, display_width * display_height);
	if (CurrentBoundTextureId != DisplayTextureId)
	{
		CurrentBoundTextureId = DisplayTextureId;
		glBindTexture(GL_TEXTURE_2D, DisplayTextureId);
	}
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, display_width, display_height, GL_RED_INTEGER, GL_UNSIGNED_BYTE, display);
}

void Hyper_BandCHIP::Renderer::ClearMenu()
{
	if (CurrentFramebuffer != MenuFBOId)
	{
		CurrentFramebuffer = MenuFBOId;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, MenuFBOId);
	}
	glClear(GL_COLOR_BUFFER_BIT);
}

void Hyper_BandCHIP::Renderer::OutputStringToMenu(std::string str, unsigned short x, unsigned short y, unsigned char color)
{
	if (CurrentFramebuffer != MenuFBOId)
	{
		CurrentFramebuffer = MenuFBOId;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, MenuFBOId);
	}
	glViewport(0, 0, 640, 320);
	if (CurrentProgramId != MenuProgramId)
	{
		glUseProgram(MenuProgramId);
		CurrentProgramId = MenuProgramId;
		glBindBufferRange(GL_UNIFORM_BUFFER, 0, FontControlUBOId, 0, sizeof(font_ctrl));
	}
	if (CurrentBoundTextureId != MenuFontTextureId)
	{
		CurrentBoundTextureId = MenuFontTextureId;
		glBindTexture(GL_TEXTURE_2D, MenuFontTextureId);
	}
	font_ctrl.FontColor = (color & 0x03);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(font_ctrl), &font_ctrl);
	size_t len = str.length();
	for (size_t i = 0; i < len; i++)
	{
		if (str[i] < 32 || str[i] > 126)
		{
			continue;
		}
		float left_x = (static_cast<float>(x + (i * 10)) / 320.0f) - 1.0f;
		float right_x = (static_cast<float>(x + (i * 10) + 8) / 320.0f) - 1.0f;
		float up_y = 1.0f - (static_cast<float>(y) / 160.0f);
		float down_y = 1.0f - (static_cast<float>(y + 10) / 160.0f);
		unsigned char current_char = static_cast<unsigned char>(str[i]) - 32;
		float tex_left_x = (static_cast<float>((current_char % 16) * 8) / 128.0f);
		float tex_right_x = (static_cast<float>(((current_char % 16) * 8) + 8) / 128.0f);
		float tex_up_y = 1.0f - (static_cast<float>(((current_char / 16) * 10)) / 64.0f);
		float tex_down_y = 1.0f - (static_cast<float>((((current_char / 16) * 10) + 10)) / 64.0f);
		vertices[0] = {{ left_x, up_y, 0.0f, 1.0f }, { tex_left_x, tex_up_y }};
		vertices[1] = {{ right_x, up_y, 0.0f, 1.0f }, { tex_right_x, tex_up_y }};
		vertices[2] = {{ left_x, down_y, 0.0f, 1.0f }, { tex_left_x, tex_down_y }};
		vertices[3] = {{ right_x, down_y, 0.0f, 1.0f }, { tex_right_x, tex_down_y }};
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, reinterpret_cast<void *>(0));
	}
	vertices[0] = {{ -1.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }};
	vertices[1] = {{ 1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }};
	vertices[2] = {{ -1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }};
	vertices[3] = {{ 1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }};
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glViewport(0, 0, 1280, 640);
}

void Hyper_BandCHIP::Renderer::SetDisplayMode(Hyper_BandCHIP::DisplayMode mode)
{
	CurrentDisplayMode = mode;
}

void Hyper_BandCHIP::Renderer::SetPaletteIndex(Hyper_BandCHIP::RGBColorData data, unsigned char index)
{
	if (index < 4)
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
			glBindBufferRange(GL_UNIFORM_BUFFER, 0, FontControlUBOId, 0, sizeof(font_ctrl));
		}
	}
}

Hyper_BandCHIP::RGBColorData Hyper_BandCHIP::Renderer::GetPaletteIndex(unsigned char index) const
{
	if (index < 4)
	{
		return { static_cast<unsigned char>(disp_ctrl.Palette[index].Color[0] * 255.0f), static_cast<unsigned char>(disp_ctrl.Palette[index].Color[1] * 255.0f), static_cast<unsigned char>(disp_ctrl.Palette[index].Color[2] * 255.0f) };
	}
	else
	{
		return { 0, 0, 0 };
	}
}

void Hyper_BandCHIP::Renderer::Render()
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
		glBindBufferRange(GL_UNIFORM_BUFFER, 0, DisplayControlUBOId, 0, sizeof(disp_ctrl));
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

bool Hyper_BandCHIP::Renderer::Fail() const
{
	return fail;
}
