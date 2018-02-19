#include <3DS.h>
#include "utils.h"
#include "letters.h"

void paint_word(u8* dest,char* word, int x, int y, char r, char g, char b) {
    int tmp_x = x;
    int i;
    int line = 0;

    for (i = 0; i < slen(word); i++) {

        if (tmp_x + 8 > 400) {
            line++;
            tmp_x = x;
        }
        paintLetter(dest,word[i], tmp_x, y + (line * 8), r, g, b);
        tmp_x = tmp_x + 8;
    }
}

void paintLetter(u8* dest,char letter, int x, int y, char r, char g, char b) {

    int i = 0;
    int k = 0;
    int c = 0;
    char mask = 0;
    //unsigned char* _letter;
    char l = 0;
    c = letter * 8;

    for (i = 0; i < 8; i++) {
        mask = 0b10000000;
        l = font[i + c];
        for (k = 0; k < 8; k++) {
            if ((mask >> k) & l) {
                paintPixel(dest,k + x, i + y, r, g, b);
            }
        }
    }
}

void paintPixel(u8* dest,int x, int y, char r, char g, char b) {
        dest[3*(240-y+x*240)+2] = r; //red
	dest[3*(240-y+x*240)+1] = g; //green
	dest[3*(240-y+x*240)] = b; //blue
}

int slen(char* string){
  int i;
  for (i = 0; ; i++){
      if (string[i] == 0x00){
        return i;
      }
  }
}

