#include "Types.r"
resource 'SIZE' (-1) {
 reserved, acceptSuspendResumeEvents, reserved, canBackground,
 needsActivateOnFGSwitch, backgroundAndForeground, dontGetFrontClicks,
 ignoreChildDiedEvents, is32BitCompatible, notHighLevelEventAware,
 onlyLocalHLEvents, notStationeryAware, dontUseTextEditServices,
 reserved, reserved, reserved, 384 * 1024, 384 * 1024
};

resource 'DLOG' (140) {
 {60, 55, 290, 455}, dBoxProc, visible, noGoAway, 0x0, 140, "Preferences", 0
};
resource 'DITL' (140) {
 {
 {196, 300, 218, 380}, Button { enabled, "OK" };
 {196, 208, 218, 288}, Button { enabled, "Cancel" };
 {12, 16, 30, 380}, StaticText { disabled, "Clarice preferences" };
 {42, 16, 60, 120}, StaticText { disabled, "Installed font:" };
 {42, 132, 60, 378}, EditText { enabled, "Monaco" };
 {72, 16, 90, 120}, StaticText { disabled, "Size (6-24):" };
 {72, 132, 90, 184}, EditText { enabled, "9" };
 {102, 16, 120, 255}, StaticText { disabled, "Retained lines (32, 64, 128):" };
 {102, 292, 120, 378}, EditText { enabled, "128" };
 {132, 16, 150, 378}, CheckBox { enabled, "White text on black" };
 {158, 16, 176, 378}, CheckBox { enabled, "Rendering cache (29 KiB; optional)" }
 }
};
resource 'ALRT' (141) {{100, 65, 220, 445}, 141, {OK, visible, sound1, OK, visible, sound1, OK, visible, sound1, OK, visible, sound1}, 0};
resource 'DITL' (141) {{
 {78, 278, 100, 358}, Button {enabled, "OK"};
 {14, 70, 65, 360}, StaticText {disabled, "Choose an installed font, size 6-24 and retained lines 32, 64 or 128."}
}};

