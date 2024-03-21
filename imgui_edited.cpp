#include "imgui_edited.hpp"
#include <vector>

using namespace ImGui;

namespace font
{
    extern ImFont* icomoon[19];
    extern ImFont* lexend_medium_x;
}

namespace image
{

}

namespace edited
{
    bool BeginChild(const char* str_id, const ImVec2& size_arg, const ImVec2& spacing, bool bg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags)
    {
        ImGuiID id = GetCurrentWindow()->GetID(str_id);

        PushStyleVar(ImGuiStyleVar_ItemSpacing, spacing);
        PushStyleVar(ImGuiStyleVar_WindowPadding, spacing);

        return BeginChildEx(str_id, id, size_arg, spacing, bg, child_flags, window_flags | ImGuiWindowFlags_AlwaysUseWindowPadding);
    }

    bool BeginChild(ImGuiID id, const ImVec2& size_arg, const ImVec2& spacing, bool bg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags)
    {
        return BeginChildEx(NULL, id, size_arg, spacing, bg, child_flags, window_flags);
    }

    bool BeginChildEx(const char* name, ImGuiID id, const ImVec2& size_arg, const ImVec2& spacing, bool bg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags)
    {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* parent_window = g.CurrentWindow;
        IM_ASSERT(id != 0);

        // Sanity check as it is likely that some user will accidentally pass ImGuiWindowFlags into the ImGuiChildFlags argument.
        const ImGuiChildFlags ImGuiChildFlags_SupportedMask_ = ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_FrameStyle;
        IM_UNUSED(ImGuiChildFlags_SupportedMask_);
        IM_ASSERT((child_flags & ~ImGuiChildFlags_SupportedMask_) == 0 && "Illegal ImGuiChildFlags value. Did you pass ImGuiWindowFlags values instead of ImGuiChildFlags?");
        IM_ASSERT((window_flags & ImGuiWindowFlags_AlwaysAutoResize) == 0 && "Cannot specify ImGuiWindowFlags_AlwaysAutoResize for BeginChild(). Use ImGuiChildFlags_AlwaysAutoResize!");
        if (child_flags & ImGuiChildFlags_AlwaysAutoResize)
        {
            IM_ASSERT((child_flags & (ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY)) == 0 && "Cannot use ImGuiChildFlags_ResizeX or ImGuiChildFlags_ResizeY with ImGuiChildFlags_AlwaysAutoResize!");
            IM_ASSERT((child_flags & (ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY)) != 0 && "Must use ImGuiChildFlags_AutoResizeX or ImGuiChildFlags_AutoResizeY with ImGuiChildFlags_AlwaysAutoResize!");
        }
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
        if (window_flags & ImGuiWindowFlags_AlwaysUseWindowPadding)
            child_flags |= ImGuiChildFlags_AlwaysUseWindowPadding;
#endif
        if (child_flags & ImGuiChildFlags_AutoResizeX)
            child_flags &= ~ImGuiChildFlags_ResizeX;
        if (child_flags & ImGuiChildFlags_AutoResizeY)
            child_flags &= ~ImGuiChildFlags_ResizeY;

        // Set window flags
        window_flags |= ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_NoTitleBar;
        window_flags |= (parent_window->Flags & ImGuiWindowFlags_NoMove); // Inherit the NoMove flag
        if (child_flags & (ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize))
            window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
        if ((child_flags & (ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY)) == 0)
            window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

        // Special framed style
        if (child_flags & ImGuiChildFlags_FrameStyle)
        {
            PushStyleColor(ImGuiCol_ChildBg, g.Style.Colors[ImGuiCol_FrameBg]);
            PushStyleVar(ImGuiStyleVar_ChildRounding, g.Style.FrameRounding);
            PushStyleVar(ImGuiStyleVar_ChildBorderSize, g.Style.FrameBorderSize);
            PushStyleVar(ImGuiStyleVar_WindowPadding, g.Style.FramePadding);
            child_flags |= ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysUseWindowPadding;
            window_flags |= ImGuiWindowFlags_NoMove;
        }

        // Forward child flags
        g.NextWindowData.Flags |= ImGuiNextWindowDataFlags_HasChildFlags;
        g.NextWindowData.ChildFlags = child_flags;

        // Forward size
        // Important: Begin() has special processing to switch condition to ImGuiCond_FirstUseEver for a given axis when ImGuiChildFlags_ResizeXXX is set.
        // (the alternative would to store conditional flags per axis, which is possible but more code)
        const ImVec2 size_avail = GetContentRegionAvail();
        const ImVec2 size_default((child_flags & ImGuiChildFlags_AutoResizeX) ? 0.0f : size_avail.x, (child_flags & ImGuiChildFlags_AutoResizeY) ? 0.0f : size_avail.y);
        const ImVec2 size = CalcItemSize(size_arg, size_default.x, size_default.y);
        SetNextWindowSize(size);

        if (bg) {
            GetWindowDrawList()->AddRectFilled(parent_window->DC.CursorPos, parent_window->DC.CursorPos + size_arg, GetColorU32(c::element::panel), c::element::rounding);
            GetWindowDrawList()->AddRect(parent_window->DC.CursorPos, parent_window->DC.CursorPos + size_arg, GetColorU32(c::element::stroke), c::element::rounding);
        }

        const char* temp_window_name;
 
        if (name)
            ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%s_%08X", parent_window->Name, name, id);
        else
            ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%08X", parent_window->Name, id);

        // Set style
        const float backup_border_size = g.Style.ChildBorderSize;
        if ((child_flags & ImGuiChildFlags_Border) == 0)
            g.Style.ChildBorderSize = 0.0f;

        // Begin into window
        const bool ret = Begin(temp_window_name, NULL, window_flags);

        // Restore style
        g.Style.ChildBorderSize = backup_border_size;
        if (child_flags & ImGuiChildFlags_FrameStyle)
        {
            PopStyleVar(3);
            PopStyleColor();
        }

        ImGuiWindow* child_window = g.CurrentWindow;
        child_window->ChildId = id;

        // Set the cursor to handle case where the user called SetNextWindowPos()+BeginChild() manually.
        // While this is not really documented/defined, it seems that the expected thing to do.
        if (child_window->BeginCount == 1)
            parent_window->DC.CursorPos = child_window->Pos;

        // Process navigation-in immediately so NavInit can run on first frame
        // Can enter a child if (A) it has navigable items or (B) it can be scrolled.
        const ImGuiID temp_id_for_activation = ImHashStr("##Child", 0, id);
        if (g.ActiveId == temp_id_for_activation)
            ClearActiveID();
        if (g.NavActivateId == id && !(window_flags & ImGuiWindowFlags_NavFlattened) && (child_window->DC.NavLayersActiveMask != 0 || child_window->DC.NavWindowHasScrollY))
        {
            FocusWindow(child_window);
            NavInitWindow(child_window, false);
            SetActiveID(temp_id_for_activation, child_window); // Steal ActiveId with another arbitrary id so that key-press won't activate child item
            g.ActiveIdSource = g.NavInputSource;
        }
        return ret;
    }

    void EndChild()
    {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* child_window = g.CurrentWindow;

        PopStyleVar(2);

        IM_ASSERT(g.WithinEndChild == false);
        IM_ASSERT(child_window->Flags & ImGuiWindowFlags_ChildWindow);   // Mismatched BeginChild()/EndChild() calls

        g.WithinEndChild = true;
        ImVec2 child_size = child_window->Size;
        End();
        if (child_window->BeginCount == 1)
        {
            ImGuiWindow* parent_window = g.CurrentWindow;
            ImRect bb(parent_window->DC.CursorPos, parent_window->DC.CursorPos + child_size);
            ItemSize(child_size);
            if ((child_window->DC.NavLayersActiveMask != 0 || child_window->DC.NavWindowHasScrollY) && !(child_window->Flags & ImGuiWindowFlags_NavFlattened))
            {
                ItemAdd(bb, child_window->ChildId);
                RenderNavHighlight(bb, child_window->ChildId);

                // When browsing a window that has no activable items (scroll only) we keep a highlight on the child (pass g.NavId to trick into always displaying)
                if (child_window->DC.NavLayersActiveMask == 0 && child_window == g.NavWindow)
                    RenderNavHighlight(ImRect(bb.Min - ImVec2(2, 2), bb.Max + ImVec2(2, 2)), g.NavId, ImGuiNavHighlightFlags_TypeThin);
            }
            else
            {
                // Not navigable into
                ItemAdd(bb, 0);

                // But when flattened we directly reach items, adjust active layer mask accordingly
                if (child_window->Flags & ImGuiWindowFlags_NavFlattened)
                    parent_window->DC.NavLayersActiveMaskNext |= child_window->DC.NavLayersActiveMaskNext;
            }
            if (g.HoveredWindow == child_window)
                g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HoveredWindow;
        }
        g.WithinEndChild = false;
        g.LogLinePosY = -FLT_MAX; // To enforce a carriage return
    }

    struct spoofer_state
    {
        ImVec4 stroke;
    };

    bool Spoof_panel(ImTextureID id_image, bool selected, const char* name, const char* info, const char* version, ImVec2 size, ImVec2 icon_size, int status)
    {
        ImGuiWindow* window = GetCurrentWindow();

        if (window->SkipItems) return false;

        const ImVec2 pos = window->DC.CursorPos;
        const ImRect rect(pos, pos + size);
        const ImGuiID id = window->GetID(name);

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;

        ItemSize(rect, 0.f);
        if (!ItemAdd(rect, id)) return false;

        bool hovered, held, pressed = ButtonBehavior(rect, id, &hovered, &held, NULL);

        static std::map<ImGuiID, spoofer_state> anim;
        spoofer_state& state = anim[id];

        state.stroke = ImLerp(state.stroke, selected ? c::accent : c::element::stroke, g.IO.DeltaTime * 6.f);

        GetWindowDrawList()->AddRectFilled(rect.Min, rect.Max, GetColorU32(c::element::filling), c::element::rounding);
        GetWindowDrawList()->AddRect(rect.Min, rect.Max, GetColorU32(state.stroke), c::element::rounding);

        const ImVec4 status_select[3] = { c::green, c::orange, c::red };

        GetWindowDrawList()->AddCircleFilled(rect.Min + ImVec2(size.x - 18, 18), 3.f, GetColorU32(status_select[status]), 100.f);
        GetWindowDrawList()->AddCircleFilled(rect.Min + ImVec2(size.x - 18, 18), 6.f, GetColorU32(status_select[status], 0.2f), 100.f);

        GetWindowDrawList()->AddText(rect.Min + ImVec2(30, (size.y - (CalcTextSize(info).y - 30)) / 2), GetColorU32(c::text::text), info);
        GetWindowDrawList()->AddText(rect.Min + ImVec2(30, (size.y - (CalcTextSize(name).y + 30)) / 2), GetColorU32(c::text::text_active), name);

        GetWindowDrawList()->AddText(rect.Min + ImVec2(40 + CalcTextSize(name).x, (size.y - (CalcTextSize(name).y + 31)) / 2), GetColorU32(c::text::text_hov), version);

        GetWindowDrawList()->AddImage(id_image, rect.Min + ImVec2(size.x - (icon_size.x + 30), (size.y - icon_size.y) / 2), rect.Max - ImVec2(30, (size.y - icon_size.y) / 2), ImVec2(0, 0), ImVec2(1, 1), GetColorU32(c::accent));

        return pressed;
    }

    struct button_state
    {
        ImVec4 background, text;
        float alpha;
    };

    bool Button(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
    {
        ImGuiWindow* window = GetCurrentWindow();

        if (window->SkipItems) return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        const ImVec2 label_size = CalcTextSize(label, NULL, true), pos = window->DC.CursorPos;

        ImVec2 size = CalcItemSize(size_arg, label_size.x, label_size.y);

        const ImRect bb(pos, pos + size);

        ItemSize(size, 0.f);
        if (!ItemAdd(bb, id)) return false;

        bool hovered, held, pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

        static std::map<ImGuiID, button_state> anim;
        button_state& state = anim[id];

        state.background = ImLerp(state.background, IsItemActive() || hovered ? c::accent : c::element::filling, g.IO.DeltaTime * 6.f);

        GetWindowDrawList()->AddRectFilled(bb.Min, bb.Max, GetColorU32(state.background), 2.f);
        GetWindowDrawList()->AddRect(bb.Min, bb.Max, GetColorU32(c::element::stroke), 2.f);

        PushFont(font::lexend_medium_x);
        GetWindowDrawList()->AddText(ImVec2(bb.Min.x + (size.x - CalcTextSize(label).x) / 2, bb.Max.y - CalcTextSize(label).y - (size.y - CalcTextSize(label).y) / 2 - 1), GetColorU32(c::text::text_active), label);
        PopFont();

        return pressed;
    }

    bool IconButton(const char* icon, const ImVec2& size_arg, ImGuiButtonFlags flags)
    {
        ImGuiWindow* window = GetCurrentWindow();

        if (window->SkipItems) return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(icon);
        const ImVec2 label_size = CalcTextSize(icon, NULL, true), pos = window->DC.CursorPos;

        ImVec2 size = CalcItemSize(size_arg, label_size.x, label_size.y);

        const ImRect bb(pos, pos + size);

        ItemSize(size, 0.f);
        if (!ItemAdd(bb, id)) return false;

        bool hovered, held, pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

        static std::map<ImGuiID, button_state> anim;
        button_state& state = anim[id];

        state.text = ImLerp(state.text, IsItemActive() || hovered ? c::text::text_active : c::text::text_hov, g.IO.DeltaTime * 6.f);

        PushFont(font::icomoon[25]);
        GetWindowDrawList()->AddText(ImVec2(bb.Min.x + (size.x - CalcTextSize(icon).x) / 2, bb.Max.y - CalcTextSize(icon).y - (size.y - CalcTextSize(icon).y) / 2), GetColorU32(state.text), icon);
        PopFont();

        return pressed;
    }

    struct page_state
    {
        ImVec4 background, text;
        float alpha;
    };

    bool page(bool selected, const char* icon, const ImVec2& size_arg)
    {
        ImGuiWindow* window = GetCurrentWindow();

        if (window->SkipItems) return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(icon);
        const ImVec2 label_size = CalcTextSize(icon, NULL, true), pos = window->DC.CursorPos;

        ImVec2 size = CalcItemSize(size_arg, label_size.x, label_size.y);

        const ImRect bb(pos, pos + size);

        PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(20, 20));
        ItemSize(size, 0.f);
        PopStyleVar();

        if (!ItemAdd(bb, id)) return false;


        bool hovered, held, pressed = ButtonBehavior(bb, id, &hovered, &held, NULL);

        static std::map<ImGuiID, page_state> anim;
        page_state& state = anim[id];

        state.text = ImLerp(state.text, selected ? c::accent : c::text::text_hov, g.IO.DeltaTime * 6.f);
        state.alpha = ImLerp(state.alpha, selected ? 1.f : 0.0f, g.IO.DeltaTime * 6.f);

        GetWindowDrawList()->AddRectFilled(bb.Min, bb.Max, GetColorU32(c::element::filling), 4.f);
        GetWindowDrawList()->AddRect(bb.Min, bb.Max, GetColorU32(c::accent, state.alpha), 4.f);

        PushFont(font::icomoon[22]);
        GetWindowDrawList()->AddText(ImVec2(bb.Min.x + (size.x - CalcTextSize(icon).x) / 2, bb.Max.y - CalcTextSize(icon).y - (size.y - CalcTextSize(icon).y) / 2), GetColorU32(state.text), icon);
        PopFont();

        return pressed;
    }

    void TextCenter(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, const char* text, const ImVec2& align)
    {

        PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(col));
        RenderTextClipped(p_min, p_max, text, NULL, NULL, align, NULL);
        PopStyleColor();
    }

    void Separator_line()
    {
        GetWindowDrawList()->AddRectFilled(GetCursorScreenPos(), GetCursorScreenPos() + ImVec2(GetContentRegionMax().x - GetStyle().WindowPadding.x, 1), GetColorU32(c::element::stroke));
        Spacing();
    }

    void info_bar(const char* name, const char* description)
    {
        ImGuiWindow* window = GetCurrentWindow();

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(name);
        const ImVec2 label_size = CalcTextSize(name, NULL, true), pos = window->DC.CursorPos;

        ImVec2 size = CalcItemSize(ImVec2(GetContentRegionMax().x - style.WindowPadding.x, CalcTextSize(name).y), label_size.x, label_size.y);

        const ImRect bb(pos, pos + size);

        ItemSize(size, 0.f);

        TextCenter(bb.Min, bb.Max, GetColorU32(c::text::text_active), name, ImVec2(0.0, 0.5));

        TextCenter(bb.Min, bb.Max, GetColorU32(c::text::text), description, ImVec2(1.0, 0.5));
    }

    void comments(const char* upd, const char* description)
    {
        ImGuiWindow* window = GetCurrentWindow();

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(description);
        const ImVec2 label_size = CalcTextSize(description, NULL, true), pos = window->DC.CursorPos;

        ImVec2 size = CalcItemSize(ImVec2(GetContentRegionMax().x - style.WindowPadding.x, CalcTextSize(description).y + 47), label_size.x, label_size.y);

        const ImRect bb(pos, pos + size);

        ItemSize(size, 0.f);

        GetWindowDrawList()->AddRectFilled(bb.Min, bb.Max, GetColorU32(c::element::filling), 4.f);
        GetWindowDrawList()->AddRect(bb.Min, bb.Max, GetColorU32(c::element::stroke), 4.f);

        GetWindowDrawList()->AddText(bb.Min + ImVec2(10, 10), GetColorU32(c::accent), "Updated:");
        GetWindowDrawList()->AddText(bb.Min + ImVec2(CalcTextSize("Updated:").x + 15, 10), GetColorU32(c::text::text_hov), upd);

        PushFont(font::lexend_medium_x);
        TextCenter(bb.Min + ImVec2(10, 35), bb.Max, GetColorU32(c::text::text_active), description, ImVec2(0.0, 0.0));
        PopFont();
    }

    void question_box(const char* question_count, const char* question_status)
    {
        ImGuiWindow* window = GetCurrentWindow();

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(question_count);
        const ImVec2 label_size = CalcTextSize(question_count, NULL, true), pos = window->DC.CursorPos;
        
        ImVec2 size = CalcItemSize(ImVec2(GetContentRegionMax().x - style.WindowPadding.x, CalcTextSize(question_status).y + 47), label_size.x, label_size.y);

        const ImRect bb(pos, pos + size);

        ItemSize(size, 0.f);

        GetWindowDrawList()->AddRectFilled(bb.Min, bb.Max, GetColorU32(c::element::filling), 4.f);
        GetWindowDrawList()->AddRect(bb.Min, bb.Max, GetColorU32(c::element::stroke), 4.f);

        GetWindowDrawList()->AddText(bb.Min + ImVec2(10, 10), GetColorU32(c::accent), "Question from:");
        GetWindowDrawList()->AddText(bb.Min + ImVec2(CalcTextSize("Question from:").x + 15, 10), GetColorU32(c::text::text_hov), question_count);

        PushFont(font::lexend_medium_x);

        std::string convert_string = question_status, first_eight = convert_string.substr(0, 25);

        TextCenter(bb.Min + ImVec2(10, 35), bb.Max, GetColorU32(c::text::text_active), first_eight.c_str(), ImVec2(0.0, 0.0));

        GetWindowDrawList()->AddText(bb.Min + ImVec2(CalcTextSize(first_eight.c_str()).x + 10, 35), GetColorU32(c::text::text_active), " . . .");

        PopFont();

    }

    struct check_state
    {
        ImVec4 background, circle, text;
        float background_opticaly, circle_offset;
    };

    bool Checkbox(const char* label, bool* v)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        const ImVec2 label_size = CalcTextSize(label, NULL, true);

        const float square_sz = 20, y_size = 20;

        const ImVec2 pos = window->DC.CursorPos;

        const float w = GetContentRegionMax().x - style.WindowPadding.x;

        const ImRect total_bb(pos + ImVec2(0, 0), pos + ImVec2(w, y_size));
        ItemSize(total_bb, 0.f);

        if (!ItemAdd(total_bb, id)) return false;

        bool hovered, held, pressed = ButtonBehavior(total_bb, id, &hovered, &held);

        static std::map<ImGuiID, check_state> anim;
        check_state& state = anim[id];

        state.background_opticaly = ImLerp(state.background_opticaly, *v ? 0.2f : 1.0f, g.IO.DeltaTime * 6.f);
        state.circle_offset = ImLerp(state.circle_offset, *v ? 0 : -square_sz * 2, g.IO.DeltaTime * 6.f);
        state.background = ImLerp(state.background, *v ? c::accent : c::element::filling, g.IO.DeltaTime * 6.f);
        state.circle = ImLerp(state.circle, *v ? c::accent : c::text::text, g.IO.DeltaTime * 6.f);
        state.text = ImLerp(state.text, *v || hovered ? c::text::text_active : c::text::text, g.IO.DeltaTime * 6.f);

        if (IsItemClicked())
        {
            *v = !(*v);
            MarkItemEdited(id);
        }

        const ImRect check_bb(pos + ImVec2(w - (square_sz * 2), (y_size - square_sz) / 2), pos + ImVec2(w, (y_size + square_sz) / 2));

        GetWindowDrawList()->AddRectFilled(check_bb.Min, check_bb.Max, GetColorU32(state.background, state.background_opticaly), 30);
        GetWindowDrawList()->AddRect(check_bb.Min, check_bb.Max, GetColorU32(c::element::stroke), 30);

        GetWindowDrawList()->AddCircleFilled(check_bb.Max - ImVec2(y_size - state.circle_offset, y_size) / 2, 5.f, GetColorU32(state.circle), 30);

        GetWindowDrawList()->AddText(pos + ImVec2(w - (55 + CalcTextSize(*v ? "ON" : "OFF").x), (y_size - CalcTextSize("OFF").y) / 2 - 2), GetColorU32(state.text), *v ? "ON" : "OFF");

        GetWindowDrawList()->AddText(pos + ImVec2(0, (y_size - CalcTextSize(label).y) / 2 - 2), GetColorU32(state.text), label);

        return pressed;
    }

}