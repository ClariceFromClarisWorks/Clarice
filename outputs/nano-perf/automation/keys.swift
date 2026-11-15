// Existing Snow CGEvent physical-key approach, extended for serial controls.
import CoreGraphics
import Foundation
let pid=pid_t(CommandLine.arguments[1])!
let data=try Data(contentsOf:URL(fileURLWithPath:CommandLine.arguments[2]))
let interval=Double(CommandLine.arguments[3])!
let letters:[CGKeyCode]=[0,11,8,2,14,3,5,4,34,38,40,37,46,45,31,35,12,15,1,17,32,9,13,7,16,6]
let digits:[CGKeyCode]=[29,18,19,20,21,23,22,26,28,25]
for b in data {
 var code:CGKeyCode=0;var flags:CGEventFlags=[]
 if b>=97 && b<=122 {code=letters[Int(b-97)]}
 else if b>=65 && b<=90 {code=letters[Int(b-65)];flags = .maskShift}
 else if b>=48 && b<=57 {code=digits[Int(b-48)]}
 else {switch b {case 8,127:code=51
 case 3:code=8;flags = .maskControl
 case 1:code=125
 case 2:code=126
 case 23:code=13;flags = .maskAlternate
 case 15:code=31;flags = .maskAlternate
 case 24:code=7;flags = .maskAlternate
 case 27:code=53
 case 28:code=47;flags = .maskCommand
 case 29:code=30;flags = .maskControl
 case 10,13:code=36
 case 32:code=49
 case 47:code=44
 case 45:code=27
 case 46:code=47
 case 62:code=47;flags = .maskShift
 case 36:code=21;flags = .maskShift
 case 63:code=44;flags = .maskShift
 case 59:code=41
 default:fatalError("Unsupported byte \(b)")}}
 let down=CGEvent(keyboardEventSource:nil,virtualKey:code,keyDown:true)!;down.flags=flags;down.postToPid(pid)
 Thread.sleep(forTimeInterval:min(0.025,interval/2))
 let up=CGEvent(keyboardEventSource:nil,virtualKey:code,keyDown:false)!;up.flags=flags;up.postToPid(pid)
 Thread.sleep(forTimeInterval:max(0.002,interval/2))
}
print("Submitted \(data.count) key pairs, requested interval \(interval)")
