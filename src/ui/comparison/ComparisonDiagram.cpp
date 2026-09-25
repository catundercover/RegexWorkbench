// Implements set-diagram geometry and rendering for language relations.
#include "ui/comparison/ComparisonDiagram.hpp"

#include "ui/comparison/ComparisonModel.hpp"
#include "ui/style/ApplicationTheme.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <imgui.h>
#include <vector>

namespace ui::comparison
{
    namespace
    {
        // Fill colors used for each region of a set diagram.
        struct ColorScheme
        {
            ImVec4 left;
            ImVec4 right;
            ImVec4 overlap;
        };

        // Bundles a comparison model with the palette selected for its relation.
        struct DiagramData
        {
            const ComparisonModel& model;
            ColorScheme colors;
        };

        // Applies diagram opacity without changing a semantic theme color's hue.
        ImVec4 with_alpha(ImVec4 color, const float alpha)
        {
            color.w = alpha;
            return color;
        }

        // Uses the same base colors as the comparison inputs and witness cards.
        ColorScheme color_scheme()
        {
            const style::ApplicationPalette& palette = style::application_palette();
            return ColorScheme{
                with_alpha(palette.primary, 0.52F),
                with_alpha(palette.accent, 0.52F),
                with_alpha(palette.success, 0.76F)
            };
        }

        // Converts a floating-point color to Dear ImGui's packed representation.
        ImU32 to_u32(const ImVec4& color)
        {
            return ImGui::ColorConvertFloat4ToU32(color);
        }

        // Converts a fill color to an opaque-enough diagram border.
        ImU32 to_border_u32(ImVec4 color)
        {
            color.w = 230.0f / 255.0f;
            return to_u32(color);
        }

        // Returns a theme-aware text color for diagram labels.
        ImU32 diagram_text_color()
        {
            return ImGui::GetColorU32(ImGuiCol_Text);
        }

        // Draws one filled and optionally bordered ellipse.
        void draw_oval(
            ImDrawList* draw_list,
            const ImVec2& center,
            float radius_x,
            float radius_y,
            ImU32 fill_color,
            float thickness = 2.0f,
            ImU32 border_color = 0
        )
        {
            draw_list->AddEllipseFilled(center, ImVec2(radius_x, radius_y), fill_color, 0.0f, 96);

            if (thickness > 0.0f && border_color != 0)
            {
                draw_list->AddEllipse(
                    center, ImVec2(radius_x, radius_y), border_color, 0.0f, 96, thickness
                );
            }
        }

        // Draws the exact lens shared by two equal, horizontally offset ellipses.
        void draw_overlap_lens(
            ImDrawList* draw_list,
            const ImVec2& left_center,
            const ImVec2& right_center,
            const float radius_x,
            const float radius_y,
            const ImU32 fill_color
        )
        {
            constexpr std::size_t ArcSegments = 32;
            constexpr float Pi = 3.14159265358979323846F;
            const float center_offset = (right_center.x - left_center.x) * 0.5F;
            if (radius_x <= 0.0F || radius_y <= 0.0F || center_offset >= radius_x)
            {
                return;
            }

            const float intersection_angle =
                std::acos(std::clamp(center_offset / radius_x, 0.0F, 1.0F));
            std::vector<ImVec2> points;
            points.reserve(ArcSegments * 2 + 2);
            for (std::size_t index = 0; index <= ArcSegments; ++index)
            {
                const float progress = static_cast<float>(index) / static_cast<float>(ArcSegments);
                const float angle = -intersection_angle + 2.0F * intersection_angle * progress;
                points.emplace_back(
                    left_center.x + radius_x * std::cos(angle),
                    left_center.y + radius_y * std::sin(angle)
                );
            }
            for (std::size_t index = 0; index <= ArcSegments; ++index)
            {
                const float progress = static_cast<float>(index) / static_cast<float>(ArcSegments);
                const float angle = Pi - intersection_angle + 2.0F * intersection_angle * progress;
                points.emplace_back(
                    right_center.x + radius_x * std::cos(angle),
                    right_center.y + radius_y * std::sin(angle)
                );
            }
            draw_list->AddConvexPolyFilled(
                points.data(), static_cast<int>(points.size()), fill_color
            );
        }

        // Draws text centered on a screen-space point.
        void draw_text_centered(
            ImDrawList* draw_list, const char* text, const ImVec2& position, ImU32 color
        )
        {
            ImVec2 text_size = ImGui::CalcTextSize(text);
            ImVec2 text_pos = ImVec2(position.x - text_size.x / 2, position.y - text_size.y / 2);
            draw_list->AddText(text_pos, color, text);
        }

        // Draws the witness label for the region outside both languages.
        void draw_neither_label(
            ImDrawList* draw_list, const ImVec2& center, float radius, const DiagramData& data
        )
        {
            if (!data.model.neither.has_value())
            {
                return;
            }

            ImVec2 neither_pos = ImVec2(center.x, center.y + radius * 1.05f);
            draw_text_centered(
                draw_list,
                data.model.neither->label.c_str(),
                neither_pos,
                ImGui::GetColorU32(ImGuiCol_Text)
            );
        }

        // Draws coincident language sets for equivalent expressions.
        void draw_equivalent(
            ImDrawList* draw_list, const ImVec2& center, float radius, const DiagramData& data
        )
        {
            draw_oval(
                draw_list,
                center,
                radius,
                radius * 0.65f,
                to_u32(data.colors.overlap),
                2.0f,
                to_border_u32(data.colors.overlap)
            );

            if (data.model.intersection.has_value())
            {
                draw_text_centered(
                    draw_list, data.model.intersection->label.c_str(), center, diagram_text_color()
                );
            }
        }

        // Draws two regions that partition the universe for complementary languages.
        void draw_complement(
            ImDrawList* draw_list, const ImVec2& center, float radius, const DiagramData& data
        )
        {
            float width = radius * 2.7f;
            float height = radius * 1.45f;

            ImVec2 top_left = ImVec2(center.x - width / 2.0f, center.y - height / 2.0f);
            ImVec2 bottom_right = ImVec2(center.x + width / 2.0f, center.y + height / 2.0f);
            ImVec2 middle_top = ImVec2(center.x, top_left.y);
            ImVec2 middle_bottom = ImVec2(center.x, bottom_right.y);

            draw_list->AddRectFilled(
                top_left,
                ImVec2(center.x, bottom_right.y),
                to_u32(data.colors.left),
                6.0f,
                ImDrawFlags_RoundCornersLeft
            );

            draw_list->AddRectFilled(
                ImVec2(center.x, top_left.y),
                bottom_right,
                to_u32(data.colors.right),
                6.0f,
                ImDrawFlags_RoundCornersRight
            );

            draw_list->AddLine(middle_top, middle_bottom, IM_COL32(80, 80, 90, 210), 2.0f);

            draw_list->AddRect(top_left, bottom_right, IM_COL32(80, 80, 90, 210), 6.0f, 0, 2.0f);

            if (data.model.only_left.has_value())
            {
                ImVec2 left_pos = ImVec2(center.x - width * 0.25f, center.y);
                draw_text_centered(
                    draw_list, data.model.only_left->label.c_str(), left_pos, diagram_text_color()
                );
            }

            if (data.model.only_right.has_value())
            {
                ImVec2 right_pos = ImVec2(center.x + width * 0.25f, center.y);
                draw_text_centered(
                    draw_list, data.model.only_right->label.c_str(), right_pos, diagram_text_color()
                );
            }
        }

        // Draws two separated sets for disjoint languages.
        void draw_disjoint(
            ImDrawList* draw_list, const ImVec2& center, float radius, const DiagramData& data
        )
        {
            float oval_radius_x = radius * 0.72f;
            float oval_radius_y = radius * 0.58f;
            float offset = radius * 0.78f;

            ImVec2 left_center = ImVec2(center.x - offset, center.y);
            ImVec2 right_center = ImVec2(center.x + offset, center.y);

            draw_oval(
                draw_list,
                left_center,
                oval_radius_x,
                oval_radius_y,
                to_u32(data.colors.left),
                2.0f,
                to_border_u32(data.colors.left)
            );

            draw_oval(
                draw_list,
                right_center,
                oval_radius_x,
                oval_radius_y,
                to_u32(data.colors.right),
                2.0f,
                to_border_u32(data.colors.right)
            );

            if (data.model.only_left.has_value())
            {
                draw_text_centered(
                    draw_list,
                    data.model.only_left->label.c_str(),
                    left_center,
                    diagram_text_color()
                );
            }

            if (data.model.only_right.has_value())
            {
                draw_text_centered(
                    draw_list,
                    data.model.only_right->label.c_str(),
                    right_center,
                    diagram_text_color()
                );
            }
        }

        // Draws one language nested inside the other.
        void draw_subset(
            ImDrawList* draw_list,
            const ImVec2& center,
            float radius,
            const DiagramData& data,
            bool left_inside_right
        )
        {
            ImVec2 outer_center = center;
            ImVec2 inner_center = ImVec2(center.x - radius * 0.22f, center.y);
            float radius_y = radius * 0.65f;

            const ImVec4& outer_color = left_inside_right ? data.colors.right : data.colors.left;
            const ImVec4& inner_color = left_inside_right ? data.colors.left : data.colors.right;

            draw_oval(
                draw_list,
                outer_center,
                radius,
                radius_y,
                to_u32(outer_color),
                2.0f,
                to_border_u32(outer_color)
            );

            float inner_radius = radius * 0.45f;

            draw_oval(
                draw_list,
                inner_center,
                inner_radius,
                inner_radius * 0.65f,
                to_u32(data.colors.overlap),
                2.0f,
                to_border_u32(inner_color)
            );

            if (data.model.intersection.has_value())
            {
                draw_text_centered(
                    draw_list,
                    data.model.intersection->label.c_str(),
                    inner_center,
                    diagram_text_color()
                );
            }

            if (left_inside_right && data.model.only_right.has_value())
            {
                ImVec2 outside_pos = ImVec2(center.x + radius * 0.48f, center.y);
                draw_text_centered(
                    draw_list,
                    data.model.only_right->label.c_str(),
                    outside_pos,
                    diagram_text_color()
                );
            }
            else if (!left_inside_right && data.model.only_left.has_value())
            {
                ImVec2 outside_pos = ImVec2(center.x + radius * 0.48f, center.y);
                draw_text_centered(
                    draw_list,
                    data.model.only_left->label.c_str(),
                    outside_pos,
                    diagram_text_color()
                );
            }
        }

        // Draws intersecting sets with all three interior regions labeled.
        void draw_overlap(
            ImDrawList* draw_list, const ImVec2& center, float radius, const DiagramData& data
        )
        {
            float offset = radius * 0.42f;
            float radius_y = radius * 0.65f;
            ImVec2 left_center = ImVec2(center.x - offset, center.y);
            ImVec2 right_center = ImVec2(center.x + offset, center.y);

            draw_oval(
                draw_list,
                left_center,
                radius,
                radius_y,
                to_u32(data.colors.left),
                2.0f,
                to_border_u32(data.colors.left)
            );

            draw_oval(
                draw_list,
                right_center,
                radius,
                radius_y,
                to_u32(data.colors.right),
                2.0f,
                to_border_u32(data.colors.right)
            );

            draw_overlap_lens(
                draw_list, left_center, right_center, radius, radius_y, to_u32(data.colors.overlap)
            );
            draw_list->AddEllipse(
                left_center,
                ImVec2(radius, radius_y),
                to_border_u32(data.colors.left),
                0.0F,
                96,
                2.0F
            );
            draw_list->AddEllipse(
                right_center,
                ImVec2(radius, radius_y),
                to_border_u32(data.colors.right),
                0.0F,
                96,
                2.0F
            );

            if (data.model.only_left.has_value())
            {
                ImVec2 left_pos = ImVec2(center.x - radius * 0.9f, center.y);
                draw_text_centered(
                    draw_list, data.model.only_left->label.c_str(), left_pos, diagram_text_color()
                );
            }

            if (data.model.only_right.has_value())
            {
                ImVec2 right_pos = ImVec2(center.x + radius * 0.9f, center.y);
                draw_text_centered(
                    draw_list, data.model.only_right->label.c_str(), right_pos, diagram_text_color()
                );
            }

            if (data.model.intersection.has_value())
            {
                draw_text_centered(
                    draw_list, data.model.intersection->label.c_str(), center, diagram_text_color()
                );
            }
        }

        // Draws the relation-specific diagram in the current ImGui window.
        void draw_diagram(const DiagramData& data, const ImVec2& size)
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();

            const ImU32 background_color = ImGui::GetColorU32(ImGuiCol_ChildBg);

            draw_list->AddRectFilled(
                pos, ImVec2(pos.x + size.x, pos.y + size.y), background_color, 8.0f
            );

            ImVec2 center = ImVec2(pos.x + size.x / 2.0f, pos.y + size.y / 2.0f);
            float radius = std::min(size.x, size.y) * 0.32f;

            switch (data.model.kind)
            {
            case RelationKind::Equivalent:
                draw_equivalent(draw_list, center, radius, data);
                break;
            case RelationKind::Complement:
                draw_complement(draw_list, center, radius, data);
                break;
            case RelationKind::Disjoint:
                draw_disjoint(draw_list, center, radius, data);
                break;
            case RelationKind::LeftSubsetRight:
                draw_subset(draw_list, center, radius, data, true);
                break;
            case RelationKind::RightSubsetLeft:
                draw_subset(draw_list, center, radius, data, false);
                break;
            case RelationKind::Overlap:
                draw_overlap(draw_list, center, radius, data);
                break;
            }

            draw_neither_label(draw_list, center, radius, data);

            draw_list->AddRect(
                pos,
                ImVec2(pos.x + size.x, pos.y + size.y),
                ImGui::GetColorU32(ImGuiCol_Border),
                8.0f,
                0,
                1.0f
            );

            ImGui::Dummy(size);
        }

    }

    void render_comparison_diagram(const ComparisonModel& comparison, const ImVec2& size)
    {
        draw_diagram(DiagramData{comparison, color_scheme()}, size);
    }
}
