#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui_internal.h>
#include "imgui_settings.h"
#include "imstb_textedit.h"

#include <cstdlib>
#include <imgui.h>

#include <D3DX11tex.h>
#pragma comment(lib, "D3DX11.lib")

#include <map>

#include <string>
#include <wtypes.h>

namespace edited
{
    bool           BeginChild(ImGuiID id, const ImVec2& size = ImVec2(0, 0), const ImVec2& spacing = ImVec2(10, 10), bool bg = false, ImGuiChildFlags child_flags = 0, ImGuiWindowFlags window_flags = 0);
    bool           BeginChild(const char* str_id, const ImVec2& size = ImVec2(0, 0), const ImVec2& spacing = ImVec2(10, 10), bool bg = false, ImGuiChildFlags child_flags = 0, ImGuiWindowFlags window_flags = 0);
    void           EndChild();
    bool           BeginChildEx(const char* name, ImGuiID id, const ImVec2& size_arg, const ImVec2& spacing, bool bg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags);

    bool           Spoof_panel(ImTextureID id_image, bool selected, const char* name, const char* info, const char* version, ImVec2 size, ImVec2 icon_size, int status);

    bool           Checkbox(const char* label, bool* v);
    bool           IconButton(const char* icon, const ImVec2& size_arg, ImGuiButtonFlags flags);


    bool           Button(const char* label, const ImVec2& size_arg = ImVec2(0, 0), ImGuiButtonFlags flags = 0);

    bool           page(bool selected, const char* icon, const ImVec2& size_arg);

    void           info_bar(const char* name, const char* description);
    void           comments(const char* upd, const char* description);
    void           question_box(const char* question_count, const char* question_status);

    void           TextCenter(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, const char* text, const ImVec2& align);

    void           Separator_line();

  // bool           Tab(bool selected, ImGuiID id, const char* icon, const ImVec2& size_arg);
  // bool           Checkbox(const char* label, bool* v);
  // void           CheckboxClicked(const char* label, bool* v);
  // bool           CheckboxPicker(const char* label, bool* v, float col[3], ImGuiColorEditFlags flags);
  // bool           CheckboxDoublePicker(const char* label, bool* v, float col1[3], float col2[3], ImGuiColorEditFlags flags);
  // void           ComboClicked(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items);
  // void           MultiComboClicked(const char* label, bool variable[], const char* labels[], int count);
  // bool           SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
  // bool           SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
  // bool           SliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, ImGuiSliderFlags flags = 0);
  // bool           RangeSliderBehavior(const ImRect& frame_bb, ImGuiID id, float* v1, float* v2, float v_min, float v_max, float power, int decimal_precision, ImGuiSliderFlags flags = 0);
  // bool           RangeSliderFloat(const char* label, float* v1, float* v2, float v_min, float v_max, const char* display_format = "(%.3f, %.3f)", float power = 1.0f);
  // bool           ColorEdit4(const char* label, float col[4], ImGuiColorEditFlags flags = 0);
  // bool           ColorPicker4(const char* label, float col[4], ImGuiColorEditFlags flags = 0, const float* ref_col = NULL);
  // bool           ColorButton(const char* desc_id, const ImVec4& col, ImGuiColorEditFlags flags = 0, const ImVec2& size = ImVec2(0, 0));
  // bool		   Selectable(const char* label, bool selected = false, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0));
  // bool		   Selectable(const char* label, bool* p_selected, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0));
  // bool		   BeginCombo(const char* label, const char* preview_value, int val = 0, bool multi = false, ImGuiComboFlags flags = 0);
  // void		   EndCombo();
  // void		   MultiCombo(const char* label, bool variable[], const char* labels[], int count);
  // bool		   Combo(const char* label, int* current_item, const char* (*getter)(void* user_data, int idx), void* user_data, int items_count, int popup_max_height_in_items = -1);
  // bool		   Combo(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1);
  // bool		   Combo(const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items = -1);
}