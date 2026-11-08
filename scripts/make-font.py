from pathlib import Path
from PIL import Image,ImageFont,ImageDraw
root=Path(__file__).resolve().parents[1]
font=ImageFont.truetype(str(root/'assets/font/JetBrainsMono-Regular.ttf'),11)
rows=[]
spec=Image.new('1',(16*8,16*14),1)
for n in range(256):
    im=Image.new('L',(8,14),0)
    if n>=32 and n!=127:
        ImageDraw.Draw(im).text((0,-2),bytes([n]).decode('mac_roman'),font=font,fill=255,stroke_width=0)
    bits=im.point(lambda x:255 if x>=110 else 0,'1')
    rows.extend(sum((1<<(7-x)) for x in range(8) if bits.getpixel((x,y))) for y in range(14))
    inv=Image.eval(bits,lambda x:255-x)
    spec.paste(inv,((n%16)*8,(n//16)*14))
(root/'src/font.h').write_text('/* Sys6 Mono bitmap derivative of JetBrains Mono; SIL OFL 1.1, assets/font/OFL.txt. */\nstatic const unsigned char font_bits[3584] = {\n'+','.join(str(v) for v in rows)+'\n};\n')
spec.resize((512,896),Image.Resampling.NEAREST).save(root/'outputs/phase1/font-specimen.png')
