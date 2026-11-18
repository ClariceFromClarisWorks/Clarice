// Test-only frontend for the existing Snow core; no emulation/runtime changes.
use snow_core::{emulator::{Emulator,MouseMode,comm::{EmulatorCommand as C,EmulatorSpeed}},mac::MacModel,tickable::Tickable};
use std::{io::{self,BufRead,Write},sync::{Arc,atomic::{AtomicBool,Ordering}},time::Duration};
fn main()->anyhow::Result<()> {
 let a:Vec<String>=std::env::args().collect();
 let rom=std::fs::read(&a[1])?;
 let (mut e,frames)=Emulator::new_with_extra(&rom,&[],MacModel::Plus,None,MouseMode::Absolute,Some(4*1024*1024),None,false,false,None,None)?;
 e.load_hdd_image(std::path::Path::new(&a[2]),0)?;e.load_hdd_image(std::path::Path::new(&a[3]),1)?;
 let cmd=e.create_cmd_sender();let events=e.create_event_recv();
 cmd.send(C::SetSpeed(EmulatorSpeed::Video))?;cmd.send(C::Run)?;
 let alive=Arc::new(AtomicBool::new(true));let running=alive.clone();
 let t=std::thread::spawn(move||{while running.load(Ordering::Relaxed){if let Err(err)=e.tick(1,()){eprintln!("{err}");break;}while events.try_recv().is_ok(){}}});
 for line in io::stdin().lock().lines(){let line=line?;let v:Vec<_>=line.split_whitespace().collect();if v.is_empty(){continue;}
 match v[0]{
 "frame"=>{drop(frames.lock().unwrap().take());std::thread::sleep(Duration::from_millis(100));if let Some(f)=frames.lock().unwrap().take(){let mut out=std::fs::File::create(v[1])?;write!(out,"P6\n{} {}\n255\n",f.width(),f.height())?;for p in f.into_inner().chunks_exact(4){out.write_all(&p[..3])?;}}},
 "click"=>{cmd.send(C::MouseUpdateAbsolute{x:v[1].parse()?,y:v[2].parse()?})?;std::thread::sleep(Duration::from_millis(100));for _ in 0..v[3].parse::<usize>()?{cmd.send(C::MouseUpdateRelative{relx:0,rely:0,btn:Some(true)})?;std::thread::sleep(Duration::from_millis(100));cmd.send(C::MouseUpdateRelative{relx:0,rely:0,btn:Some(false)})?;std::thread::sleep(Duration::from_millis(100));}},
 "quit"=>break,
 _=>eprintln!("unknown input")}
 println!("ok");io::stdout().flush()?;
 }
 alive.store(false,Ordering::Relaxed);t.join().unwrap();Ok(())
}
