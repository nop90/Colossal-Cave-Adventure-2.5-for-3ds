#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include "swkbd.h"
#include "kbd_default.h"
#include "swkbfont.h"

//#define CLOCKS_PER_SEC 0x80C0000
#define MAX_SKIN 8
#define SWKBD_MAGIK 0xFF

// this is a dirty hack for Adventure - Not related to swKeyboard
extern int txtColorSwitched;

void key(u8* buf,int cursorpos,u8 ch);
void drawTxtArea(u8* dest, u32 txtborderrgb, u32 txtbgrgb);
void printWord(u8* dest,u8* word, int x, int y, u8 r, u8 g, u8 b);
void printLetter(u8* dest,u8 letter, int x, int y, u8 r, u8 g, u8 b);
void printPixel(u8* dest,int x, int y, u8 r, u8 g, u8 b);
int loadBitmap(u8* path, u32 width, u32 height, void* buf, u32 alpha, u32 startx, u32 stride, u32 flags);

//FS_archive sdmcArchive;

u8* topscreenbuf;
u8* skin;
u32 fontcolor;
u32 bgcolor;
u32 txtbgcolor;
u32 txtbordercolor;
u32 check=0;
u32 BUFLEN;

static u8 kbd_layout [3][3][10] =
	{
	  {
 	    {'Q','W','E','R','T','Y','U','I','O','P'},
 	    { 0 ,'A','S','D','F','G','H','J','K','L'},
 	    { 0 ,'Z','X','C','V','B','N','M', 0 , 0 }
	  } , {
 	    {'1','2','3','4','5','6','7','8','9','0'},
 	    {'-','/',':','~','(',')','$','&','\'','"'},
 	    {'@','.',',','?','!','#','%','*', 0 , 0 }
	  } , {
 	    {'q','w','e','r','t','y','u','i','o','p'},
 	    { 0 ,'a','s','d','f','g','h','j','k','l'},
 	    { 0 ,'z','x','c','v','b','n','m', 0 , 0 }
	  } 
	}; 

void swkbd_Init()
{
	if(check!=SWKBD_MAGIK)
	{
	  skin = (kbd_default + 0x36);
	  fontcolor = 0x00000000;
	  bgcolor = 0x00dddddd;
	  txtbgcolor = 0x00ffffff;
	  txtbordercolor = 0x00000000;
	  check=SWKBD_MAGIK;
	  topscreenbuf=NULL;
	}
}

void swkbd_Exit()
{
}

void swkbd_SetFontColor(u32 color_0rfb)
{
	fontcolor=color_0rfb;
}

void swkbd_SetBgColor(u32 color_0rfb)
{
	bgcolor=color_0rfb;
}

void swkbd_GetStr(u8* buf, u32 buflen)
{
//    APP_STATUS status;
    u32 lastKeysPressed= 0x0fffffff; // to avoid unwanted exit when function is called pressing a key
    u32 keysPressed;
    u8 touchstate=0;
    u8 lasttouchstate=1;
    touchPosition touchpos;
    int updatescreen=1;
    int cursorpos;
    int cursorpos2;
    int sLen;
    u8 kbdstate=2;
    u8 shiftstate=0;
    u8* bfb;
    u8 r,g,b;
    u32 x,y;
    u8 kbdtouchrow;    
    u8 kbdtouchcol;
    u8 printtextbuf[23];

    BUFLEN=buflen;

    for (cursorpos=0; (buf[cursorpos]>0) && (cursorpos < buflen); cursorpos++);
    if (cursorpos==buflen) { 
      cursorpos=0;
      buf[0]=0;
    }
    if (cursorpos>=25) cursorpos2=25;
    else cursorpos2=cursorpos;
    while(aptMainLoop()) {
        gspWaitForVBlank();
	hidScanInput();
	hidTouchRead(&touchpos);
	keysPressed = hidKeysHeld();

// quick and dirty addon to change console textcolor in Adventue. Not related to swkeyboard 	
	if (!txtColorSwitched && (keysPressed & KEY_R) && (hidKeysDown() & KEY_UP))
	{ 
		printf("\x1b[37;1mtext color switched to white\n\n>");
		txtColorSwitched=1;
    }
	if (txtColorSwitched && (keysPressed & KEY_R) && (hidKeysDown() & KEY_DOWN))
	{ 
		printf("\x1b[32;1mText color switched to green\n\n>");
		txtColorSwitched=0;
    }

	touchstate=0;

	if ((keysPressed & KEY_A) && !(lastKeysPressed & KEY_A))
	{ 
		bfb = gfxGetFramebuffer(GFX_BOTTOM, GFX_BOTTOM, NULL, NULL);
		memset(bfb,0,240*320*3);
		gfxFlushBuffers();
		gfxSwapBuffers();
		return; 
	}
	if ((keysPressed & KEY_B) && !(lastKeysPressed & KEY_B))
	{
	  if (cursorpos>0)
	  {
	    int i=--cursorpos;
	    for(;(buf[i]!=0) & (i<(BUFLEN-1));i++) buf[i]=buf[i+1]; 
	    if (shiftstate) shiftstate=kbdstate=2;
	    updatescreen=1;
	  }
	}
	if ((keysPressed & KEY_X) && !(lastKeysPressed & KEY_X))
	{
	  if (cursorpos>0)
	  {
	    buf[0]=0;
	    cursorpos=0;
	    updatescreen=1;
	  }
	}
	if ((keysPressed & KEY_DLEFT) && !(lastKeysPressed & KEY_DLEFT))
	{
	  if (cursorpos>0)
	  {
	    cursorpos-- ;
	    updatescreen=1;
	  }
	}
	if ((keysPressed & KEY_DRIGHT) && !(lastKeysPressed & KEY_DRIGHT))
	{
	  if ((cursorpos<BUFLEN)&&(buf[cursorpos]!=0))
	  {
	    cursorpos++ ;
	    updatescreen=1;
	  }
	}

	lastKeysPressed = keysPressed;
	

	kbdtouchrow=4; // default = out of keyboard
	if (touchpos.py>=3+95) {
	  touchstate=1;
	  if (touchpos.py<=36+95) {
	    kbdtouchrow=0;		
	  } else if (touchpos.py<=73+95) {
	    kbdtouchrow=1;		
	  } else if (touchpos.py<=109+95) {
	    kbdtouchrow=2;		
	  } else kbdtouchrow=3;		
	}

	if (!lasttouchstate) switch (kbdtouchrow)
	{
	  case 0:
	    kbdtouchcol= (touchpos.px / 32) ;
	    key(buf,cursorpos,kbd_layout[kbdstate][kbdtouchrow][kbdtouchcol]);
	    cursorpos++;
	    if (shiftstate)
	    {
	      shiftstate=0;
	      kbdstate=2;
	    }
	    updatescreen=1;
	    break;
	  case 1:
	    kbdtouchcol= (touchpos.px / 32) ;
	    if (kbdtouchcol==0)
	    {
	      switch (kbdstate)
	      {
	        case 0:
	          kbdstate=2;
	          break;
	        case 1:
	          key(buf,cursorpos,kbd_layout[kbdstate][kbdtouchrow][kbdtouchcol]); // '-'
	          cursorpos++;
	          break;
	        case 2:
	          kbdstate=0;
	          break;
	      }
	    } else {
	      key(buf,cursorpos,kbd_layout[kbdstate][kbdtouchrow][kbdtouchcol]);
	      cursorpos++;
	    }
	    if (shiftstate)
	    {
	      shiftstate=0;
	      kbdstate=2;
	    }
	    updatescreen=1;
	    break;
	  case 2:
	    if (touchpos.px<=48) 	// Caps on/of or @ char	
	    {
	      switch (kbdstate)
	      {
	        case 0:
	          kbdstate=2;
	          shiftstate=0;
	          break;
	        case 1:
	          key(buf,cursorpos,kbd_layout[kbdstate][kbdtouchrow][0]); // '@'
	          cursorpos++;
	          break;
	        case 2:
	          kbdstate=0;
	          shiftstate=1;
	          break;
	      }
	    } else {
	      if (touchpos.px>272) {  //Backspace
	        if (cursorpos>0) {
	          int i=--cursorpos;
	          for(;(buf[i]!=0) & (i<(BUFLEN-1));i++) buf[i]=buf[i+1]; 
	          if (shiftstate)
	          {
	            shiftstate=0;
	            kbdstate=2;
	          }
	        }
	      } else {
	        kbdtouchcol= ((touchpos.px -16) / 32) ;
	        key(buf,cursorpos,kbd_layout[kbdstate][kbdtouchrow][kbdtouchcol]);
	        cursorpos++;
	        if (shiftstate)
	        {
	          shiftstate=0;
	          kbdstate=0;
	        }
	      }
	    }
	    updatescreen=1;
	    break;
	  case 3:
	    if (touchpos.px>=256) { 
			bfb = gfxGetFramebuffer(GFX_BOTTOM, GFX_BOTTOM, NULL, NULL);
			memset(bfb,0,240*320*3);
			gfxFlushBuffers();
			gfxSwapBuffers();
			return;  // Ends input
		}
	    if (touchpos.px<=64) 	// Switch Letters / Numbers & Symbols	
	    {
	      switch (kbdstate)
	      {
	        case 0:
	          kbdstate=1;
	          break;
	        case 1:
	          kbdstate=2;
	          break;
	        case 2:
	          kbdstate=1;
	          break;
	      }
	    } else {
	      key(buf,cursorpos,' ');
	      cursorpos++;
	    }
	    if (shiftstate)
	    {
	      shiftstate=0;
	      kbdstate=2;
	    }
	    updatescreen=1;
	    break;
	  case 4:
	    break;
	}
	lasttouchstate=touchstate;


	if (updatescreen)
	{
	  r=(bgcolor&0xff0000)>>16;
	  g=(bgcolor&0xff00)>>8;
	  b=(bgcolor&0xff);
	  bfb = gfxGetFramebuffer(GFX_BOTTOM, GFX_BOTTOM, NULL, NULL);
	  for(x=0; x < 320; x++)
	    for(y=0; y < 145; y++)
            {
              bfb[3*(y+x*240)] = skin[3 * (y+145*kbdstate) * 320 + 3 * x ]; //red
	      bfb[3*(y+x*240)+1] = skin[3 * (y+145*kbdstate) * 320 + 3 * x + 1]; //green
	      bfb[3*(y+x*240)+2] = skin[3 * (y+145*kbdstate) * 320 + 3 * x + 2]; //blue
	    }
	  for(x=0; x < 320; x++)
	    for(y=146; y < 240; y++)
            {
              bfb[3*(y+x*240)] = r; 
	      bfb[3*(y+x*240)+1] = g; 
	      bfb[3*(y+x*240)+2] = b; 
	    }

	  drawTxtArea(bfb, txtbordercolor, txtbgcolor);

	  r=(fontcolor&0xff0000)>>16;
	  g=(fontcolor&0xff00)>>8;
	  b=(fontcolor&0xff);

	  sLen=strlen(buf);	
	  if (sLen<=21)
	  {
	    printWord(bfb,buf, 29, 42, r, g, b);
	    cursorpos2=cursorpos;
	  } else { 
	    if (cursorpos<=11)
	    {
	      for(x=0;x<19;x++) printtextbuf[x]=buf[x];
	      printtextbuf[19]='.'; 
	      printtextbuf[20]='.'; 
	      printtextbuf[21]='.'; 
	      printtextbuf[22]=0; 
	      cursorpos2=cursorpos;
	    }  else {
	      if(sLen-cursorpos<=11) {
	        printtextbuf[0]='.'; 
	        printtextbuf[1]='.'; 
	        printtextbuf[2]='.'; 
	        for(x=3;x<22;x++) printtextbuf[x]=buf[sLen-22+x];
	        printtextbuf[22]=0; 
	        cursorpos2=22-sLen+cursorpos;
	      }  else { 
	        printtextbuf[0]='.'; 
	        printtextbuf[1]='.'; 
	        printtextbuf[2]='.'; 
	        for(x=3;x<19;x++) printtextbuf[x]=buf[cursorpos-11+x];
	        printtextbuf[19]='.'; 
	        printtextbuf[20]='.'; 
	        printtextbuf[21]='.'; 
	        printtextbuf[22]=0; 
	        cursorpos2=11;
	      } 
	    } 
	    printWord(bfb,printtextbuf, 29, 42, r, g, b);
	  } 

	  for(y=0;y<16;y++) printPixel(bfb,28 + cursorpos2 * 12, 42 + y, r, g, b); // cursor 
	  updatescreen=0;

	  // Flush and swap framebuffers
	  gfxFlushBuffers();
	  gfxSwapBuffers();
	}
    }
	exit(0); // to exit program if it's closed from 3ds main menu
}

// Private functions

void key(u8* buf,int cursorpos,u8 ch)
{
  u32 i = strlen(buf)+1;
  if (i<BUFLEN-1)
  {
    for (;i>cursorpos;i--) buf[i]=buf[i-1];
    buf[cursorpos]= ch;
  }
}

void drawTxtArea(u8* dest, u32 txtborderrgb, u32 txtbgrgb){
int i,j;
u8 r1=(txtbgrgb & 0xff0000)>>16;
u8 g1=(txtbgrgb & 0xff00)>>8;
u8 b1=(txtbgrgb & 0xff);
u8 r2=(txtborderrgb & 0xff0000)>>16;
u8 g2=(txtborderrgb & 0xff00)>>8;
u8 b2=(txtborderrgb & 0xff);

  for(i=28;i<=292;i++){
    printPixel(dest,i,39, r2, g2, b2);
    printPixel(dest,i,60, r2, g2, b2);
    for(j=40;j<=59;j++){
      printPixel(dest,i,j, r1, g1, b1);
    }
  }
}

void printWord(u8* dest,u8* word, int x, int y, u8 r, u8 g, u8 b) {
    int tmp_x = x;
    int i;
    for (i = 0; i < strlen(word); i++) {
        printLetter(dest,word[i], tmp_x, y, r, g, b);
        tmp_x = tmp_x + 12;
    }
}

void printLetter(u8* dest,u8 letter, int x, int y, u8 r, u8 g, u8 b) {
    int i;
    int k = 0;
    u8 mask;
    u8 l1 = 0;
    u8 l2 = 0;

    for (i = 0; i < 12; i++) {
        mask = 0b10000000;
        l1 = swkbdfont[letter][i*2];
        l2 = swkbdfont[letter][(i*2) + 1];
        for (k = 0; k < 8; k++) {
            if ((mask >> k) & l1) 
                printPixel(dest,i + x, 16 - k + y, r, g, b);
            if ((mask >> k) & l2) 
                printPixel(dest,i + x, 8 - k + y , r, g, b);
        }
    }
}

void printPixel(u8* dest,int x, int y, u8 r, u8 g, u8 b) {
        dest[3*(240-y+x*240)+2] = r; //red
	dest[3*(240-y+x*240)+1] = g; //green
	dest[3*(240-y+x*240)] = b; //blue
}

