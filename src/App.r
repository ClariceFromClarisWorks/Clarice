#include "Types.r"
resource 'SIZE' (-1) {
 reserved, acceptSuspendResumeEvents, reserved, canBackground,
 needsActivateOnFGSwitch, backgroundAndForeground, dontGetFrontClicks,
 ignoreChildDiedEvents, is32BitCompatible, notHighLevelEventAware,
 onlyLocalHLEvents, notStationeryAware, dontUseTextEditServices,
 reserved, reserved, reserved, 768 * 1024, 768 * 1024
};

resource 'DLOG' (140) {
 {40, 75, 284, 435}, dBoxProc, visible, noGoAway, 0x0, 140, "Preferences", 0
};
resource 'DITL' (140) {{
 {212, 260, 234, 340}, Button { enabled, "OK" };
 {212, 168, 234, 248}, Button { enabled, "Cancel" };
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
 {186, 16, 204, 338}, CheckBox { enabled, "Fill screen (keep menu bar)" }
}};
resource 'ALRT' (141) {{100, 65, 220, 445}, 141, {OK, visible, sound1, OK, visible, sound1, OK, visible, sound1, OK, visible, sound1}, 0};
resource 'DITL' (141) {{
 {78, 278, 100, 358}, Button {enabled, "OK"};
 {14, 70, 65, 360}, StaticText {disabled, "Use size 6-24 and columns 20-130 that fit the screen and selected font."}
}};

resource 'ALRT' (151) { {80,60,210,452},151,{OK,visible,silent,OK,visible,silent,OK,visible,silent,OK,visible,silent},0 };
resource 'DITL' (151) {{
 {92,300,114,372}, Button {enabled,"OK"};
 {14,16,82,370}, StaticText {disabled,"^0"}
}};

#include "update/version.h"
type 'CLUP' { longint; longint; integer; integer; integer; integer; };
resource 'CLUP' (128) { 'CLU1', CLARICE_UPDATE_BUILD, 0x0600, 1, 1, 2 };
