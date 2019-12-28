extern void io_hlt(void);
extern void io_cli(void);
extern void io_out8(int port, int data);
extern int  io_load_eflags(void);
extern void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill_8(unsigned char *vram,
              int xsize,
              unsigned char c,
              int x0,
              int y0,
              int x1,
              int y1);

void init_screen(char *vram, int x, int y);
void put_font8(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void init_mouse_cursor8(char *mouse, char background_color);
void putblock8_8(char *vram,
                 int vxsize,
                 int pxsize,
                 int pysize,
                 int px0,
                 int py0,
                 char *buf,
                 int bxsize);

extern void sprintf(char *str, char *fmt, ...);

#define COL8_000000     0
#define COL8_FF0000     1
#define COL8_00FF00     2
#define COL8_FFFF00     3
#define COL8_0000FF     4
#define COL8_FF00FF     5
#define COL8_00FFFF     6
#define COL8_FFFFFF     7
#define COL8_C6C6C6     8
#define COL8_840000     9
#define COL8_008400     10
#define COL8_848400     11
#define COL8_000084     12
#define COL8_840084     13
#define COL8_008484     14
#define COL8_848484     15

struct BootInfo {
    char cyls, leds, vmode, reserve;  // 1 byte
    short scrnx, scrny;               // 2 byte
    char *vram;                       // 4 byte
};

struct SegmentDescriptor { // 8byte
    short limit_low, base_low; // 4byte
    char base_mid, access_right; // 2byte
    char limit_high, base_high; // 2byte
};

struct GateDescriptor { // 10byte
    short offset_low, selector; // 4byte
    char dw_count, access_right; // 2byte
    short offset_high; // 4byte
};

void set_segmdesc(struct SegmentDescriptor *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GateDescriptor *gd, int offset, int selector, int ar);
void init_gdtidt(void);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

void HariMain(void)
{
    struct BootInfo *binfo = (struct BootInfo *) 0x0ff0;
    extern char hankaku[4096];
    char s[40], mcursor[256];
    int mx, my;

    init_palette();
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
    putfonts8_asc(binfo->vram, binfo->scrnx,  8,  8, COL8_FFFFFF, "ABC 123");
    putfonts8_asc(binfo->vram, binfo->scrnx, 31, 31, COL8_000000, "Haribote OS");
    putfonts8_asc(binfo->vram, binfo->scrnx, 30, 30, COL8_FFFFFF, "Haribote OS");

    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;

    init_mouse_cursor8(mcursor, COL8_008484);
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

    sprintf(s, "scrnx = %d", binfo->scrnx);
    putfonts8_asc(binfo->vram, binfo->scrnx, 16, 64, COL8_FFFFFF, s);
    for (;;) {
        io_hlt();
    }

    return;
}

void init_palette(void)
{
    static unsigned char table_rgb[16 * 3] = {
        0x00, 0x00, 0x00,   // 000000 : 0 : black
        0xff, 0x00, 0x00,   // ff0000 : 1 : light red
        0x00, 0xff, 0x00,   // 00ff00 : 2 : light green
        0xff, 0xff, 0x00,   // ffff00 : 3 : yellow
        0x00, 0x00, 0xff,   // 0000ff : 4 : light blue
        0xff, 0x00, 0xff,   // ff00ff : 5 : light purple
        0x00, 0xff, 0xff,   // 00ffff : 6 : light blue
        0xff, 0xff, 0xff,   // ffffff : 7 : white
        0xc6, 0xc6, 0xc6,   // c6c6c6 : 8 : light gray
        0x84, 0x00, 0x00,   // 840000 : 9 : dark red
        0x00, 0x84, 0x00,   // 008400 : 10: dark green
        0x84, 0x84, 0x00,   // 848400 : 11: dark yellow
        0x00, 0x00, 0x84,   // 000084 : 12: dark blue
        0x84, 0x00, 0x84,   // 840084 : 13: dark purple
        0x00, 0x84, 0x84,   // 008484 : 14: dark light blue
        0x84, 0x84, 0x84,   // 848484 : 15: dark gray
    };

    set_palette(0, 15, table_rgb);

    return;
}

void set_palette(int start, int end, unsigned char *rgb)
{
    int i, eflags;
    eflags = io_load_eflags(); // saving enable interrupt flag
    io_cli();                  // set 0 to interrupt flag, disable interrupt
    io_out8(0x03c8, start);
    for (i = start; i <= end; i++) {
        io_out8(0x03c9, rgb[0] / 4);
        io_out8(0x03c9, rgb[1] / 4);
        io_out8(0x03c9, rgb[2] / 4);
        rgb += 3;
    }
    io_store_eflags(eflags); // undo interrupt flag

    return;
}

void boxfill_8(unsigned char *vram,
              int xsize,
              unsigned char c,
              int x0,
              int y0,
              int x1,
              int y1)
{
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            vram[y * xsize + x] = c;
        }
    }

    return;
}

void init_screen(char *vram, int x, int y)
{
    boxfill_8(vram, x, COL8_008484,  0,     0,      x -  1, y - 29);
    boxfill_8(vram, x, COL8_C6C6C6,  0,     y - 28, x -  1, y - 28);
    boxfill_8(vram, x, COL8_FFFFFF,  0,     y - 27, x -  1, y - 27);
    boxfill_8(vram, x, COL8_C6C6C6,  0,     y - 26, x -  1, y -  1);

    boxfill_8(vram, x, COL8_FFFFFF,  3,     y - 24, 59,     y - 24);
    boxfill_8(vram, x, COL8_FFFFFF,  2,     y - 24,  2,     y -  4);
    boxfill_8(vram, x, COL8_848484,  3,     y -  4, 59,     y -  4);
    boxfill_8(vram, x, COL8_848484, 59,     y - 23, 59,     y -  5);
    boxfill_8(vram, x, COL8_000000,  2,     y -  3, 59,     y -  3);
    boxfill_8(vram, x, COL8_000000, 60,     y - 24, 60,     y -  3);

    boxfill_8(vram, x, COL8_848484, x - 47, y - 24, x -  4, y - 24);
    boxfill_8(vram, x, COL8_848484, x - 47, y - 23, x - 47, y -  4);
    boxfill_8(vram, x, COL8_FFFFFF, x - 47, y -  3, x -  4, y -  3);
    boxfill_8(vram, x, COL8_FFFFFF, x -  3, y - 24, x -  3, y -  3);

    return;
}

void putfont_8(char *vram, int xsize, int x, int y, char c, char *font)
{
    char d;
    char *p;

    for (int i = 0; i < 16; i++) {
        p = vram + (y + i) * xsize + x;
        d = font[i];

        if ((d & 0x80) != 0) { p[0] = c; }
        if ((d & 0x40) != 0) { p[1] = c; }
        if ((d & 0x20) != 0) { p[2] = c; }
        if ((d & 0x10) != 0) { p[3] = c; }
        if ((d & 0x08) != 0) { p[4] = c; }
        if ((d & 0x04) != 0) { p[5] = c; }
        if ((d & 0x02) != 0) { p[6] = c; }
        if ((d & 0x01) != 0) { p[7] = c; }
    }

    return;
}

void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
    extern char hankaku[4096];
    for (; *s != 0x00; s++) {
        putfont_8(vram, xsize, x, y, c, hankaku + *s * 16);
        x += 8;
    }

    return;
}

void init_mouse_cursor8(char *mouse, char background_color)
{
    static char cursor[16][16] = {
        "**************..",   // 1
        "*ooooooooooo*...",   // 2
        "*oooooooooo*....",   // 3
        "*ooooooooo*.....",   // 4
        "*oooooooo*......",   // 5
        "*ooooooo*.......",   // 6
        "*ooooooo*.......",   // 7
        "*oooooooo*......",   // 8
        "*oooo**ooo*.....",   // 9
        "*ooo*..*ooo*....",   // 10
        "*oo*....*ooo*...",   // 11
        "*o*......*ooo*..",   // 12
        "**........*ooo*.",   // 13
        "*..........*ooo*",   // 14
        ".. .........*oo*",   // 15
        ".............***",   // 12
    };

    int x, y;

    for(y=0; y < 16; y++) {
        for(x=0; x < 16; x++) {
            if(cursor[y][x] == '*') {
                mouse[y * 16 + x] = COL8_FFFFFF;
            }
            if(cursor[y][x] == 'o') {
                mouse[y * 16 + x] = COL8_FFFFFF;
            }
            if(cursor[y][x] == '.') {
                mouse[y * 16 + x] = background_color;
            }
        }
    }

    return;
}

void putblock8_8(char *vram, // 0xa0000
                 int vxsize, // 320
                 int pxsize,
                 int pysize,
                 int px0,
                 int py0,
                 char *buf,
                 int bxsize)
{
    int x, y;

    for(y = 0; y < pysize; y++) {
        for(x = 0; x < pxsize; x++) {
            vram[(py0+y) * vxsize + (px0+x)] = buf[y * bxsize + x];
        }
    }

    return;
}

/*
  - initialize global & interrupt descriptor table
*/
void init_gdtidt(void)
{
    // GDT(Gloabal(Segment) Descriptor Table) : 0x00270000 ~ 0x0027ffff (65535 byte)
    struct SegmentDescriptor *gdt = (struct SegmentDescriptor *) 0x00270000; // 0x00270000 ~ 0x00270009 (8 byte)
    // GateDescriptor : 0x0026f800 ~ 0x0026ffff (28672 byte)
    struct GateDescriptor *idt = (struct GateDescriptor *) 0x0026f800;

    // initialize GDT
    for (int i = 0; i < 8192; i++) {
        set_segmdesc(gdt + i, 0, 0, 0);
    }
    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
    set_segmdesc(gdt + 2, 0x0007ffff, 0x409a);
    load_gdtr(0xffff, 0x00270000);

    for (int i = 0; i < 256; i++ ) {
        set_gatedesc(idt + i, 0, 0, 0);
    }
    load_idtr(0x7ff, 0x0026f800);

    return;
}

void set_segmdesc(struct SegmentDescriptor *sd,
                  unsigned int limit,
                  int base,
                  int ar
    )
{
    if (limit > 0xffff) {
        ar |= 0x8000; // G_bit = 1
        limit /= 0x1000;
    }

    sd->limit_low = limit & 0xffff;
    sd->base_low = base & 0xffff;
    sd->base_mid = (base >> 16) & 0xff;
    sd->access_right = ar & 0xff;
    sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->base_high = ((base >> 24)) & 0xff;

    return;
}

void set_gatedesc(struct GateDescriptor *gd,
                  int offset,
                  int selector,
                  int ar
    )
{
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->dw_count = ((ar >> 8)) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;

    return;
}
