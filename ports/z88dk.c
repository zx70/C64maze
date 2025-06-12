
//  zcc +zx81 -clib=wrx -subtype=_wrx -pragma-define:hrgpage=40000 -create-app -DGFXSCALEX=4/5 -DGFXSCALEY=4/5 -lm -O3 -DPLATFORM_MAZE=z88dk c64maze.c ports/z88dk.c

// zcc +zx -lndos -create-app -DGFXSCALEX=4/5 -DGFXSCALEY=4/5 -lm -DPLATFORM_MAZE=z88dk c64maze.c ports/z88dk.c


#include "stdio.h"
#include "graphics.h"
#include "games.h"
#include "time.h"
#include "conio.h"
#include <bgi.h>

#define	clear_area(a,b,c,d)	stencil_init(bgi_stencil);stencil_add_side((a),(b),(a),(d),bgi_stencil);stencil_add_side((c),(b),(c),(d),bgi_stencil);stencil_render(bgi_stencil,0)

//#include "../vic_font.h"
//#include "../sid_tune.h"
#include "UNIX.h"
#include "../c64maze.h"
#include <time.h>
#include <stdbool.h>
/* local definitions */
#define VIC_II_START 53248U
#define VIC_II_Y_SCROLL 53265U
#define     BMM 32  // Monochromatic high resolution mode
#define VIC_II_SCREEN_CHAR 53272U
#define SCREEN_BORDER 53280U

#define BASE 0xA000U
#define COLOR_MEM 0x8C00U


/* local vars */


static unsigned char pix_pos[]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
unsigned int cnt;
#define STACK_SIZE 256
unsigned char stackSize[STACK_SIZE];

unsigned char *list1;
unsigned char *ptr1;
unsigned char wsh1;

unsigned char *list2;
unsigned char *ptr2;
unsigned char wsh2;

unsigned char *list3;
unsigned char *ptr3;
unsigned char wsh3;


struct font{
    unsigned char *pDesc;   /* Pointer to the character raster array */
    unsigned int pos[256]; /* Position of the symbol */
    unsigned char incX;     /* Increment in X (-1 means a proportional font) */
    unsigned char incY;     /* Increment in Y */
    unsigned char magnification;  /* Magnification */
};

struct font f;

/* local funcs defs */
static void port_process_voice(unsigned char **ptr, unsigned char *sid_pointer,
    unsigned char *wsh);
/* funcs */
void port_pset(unsigned int x, unsigned int y)
{
    plot(x, y);
}

void port_clearHGRpage(void)
{
	clg();
}

void port_clearMazeRegion(void)
{

	clear_area(0,0,disp_bounds.labyrinthx,disp_bounds.labyrinthy);
	
}

/** Switch on the HGR monochrome graphic mode.
*/
int port_graphics_init(void)
{
    initgraph(0,0,0);
	
    if(getmaxx() < getmaxy()) {
        disp_bounds.szx = getmaxx();
        disp_bounds.szy = getmaxx() * SIZEY / SIZEX;
        /* calculate labyrinth size */
        disp_bounds.labyrinthx = getmaxx() * 2 / 3;
        disp_bounds.labyrinthy = disp_bounds.labyrinthx / SIZEX * SIZEY;
    } else {
        disp_bounds.szy = getmaxy();
        disp_bounds.szx = getmaxy() * SIZEX / SIZEY;
        disp_bounds.labyrinthy = getmaxy() * 2 / 3;
        disp_bounds.labyrinthx = disp_bounds.labyrinthy * SIZEY / SIZEX;
    }
    disp_bounds.stepszx = disp_bounds.labyrinthx * STEPSIZEX / SIZEX;
    disp_bounds.stepszy = disp_bounds.labyrinthx * STEPSIZEY / SIZEX;
}

void port_vert_line(unsigned short x1, unsigned short y1, unsigned short y2)
{
    line(x1,y1,x1,y2);
}

void port_diag_line(unsigned short x1, unsigned short y1, unsigned short ix,
    short incx, short incy)
{
    //putpixel(x1,y1,1);
	line(x1, y1, x1+incx, y1+incy);
}

void port_hor_line(unsigned short x1, unsigned short x2, unsigned short y1)
{
    line(x1, y1, x2, y1);
}

void port_printat(unsigned short x, unsigned short y, char *s)
{
//	gotoxy(x,y);
//	printf("%s",s);
	outtextxy(x,y,s);

}

/* Plot a line using the Bresenham algorithm
   from Nelson Johnson, "Advanced Graphics in C"
   ed. Osborne, McGraw-Hill.
   Horizontal and vertical lines need to be considerably
   speeded up by using a direct access to video RAM.
*/
void port_line(unsigned short x1, unsigned short y1,
        unsigned short x2, unsigned short y2)
{
    line(x1, y1, x2, y2);
}

unsigned long port_get_time(void)
{
    return time(NULL);
}

void port_colour_banner(void)
{
}

long port_get_current_time(void)
{
    return time(NULL);
}

static void port_process_voice(unsigned char **ptr, unsigned char *sid_pointer,
    unsigned char *wsh)
{
    /* TODO */
}
unsigned char port_sound_irq(void)
{
    /* TODO */
    return 0;
}

void port_start_sound(unsigned char *l1, unsigned char *l2, unsigned char *l3)
{
    /* TODO */
}

void port_loadVICFont(unsigned char magnification)
{
	settextstyle(DEFAULT_FONT, HORIZ_DIR, magnification*11);
	
}

char port_getch(void)
{
    return fgetc_cons();
}

void port_fflushMazeRegion(void)
{
}
void port_music_on(void){}
void port_music_off(void){}
void port_font_magnification(unsigned char magnification)
{
//    (void)magnification;
}
void port_exit(void)
{

	exit(0);
}
