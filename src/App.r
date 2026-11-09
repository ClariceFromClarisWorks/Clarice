#include "Types.r"
resource 'SIZE' (-1) {
 reserved, acceptSuspendResumeEvents, reserved, canBackground,
 needsActivateOnFGSwitch, backgroundAndForeground, dontGetFrontClicks,
 ignoreChildDiedEvents, is32BitCompatible, notHighLevelEventAware,
 onlyLocalHLEvents, notStationeryAware, dontUseTextEditServices,
 reserved, reserved, reserved, 768 * 1024, 768 * 1024
};

resource 'DLOG' (140) {
 {40, 75, 314, 435}, dBoxProc, visible, noGoAway, 0x0, 140, "Preferences", 0
};
resource 'DITL' (140) {{
 {242, 260, 264, 340}, Button { enabled, "OK" };
 {242, 168, 264, 248}, Button { enabled, "Cancel" };
 {10, 16, 28, 340}, StaticText { disabled, "Clarice preferences" };
 {38, 16, 56, 125}, StaticText { disabled, "Installed font:" };
 {36, 132, 58, 338}, UserItem { enabled };
 {70, 16, 88, 120}, StaticText { disabled, "Size (6-24):" };
 {70, 132, 88, 174}, EditText { enabled, "9" };
 {70, 190, 88, 275}, StaticText { disabled, "Columns:" };
 {70, 290, 88, 338}, EditText { enabled, "80" };
 {98, 16, 116, 338}, CheckBox { enabled, "White text on black" };
 {128, 16, 146, 125}, StaticText { disabled, "Scrollback:" };
 {126, 132, 148, 338}, UserItem { enabled };
 {158, 16, 176, 125}, StaticText { disabled, "Paging:" };
 {156, 132, 178, 338}, UserItem { enabled };
 {188, 16, 206, 125}, StaticText { disabled, "Render cache:" };
 {186, 132, 208, 338}, UserItem { enabled };
 {216, 16, 234, 338}, CheckBox { enabled, "Fill screen (keep menu bar)" }
}};
resource 'ALRT' (141) {{100, 65, 220, 445}, 141, {OK, visible, sound1, OK, visible, sound1, OK, visible, sound1, OK, visible, sound1}, 0};
resource 'DITL' (141) {{
 {78, 278, 100, 358}, Button {enabled, "OK"};
 {14, 70, 65, 360}, StaticText {disabled, "Use size 6-24 and columns 20-130 that fit the screen and selected font."}
}};
