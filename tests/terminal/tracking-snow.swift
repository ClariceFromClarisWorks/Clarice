// Host-side Snow input only. Capture cancellation while the button is still
// held, so a prompt appearing only after release cannot pass acceptance.
import AppKit
import CoreGraphics
import ImageIO
let pid=pid_t(CommandLine.arguments[1])!
let windows=CGWindowListCopyWindowInfo(.optionOnScreenOnly,kCGNullWindowID) as! [[String:Any]]
let w=windows.first{($0[kCGWindowOwnerPID as String] as? Int)==Int(pid)&&($0[kCGWindowLayer as String] as? Int)==0}!
let bounds=CGRect(dictionaryRepresentation:w[kCGWindowBounds as String] as! CFDictionary)!
let id=CGWindowID(w[kCGWindowNumber as String] as! Int)
NSRunningApplication(processIdentifier:pid)?.activate(options:[.activateIgnoringOtherApps])
Thread.sleep(forTimeInterval:0.3)
let p=CGPoint(x:bounds.minX+Double(CommandLine.arguments[2])!,y:bounds.minY+Double(CommandLine.arguments[3])!)
let key=CGKeyCode(CommandLine.arguments[5])!
let flags:CGEventFlags=CommandLine.arguments[4]=="command" ? .maskCommand : .maskControl
func mouse(_ type:CGEventType){CGEvent(mouseEventSource:nil,mouseType:type,mouseCursorPosition:p,mouseButton:.left)!.post(tap:.cghidEventTap)}
func shot(_ suffix:String){let proc=Process();proc.executableURL=URL(fileURLWithPath:"/usr/sbin/screencapture");proc.arguments=["-x","-l",String(id),CommandLine.arguments[6]+suffix+".png"];try! proc.run();proc.waitUntilExit();precondition(proc.terminationStatus==0)}
mouse(.mouseMoved);Thread.sleep(forTimeInterval:0.1);mouse(.leftMouseDown)
Thread.sleep(forTimeInterval:0.5);shot("-held")
let started=Date()
for down in [true,false]{let e=CGEvent(keyboardEventSource:nil,virtualKey:key,keyDown:down)!;e.flags=flags;e.post(tap:.cghidEventTap);Thread.sleep(forTimeInterval:0.05)}
Thread.sleep(forTimeInterval:0.7);shot("-cancel-held");print("Cancellation capture while held, host seconds:",Date().timeIntervalSince(started))
mouse(.leftMouseUp);Thread.sleep(forTimeInterval:0.3)
