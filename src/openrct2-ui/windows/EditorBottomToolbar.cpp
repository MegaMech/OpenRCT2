/*****************************************************************************
 * Copyright (c) 2014-2019 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "../interface/Theme.h"

#include <openrct2-ui/interface/Widget.h>
#include <openrct2-ui/windows/Window.h>
#include <openrct2/Context.h>
#include <openrct2/Editor.h>
#include <openrct2/EditorObjectSelectionSession.h>
#include <openrct2/Game.h>
#include <openrct2/Input.h>
#include <openrct2/OpenRCT2.h>
#include <openrct2/audio/audio.h>
#include <openrct2/localisation/Localisation.h>
#include <openrct2/management/Research.h>
#include <openrct2/scenario/Scenario.h>
#include <openrct2/sprites.h>
#include <openrct2/windows/Intent.h>
#include <openrct2/world/Park.h>
#include <openrct2/world/Scenery.h>
#include <string>

// clang-format off
enum {
    WIDX_PREVIOUS_IMAGE,        // 1
    WIDX_PREVIOUS_STEP_BUTTON,  // 2
    WIDX_NEXT_IMAGE,            // 4
    WIDX_NEXT_STEP_BUTTON,      // 8
};

static rct_widget window_editor_bottom_toolbar_widgets[] = {
    { WWT_IMGBTN, 0, 0, 199, 0, 33, 0xFFFFFFFF, 0xFFFF },           // 1        0x9A9958
    { WWT_FLATBTN, 0, 2, 197, 2, 31, 0xFFFFFFFF, 0xFFFF },          // 2        0x9A9968
    { WWT_IMGBTN, 0, 440, 639, 0, 33, 0xFFFFFFFF, 0xFFFF },         // 4        0x9A9978
    { WWT_FLATBTN, 0, 442, 637, 2, 31, 0xFFFFFFFF, 0xFFFF },        // 8        0x9A9988
    { WIDGETS_END },
};

static void window_editor_bottom_toolbar_mouseup(rct_window *w, rct_widgetindex widgetIndex);
static void window_editor_bottom_toolbar_invalidate(rct_window *w);
static void window_editor_bottom_toolbar_paint(rct_window *w, rct_drawpixelinfo *dpi);

static void window_editor_bottom_toolbar_jump_back_to_object_selection();
static void window_editor_bottom_toolbar_jump_back_to_landscape_editor();
static void window_editor_bottom_toolbar_jump_back_to_invention_list_set_up();
static void window_editor_bottom_toolbar_jump_back_to_options_selection();

static void window_editor_bottom_toolbar_jump_forward_from_object_selection();
static void window_editor_bottom_toolbar_jump_forward_to_invention_list_set_up();
static void window_editor_bottom_toolbar_jump_forward_to_options_selection();
static void window_editor_bottom_toolbar_jump_forward_to_objective_selection();
static void window_editor_bottom_toolbar_jump_forward_to_save_scenario();

static rct_window_event_list window_editor_bottom_toolbar_events = {
    nullptr,
    window_editor_bottom_toolbar_mouseup, // 0x0066f5ae,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    window_editor_bottom_toolbar_invalidate, // 0x0066f1c9,
    window_editor_bottom_toolbar_paint, // 0x0066f25c,
    nullptr
};

static EMPTY_ARGS_VOID_POINTER *previous_button_mouseup_events[] = {
    nullptr,
    window_editor_bottom_toolbar_jump_back_to_object_selection,
    window_editor_bottom_toolbar_jump_back_to_landscape_editor,
    window_editor_bottom_toolbar_jump_back_to_invention_list_set_up,
    window_editor_bottom_toolbar_jump_back_to_options_selection,
    nullptr,
    window_editor_bottom_toolbar_jump_back_to_object_selection,
    nullptr
};

static EMPTY_ARGS_VOID_POINTER *next_button_mouseup_events[] = {
    window_editor_bottom_toolbar_jump_forward_from_object_selection,
    window_editor_bottom_toolbar_jump_forward_to_invention_list_set_up,
    window_editor_bottom_toolbar_jump_forward_to_options_selection,
    window_editor_bottom_toolbar_jump_forward_to_objective_selection,
    window_editor_bottom_toolbar_jump_forward_to_save_scenario,
    nullptr,
    nullptr,
    nullptr
};

static constexpr const rct_string_id EditorStepNames[] = {
    STR_EDITOR_STEP_OBJECT_SELECTION,
    STR_EDITOR_STEP_LANDSCAPE_EDITOR,
    STR_EDITOR_STEP_INVENTIONS_LIST_SET_UP,
    STR_EDITOR_STEP_OPTIONS_SELECTION,
    STR_EDITOR_STEP_OBJECTIVE_SELECTION,
    STR_EDITOR_STEP_SAVE_SCENARIO,
    STR_EDITOR_STEP_ROLLERCOASTER_DESIGNER,
    STR_EDITOR_STEP_TRACK_DESIGNS_MANAGER,
};
// clang-format on

/**
 * Creates the main editor top toolbar window.
 * rct2: 0x0066F052 (part of 0x0066EF38)
 */
rct_window* window_editor_bottom_toolbar_open()
{
    rct_window* window = window_create(
        ScreenCoordsXY(0, context_get_height() - 32), context_get_width(), 32, &window_editor_bottom_toolbar_events,
        WC_BOTTOM_TOOLBAR, WF_STICK_TO_FRONT | WF_TRANSPARENT | WF_NO_BACKGROUND);
    window->widgets = window_editor_bottom_toolbar_widgets;

    window->enabled_widgets |= (1 << WIDX_PREVIOUS_STEP_BUTTON) | (1 << WIDX_NEXT_STEP_BUTTON) | (1 << WIDX_PREVIOUS_IMAGE)
        | (1 << WIDX_NEXT_IMAGE);

    window_init_scroll_widgets(window);
    set_all_scenery_items_invented();

    return window;
}

/**
 *
 *  rct2: 0x0066F619
 */
void window_editor_bottom_toolbar_jump_back_to_object_selection()
{
    window_close_all();
    gS6Info.editor_step = EDITOR_STEP_OBJECT_SELECTION;
    gfx_invalidate_screen();
}

/**
 *
 *  rct2: 0x0066F62C
 */
void window_editor_bottom_toolbar_jump_back_to_landscape_editor()
{
    window_close_all();
    set_all_scenery_items_invented();
    scenery_set_default_placement_configuration();
    gS6Info.editor_step = EDITOR_STEP_LANDSCAPE_EDITOR;
    context_open_window(WC_MAP);
    gfx_invalidate_screen();
}

/**
 *
 *  rct2: 0x0066F64E
 */
static void window_editor_bottom_toolbar_jump_back_to_invention_list_set_up()
{
    window_close_all();
    context_open_window(WC_EDITOR_INVENTION_LIST);
    gS6Info.editor_step = EDITOR_STEP_INVENTIONS_LIST_SET_UP;
    gfx_invalidate_screen();
}

/**
 *
 *  rct2: 0x0066F666
 */
void window_editor_bottom_toolbar_jump_back_to_options_selection()
{
    window_close_all();
    context_open_window(WC_EDITOR_SCENARIO_OPTIONS);
    gS6Info.editor_step = EDITOR_STEP_OPTIONS_SELECTION;
    gfx_invalidate_screen();
}

/**
 *
 *  rct2: 0x006AB1CE
 */
static bool window_editor_bottom_toolbar_check_object_selection()
{
    rct_window* w;

    int32_t missingObjectType = Editor::CheckObjectSelection();
    if (missingObjectType < 0)
    {
        window_close_by_class(WC_EDITOR_OBJECT_SELECTION);
        return true;
    }

    context_show_error(STR_INVALID_SELECTION_OF_OBJECTS, gGameCommandErrorText);
    w = window_find_by_class(WC_EDITOR_OBJECT_SELECTION);
    if (w != nullptr)
    {
        // Click tab with missing object
        window_event_mouse_up_call(w, WC_EDITOR_OBJECT_SELECTION__WIDX_TAB_1 + missingObjectType);
    }
    return false;
}

/**
 *
 *  rct2: 0x0066F6B0
 */
void window_editor_bottom_toolbar_jump_forward_from_object_selection()
{
    if (!window_editor_bottom_toolbar_check_object_selection())
        return;

    finish_object_selection();
    if (gScreenFlags & SCREEN_FLAGS_TRACK_DESIGNER)
    {
        context_open_window(WC_CONSTRUCT_RIDE);
    }
    else
    {
        context_open_window(WC_MAP);
    }
}

/**
 *
 *  rct2: 0x0066F758
 */
void window_editor_bottom_toolbar_jump_forward_to_invention_list_set_up()
{
    if (Editor::CheckPark())
    {
        window_close_all();
        context_open_window(WC_EDITOR_INVENTION_LIST);
        gS6Info.editor_step = EDITOR_STEP_INVENTIONS_LIST_SET_UP;
    }
    else
    {
        context_show_error(STR_CANT_ADVANCE_TO_NEXT_EDITOR_STAGE, gGameCommandErrorText);
    }

    gfx_invalidate_screen();
}

/**
 *
 *  rct2: 0x0066f790
 */
void window_editor_bottom_toolbar_jump_forward_to_options_selection()
{
    window_close_all();
    context_open_window(WC_EDITOR_SCENARIO_OPTIONS);
    gS6Info.editor_step = EDITOR_STEP_OPTIONS_SELECTION;
    gfx_invalidate_screen();
}

/**
 *
 *  rct2: 0x0066f7a8
 */
void window_editor_bottom_toolbar_jump_forward_to_objective_selection()
{
    window_close_all();
    context_open_window(WC_EDTIOR_OBJECTIVE_OPTIONS);
    gS6Info.editor_step = EDITOR_STEP_OBJECTIVE_SELECTION;
    gfx_invalidate_screen();
}

/**
 *
 *  rct2: 0x0066F7C0
 */
void window_editor_bottom_toolbar_jump_forward_to_save_scenario()
{
    if (!scenario_prepare_for_save())
    {
        context_show_error(STR_UNABLE_TO_SAVE_SCENARIO_FILE, gGameCommandErrorText);
        gfx_invalidate_screen();
        return;
    }

    window_close_all();
    auto intent = Intent(WC_LOADSAVE);
    intent.putExtra(INTENT_EXTRA_LOADSAVE_TYPE, LOADSAVETYPE_SAVE | LOADSAVETYPE_SCENARIO);
    intent.putExtra(INTENT_EXTRA_PATH, std::string{ gS6Info.name });
    context_open_intent(&intent);
}

/**
 *
 *  rct2: 0x0066F5AE
 */
static void window_editor_bottom_toolbar_mouseup([[maybe_unused]] rct_window* w, rct_widgetindex widgetIndex)
{
    if (widgetIndex == WIDX_PREVIOUS_STEP_BUTTON)
    {
        if ((gScreenFlags & SCREEN_FLAGS_TRACK_DESIGNER)
            || (gSpriteListCount[SPRITE_LIST_FREE] == MAX_SPRITES && !(gParkFlags & PARK_FLAGS_SPRITES_INITIALISED)))
        {
            previous_button_mouseup_events[gS6Info.editor_step]();
        }
    }
    else if (widgetIndex == WIDX_NEXT_STEP_BUTTON)
    {
        next_button_mouseup_events[gS6Info.editor_step]();
    }
}

static void hide_previous_step_button()
{
    window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_STEP_BUTTON].type = WWT_EMPTY;
    window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].type = WWT_EMPTY;
}

static void hide_next_step_button()
{
    window_editor_bottom_toolbar_widgets[WIDX_NEXT_STEP_BUTTON].type = WWT_EMPTY;
    window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].type = WWT_EMPTY;
}

/**
 *
 *  rct2: 0x0066F1C9
 */
void window_editor_bottom_toolbar_invalidate(rct_window* w)
{
    colour_scheme_update_by_class(
        w, (gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR) ? WC_EDITOR_SCENARIO_BOTTOM_TOOLBAR : WC_EDITOR_TRACK_BOTTOM_TOOLBAR);

    uint16_t screenWidth = context_get_width();
    window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].left = screenWidth - 200;
    window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].right = screenWidth - 1;
    window_editor_bottom_toolbar_widgets[WIDX_NEXT_STEP_BUTTON].left = screenWidth - 198;
    window_editor_bottom_toolbar_widgets[WIDX_NEXT_STEP_BUTTON].right = screenWidth - 3;

    window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_STEP_BUTTON].type = WWT_FLATBTN;
    window_editor_bottom_toolbar_widgets[WIDX_NEXT_STEP_BUTTON].type = WWT_FLATBTN;
    window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].type = WWT_IMGBTN;
    window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].type = WWT_IMGBTN;

    if (gScreenFlags & SCREEN_FLAGS_TRACK_MANAGER)
    {
        hide_previous_step_button();
        hide_next_step_button();
    }
    else
    {
        if (gS6Info.editor_step == EDITOR_STEP_OBJECT_SELECTION)
        {
            hide_previous_step_button();
        }
        else if (gS6Info.editor_step == EDITOR_STEP_ROLLERCOASTER_DESIGNER)
        {
            hide_next_step_button();
        }
        else if (!(gScreenFlags & SCREEN_FLAGS_TRACK_DESIGNER))
        {
            if (gSpriteListCount[SPRITE_LIST_FREE] != MAX_SPRITES || gParkFlags & PARK_FLAGS_SPRITES_INITIALISED)
            {
                hide_previous_step_button();
            }
        }
    }
}

/**
 *
 *  rct2: 0x0066F25C
 */
void window_editor_bottom_toolbar_paint(rct_window* w, rct_drawpixelinfo* dpi)
{
    bool drawPreviousButton = false;
    bool drawNextButton = false;

    if (gS6Info.editor_step == EDITOR_STEP_OBJECT_SELECTION)
    {
        drawNextButton = true;
    }
    else if (gScreenFlags & SCREEN_FLAGS_TRACK_DESIGNER)
    {
        drawPreviousButton = true;
    }
    else if (gSpriteListCount[SPRITE_LIST_FREE] != MAX_SPRITES)
    {
        drawNextButton = true;
    }
    else if (gParkFlags & PARK_FLAGS_SPRITES_INITIALISED)
    {
        drawNextButton = true;
    }
    else
    {
        drawPreviousButton = true;
    }

    if (!(gScreenFlags & SCREEN_FLAGS_TRACK_MANAGER))
    {
        if (drawPreviousButton)
        {
            gfx_filter_rect(
                dpi, window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].left + w->windowPos.x,
                window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].top + w->windowPos.y,
                window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].right + w->windowPos.x,
                window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].bottom + w->windowPos.y, PALETTE_51);
        }

        if ((drawPreviousButton || drawNextButton) && gS6Info.editor_step != EDITOR_STEP_ROLLERCOASTER_DESIGNER)
        {
            gfx_filter_rect(
                dpi, window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].left + w->windowPos.x,
                window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].top + w->windowPos.y,
                window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].right + w->windowPos.x,
                window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].bottom + w->windowPos.y, PALETTE_51);
        }
    }

    window_draw_widgets(w, dpi);

    if (!(gScreenFlags & SCREEN_FLAGS_TRACK_MANAGER))
    {
        if (drawPreviousButton)
        {
            gfx_fill_rect_inset(
                dpi, window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].left + 1 + w->windowPos.x,
                window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].top + 1 + w->windowPos.y,
                window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].right - 1 + w->windowPos.x,
                window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].bottom - 1 + w->windowPos.y, w->colours[1],
                INSET_RECT_F_30);
        }

        if ((drawPreviousButton || drawNextButton) && gS6Info.editor_step != EDITOR_STEP_ROLLERCOASTER_DESIGNER)
        {
            gfx_fill_rect_inset(
                dpi, window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].left + 1 + w->windowPos.x,
                window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].top + 1 + w->windowPos.y,
                window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].right - 1 + w->windowPos.x,
                window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].bottom - 1 + w->windowPos.y, w->colours[1],
                INSET_RECT_F_30);
        }

        int16_t stateX = (window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].right
                          + window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].left)
                / 2
            + w->windowPos.x;
        int16_t stateY = w->height - 0x0C + w->windowPos.y;
        gfx_draw_string_centred(
            dpi, EditorStepNames[gS6Info.editor_step], stateX, stateY, NOT_TRANSLUCENT(w->colours[2]) | COLOUR_FLAG_OUTLINE,
            nullptr);

        if (drawPreviousButton)
        {
            gfx_draw_sprite(
                dpi, SPR_PREVIOUS, window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].left + 6 + w->windowPos.x,
                window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].top + 6 + w->windowPos.y, 0);

            int32_t textColour = NOT_TRANSLUCENT(w->colours[1]);
            if (gHoverWidget.window_classification == WC_BOTTOM_TOOLBAR
                && gHoverWidget.widget_index == WIDX_PREVIOUS_STEP_BUTTON)
            {
                textColour = COLOUR_WHITE;
            }

            int16_t textX = (window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].left + 30
                             + window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].right)
                    / 2
                + w->windowPos.x;
            int16_t textY = window_editor_bottom_toolbar_widgets[WIDX_PREVIOUS_IMAGE].top + 6 + w->windowPos.y;

            rct_string_id stringId = EditorStepNames[gS6Info.editor_step - 1];
            if (gScreenFlags & SCREEN_FLAGS_TRACK_DESIGNER)
                stringId = STR_EDITOR_STEP_OBJECT_SELECTION;

            gfx_draw_string_centred(dpi, STR_BACK_TO_PREVIOUS_STEP, textX, textY, textColour, nullptr);
            gfx_draw_string_centred(dpi, stringId, textX, textY + 10, textColour, nullptr);
        }

        if ((drawPreviousButton || drawNextButton) && gS6Info.editor_step != EDITOR_STEP_ROLLERCOASTER_DESIGNER)
        {
            gfx_draw_sprite(
                dpi, SPR_NEXT, window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].right - 29 + w->windowPos.x,
                window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].top + 6 + w->windowPos.y, 0);

            int32_t textColour = NOT_TRANSLUCENT(w->colours[1]);

            if (gHoverWidget.window_classification == WC_BOTTOM_TOOLBAR && gHoverWidget.widget_index == WIDX_NEXT_STEP_BUTTON)
            {
                textColour = COLOUR_WHITE;
            }

            int16_t textX = (window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].left
                             + window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].right - 30)
                    / 2
                + w->windowPos.x;
            int16_t textY = window_editor_bottom_toolbar_widgets[WIDX_NEXT_IMAGE].top + 6 + w->windowPos.y;

            rct_string_id stringId = EditorStepNames[gS6Info.editor_step + 1];
            if (gScreenFlags & SCREEN_FLAGS_TRACK_DESIGNER)
                stringId = STR_EDITOR_STEP_ROLLERCOASTER_DESIGNER;

            gfx_draw_string_centred(dpi, STR_FORWARD_TO_NEXT_STEP, textX, textY, textColour, nullptr);
            gfx_draw_string_centred(dpi, stringId, textX, textY + 10, textColour, nullptr);
        }
    }
}
