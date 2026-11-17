#include "Types.r"
resource 'SIZE' (-1) {
 reserved, acceptSuspendResumeEvents, reserved, canBackground,
 needsActivateOnFGSwitch, backgroundAndForeground, dontGetFrontClicks,
 ignoreChildDiedEvents, is32BitCompatible, notHighLevelEventAware,
 onlyLocalHLEvents, notStationeryAware, dontUseTextEditServices,
 reserved, reserved, reserved, 768 * 1024, 768 * 1024
};

resource 'DLOG' (140) {
 {36, 75, 306, 435}, dBoxProc, visible, noGoAway, 0x0, 140, "Preferences", 0
};
resource 'DITL' (140) {{
 {238, 260, 260, 340}, Button { enabled, "OK" };
 {238, 168, 260, 248}, Button { enabled, "Cancel" };
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
 {212, 16, 230, 338}, CheckBox { enabled, "Fill screen (keep menu bar)" };
 {186, 16, 204, 125}, StaticText { disabled, "Control key:" };
 {184, 132, 206, 338}, UserItem { enabled };
 {154, 132, 176, 338}, Button { enabled, "Telnet server..." }
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

resource 'DLOG' (142) {{40,60,300,452},dBoxProc,visible,noGoAway,0x0,142,"Telnet server",0};
resource 'DITL' (142) {{
 {228,290,250,372},Button {enabled,"Apply"};
 {228,196,250,278},Button {enabled,"Cancel"};
 {12,16,30,170},StaticText {disabled,"Allowed peer (IPv4):"};
 {12,180,30,370},EditText {enabled,""};
 {42,16,60,170},StaticText {disabled,"Port:"};
 {42,180,60,260},EditText {enabled,"2323"};
 {96,180,114,370},EditText {enabled,""};
 {130,16,148,370},CheckBox {enabled,"Enable now (off after Clarice restarts)"};
 {78,16,118,172},StaticText {disabled,"Password: blank keeps existing"};
 {164,16,218,370},StaticText {disabled,"One remote shell. Password required. Telnet sends data without encryption; use only a trusted network. Apply disconnects any remote session."}
}};
