#include "Types.r"
resource 'SIZE' (-1) { reserved,acceptSuspendResumeEvents,reserved,canBackground,needsActivateOnFGSwitch,backgroundAndForeground,dontGetFrontClicks,ignoreChildDiedEvents,is32BitCompatible,notHighLevelEventAware,onlyLocalHLEvents,notStationeryAware,dontUseTextEditServices,reserved,reserved,reserved,160*1024,160*1024 };
resource 'ALRT' (151) { {60,45,235,467},151,{OK,visible,silent,OK,visible,silent,OK,visible,silent,OK,visible,silent},0 };
resource 'DITL' (151) {{
 {136,320,158,400}, Button {enabled,"OK"};
 {14,16,126,400}, StaticText {disabled,"^0"}
}};
