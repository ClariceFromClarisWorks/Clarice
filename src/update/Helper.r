#include "Types.r"
resource 'SIZE' (-1) { reserved,acceptSuspendResumeEvents,reserved,canBackground,needsActivateOnFGSwitch,backgroundAndForeground,dontGetFrontClicks,ignoreChildDiedEvents,is32BitCompatible,notHighLevelEventAware,onlyLocalHLEvents,notStationeryAware,dontUseTextEditServices,reserved,reserved,reserved,160*1024,160*1024 };
resource 'ALRT' (151) { {60,45,235,467},151,{OK,visible,silent,OK,visible,silent,OK,visible,silent,OK,visible,silent},0 };
resource 'DITL' (151) {{
 {136,320,158,400}, Button {enabled,"OK"};
 {14,16,126,400}, StaticText {disabled,"^0"}
}};

resource 'ALRT' (152) { {60,45,255,467},152,{OK,visible,silent,OK,visible,silent,OK,visible,silent,OK,visible,silent},0 };
resource 'DITL' (152) {{
 {156,310,178,400}, Button {enabled,"Restore"};
 {156,205,178,295}, Button {enabled,"Cancel"};
 {14,16,146,400}, StaticText {disabled,"^0"}
}};
