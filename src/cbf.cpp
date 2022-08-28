#include "cbf.h"
#include <cstring>
#include <memory>
#include <bit>

Hyper_BandCHIP::CBF::Program::Program(std::vector<unsigned char> &&program_data) : valid(true)
{
	memcpy(&header, &program_data[0], sizeof(header));
	if (header.magic_number[0] != 'C' || header.magic_number[1] != 'B' || header.magic_number[2] != 'F')
	{
		valid = false;
		return;
	}
	if (header.bytecode_table_ptr)
	{
		size_t current_bytecode_table_address = header.bytecode_table_ptr;
		while (program_data[current_bytecode_table_address] != 0x00)
		{
			BytecodeTableEntry current_entry;
			unsigned short file_offset = 0;
			unsigned short size = 0;
			memcpy(&current_entry.platform, &program_data[current_bytecode_table_address], sizeof(PlatformType));
			++current_bytecode_table_address;
			memcpy(&file_offset, &program_data[current_bytecode_table_address], sizeof(unsigned short));
			current_bytecode_table_address += sizeof(unsigned short);
			memcpy(&size, &program_data[current_bytecode_table_address], sizeof(unsigned short));
			current_bytecode_table_address += sizeof(unsigned short);
			if (std::endian::native != std::endian::big)
			{
				file_offset = ((file_offset << 8) | (file_offset >> 8));
				size = ((size << 8) | (size >> 8));
			}
			current_entry.code.resize(size);
			memcpy(current_entry.code.data(), &program_data[file_offset], size);
			bytecode_table.push_back(std::move(current_entry));
		}
	}
	if (header.properties_table_ptr)
	{
		size_t current_properties_table_address = header.properties_table_ptr;
		while (program_data[current_properties_table_address] != 0x00)
		{
			PropertiesTableEntry current_entry;
			unsigned short pointer = 0;
			memcpy(&current_entry.type, &program_data[current_properties_table_address], sizeof(PropertyType));
			++current_properties_table_address;
			memcpy(&pointer, &program_data[current_properties_table_address], sizeof(unsigned short));
			current_properties_table_address += sizeof(unsigned short);
			if (std::endian::native != std::endian::big)
			{
				pointer = ((pointer << 8) | (pointer >> 8));
			}
			switch (current_entry.type)
			{
				case PropertyType::DesiredExecutionSpeed:
				{
					unsigned int cycles_per_frame = 0;
					memcpy(&cycles_per_frame, &program_data[pointer], 3);
					if (std::endian::native == std::endian::big)
					{
						cycles_per_frame >>= 8;
					}
					else
					{
						cycles_per_frame = (((cycles_per_frame & 0xFF0000) >> 16) | (cycles_per_frame & 0xFF00) | ((cycles_per_frame & 0xFF) << 16));
					}
					current_entry.data = cycles_per_frame;
					break;
				}
				case PropertyType::ProgramName:
				case PropertyType::ProgramDescription:
				case PropertyType::ProgramAuthor:
				case PropertyType::ProgramURL:
				case PropertyType::ToolVanity:
				case PropertyType::LicenseInformation:
				{
					std::string text = "";
					while (program_data[pointer] != '\0')
					{
						text += program_data[pointer];
						++pointer;
					}
					current_entry.data = std::move(text);
					break;
				}
				case PropertyType::ProgramReleaseDate:
				{
					unsigned int release_date = 0;
					memcpy(&release_date, &program_data[pointer], sizeof(release_date));
					if (std::endian::native != std::endian::big)
					{
						release_date = ((release_date >> 24) | ((release_date & 0xFF0000) >> 8) | ((release_date & 0xFF00) << 8) | (release_date << 24));
					}
					current_entry.data = release_date;
					break;
				}
				case PropertyType::CoverArt:
				{
					CoverArtData cover_art;
					memcpy(&cover_art.planes, &program_data[pointer], sizeof(unsigned char));
					++pointer;
					memcpy(&cover_art.width, &program_data[pointer], sizeof(unsigned char));
					++pointer;
					memcpy(&cover_art.height, &program_data[pointer], sizeof(unsigned char));
					++pointer;
					size_t cover_art_size = cover_art.planes * cover_art.width * cover_art.height;
					cover_art.data.resize(cover_art_size);
					memcpy(cover_art.data.data(), &program_data[pointer], cover_art_size);
					current_entry.data = std::move(cover_art);
					break;
				}
				case PropertyType::KeyInputConfiguration:
				{
					unsigned char key_input_configuration_list_size = 0;
					memcpy(&key_input_configuration_list_size, &program_data[pointer], sizeof(unsigned char));
					++pointer;
					std::vector<KeyInputData> key_input_configuration_list;
					for (unsigned char i = 0; i < key_input_configuration_list_size; ++i)
					{
						KeyInputData current_key_input;
						memcpy(&current_key_input, &program_data[pointer], sizeof(current_key_input));
						pointer += sizeof(current_key_input);
						key_input_configuration_list.push_back(current_key_input);
					}
					current_entry.data = std::move(key_input_configuration_list);
					break;
				}
				case PropertyType::ColorConfiguration:
				{
					unsigned char color_configuration_list_size = 0;
					memcpy(&color_configuration_list_size, &program_data[pointer], sizeof(unsigned char));
					++pointer;
					std::vector<ColorData> color_configuration_list;
					for (unsigned char i = 0; i < color_configuration_list_size; ++i)
					{
						ColorData current_color_data;
						memcpy(&current_color_data, &program_data[pointer], sizeof(current_color_data));
						pointer += sizeof(current_color_data);
						color_configuration_list.push_back(current_color_data);
					}
					current_entry.data = std::move(color_configuration_list);
					break;
				}
				case PropertyType::ScreenOrientation:
				{
					ScreenOrientation orientation;
					memcpy(&orientation, &program_data[pointer], sizeof(orientation));
					current_entry.data = orientation;
					break;
				}
				case PropertyType::FontData:
				{
					FontData font_data;
					memcpy(&font_data.address, &program_data[pointer], sizeof(font_data.address));
					pointer += sizeof(font_data.address);
					if (std::endian::native != std::endian::big)
					{
						font_data.address = ((font_data.address >> 8) | (font_data.address << 8));
					}
					unsigned char size = 0;
					memcpy(&size, &program_data[pointer], sizeof(size));
					++pointer;
					font_data.data.resize(size);
					memcpy(font_data.data.data(), &program_data[pointer], size);
					current_entry.data = std::move(font_data);
					break;
				}
			}
			properties_table.push_back(std::move(current_entry));
		}
	}
}

Hyper_BandCHIP::CBF::Program::~Program()
{
}

unsigned char Hyper_BandCHIP::CBF::Program::GetVersion() const
{
	return header.version;
}

std::vector<unsigned char> *Hyper_BandCHIP::CBF::Program::GetBytecodeData(PlatformType platform)
{
	for (auto &i : bytecode_table)
	{
		if (i.platform == platform)
		{
			return &i.code;
		}
	}
	return nullptr;
}

bool Hyper_BandCHIP::CBF::Program::HasPropertyType(PropertyType type) const
{
	for (auto &i : properties_table)
	{
		if (i.type == type)
		{
			return true;
		}
	}
	return false;
}

unsigned int Hyper_BandCHIP::CBF::Program::GetDesiredExecutionSpeed() const
{
	for (auto &i : properties_table)
	{
		if (i.type == PropertyType::DesiredExecutionSpeed)
		{
			return std::get<unsigned int>(i.data);
		}
	}
	return 0;
}

std::string Hyper_BandCHIP::CBF::Program::GetProgramName() const
{
	for (auto &i : properties_table)
	{
		if (i.type == PropertyType::ProgramName)
		{
			return std::get<std::string>(i.data);
		}
	}
	return "";
}

std::string Hyper_BandCHIP::CBF::Program::GetProgramDescription() const
{
	for (auto &i : properties_table)
	{
		if (i.type == PropertyType::ProgramDescription)
		{
			return std::get<std::string>(i.data);
		}
	}
	return "";
}

std::vector<std::string> Hyper_BandCHIP::CBF::Program::GetProgramAuthors() const
{
	std::vector<std::string> authors;
	for (auto &i : properties_table)
	{
		if (i.type == PropertyType::ProgramAuthor)
		{
			authors.push_back(std::get<std::string>(i.data));
		}
	}
	return authors;
}

std::vector<std::string> Hyper_BandCHIP::CBF::Program::GetProgramURLs() const
{
	std::vector<std::string> URLs;
	for (auto &i : properties_table)
	{
		if (i.type == PropertyType::ProgramURL)
		{
			URLs.push_back(std::get<std::string>(i.data));
		}
	}
	return URLs;
}

unsigned int Hyper_BandCHIP::CBF::Program::GetProgramReleaseDate() const
{
	for (auto &i : properties_table)
	{
		if (i.type == PropertyType::ProgramReleaseDate)
		{
			return std::get<unsigned int>(i.data);
		}
	}
	return 0;
}

std::vector<Hyper_BandCHIP::CBF::ColorData> *Hyper_BandCHIP::CBF::Program::GetColorConfiguration()
{
	for (auto &i : properties_table)
	{
		if (i.type == PropertyType::ColorConfiguration)
		{
			return std::get_if<std::vector<ColorData>>(&i.data);
		}
	}
	return nullptr;
}

Hyper_BandCHIP::CBF::FontData *Hyper_BandCHIP::CBF::Program::GetFontData()
{
	for (auto &i : properties_table)
	{
		if (i.type == PropertyType::FontData)
		{
			return std::get_if<FontData>(&i.data);
		}
	}
	return nullptr;
}

std::string Hyper_BandCHIP::CBF::Program::GetToolVanity() const
{
	for (auto &i : properties_table)
	{
		if (i.type == PropertyType::ToolVanity)
		{
			return std::get<std::string>(i.data);
		}
	}
	return "";
}

std::string Hyper_BandCHIP::CBF::Program::GetLicenseInformation() const
{
	for (auto &i : properties_table)
	{
		if (i.type == PropertyType::LicenseInformation)
		{
			return std::get<std::string>(i.data);
		}
	}
	return "";
}
