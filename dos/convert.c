#include <stdio.h>
#include <allegro.h>

FILE *out;
BITMAP *in;
RGB pal[256];

int x,y,count, skip, writ,r,g,b, c, back;

void main(int argc, char *argv[])
{
  if (argc<3)
  {
    printf("Pass infile.bmp outfile.txt\n");
    exit(1);
  }

  skip=0;
  allegro_init();

  set_color_depth(8);

  while (skip<99)
  { skip++;

  count=0;
  writ=0;

  out=fopen(argv[2],"w");
  in=load_bmp(argv[1], pal);
  back=getpixel(in,0,0);

  for (y=0; y<160; y++)
    for (x=0; x<240; x+=2)		// GBA stars are two pixels wide
      if (different(x, y, back))
      {
        if (count%skip == 0)
        { 
			/* gba color runs from 0-31, these from 0-63 */
			c=getpixel(in,x,y);
			r=pal[c].r;
			g=pal[c].g;
			b=pal[c].b;
			/* average 2 pixels, and divide by 2 for proper range */
			c=getpixel(in,x+1,y);
			r=(r+pal[c].r)/4;
			g=(g+pal[c].g)/4;
			b=(b+pal[c].b)/4;

          fprintf(out,"{ %d, %d,  %d},\n",x,y, (b<<10)|(g<<5)|(r));
          writ++;
        }
        count++;
      }

  fclose(out);
  destroy_bitmap(in);
  printf("\nSkip %d - %d dots, %d written\n",skip, count, writ);
  if (writ<500) skip=999;
  }
}
      
int different(int x, int y, int back)
{
RGB one, two;
int c;

one.r=pal[back].r;
one.g=pal[back].g;
one.b=pal[back].b;

c=getpixel(in,x,y);
two.r=pal[c].r;
two.g=pal[c].g;
two.b=pal[c].b;
/* average 2 pixels */
c=getpixel(in,x+1,y);
two.r=(two.r+pal[c].r)/2;
two.g=(two.g+pal[c].g)/2;
two.b=(two.b+pal[c].b)/2;

if (abs(one.r-two.r)+abs(one.g-two.g)+abs(one.b-two.b) >= 15)
  return 1;

return 0;
}

int abs(int x)
{
if (x>=0) return x;
return (x*(-1));
}



