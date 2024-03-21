#include "imgui.h"

namespace c
{

	inline ImVec4 accent = ImColor(171, 189, 255);

	inline ImVec4 green = ImColor(49, 255, 57);
	inline ImVec4 orange = ImColor(255, 173, 49);
	inline ImVec4 red = ImColor(255, 49, 49);

	namespace background
	{
		inline ImVec4 filling = ImColor(24, 25, 29);
		inline ImVec4 stroke = ImColor(31, 35, 49);
		inline ImVec2 size = ImVec2(750, 450);

		inline float rounding = 8.f;
	}

	namespace element
	{
		inline ImVec4 filling = ImColor(26, 28, 35, 125);
		inline ImVec4 stroke = ImColor(47, 53, 75, 80);
		inline ImVec4 panel = ImColor(0, 0, 0, 45);

		inline float rounding = 8.f;
	}
	
	namespace text
	{
		inline ImVec4 text_active = ImColor(255, 255, 255);
		inline ImVec4 text_hov = ImColor(92, 107, 161, 125);
		inline ImVec4 text = ImColor(66, 72, 93);
	}

}