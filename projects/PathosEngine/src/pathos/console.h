#pragma once

#include "render/render_command_list.h"

#include <stdint.h>
#include <memory>
#include <string>
#include <list>
#include <vector>

namespace pathos {

	class ConsoleWindow {

	public:
		ConsoleWindow();
		ConsoleWindow(const ConsoleWindow&) = delete;
		ConsoleWindow& operator=(const ConsoleWindow&) = delete;
		~ConsoleWindow();

		bool initialize(uint16 width, uint16 height);

		void renderConsoleWindow(RenderCommandList& cmdList);
		void toggle();

		bool isVisible() const;

		void onKeyPress(unsigned char ascii);

		class Label* addLine(const char* text);
		class Label* addLine(const wchar_t* text);

	private:
		void evaluate(const wchar_t* text);

		bool initialized;
		bool visible;
		uint16 windowWidth;
		uint16 windowHeight;

		class OverlayRenderer* renderer;
		class DisplayObject2D* root;
		class Rectangle* background;
		class Label* inputText;
		std::list<class Label*> textList;

		std::wstring input;

	};

	class ConsoleVariableManager {
		friend class ConsoleVariableBase;
	public:
		static ConsoleVariableBase* find(const char* name);
	private:
		static std::vector<ConsoleVariableBase*> registry;
	};
	
	class ConsoleVariableBase {
		friend class ConsoleVariableManager;
	public:
		virtual void print(ConsoleWindow* window) const = 0;
		virtual void parse(const char* msg, ConsoleWindow* window) = 0;
		virtual int32_t getInt() const = 0;
		virtual float getFloat() const = 0;
	protected:
		ConsoleVariableBase();
		std::string name;
	};

	template<typename T>
	class ConsoleVariable : public ConsoleVariableBase {
	public:
		ConsoleVariable(const char* varName, T defaultValue, const char* help)
			: help(help)
			, value(defaultValue)
		{
			name = varName;
		}
		T getValue() const { return value; }
		void setValue(T newValue) { value = newValue; }
		virtual void print(ConsoleWindow* window) const override {}
		virtual void parse(const char* msg, ConsoleWindow* window) override {}
		virtual int32_t getInt() const override { return static_cast<int32_t>(value); }
		virtual float getFloat() const override { return static_cast<float>(value); }
	private:
		std::string help;
		T value;
		
	};

	template<>
	virtual void ConsoleVariable<float>::print(ConsoleWindow* window) const override {
		wchar_t buffer[256];
		swprintf_s(buffer, L"> %f", value);
		window->addLine(buffer);
	}
	template<>
	virtual void ConsoleVariable<int32_t>::print(ConsoleWindow* window) const override {
		wchar_t buffer[256];
		swprintf_s(buffer, L"> %d", value);
		window->addLine(buffer);
	}

	template<>
	virtual void ConsoleVariable<float>::parse(const char* msg, ConsoleWindow* window) override {
		float newValue;
		if (sscanf_s(msg, "%f", &newValue) == 1) {
			setValue(newValue);
		} else {
			window->addLine(L"Failed to set: not a float value.");
		}
	}
	template<>
	virtual void ConsoleVariable<int32_t>::parse(const char* msg, ConsoleWindow* window) override {
		int32_t newValue;
		if (sscanf_s(msg, "%d", &newValue) == 1) {
			setValue(newValue);
		} else {
			window->addLine(L"Failed to set: not an integer value.");
		}
	}

}
