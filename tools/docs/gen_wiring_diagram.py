# Generates docs/img/wiring-diagram.{svg,png}. Run from the repo root: python3 tools/docs/gen_wiring_diagram.py (needs cairosvg)
W,H=1600,1130
o=[]
a=o.append
F='font-family="DejaVu Sans, sans-serif"'
def rect(x,y,w,h,fill,stroke='#333',rx=10,dash=None,sw=2):
    d=f' stroke-dasharray="{dash}"' if dash else ''
    a(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{rx}" fill="{fill}" stroke="{stroke}" stroke-width="{sw}"{d}/>')
def t(x,y,s,size=15,anchor='start',w='normal',c='#222'):
    a(f'<text x="{x}" y="{y}" {F} font-size="{size}" text-anchor="{anchor}" font-weight="{w}" fill="{c}">{s}</text>')
def line(pts,c,sw=3):
    p=' '.join(f'{x},{y}' for x,y in pts)
    a(f'<polyline points="{p}" fill="none" stroke="{c}" stroke-width="{sw}" stroke-linejoin="round"/>')
def dot(x,y,c): a(f'<circle cx="{x}" cy="{y}" r="5" fill="{c}"/>')
def pin(x,y,label,side):
    a(f'<circle cx="{x}" cy="{y}" r="4" fill="#fff" stroke="#333" stroke-width="2"/>')
    if side=='L': t(x+10,y+5,label,14)
    else: t(x-10,y+5,label,14,'end')
RED='#d62828';BLK='#222';GRN='#2a9d3f';WHT='#9aa0a6';BLU='#1d6fd8';ORG='#e07b00';PUR='#7b3fbf'
a(f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}"><rect width="{W}" height="{H}" fill="#ffffff"/>')
t(40,50,'Smart Grind-by-Weight – Lelit William PL72: wiring diagram',26,w='bold')
t(40,80,'Waveshare ESP32-S3-Touch-LCD-2 • HX711 • MAVIN ZZPH6 1 kg • Pololu 5426 • 18650 • KCD1-11',16,c='#555')

# Battery
rect(40,380,210,150,'#fff4d6')
t(145,410,'18650 cell',17,'middle','bold'); t(145,432,'Samsung INR18650-25R',13,'middle',c='#555'); t(145,452,'in 18650 holder',13,'middle',c='#555')
pin(250,440,'+','R'); pin(250,500,'−','R')
# Switch
rect(330,405,130,70,'#eef2f7')
t(395,428,'KCD1-11',15,'middle','bold'); t(395,448,'power switch',13,'middle',c='#555')
pin(330,440,'','L'); pin(460,440,'','L')
a('<line x1="365" y1="462" x2="420" y2="452" stroke="#333" stroke-width="2"/><circle cx="365" cy="462" r="3" fill="#333"/><circle cx="425" cy="462" r="3" fill="#333"/>')
line([(254,440),(326,440)],RED); line([(464,440),(556,440)],RED)
line([(254,500),(556,500)],BLK)
t(500,430,'BAT+',13,'middle',c=RED); t(500,520,'BAT−',13,'middle',c=BLK)

# Board
rect(560,170,380,620,'#e3f0ff','#1d4f91',sw=3)
t(750,205,'Waveshare',16,'middle',c='#1d4f91'); t(750,228,'ESP32-S3-Touch-LCD-2',19,'middle','bold',c='#1d4f91')
rect(610,250,280,90,'#0f2033','#0f2033',rx=6)
t(750,285,'LCD ST7789 240×320',14,'middle',c='#fff'); t(750,308,'CST816D touch (on-board)',13,'middle',c='#cfe3ff')
t(580,420,'Battery connector',13,c='#1d4f91'); t(580,470,'MX1.25',14,w='bold',c='#1d4f91')
pin(560,440,'BAT+','L'); pin(560,500,'BAT−','L')
t(580,560,'ETA6098 charger',13,c='#555'); t(580,580,'Battery ADC: GPIO5 (on-board)',13,c='#555'); t(925,350,'',1); t(925,620,'header P2',13,'end','bold',c='#1d4f91')
rect(690,740,120,50,'#cfd8e3',rx=6); t(750,770,'USB-C',15,'middle','bold')
t(750,815,'power / charging / flashing',13,'middle',c='#555')
for y,l in [(390,'P2-1  3V3'),(440,'P2-2  GND'),(500,'P2-7  GPIO15'),(550,'P2-8  GPIO13'),(660,'P2-9  GPIO11')]: pin(940,y,l,'R')

# HX711
rect(1090,350,210,240,'#e9f7ec','#2a7a3a',sw=2)
t(1195,380,'HX711',18,'middle','bold',c='#2a7a3a'); t(1195,400,'load cell amplifier',12,'middle',c='#555')
for y,l in [(390+30,'VCC'),(440,'GND'),(500,'DT / DOUT'),(550,'SCK')]: pass
pin(1090,420,'VCC','L'); pin(1090,460,'GND','L'); pin(1090,500,'SCK','L'); pin(1090,550,'DT (DOUT)','L')
for y,l in [(420,'E+'),(460,'E−'),(500,'A−'),(540,'A+')]: pin(1300,y,l,'R')

# Load cell
rect(1420,390,150,180,'#f3f3f3')
t(1495,425,'Load cell',16,'middle','bold'); t(1495,445,'MAVIN ZZPH6',13,'middle',c='#555'); t(1495,463,'1 kg',13,'middle',c='#555')
t(1495,545,'(scale platform)',12,'middle',c='#777')
for y,c,n in [(420,RED,'red'),(460,BLK,'black'),(500,WHT,'white'),(540,GRN,'green')]:
    line([(1304,y),(1420,y)],c); t(1362,y-6,n,11,'middle',c='#444')

# Shield wire
a('<polyline points="1495,570 1495,610 1030,610" fill="none" stroke="#b8c4cc" stroke-width="3" stroke-dasharray="7,4"/>')
dot(1030,610,BLK)
t(1250,603,'clear = shield → GND (HX711 side only)',12,'middle',c='#555')
# Pololu
rect(1090,630,210,200,'#f6ecff','#6b3fa0',sw=2)
t(1195,660,'Pololu 5426',18,'middle','bold',c='#6b3fa0'); t(1195,680,'isolated SSR, max 100 V',12,'middle',c='#555')
pin(1090,700,'EN','L'); pin(1090,750,'GND','L'); pin(1090,800,'VIN','L')
pin(1300,720,'OUT','R'); pin(1300,790,'OUT','R')

# Power buses
line([(944,390),(1086,420)] if False else [(944,390),(1000,390),(1000,420),(1086,420)],RED)
line([(1000,420),(1000,800),(1086,800)],RED); dot(1000,420,RED)
line([(944,440),(1030,440),(1030,460),(1086,460)],BLK)
line([(1030,460),(1030,750),(1086,750)],BLK); dot(1030,460,BLK)
line([(944,500),(1086,500)],ORG); t(1015,494,'',1)
line([(944,550),(1086,550)],BLU)
line([(944,660),(970,660),(970,700),(1086,700)],PUR)
# pull-down resistor EN->GND at x=1060
dot(1060,700,PUR)
a('<rect x="1052" y="710" width="16" height="30" fill="#fff" stroke="#333" stroke-width="2"/>')
a('<line x1="1060" y1="700" x2="1060" y2="710" stroke="#7b3fbf" stroke-width="2"/>')
dot(1060,750,BLK)
t(1047,731,'R',13,'end','bold')
t(960,860,'R = 3–100 kΩ pull-down EN→GND (3 kΩ used)',13,c='#6b3fa0',w='bold')
line([(1060,780),(1060,780)],BLK)

# Grinder
rect(1360,640,220,330,'#fafafa','#888',dash='8,6')
t(1470,668,'Lelit William PL72',15,'middle','bold'); t(1470,688,'(circuits unmodified)',12,'middle',c='#666')
rect(1390,705,160,105,'#fff','#555',rx=6)
t(1470,728,'Original button',13,'middle','bold'); t(1470,745,'add/stop (contacts)',12,'middle',c='#555')
# button symbol
a('<line x1="1420" y1="760" x2="1420" y2="795" stroke="#333" stroke-width="2"/><line x1="1520" y1="760" x2="1520" y2="795" stroke="#333" stroke-width="2"/>')
a('<line x1="1440" y1="770" x2="1500" y2="770" stroke="#333" stroke-width="3"/><line x1="1470" y1="770" x2="1470" y2="757" stroke="#333" stroke-width="2"/>')
a('<line x1="1420" y1="780" x2="1440" y2="780" stroke="#333" stroke-width="2"/><line x1="1500" y1="780" x2="1520" y2="780" stroke="#333" stroke-width="2"/>')
line([(1304,720),(1340,720),(1340,780),(1420,780)],'#555')
line([(1304,790),(1330,790),(1330,830),(1530,830),(1530,780),(1520,780)],'#555')
dot(1420,780,'#555'); dot(1520,780,'#555')
rect(1390,860,160,80,'#eee','#555',rx=6)
t(1470,893,'Factory motor',13,'middle','bold'); t(1470,912,'controller (230 V)',12,'middle',c='#555')
a('<line x1="1440" y1="830" x2="1440" y2="860" stroke="#555" stroke-width="2" stroke-dasharray="4,3"/><line x1="1500" y1="830" x2="1500" y2="860" stroke="#555" stroke-width="2" stroke-dasharray="4,3"/>')
t(1335,950,'SSR in parallel with button contacts',12,'end',c='#444')

# Legend / notes
rect(40,600,470,500,'#fff','#bbb',rx=8)
t(60,630,'Connection table',17,w='bold')
rows=[('3V3 (P2-1)','HX711 VCC, Pololu VIN',RED),('GND (P2-2)','HX711 GND, Pololu GND, R',BLK),('GPIO15 (P2-7)','HX711 SCK',ORG),('GPIO13 (P2-8)','HX711 DT (DOUT)',BLU),('GPIO11 (P2-9)','Pololu EN (+ R to GND)',PUR),('Load cell shield','clear wire → GND','#b8c4cc'),('Pololu OUT×2','in parallel with button contacts','#555'),('BAT+ / BAT−','18650 via KCD1-11 (+ line)',RED)]
y=660
for a1,b,c in rows:
    a(f'<rect x="60" y="{y-12}" width="14" height="14" fill="{c}"/>'); t(84,y,a1,13,w='bold'); t(215,y,b,13); y+=26
t(60,y+14,'Notes',16,w='bold'); y+=40
notes=['Before connecting the SSR, measure the voltage on the',' button contacts. Above 100 V or mains 230 V – do not connect.','Check the MX1.25 plug polarity with a multimeter.','Load cell: red E+, black E−, green A+,',' white A−, clear = shield (to GND).','Everything on header P2; do not attach a camera.','KCD1-11 cuts the battery only; USB-C still powers the board.','Firmware: relay LOW after reset, 18 s continuous-run','  limit, 30 s timeout, touch stop.']
for n in notes: t(60,y,n,13,c='#333'); y+=21
a('</svg>')
open('docs/img/wiring-diagram.svg','w').write('\n'.join(o))
import cairosvg; cairosvg.svg2png(url='docs/img/wiring-diagram.svg',write_to='docs/img/wiring-diagram.png')
