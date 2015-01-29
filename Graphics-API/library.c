#include <sys/types.h>
#include <time.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <termios.h>
#include <sys/stat.h>

typedef unsigned short color_t;
int fileDescription;
unsigned short *fbp;
unsigned long xVirtR;
unsigned long yVirtR;
unsigned long size;
void init_graphics();
void exit_graphics();
void clear_screen();
void get_key();
void sleep_ms( long );
void get_pixel(int, int, color_t);
void get_rect(int, int, int, int, color_t);

void init_graphics()
{
	struct fb_var_screeninfo scrinf;
	struct fb_fix_screeninfo bD;
	struct termios termiosVar;
	int file_desc = open("/dev/fb0", O_RDWR);
	ioctl( file_desc, FBIOGET_VSCREENINFO, &scrinf );
	ioctl( file_desc, FBIOGET_FSCREENINFO, &bD );
	fileDescription = file_desc;
	xVirtR = scrinf.xres_virtual;
	yVirtR = scrinf.yres_virtual;
	size = bD.line_length;
	fbp = (unsigned short *)mmap( NULL, yVirtR * size , PROT_WRITE, MAP_SHARED, file_desc, 0 );
	ioctl( STDIN_FILENO, TCGETS, &termiosVar );
	termiosVar.c_lflag &= ~ICANON;
	termiosVar.c_lflag &= ~ECHO;
	ioctl( STDIN_FILENO, TCSETS, &termiosVar );
	clear_screen();	
}
void exit_graphics()
{
	struct termios termiosVar;
	ioctl( STDIN_FILENO, TCGETS, &termiosVar );	
	termiosVar.c_lflag |= ECHO;
	termiosVar.c_lflag |= ICANON;
	ioctl( STDIN_FILENO, TCSETS, &termiosVar );
	munmap( fbp, size * yVirtR ); 
	close( fileDescription );
}
void clear_screen()
{	
	int clearScreen = write( 1, "\033[2J", 8 );
}
char getkey()
{
	fd_set fdSetVar;
	struct timeval timeout;
	int r;
	char b;
	FD_ZERO(&fdSetVar);
	FD_SET( STDIN_FILENO, &fdSetVar );
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	r = select( STDIN_FILENO+1, &fdSetVar, NULL, NULL, &timeout );	
	if( r > 0 )
	{
		read( 0, &b, sizeof(b) );
	}
	return b;	
}
void sleep_ms( long ms )
{
    struct timespec timespecVar;
	timespecVar.tv_sec = 0;
	timespecVar.tv_nsec = ms * 1000000;
	nanosleep( &timespecVar, NULL );
}
void draw_pixel(int x, int y, color_t color)
{
	if(x < 0 || x >= xVirtR)
	{
		if(y < 0 || y >= yVirtR)
		{
		return;
		}
	}
 	unsigned long v_o = (size/2) * y;
	unsigned long h_o = x;
	unsigned short *s_ptr = (fbp + v_o + h_o);
	*s_ptr = color;	
}
void draw_rect(int x1, int y1, int width, int height, color_t color )
{
	int x;
	int y;	
	for(x = x1; x < x1 + width; x++)
	{
		for(y = y1; y < y1 + height; y++)
		{
			draw_pixel(x, y, color);
		}
	}
}

void draw_circle(int x0,int y0,int radius)
{
    int x = radius;
    int y =0;
    int radiusError = 1-x;
    
    while(x>=y)
    {
        draw_pixel(x+x0,y+y0,15);
        draw_pixel(y+x0,x+y0,15);
        draw_pixel(-x+x0,y+y0,15);
        draw_pixel(-y+x0,x+y0,15);
        draw_pixel(-x+x0,-y+y0,15);
        draw_pixel(-y+x0,-x+y0,15);
        draw_pixel(x+x0,-y+y0,15);
        draw_pixel(y+x0,-x+y0,15);
        y++;
        if(radiusError<0)
        {
            radiusError += 2*y + 1;
        }
        else
        {
            x--;
            radiusError += 2*(y-x+1);
        }
    }
}

