#include<stdlib.h>
#include<stdio.h>
#include "c64maze.h"
#ifndef NO_SOUND
#include"sid_tune.h"
#endif
#define EXPAND1(x) x
#define EXPAND2(x, y)    EXPAND1(x)y
#define EXPAND3(x, y, z) EXPAND2(x, y)z

#undef INCLUDE_NAME
#define INCLUDE_NAME <ports/EXPAND2(PLATFORM_MAZE,.h)>
#include INCLUDE_NAME

char labyrinth[] =  "****************************************"
                    "*      *    *     * *   *        *     *"
                    "* **** * ******** * *** * *** **   ** **"
                    "* * ** *   *  *   *     *   * *  *  *  *"
                    "*   *  * *  * *** ***** * * * * ***  * *"
                    "***** ** ** *             *** *   **** *"
                    "*             * ** **** * *     *  *   *"
                    "* * *********** *     * * * ****** * * *"
                    "* *         *   ** ** *   * *        * *"
                    "***** ***** * * *  *  * * * **** ** *  *"
                    "* *       * *** **** ** **     *  * ****"
                    "* * *** * * *     *  *    * ** * *     *"
                    "*     *** * ***** * ** **** *  * **** **"
                    "** ** *   *     * * *   *   * **    *  *"
                    "*  * ** ******* * * *** * *** *  ** ** *"
                    "**   *  *         * *   *      * *     *"
                    "****************************************";

char startx;
char starty;
char positionx;
char positiony;
unsigned char style=0x1;

/*#if (P_CURRENT== P_C64)
#define LABYRINTHSZX_DYN	SIZEX
#define LABYRINTHSZY_DYN	SIZEY
#define LABSTEPSZX_DYN	STEPSIZEX
#define LABSTEPSZY_DYN	STEPSIZEY
#else*/
display_bounds_t disp_bounds;
#define LABYRINTHSZX_DYN	disp_bounds.labyrinthx
#define LABYRINTHSZY_DYN	disp_bounds.labyrinthy
#define LABSTEPSZX_DYN	disp_bounds.stepszx
#define LABSTEPSZY_DYN 	disp_bounds.stepszy
//#endif


char exitx=13;
char exity=1;
    /*  0 = north
        1 = west
        2 = sud
        3 = east
    */
char orientation=0;
/* Turn on a pixel by accessing directly to the video RAM */
void pset(unsigned int x, unsigned int y)
{
    port_pset(x, y);
}

/* printat prints a text at the given location unsing the font described
   by the structure font */
void printat(unsigned short x, unsigned short y, char *s)
{
    port_printat(x, y, s);
}

void clearMazeRegion(void)
{
    port_clearMazeRegion();
}

void fflushMazeRegion(void)
{
    port_fflushMazeRegion();
}

void graphics_init(void)
{
    port_graphics_init();
}

void vert_line(unsigned short x1, unsigned short y1, unsigned short y2)
{
    port_vert_line(x1, y1, y2);
}

void diag_line(unsigned short x1, unsigned short y1, unsigned short ix,
    short incx, short incy)
{
    port_diag_line(x1, y1, ix, incx, incy);
}

void hor_line(unsigned short x1, unsigned short x2, unsigned short y1)
{
    port_hor_line(x1, x2, y1);
}

void line(unsigned short x1, unsigned short y1,
        unsigned short x2, unsigned short y2)
{
    port_line(x1, y1, x2, y2);
}

/* box draws a box given the coordinates of the two
   diagonal corners. From Nelson Johnson, "Advanced
   Graphics in C" ed. Osborne, McGraw-Hill
 */
void box(unsigned short x1, unsigned short y1, unsigned short x2,
    unsigned short y2)
{
    static unsigned short xul;
    static unsigned short yul;
    static unsigned short xlr;
    static unsigned short ylr;

    xul=(x2>x1) ? x1 : x2;
    yul=(y2>y1) ? y1 : y2;
    xlr=(x2>x1) ? x2 : x1;
    ylr=(y2>y1) ? y2 : y1;
    // The simplified code does not handle every style lines, so one must check
    // for it.
    if(style==0x1) {
        hor_line(xul, xlr, yul);
        hor_line(xul, xlr, ylr);
        vert_line(xul, yul, ylr);
        vert_line(xlr, yul, ylr);
    } else {
        line(xul, yul, xlr, yul);
        line(xlr, yul, xlr, ylr);
        line(xlr, ylr, xul, ylr);
        line(xul, ylr, xul, yul);
    }
}

char getch(void)
{
    return port_getch();
}

unsigned long get_time(void)
{
    return port_get_time();
}
/** Choose randomly the starting position in the maze.
*/
void choose_start_position()
{
    unsigned int time=get_time();
    srand(time);
    do {
        startx=(labyrinthSizeX*(rand()/(RAND_MAX/100)))/100;
        starty=(labyrinthSizeY*(rand()/(RAND_MAX/100)))/100;
    } while(labyrinth[startx+starty*labyrinthSizeX]!=' ');
    positionx=startx;
    positiony=starty;
}

int leftx;
int lefty;
int rightx;
int righty;
int advancex;
int advancey;

void set_orientation(void)
{
    switch(orientation) {
        case 0:
            leftx=-1;
            lefty=0;
            rightx=1;
            righty=0;
            advancex=0;
            advancey=-1;
            break;
        case 1:
            leftx=0;
            lefty=1;
            rightx=0;
            righty=-1;
            advancex=-1;
            advancey=0;
            break;
        case 2:
            leftx=1;
            lefty=0;
            rightx=-1;
            righty=0;
            advancex=0;
            advancey=1;
            break;
        case 3:
            leftx=0;
            lefty=-1;
            rightx=0;
            righty=1;
            advancex=1;
            advancey=0;
            break;
    }
}

/** In this function, the maze view is shown from the current point and
    orientation of the player (those are specified in global variables).
*/
void drawLabyrinthView()
{
    unsigned char posx=positionx;
    unsigned char posy=positiony;
    unsigned char step=0;
    unsigned int sszx;
    unsigned int sszy;
    unsigned int sszxp1;
    unsigned int sszyp1;
    unsigned char wall=FALSE;
    unsigned char wayout=FALSE;

    style=0x1;
    set_orientation();

    /* Draw the maze in isometric perspective starting from the position of
       the player and going progressively farther from him, until a wall is
       hit or until the distance becomes greater than 5 steps.
    */
    for(step=0;(wall==FALSE)&&(step<6);++step) {
        /* Specify the line style so that walls and corners far from the
           player somewhat appears to be less definite, thus adding a certain
           3D effect.
        */
        switch(step) {
            case 3:
                style=0x2;
                break;
            case 4:
                style=0x8;
                break;
            case 5:
                style=0x20;
                break;
            default:
                style=0x1;
                break;
        }
        // We hit a wall!
        if(labyrinth[posx+posy*labyrinthSizeX]=='*') {
            break;
        }
        // Some pre-calculated data for wall and corner drawing.
        sszx=step*LABSTEPSZX_DYN;
        sszy=step*LABSTEPSZY_DYN;
        sszxp1=(step+1)*LABSTEPSZX_DYN;
        sszyp1=(step+1)*LABSTEPSZY_DYN;
        // Wall on the right?
        if(labyrinth[posx+rightx+(posy+righty)*labyrinthSizeX]=='*') {
            line(LABYRINTHSZX_DYN-sszx,sszy,
                 LABYRINTHSZX_DYN-sszxp1,sszyp1);
            line(LABYRINTHSZX_DYN-sszx,LABYRINTHSZY_DYN-sszy,
                 LABYRINTHSZX_DYN-sszxp1,LABYRINTHSZY_DYN-sszyp1);
        } else {
            // Closer vertical line
            line(LABYRINTHSZX_DYN-sszx,sszy,
                 LABYRINTHSZX_DYN-sszx,LABYRINTHSZY_DYN-sszy);
            // Farther vertical line
            if(labyrinth[posx+advancex+(posy+advancey)*labyrinthSizeX]!='*') {
                line(LABYRINTHSZX_DYN-sszxp1,sszyp1,
                    LABYRINTHSZX_DYN-sszxp1,LABYRINTHSZY_DYN-sszyp1);
            }
            // Upper horisontal line
            line(LABYRINTHSZX_DYN-sszxp1,sszyp1,
                 LABYRINTHSZX_DYN-sszx,sszyp1);
            // Lower horisontal line
            line(LABYRINTHSZX_DYN-sszxp1,LABYRINTHSZY_DYN-sszyp1,
                 LABYRINTHSZX_DYN-sszx,LABYRINTHSZY_DYN-sszyp1);
        }
        // Wall on the left?
        if(labyrinth[posx+leftx+(posy+lefty)*labyrinthSizeX]=='*') {
            line(sszx,sszy,
                 sszxp1,sszyp1);
            line(sszx,LABYRINTHSZY_DYN-sszy,
                 sszxp1,LABYRINTHSZY_DYN-sszyp1);
        } else {
            // Closer vertical line
            line(sszx,sszy,
                 sszx,LABYRINTHSZY_DYN-sszy);
            // Farter vertical line
            if(labyrinth[posx+advancex+(posy+advancey)*labyrinthSizeX]!='*') {
                line(sszxp1,sszyp1,
                 sszxp1,LABYRINTHSZY_DYN-sszyp1);
            }
            // Upper horisontal line
            line(sszxp1,sszyp1,
                 sszx,sszyp1);
            // Lower horisontal line
            line(sszxp1,LABYRINTHSZY_DYN-sszyp1,
                 sszx,LABYRINTHSZY_DYN-sszyp1);
        }
        // Advance one step farther from the player.
        posx+=advancex;
        posy+=advancey;
        // Exit is in sight!
        if (posx==exitx && posy==exity) {
            wall=TRUE;
            wayout=TRUE;
        }
        // A wall is in sight!
        if(labyrinth[posx+posy*labyrinthSizeX]=='*') {
            wall=TRUE;
        }
    }
    // We have a wall at the end of our sight
    if(wall==TRUE)
        box(step*LABSTEPSZX_DYN,step*LABSTEPSZY_DYN,
            LABYRINTHSZX_DYN-step*LABSTEPSZX_DYN,LABYRINTHSZY_DYN-step*LABSTEPSZY_DYN);
    // The exit is in sight!
    if(wayout) {
        ++step;
        box(step*LABSTEPSZX_DYN,step*LABSTEPSZY_DYN,
            LABYRINTHSZX_DYN-step*LABSTEPSZX_DYN,LABYRINTHSZY_DYN-step*LABSTEPSZY_DYN);
        line(step*LABSTEPSZX_DYN,step*LABSTEPSZY_DYN,
            LABYRINTHSZX_DYN-step*LABSTEPSZX_DYN,LABYRINTHSZY_DYN-step*LABSTEPSZY_DYN);
        line(LABYRINTHSZX_DYN-step*LABSTEPSZX_DYN,step*LABSTEPSZY_DYN,
            step*LABSTEPSZX_DYN,LABYRINTHSZY_DYN-step*LABSTEPSZY_DYN);
    }
}

/** Be sure that the current player position is correct.
*/
void validate_data()
{
    if(positionx>=labyrinthSizeX)
        positionx=labyrinthSizeX-1;
    if(positiony>=labyrinthSizeY)
        positiony=labyrinthSizeY-1;
    if(positiony<=0)
        positiony=0;
    if(positiony<=0)
        positiony=0;
}

/** Advance in the forward direction, following the current orientation.
*/
void move_forward()
{
    switch(orientation) {
        case 0:
            --positiony;
            break;
        case 1:
            --positionx;
            break;
        case 2:
            ++positiony;
            break;
        case 3:
            ++positionx;
            break;
    }
    validate_data();
}

/** Go backwards one step, following the current orientation.
*/
void move_backwards()
{
    switch(orientation) {
        case 0:
            ++positiony;
            break;
        case 1:
            ++positionx;
            break;
        case 2:
            --positiony;
            break;
        case 3:
            --positionx;
            break;
    }
    validate_data();
}

/** Colour the right side banner in the screen view.
*/
void colour_banner(void)
{
    port_colour_banner();
}

/** Draw the banner on the right side of the screen.
*/
void draw_banner(void)
{
    colour_banner();
    port_loadVICFont(2);
#if P_CURRENT == P_C64
    printat(204,17,"c64maze");
    port_loadVICFont(1);
    box(251,53,278,84);
    printat(260,55,"t");
    printat(252,65,"f+g");
    printat(260,75,"v");
    printat(207,100,"[p] view maze");
#ifndef NO_SOUND
    printat(207,110,"[m] music 1/0");
#endif
    printat(207,120,"[a] restart  ");
    printat(207,170,"d. bucci 2017");
    printat(207,160,"igor1101 2019");
    port_loadVICFont(2);
    line(200,0,200,199);
#elif (P_CURRENT == P_UNIX)
    unsigned short xoffset = disp_bounds.bannerx;
    unsigned short xbannersz = disp_bounds.bannerx_end - disp_bounds.bannerx;
    unsigned short xbannerhalfsz = xbannersz / 2;
    unsigned short xbanner13sz = xbannersz / 3;
    unsigned short xbanner23sz = xbannersz * 2 / 3;
    unsigned short xmiddle = xoffset + xbannerhalfsz;
    unsigned short xmiddletxt = xoffset + xbanner13sz;
    unsigned short xmiddletxt_end = xoffset + xbanner23sz;
    unsigned short ymiddle = disp_bounds.bannery_end / 2;
    unsigned short yend = disp_bounds.bannery_end;
    printat(xmiddletxt,17,"c64maze");
    port_loadVICFont(1);
    box(xoffset, 0, xoffset + xbannersz, yend);
    box(xmiddletxt - 2,ymiddle - 40 ,xmiddletxt_end + 2,ymiddle + 40);
    printat(xmiddle,ymiddle - 20,"t");
    printat(xmiddle - 18,ymiddle,"f+g");
    printat(xmiddle,ymiddle + 20,"v");
    printat(xoffset,ymiddle + 40,"[p] view maze");
#ifndef NO_SOUND
    printat(xoffset,ymiddle + 60,"[m] music 1/0");
#endif
    printat(xoffset,ymiddle + 80,"[a] restart  ");
    printat(xoffset,ymiddle - 60,"d. bucci 2017");
    printat(xoffset,ymiddle - 80,"igor1101 2019");
    port_loadVICFont(2);
    line(xoffset,0,200,199);
#endif /* platform == C64 */
}

long start_time;

long get_current_time(void)
{
    return port_get_current_time();
}

char *write_time(char *message, unsigned char start)
{
    long approx=0;
    char thousands=0;
    char hundreds=0;
    char tens=0;
    char seconds=0;
    approx=get_current_time()-start_time;
    if(approx>60*1000) {
        thousands = approx/60l/1000l;
        approx-=thousands*60l*1000l;
    }
    if(approx>60*100) {
        hundreds= approx/60l/100l;
        approx-=hundreds*60l*100l;
    }
    if(approx>60*10) {
        tens= approx/60l/10l;
        approx-=tens*60l*10l;
    }
    if(approx>60) {
        seconds=approx/60l;
        approx-=seconds*60l;
    }

    message[start++]= thousands+'0';
    message[start++]= hundreds+'0';
    message[start++]= tens + '0';
    message[start++]= seconds + '0';
    return message;
}

/** Show the maze with the current position and orientation.
*/
void show_maze()
{
    unsigned char x = 0;
    unsigned char y = 0;
    unsigned int by = 0;
    const char *pt = 0;

    char message[]="elapsed:      s";

    style=0x1;

    start_time-=60l*30l; // 30 seconds penalty.

    port_clearHGRpage();
    /* findout appropriate box size */
    for(y=0; y<labyrinthSizeY;++y) {
        //by=COLOR_MEM+y*40;
        pt=labyrinth+y*labyrinthSizeX;
        for(x=0; x<labyrinthSizeX;++x) {
            if(pt[x]=='*') {
                //POKE(by,9);
#ifndef SZ
#define SZ	8
#endif
                box(x*SZ,y*SZ, x*SZ+SZ, y*SZ+SZ);
            } else if(positiony==y && positionx==x) {
                box(x*SZ+2,y*SZ+2,x*SZ+5,y*SZ+5);
                switch(orientation) {
                    case 0:
                        line(x*SZ+3,y*SZ,x*SZ+3,y*SZ+2);
                        line(x*SZ+4,y*SZ,x*SZ+4,y*SZ+2);
                        break;
                    case 1:
                        line(x*SZ,y*SZ+3,x*SZ+2,y*SZ+3);
                        line(x*SZ,y*SZ+4,x*SZ+2,y*SZ+4);
                        break;
                    case 2:
                        line(x*SZ+3,y*SZ+5,x*SZ+3,y*SZ+7);
                        line(x*SZ+4,y*SZ+5,x*SZ+4,y*SZ+7);
                        break;
                    case 3:
                        line(x*SZ+5,y*SZ+3,x*SZ+7,y*SZ+3);
                        line(x*SZ+5,y*SZ+4,x*SZ+7,y*SZ+4);
                        break;
                }
            }
            ++by;
        }
    }
    port_font_magnification(1);
    printat(15,150,"https://github.com/darwinne/c64maze");
    port_font_magnification(2);
    write_time(message,9);
    printat(40,170,message);
    fflushMazeRegion();
    getch();
    port_clearHGRpage();
    draw_banner();
}

// byte 1: lo byte timestamp in 1/60's of second
// byte 2: hi byte timestamp in 1/60's of second
// byte 3: message
// byte 4: first byte of the message (if applicable)
// byte 5: second byte of the message (if applicable)
//

#ifndef NO_SOUND

unsigned char sound_irq(void)
{
    return port_sound_irq();
}
void start_sound(unsigned char *l1, unsigned char *l2, unsigned char *l3)
{
    port_start_sound(l1, l2, l3);
}

#endif

void start_game(void)
{
    start_time=get_current_time();
    choose_start_position();
    draw_banner();
}

/** Starting point of the program.
*/
void main(void)
{
    unsigned char oldx=0;
    unsigned char oldy=0;
    char oldo=0;
    char c;
    char iv=TRUE;
    unsigned char music=TRUE;
    char time_spent[]="     s";

#ifndef NO_SOUND
    start_sound(music_v1, music_v2, music_v3);
#endif

    graphics_init();
    restart:
    start_game();

    while(TRUE) {
        if(oldx!=positionx || oldy!=positiony || oldo!=orientation) {
            oldx=positionx;
            oldy=positiony;
            oldo=orientation;
            clearMazeRegion();
            drawLabyrinthView();
            fflushMazeRegion();
            if (positionx==startx && positiony==starty)
                printat(40,100,"step in!");
            if (positionx==exitx && positiony==exity) {
                printat(40,70,"way out!");
                write_time(time_spent,0);
                printat(50,100,time_spent);
                port_loadVICFont(1);
                printat(55, 140,  "press a key");
                printat(47, 150, "to play again");
                getch();
                oldx=0;
                goto restart;   // No program for the C64 would be complete
                                // without at least a GOTO statement somewhere.
            }
            //POKE(SCREEN_BORDER,4);
        }
        do {
            c=getch();
            iv=FALSE;
            switch(c) {
                case 'q':   //Exit
                	game_exit();
                	break;
                case 't':   // Forward
                    move_forward();
                    break;
                case 'v':   // Backwards
                    move_backwards();
                    break;
                case 'f':   // Turn left
                    if(orientation==3)
                        orientation=0;
                    else
                        ++orientation;
                    break;
                case 'g':   // Turn right
                    if(orientation==0)
                        orientation=3;
                    else
                        --orientation;
                    break;
                case 'p':   // Show maze and position
                    show_maze();
                    // force a redraw.
                    oldx=0;
                    break;
                case 'a':   // Start again
                    port_clearHGRpage();
                    start_game();
                    oldx=0;
                    break;
#ifndef NO_SOUND
                case 'm':   // Toggle music on/off
                    if(music==TRUE) {
                        port_music_off();
                        music=FALSE;
                    } else {
                        port_music_on();
                        music=TRUE;
                    }
                    break;
#endif
                default:
                    iv=TRUE;
            };
        } while(iv==TRUE);
        if(labyrinth[positionx+positiony*labyrinthSizeX]=='*') {
            //POKE(SCREEN_BORDER,1);
            positionx=oldx;
            positiony=oldy;
        }
    }
}

void game_exit(void)
{
	port_exit();
}
