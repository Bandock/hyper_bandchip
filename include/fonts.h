#ifndef _FONTS_H_
#define _FONTS_H_

#include <string>
#include <array>
#include <vector>
#include <fstream>
#include <bit>
#include <iostream>
#include <msbtfont/msbtfont.h>

namespace Hyper_BandCHIP
{
	namespace Fonts
	{
		enum class FontStyleType
		{
			LoRes, HiRes
		};

		template <int size>
		struct FontData
		{
			unsigned char data[size];
		};

		consteval unsigned char GenerateBitmapBitmask(unsigned char width)
		{
			unsigned char bitmask = 0x00;
			for (unsigned char i = 0; i < width; ++i)
			{
				bitmask |= (0x01 << (7 - i));
			}
			return bitmask;
		}

		template <unsigned char width, unsigned char height>
		class FontStyle
		{
			public:
				FontStyle(std::string filename) : loaded(false)
				{
					std::ifstream font_file(filename, std::ios::binary);
					msbtfont_header header;
					font_file.read(reinterpret_cast<char *>(&header), sizeof(header));
					msbtfont_filedata filedata;
					msbtfont_retcode ret = msbtfont_create_filedata(&header, &filedata);
					if (ret == MSBTFONT_SUCCESS)
					{
						if (header.max_font_width + 1 == width && header.max_font_height + 1 == height)
						{
							font_file.read(reinterpret_cast<char *>(filedata.data), filedata.size);
							for (unsigned char i = 0; i < 10; ++i)
							{
								if (header.font_name[i] == 0x00)
								{
									break;
								}
								name += header.font_name[i];
							}
							unsigned int font_character_count = (std::endian::native == std::endian::little) ? header.font_character_count_le : header.font_character_count_be;
							if (font_character_count > 16)
							{
								font_character_count = 16;
							}
							character_list.resize(font_character_count);
							constexpr size_t font_data_size = ((width * height) / 8) + ((width * height) % 8 != 0);
							std::array<uint8_t, font_data_size> font_character_data;
							for (unsigned int i = 0; i < font_character_count; ++i)
							{
								size_t c_offset = 0;
								size_t c_bit_offset = 0;
								msbtfont_load_font_character_data(&header, &filedata, font_character_data.data(), i);
								FontData<height> &current_font_character = character_list[i];
								for (size_t i2 = 0; i2 < height; ++i2)
								{
									uint8_t bitmask = (GenerateBitmapBitmask(width) >> c_bit_offset);
									current_font_character.data[i2] = ((font_character_data[c_offset] & bitmask) << c_bit_offset);
									c_bit_offset += width;
									if (c_bit_offset >= 8)
									{
										c_bit_offset -= 8;
										++c_offset;
									}
								}
							}
							loaded = true;
						}
						msbtfont_delete_filedata(&filedata);
					}
				}

				~FontStyle() = default;

				bool IsLoaded() const
				{
					return loaded;
				}

				std::string GetName() const
				{
					return name;
				}

				const unsigned char *GetFontStyleData()
				{
					return reinterpret_cast<const unsigned char *>(character_list.data());
				}

				size_t GetFontStyleDataSize() const
				{
					return character_list.size() * height;
				}
			private:
				bool loaded;
				std::string name;
				std::vector<FontData<height>> character_list;

		};
	}
}

#endif
