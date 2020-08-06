#ifdef TARGET_DOS

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <dos.h>
#include <pc.h>
#include <go32.h>
#include <dpmi.h>
#include <sys/farptr.h>
#include <sys/nearptr.h>

#include "macros.h"
#include "audio_api.h"

#define SRATE 32000
#define BUFSIZE 4096
#define SAMPLES (BUFSIZE / 2)
#define OUTMASK (SAMPLES-1)

static short *buffer = NULL;

static int dsp_port = 0;
static int irq = 0;
static int low_dma = 0;
static int high_dma = 0;
static int mixer_port = 0;
static int mpu401_port = 0;
static int dma = 0;
static int samplepos = 0;

static int dsp_version = 0;
static int dsp_version_minor = 0;

static int page_reg[] = { 0x87, 0x83, 0x81, 0x82, 0x8f, 0x8b, 0x89, 0x8a };
static int addr_reg[] = { 0, 2, 4, 6, 0xc0, 0xc4, 0xc8, 0xcc };
static int count_reg[] = { 1, 3, 5, 7, 0xc2, 0xc6, 0xca, 0xce };

static int mode_reg;
static int flipflop_reg;
static int disable_reg;
static int clear_reg;

static unsigned conventional_memory = -1U;

static void map_in_conventional_memory(void) {
    if (conventional_memory == -1U) {
        if (__djgpp_nearptr_enable())
            conventional_memory = __djgpp_conventional_base;
    }
}

static unsigned int ptr2real(void *ptr) {
    map_in_conventional_memory();
    return (int)ptr - conventional_memory;
}

static void *real2ptr(unsigned int real) {
    map_in_conventional_memory();
    return (void *)(real + conventional_memory);
}

static void *dos_getmemory(int size) {
    int rc;
    _go32_dpmi_seginfo info;

    info.size = (size + 15) / 16;
    rc = _go32_dpmi_allocate_dos_memory(&info);
    if (rc) return NULL;

    return real2ptr((int)info.rm_segment << 4);
}

static bool parse_blaster_env(void) {
    char *BLASTER;
    char *param;

    BLASTER = getenv("BLASTER");
    if (!BLASTER) return false;

    param = strchr(BLASTER, 'A');
    if (!param) param = strchr(BLASTER, 'a');
    if (!param) return false;
    sscanf(param+1, "%x", &dsp_port);

    param = strchr(BLASTER, 'I');
    if (!param) param = strchr(BLASTER, 'i');
    if (!param) return false;
    sscanf(param+1, "%d", &irq);

    param = strchr(BLASTER, 'D');
    if (!param) param = strchr(BLASTER, 'd');
    if (!param) return false;
    sscanf(param+1, "%d", &low_dma);

    param = strchr(BLASTER, 'H');
    if (!param) param = strchr(BLASTER, 'h');
    if (param) sscanf(param+1, "%d", &high_dma);

    param = strchr(BLASTER, 'M');
    if (!param) param = strchr(BLASTER, 'm');
    if (param)
      sscanf(param+1, "%x", &mixer_port);
    else
      mixer_port = dsp_port;

    param = strchr(BLASTER, 'P');
    if (!param) param = strchr(BLASTER, 'p');
    if (param) sscanf(param+1, "%x", &mpu401_port);

    printf("dsp_port %x\n", dsp_port);

    return true;
}

static bool reset_dsp(void) {
    volatile int i;

    outportb(dsp_port + 6, 1);
    for (i=65536 ; i ; i--) ;
    outportb(dsp_port + 6, 0);
    for (i=65536 ; i ; i--) {
        if (!(inportb(dsp_port + 0xe) & 0x80)) continue;
        if (inportb(dsp_port + 0xa) == 0xaa) break;
    }

    return !i;
}

static bool read_dsp(void) {
    while (!(inportb(dsp_port+0xe)&0x80)) ;
    return inportb(dsp_port+0xa);
}

static void write_dsp(int val) {
    while ((inportb(dsp_port+0xc)&0x80)) ;
    outportb(dsp_port+0xc, val);
}

static bool read_mixer(int addr) {
    outportb(mixer_port+4, addr);
    return inportb(mixer_port+5);
}

static void write_mixer(int addr, int val) {
    outportb(mixer_port+4, addr);
    outportb(mixer_port+5, val);
}

static void start_dma(void) {
    int mode;
    int realaddr;

    realaddr = ptr2real(buffer);

    // use a high dma channel if specified
    if (high_dma && dsp_version >= 4)
        dma = high_dma;
    else
        dma = low_dma;

    if (dma > 3) {
        mode_reg = 0xd6;
        flipflop_reg = 0xd8;
        disable_reg = 0xd4;
        clear_reg = 0xdc;
    } else {
        mode_reg = 0xb;
        flipflop_reg = 0xc;
        disable_reg = 0xa;
        clear_reg = 0xe;
    }

    outportb(disable_reg, dma|4); // disable channel
    // set mode- see "undocumented pc", p.876
    mode =   (1<<6)   // single-cycle
           + (0<<5)   // address increment
           + (1<<4)   // auto-init dma
           + (2<<2)   // read
           + (dma&3); // channel #
    outportb(mode_reg, mode);

    // set address, set page
    outportb(page_reg[dma], realaddr >> 16);

    if (dma > 3) {
        // address is in words
        outportb(flipflop_reg, 0); // prepare to send 16-bit value
        outportb(addr_reg[dma], (realaddr>>1) & 0xff);
        outportb(addr_reg[dma], (realaddr>>9) & 0xff);

        outportb(flipflop_reg, 0); // prepare to send 16-bit value
        outportb(count_reg[dma], ((BUFSIZE>>1)-1) & 0xff);
        outportb(count_reg[dma], ((BUFSIZE>>1)-1) >> 8);
    } else {
        // address is in bytes
        outportb(flipflop_reg, 0); // prepare to send 16-bit value
        outportb(addr_reg[dma], realaddr & 0xff);
        outportb(addr_reg[dma], (realaddr>>8) & 0xff);

        outportb(flipflop_reg, 0); // prepare to send 16-bit value
        outportb(count_reg[dma], (BUFSIZE-1) & 0xff);
        outportb(count_reg[dma], (BUFSIZE-1) >> 8);
    }

    outportb(clear_reg, 0); // clear write mask
    outportb(disable_reg, dma&~4);
}

static void start_sb(void) {
    write_dsp(0xd1); // turn on speaker
    write_dsp(0x41);

    write_dsp(SRATE>>8);
    write_dsp(SRATE&0xff);

    write_dsp(0xb6); // 16-bit output
    write_dsp(0x30); // stereo
    write_dsp((SAMPLES-1) & 0xff); // # of samples - 1
    write_dsp((SAMPLES-1) >> 8);
}

static bool audio_sb_init(void) {
    if (!parse_blaster_env()) {
        printf("could not parse BLASTER\n");
        abort();
        return false;
    }

    if (reset_dsp()) {
        printf("could not reset DSP\n");
        abort();
        return false;
    }

    write_dsp(0xE1);
    dsp_version = (read_dsp(), 4); // !HACK: dosbox's sb16 emu reports 1 here, so we force 4
    dsp_version_minor = read_dsp();

    if (dsp_version < 4) {
        printf("SB16 required, got version %d\n", dsp_version);
        abort();
        return false;
    }

    buffer = dos_getmemory(BUFSIZE * 2);
    if (!buffer) {
        printf("could not get memory\n");
        abort();
        return false;
    }

    int realaddr = ptr2real(buffer);
    realaddr = (realaddr + BUFSIZE) & ~(BUFSIZE-1);
    buffer = (short *)real2ptr(realaddr);

    memset(buffer, 0, BUFSIZE);

    start_dma();
    start_sb();

    return true;
}

static int audio_sb_buffered(void) {
    int count = 0;

    inportb(dsp_port + 0xf); // 16 bit audio

    if (dma < 4) {
        outportb(0xc, 0);
        count = inportb(dma*2+1);
        count += inportb(dma*2+1) << 8;
        count /= 2;
        count = SAMPLES - (count+1);
    } else {
        outportb(0xd8, 0);
        count = inportb(0xc0+(dma-4)*4+2);
        count += inportb(0xc0+(dma-4)*4+2) << 8;
        count = SAMPLES - (count+1);
    }

    samplepos = count & (SAMPLES - 1);
    return samplepos;
}

static int audio_sb_get_desired_buffered(void) {
    return 1100;
}

static void audio_sb_play(const uint8_t *buf, size_t len) {
    const short *p = (short *)buf;
    int out_idx = samplepos;
    len >>= 1;
    while (len--) {
        buffer[out_idx] = *(p++);
        out_idx = (out_idx + 1) & OUTMASK;
    }
}

struct AudioAPI audio_sb = {
    audio_sb_init,
    audio_sb_buffered,
    audio_sb_get_desired_buffered,
    audio_sb_play
};

#endif
