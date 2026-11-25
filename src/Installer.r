#include "Types.r"
resource 'DLOG' (152) {{32,32,318,480},dBoxProc,visible,noGoAway,0,152,"Install Clarice",0};
resource 'DITL' (152) {{
 {256,326,278,432}, Button {enabled,"Install/Upgrade"};
 {256,220,278,310}, Button {enabled,"Cancel"};
 {10,16,48,432}, StaticText {disabled,"Welcome to Clarice. Choose installation folders.\rQuit Clarice before installing. Cancel leaves files unchanged."};
 {166,16,180,432}, UserItem {disabled};
 {188,16,226,432}, StaticText {disabled,""};
 {230,16,246,432}, StaticText {disabled,""};
 {56,336,76,432}, Button {enabled,"Core..."};
 {100,336,120,432}, Button {enabled,"Data..."};
 {54,16,94,328}, StaticText {disabled,""};
 {98,16,138,328}, StaticText {disabled,""};
 {142,16,160,432}, CheckBox {enabled,"Repair missing or changed standard tools"};
 {256,16,278,114}, Button {enabled,"Uninstall..."};
 {142,16,160,432}, CheckBox {enabled,"Also delete ALL Clarice Data, including /home"}
}};
