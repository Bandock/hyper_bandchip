#ifndef _MENU_H_
#define _MENU_H_

#include <string>
#include <vector>
#include <concepts>
#include <sstream>

#ifdef RENDERER_OPENGLES3
#include "../include/renderer_opengles3.h"
#endif

namespace Hyper_BandCHIP
{
	enum class MenuItemType { Text, Button, Toggle, MultiChoice };

	template <typename T>
	concept HasText = requires(T obj)
	{
		obj.Text;
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
		obj.current_option;
		obj.Options;
	};

	template <typename T>
	concept HasHidden = requires(T obj)
	{
		obj.hidden;
	};

	template <typename T>
	concept HasToggleSupport = requires(T obj)
	{
		obj.toggle;
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
	concept HasStatus = requires(T obj)
	{
		obj.Status;
	};

	template <typename T>
	concept DisplayableText = HasText<T> && HasCoordinates<T> && HasHidden<T>;

	template <typename T>
	concept DisplayableStatus = HasText<T> && HasStatus<T> && HasCoordinates<T> && HasHidden<T>;

	template <typename T>
	concept DisplayableOptions = HasText<T> && HasCoordinates<T> && HasHidden<T> && HasOptionSupport<T>;

	template <typename T>
	concept DisplayableToggle = HasText<T> && HasCoordinates<T> && HasHidden<T> && HasToggleSupport<T>;

	template <typename T>
	concept DisplayableValue = HasText<T> && HasValue<T> && HasCoordinates<T> && HasHidden<T>;

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

	struct LeftRightNumberItem
	{
		std::string Text;
		unsigned short x;
		unsigned short y;
		const unsigned int min;
		const unsigned int max;
		unsigned int value;
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
			c_strstream << item.Text << ": " << item.value;
			renderer.OutputStringToMenu(c_strstream.str(), item.x, item.y, color);
		}
	}
}

#endif
