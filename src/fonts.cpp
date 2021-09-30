#include "../include/fonts.h"
#include <cstring>

template <int size, int character_count>
Hyper_BandCHIP::Font<size, character_count>::Font(const FontData<size> CharacterSource[character_count])
{
	memcpy(Character, CharacterSource, sizeof(Character));
}

template <int size, int character_count>
Hyper_BandCHIP::Font<size, character_count>::~Font()
{
}

template <int size, int character_count>
void Hyper_BandCHIP::Font<size, character_count>::operator>>(unsigned char *dest)
{
	memcpy(dest, Character, sizeof(Character));
}
