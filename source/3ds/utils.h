#pragma once

int slen(char* string);
void paint_word(u8* dest,char* word, int x, int y, char r, char g, char b);
void paintLetter(u8* dest,char letter, int x, int y, char r, char g, char b);
void paintPixel(u8* dest,int x, int y, char r, char g, char b);

