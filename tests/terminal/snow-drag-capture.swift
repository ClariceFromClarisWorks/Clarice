// Optional Snow observation: 2x guest, host window at 220,52 size 1000x782.
// Compile with swiftc -O -target arm64-apple-macosx13.0. Arguments: PID JSON X Y END_Y.
// Sampling detects captured blank bands, not flashes shorter than the capture interval.
import AppKit
import CoreGraphics
import Foundation
import ImageIO
let a=CommandLine.arguments,pid=pid_t(a[1])!,out=a[2]
guard NSWorkspace.shared.frontmostApplication?.processIdentifier==pid else {fatalError("Focus lost")}
let x=Double(a[3])!,y=Double(a[4])!,end=Double(a[5])!
func send(_ t:CGEventType,_ yy:Double){CGEvent(mouseEventSource:nil,mouseType:t,mouseCursorPosition:CGPoint(x:x,y:yy),mouseButton:.left)!.post(tap:.cghidEventTap)}
let crop=CGRect(x:245,y:243,width:930,height:484)
func inks(_ index:Int)->[Int]{let im=CGWindowListCreateImage(crop,.optionOnScreenOnly,kCGNullWindowID,[])!;if index==0 || index==40 || index==79 {let url=URL(fileURLWithPath:out+"-"+String(index)+".png") as CFURL;let dest=CGImageDestinationCreateWithURL(url,"public.png" as CFString,1,nil)!;CGImageDestinationAddImage(dest,im,nil);CGImageDestinationFinalize(dest)};let d=im.dataProvider!.data!;let p=CFDataGetBytePtr(d)!;let stride=im.bytesPerRow,bpp=im.bitsPerPixel/8;let bands=22,h=im.height/bands;var counts=[Int](repeating:0,count:bands);for yy in 0..<h*bands {for xx in 0..<im.width {let i=yy*stride+xx*bpp;if Int(p[i])+Int(p[i+1])+Int(p[i+2])<192 {counts[yy/h]+=1}}};return counts}
send(.mouseMoved,y);Thread.sleep(forTimeInterval:0.2);send(.leftMouseDown,y);Thread.sleep(forTimeInterval:0.3)
let start=ProcessInfo.processInfo.systemUptime;var samples=[[String:Any]]();var frames=[[Int]]()
for i in 0..<80 {if NSWorkspace.shared.frontmostApplication?.processIdentifier != pid {send(.leftMouseUp,y);exit(2)};let f=Double(i)/79;let yy=y+(end-y)*f;send(.leftMouseDragged,yy);let c=inks(i);frames.append(c);samples.append(["seconds":ProcessInfo.processInfo.systemUptime-start,"ink":c.reduce(0,+)]);Thread.sleep(forTimeInterval:0.02)}
send(.leftMouseUp,end)
var blanks=0;for i in 1..<frames.count-1 {for row in 0..<22 {if frames[i][row]<4 && frames[i-1][row]>100 && frames[i+1][row]>100 {blanks+=1}}}
let data=try JSONSerialization.data(withJSONObject:["samples":samples,"transient_blank_bands":blanks,"note":"Host capture during real thumb drag; 22 text bands, no guarantee for sub-sample flashes; inspect before/after to confirm gesture moved history."],options:[.prettyPrinted,.sortedKeys]);try data.write(to:URL(fileURLWithPath:out));print("capture complete, blank bands",blanks)
