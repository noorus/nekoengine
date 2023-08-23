#include "pch.h"
#include "gfx_imguistyle.h"

namespace neko {

  void setImGuiStyle( ImGuiStyle& style )
  {
    style.WindowPadding = { 10, 8 };
    style.FramePadding = { 8, 4 };
    style.CellPadding = { 8, 2 };
    style.ItemSpacing = { 8, 4 };
    style.ItemInnerSpacing = { 8, 4 };
    style.IndentSpacing = 10;
    style.ScrollbarSize = 16;
    style.GrabMinSize = 6;
    style.WindowBorderSize = 1;
    style.ChildBorderSize = 1;
    style.PopupBorderSize = 1;
    style.FrameBorderSize = 0;
    style.TabBorderSize = 1;
    style.WindowRounding = 10;
    style.ChildRounding = 4;
    style.FrameRounding = 2;
    style.PopupRounding = 0;
    style.ScrollbarRounding = 4;
    style.GrabRounding = 4;
    style.LogSliderDeadzone = 4;
    style.TabRounding = 4;
    style.WindowTitleAlign = { 0.03f, 0.5f };
    style.WindowMenuButtonPosition = ImGuiDir_Right;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.SeparatorTextPadding = ImVec2( 16, 4 );

    auto colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4( 0.96f, 0.95f, 0.94f, 1.00f );
    colors[ImGuiCol_TextDisabled] = ImVec4( 0.50f, 0.42f, 0.34f, 0.90f );
    colors[ImGuiCol_WindowBg] = ImVec4( 0.03f, 0.03f, 0.04f, 0.85f );
    colors[ImGuiCol_ChildBg] = ImVec4( 0.00f, 0.00f, 0.00f, 0.41f );
    colors[ImGuiCol_PopupBg] = ImVec4( 0.08f, 0.08f, 0.08f, 0.94f );
    colors[ImGuiCol_Border] = ImVec4( 0.78f, 0.89f, 0.87f, 0.50f );
    colors[ImGuiCol_BorderShadow] = ImVec4( 0.00f, 0.00f, 0.00f, 0.39f );
    colors[ImGuiCol_FrameBg] = ImVec4( 0.14f, 0.31f, 0.34f, 0.54f );
    colors[ImGuiCol_FrameBgHovered] = ImVec4( 0.17f, 0.80f, 0.89f, 0.40f );
    colors[ImGuiCol_FrameBgActive] = ImVec4( 0.25f, 0.87f, 0.96f, 0.55f );
    colors[ImGuiCol_TitleBg] = ImVec4( 0.05f, 0.11f, 0.15f, 1.00f );
    colors[ImGuiCol_TitleBgActive] = ImVec4( 0.00f, 0.25f, 0.23f, 0.91f );
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4( 0.13f, 0.07f, 0.28f, 0.58f );
    colors[ImGuiCol_MenuBarBg] = ImVec4( 0.04f, 0.12f, 0.18f, 0.48f );
    colors[ImGuiCol_ScrollbarBg] = ImVec4( 0.00f, 0.00f, 0.00f, 0.15f );
    colors[ImGuiCol_ScrollbarGrab] = ImVec4( 1.00f, 1.00f, 1.00f, 0.18f );
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4( 1.00f, 1.00f, 1.00f, 0.46f );
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4( 1.00f, 1.00f, 1.00f, 0.59f );
    colors[ImGuiCol_CheckMark] = ImVec4( 0.74f, 1.00f, 0.48f, 1.00f );
    colors[ImGuiCol_SliderGrab] = ImVec4( 0.82f, 1.00f, 0.62f, 1.00f );
    colors[ImGuiCol_SliderGrabActive] = ImVec4( 0.95f, 1.00f, 0.90f, 1.00f );
    colors[ImGuiCol_Button] = ImVec4( 0.34f, 0.88f, 0.84f, 0.40f );
    colors[ImGuiCol_ButtonHovered] = ImVec4( 0.22f, 1.00f, 0.94f, 0.63f );
    colors[ImGuiCol_ButtonActive] = ImVec4( 0.54f, 1.00f, 0.92f, 0.80f );
    colors[ImGuiCol_Header] = ImVec4( 0.26f, 0.59f, 0.98f, 0.31f );
    colors[ImGuiCol_HeaderHovered] = ImVec4( 0.26f, 0.59f, 0.98f, 0.80f );
    colors[ImGuiCol_HeaderActive] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
    colors[ImGuiCol_Separator] = ImVec4( 0.43f, 0.43f, 0.50f, 0.50f );
    colors[ImGuiCol_SeparatorHovered] = ImVec4( 0.10f, 0.40f, 0.75f, 0.78f );
    colors[ImGuiCol_SeparatorActive] = ImVec4( 0.10f, 0.40f, 0.75f, 1.00f );
    colors[ImGuiCol_ResizeGrip] = ImVec4( 0.26f, 0.98f, 0.80f, 0.20f );
    colors[ImGuiCol_ResizeGripHovered] = ImVec4( 0.48f, 0.98f, 0.88f, 0.74f );
    colors[ImGuiCol_ResizeGripActive] = ImVec4( 0.71f, 1.00f, 0.94f, 0.95f );
    colors[ImGuiCol_Tab] = ImVec4( 0.18f, 0.35f, 0.58f, 0.86f );
    colors[ImGuiCol_TabHovered] = ImVec4( 0.26f, 0.59f, 0.98f, 0.80f );
    colors[ImGuiCol_TabActive] = ImVec4( 0.20f, 0.41f, 0.68f, 1.00f );
    colors[ImGuiCol_TabUnfocused] = ImVec4( 0.07f, 0.10f, 0.15f, 0.97f );
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4( 0.14f, 0.26f, 0.42f, 1.00f );
    colors[ImGuiCol_DockingPreview] = ImVec4( 0.26f, 0.59f, 0.98f, 0.70f );
    colors[ImGuiCol_DockingEmptyBg] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
    colors[ImGuiCol_PlotLines] = ImVec4( 0.61f, 0.61f, 0.61f, 1.00f );
    colors[ImGuiCol_PlotLinesHovered] = ImVec4( 1.00f, 0.43f, 0.35f, 1.00f );
    colors[ImGuiCol_PlotHistogram] = ImVec4( 0.90f, 0.70f, 0.00f, 1.00f );
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4( 1.00f, 0.60f, 0.00f, 1.00f );
    colors[ImGuiCol_TableHeaderBg] = ImVec4( 0.19f, 0.19f, 0.20f, 1.00f );
    colors[ImGuiCol_TableBorderStrong] = ImVec4( 0.31f, 0.31f, 0.35f, 1.00f );
    colors[ImGuiCol_TableBorderLight] = ImVec4( 0.23f, 0.23f, 0.25f, 1.00f );
    colors[ImGuiCol_TableRowBg] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
    colors[ImGuiCol_TableRowBgAlt] = ImVec4( 1.00f, 1.00f, 1.00f, 0.06f );
    colors[ImGuiCol_TextSelectedBg] = ImVec4( 0.26f, 0.59f, 0.98f, 0.35f );
    colors[ImGuiCol_DragDropTarget] = ImVec4( 1.00f, 1.00f, 0.00f, 0.90f );
    colors[ImGuiCol_NavHighlight] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4( 1.00f, 1.00f, 1.00f, 0.70f );
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4( 0.80f, 0.80f, 0.80f, 0.20f );
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4( 0.80f, 0.80f, 0.80f, 0.35f );
  }

}