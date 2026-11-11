// Snow must be foreground. HID delivery is required; process-targeted mouse events did not drag.
import CoreGraphics
import Foundation
let pid=pid_t(CommandLine.arguments[1])!
func send(_ type:CGEventType,_ x:Double,_ y:Double){CGEvent(mouseEventSource:nil,mouseType:type,mouseCursorPosition:CGPoint(x:x,y:y),mouseButton:.left)!.post(tap:.cghidEventTap)}
send(.mouseMoved,330,280);Thread.sleep(forTimeInterval:0.3)
send(.leftMouseDown,330,280);Thread.sleep(forTimeInterval:0.2)
for i in 0...60 {send(.leftMouseDragged,330+Double(i)*5,280+Double(i)*3);Thread.sleep(forTimeInterval:0.05)}
Thread.sleep(forTimeInterval:0.4);send(.leftMouseUp,630,460)
