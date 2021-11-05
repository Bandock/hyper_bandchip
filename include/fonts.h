#ifndef _FONTS_H_
#define _FONTS_H_

namespace Hyper_BandCHIP
{
	template <int size>
	struct FontData
	{
		unsigned char data[size];
	};

	template <int size, int character_count>
	class Font
	{
		public:
			Font(const FontData<size> CharacterSource[character_count]);
			~Font();
			void operator>>(unsigned char *dest);
		private:
			FontData<size> Character[character_count];
	};

	template class Font<5, 16>;
	template class Font<10, 10>;
	template class Font<10, 16>;
}

#endif
