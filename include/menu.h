#ifndef _MENU_H_
#define _MENU_H_

#include <string>
#include <vector>
#include <concepts>
#include <sstream>
#include <iomanip>

#ifdef RENDERER_OPENGLES3
#include "../include/renderer_opengles3.h"
#endif

namespace Hyper_BandCHIP
{
	enum class ValueBaseType { Dec, Hex };

	template <typename T>
	concept HasText = requires(T obj)
	{
		{ obj.Text } -> std::convertible_to<std::string>;
	};

	template <typename T>
	concept HasCoordinates = requires(T obj)
	{
		obj.x;
		obj.y;
	};

	template <typename T>
	concept HasEventId = requires(T obj)
	{
		obj.event_id;
	};

	template <typename T>
	concept HasOptionSupport = requires(T obj)
	{
		{ obj.current_option } -> std::convertible_to<unsigned int>;
		obj.Options;
	};

	template <typename T>
	concept HasHidden = requires(T obj)
	{
		{ obj.hidden } -> std::convertible_to<bool>;
	};

	template <typename T>
	concept HasToggleSupport = requires(T obj)
	{
		{ obj.toggle } -> std::convertible_to<bool>;
	};

	template <typename T>
	concept HasMinimum = requires(T obj)
	{
		obj.min;
	};

	template <typename T>
	concept HasMaximum = requires(T obj)
	{
		obj.max;
	};

	template <typename T>
	concept HasValue = requires(T obj)
	{
		obj.value;
	};

	template <typename T>
	concept HasValueBaseType = requires(T obj)
	{
		{ obj.value_base_type } -> std::convertible_to<ValueBaseType>;
	};

	template <typename T>
	concept HasDisplayDigitCount = requires(T obj)
	{
		obj.display_digit_count;
	};

	template <typename T>
	concept HasStatus = requires(T obj)
	{
		{ obj.Status } -> std::convertible_to<std::string>;
	};

	template <typename T>
	concept HasInputData = requires(T obj)
	{
		obj.input_data;
	};

	template <typename T>
	concept DisplayableText = HasText<T> && HasCoordinates<T> && HasHidden<T>;

	template <typename T>
	concept DisplayableStatus = HasText<T> && HasStatus<T> && HasCoordinates<T> && HasHidden<T>;

	template <typename T>
	concept DisplayableInput = HasText<T> && HasInputData<T> && HasCoordinates<T> && HasHidden<T>;

	template <typename T>
	concept DisplayableOptions = HasText<T> && HasCoordinates<T> && HasHidden<T> && HasOptionSupport<T>;

	template <typename T>
	concept DisplayableToggle = HasText<T> && HasCoordinates<T> && HasHidden<T> && HasToggleSupport<T>;

	template <typename T>
	concept DisplayableValue = HasText<T> && HasValue<T> && HasValueBaseType<T> && HasDisplayDigitCount<T> && HasCoordinates<T> && HasHidden<T>;

	struct TextItem
	{
		std::string Text;
		unsigned short x;
		unsigned short y;
		bool hidden;
	};

	struct StatusTextItem
	{
		std::string Text;
		unsigned short x;
		unsigned short y;
		std::string Status;
		bool hidden;
	};

	template <unsigned int size>
	struct InputItem
	{
		std::string Text;
		unsigned short x;
		unsigned short y;
		bool hidden;
		char input_data[size];
	};

	struct ButtonItem
	{
		std::string Text;
		unsigned short x;
		unsigned short y;
		unsigned int event_id;
		bool hidden;
	};

	struct ToggleItem
	{
		std::string Text;
		unsigned short x;
		unsigned short y;
		bool toggle;
		bool hidden;
	};

	struct MultiChoiceItem
	{
		std::string Text;
		unsigned short x;
		unsigned short y;
		unsigned int event_id;
		unsigned int current_option;
		std::vector<std::string> Options;
		bool hidden;
	};

	struct ValueItem
	{
		std::string Text;
		unsigned short x;
		unsigned short y;
		const ValueBaseType value_base_type;
		unsigned int value;
		unsigned char display_digit_count;
		bool hidden;
	};

	struct AdjustableValueItem
	{
		std::string Text;
		unsigned short x;
		unsigned short y;
		const unsigned int min;
		const unsigned int max;
		const ValueBaseType value_base_type;
		unsigned int value;
		unsigned char display_digit_count;
		bool hidden;
	};

	template <DisplayableText T>
	void DisplayItem(Renderer &renderer, T &item, unsigned char color)
	{
		if (!item.hidden)
		{
			renderer.OutputStringToMenu(item.Text, item.x, item.y, color);
		}
	}

	template <DisplayableStatus T>
	void DisplayItem(Renderer &renderer, T &item, unsigned char color)
	{
		if (!item.hidden)
		{
			std::string CurrentText = item.Text + ": " + item.Status;
			renderer.OutputStringToMenu(CurrentText, item.x, item.y, color);
		}
	}

	template <DisplayableInput T>
	void DisplayItem(Renderer &renderer, T &item, unsigned char color)
	{
		if (!item.hidden)
		{
			std::string CurrentText = item.Text + ": " + item.input_data;
			renderer.OutputStringToMenu(CurrentText, item.x, item.y, color);
		}
	}

	template <DisplayableOptions T>
	void DisplayItem(Renderer &renderer, T &item, unsigned char color)
	{
		if (!item.hidden)
		{
			unsigned int current_option = item.current_option;
			size_t option_count = item.Options.size();
			if (current_option >= static_cast<unsigned int>(option_count))
			{
				current_option = static_cast<unsigned int>(option_count) - 1;
			}
			std::string CurrentText = item.Text + ": " + item.Options[item.current_option];
			renderer.OutputStringToMenu(CurrentText, item.x, item.y, color);
		}
	}

	template <DisplayableToggle T>
	void DisplayItem(Renderer &renderer, T &item, unsigned char color)
	{
		if (!item.hidden)
		{
			std::string CurrentText = item.Text + ": " + (item.toggle ? "On" : "Off");
			renderer.OutputStringToMenu(CurrentText, item.x, item.y, color);
		}
	}

	template <DisplayableValue T>
	void DisplayItem(Renderer &renderer, T &item, unsigned char color)
	{
		if (!item.hidden)
		{
			std::ostringstream c_strstream;
			c_strstream << item.Text << ": ";
			switch (item.value_base_type)
			{
				case ValueBaseType::Dec:
				{
					c_strstream << std::dec;
					break;
				}
				case ValueBaseType::Hex:
				{
					c_strstream << std::hex;
					break;
				}
			}
			if (item.display_digit_count > 0)
			{
				c_strstream << std::setfill('0') << std::setw(item.display_digit_count);
			}
			c_strstream << item.value;
			renderer.OutputStringToMenu(c_strstream.str(), item.x, item.y, color);
		}
	}
}

#endif
