/****************************************************************************
 * Copyright 2009 The Lemon Man and thanks to luccax, Wiipower, Aurelio and crediar
 * Copyright 2010 Dimok
 *
 * Original forwarder source by
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/

#include <gccore.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ogc/machine/processor.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "dolloader.h"
#include "filelist.h"
#include "devicemounter.h"

#define url "http://m.facebook.com/"

void __exception_setreload(int t);

static FILE * open_file(const char * dev, char * filepath)
{
    sprintf(filepath, "%s:/apps/wiiflow/boot.dol", dev);
    // sprintf(filepath, "%s:/apps/wiibrowser/boot.dol", dev);

    FILE * exeFile = fopen(filepath ,"rb");

    if (exeFile == NULL)
    {
        sprintf(filepath, "%s:/wiibrowser/boot.dol", dev);

        exeFile = fopen(filepath ,"rb");
    }

    return exeFile;
}

#define MAX_CMDLINE 4096
#define MAX_ARGV    1000

struct __argv args;
char cmdline[MAX_CMDLINE];
char *a_argv[MAX_ARGV];
char *meta_buf = NULL;

void arg_init()
{
    memset(&args, 0, sizeof(args));
    memset(cmdline, 0, sizeof(cmdline));
    memset(a_argv, 0, sizeof(a_argv));
    args.argvMagic = ARGV_MAGIC;
    args.length = 1; // double \0\0
    args.argc = 0;
    args.commandLine = cmdline;
    args.argv = a_argv;
    args.endARGV = a_argv;
}

char* strcopy(char *dest, const char *src, int size)
{
    strncpy(dest,src,size);
    dest[size-1] = 0;
    return dest;
}

int arg_addl(char *arg, int len)
{
    if (args.argc >= MAX_ARGV) return -1;
    if (args.length + len + 1 > MAX_CMDLINE) return -1;
    strcopy(cmdline + args.length - 1, arg, len+1);
    args.length += len + 1; // 0 term.
    cmdline[args.length - 1] = 0; // double \0\0
    args.argc++;
    args.endARGV = args.argv + args.argc;
    return 0;
}

int arg_add(char *arg)
{
    return arg_addl(arg, strlen(arg));
}

int load_file(char *name, void *buf, int size)
{
    int fd;
    int ret;
    fd = open(name, O_RDONLY);
    if (fd < 0)
    {
        return -1;
    }
    ret = read(fd, buf, size);
    close(fd);
    if (ret != size)
    {
        return -2;
    }
    return 0;
}

void load_meta( const char *exe_path)
{
    char meta_path[200];
    const char *p;
    struct stat st;

    p = strrchr(exe_path, '/');
    snprintf( meta_path, sizeof(meta_path), "%.*smeta.xml",
              p ? p-exe_path+1 : 0, exe_path );

    if (stat(meta_path, &st) != 0)
    {
        return;
    }
    if (st.st_size > 64*1024)
    {
        return;
    }
    // +1 so that the buf is 0 terminated
    meta_buf = calloc(st.st_size + 1, 1);
    if (!meta_buf)
    {
        return;
    }
    load_file(meta_path, meta_buf, st.st_size);
}

void parse_meta()
{
    char *p;
    char *e, *end;
    if (meta_buf == NULL) return;
    if (!strstr(meta_buf, "<app") || !strstr(meta_buf, "</app>"))
    {
        return;
    }
    p = strstr(meta_buf, "<arguments>");
    if (!p) return;
    end = strstr(meta_buf, "</arguments>");
    if (!end) return;
    do
    {
        p = strstr(p, "<arg>");
        if (!p) return;
        p += 5; //strlen("<arg>");
        e = strstr(p, "</arg>");
        if (!e) return;
        arg_addl(p, e-p);
        p = e + 6;
    }
    while (p < end);
    if (meta_buf)
    {
        free(meta_buf);
        meta_buf = NULL;
    }
}

int main(int argc, char **argv)
{
    u32 cookie;
    FILE *exeFile = NULL;
    void * exeBuffer = (void *)EXECUTABLE_MEM_ADDR;
    u32 exeSize = 0;
    u32 exeEntryPointAddress = 0;
    entrypoint exeEntryPoint;
    __exception_setreload(0);

    // initial video
    void *xfb = NULL;
    GXRModeObj *rmode = NULL;

    VIDEO_Init();
    rmode = VIDEO_GetPreferredMode(NULL);
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

    printf("\x1b[2;0H");
    char filepath[200];

    // try SD Card First
    SDCard_Init();
    exeFile = open_file(DeviceName[SD], filepath);
    // if app not found on SD Card try USB
    if (exeFile == NULL)
    {
        USBDevice_Init();
        int dev;
        for(dev = USB1; dev < MAXDEVICES; ++dev)
        {
            if(!exeFile)
                exeFile = open_file(DeviceName[dev], filepath);
        }
    }

    // if nothing found exiting
    if (exeFile == NULL)
    {
        fclose (exeFile);
        SDCard_deInit();
        USBDevice_deInit();
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
    }

    fseek (exeFile, 0, SEEK_END);
    exeSize = ftell(exeFile);
    rewind (exeFile);

    if(fread (exeBuffer, 1, exeSize, exeFile) != exeSize)
    {
        fclose (exeFile);
        SDCard_deInit();
        USBDevice_deInit();
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
    }
    fclose (exeFile);

    /* load entry point */
    /*
    struct __argv args;
    bzero(&args, sizeof(args));
    args.argvMagic = ARGV_MAGIC;
    args.length = strlen(filepath) + 2;
    args.commandLine = (char*)malloc(args.length);
    if (!args.commandLine) SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
    strcpy(args.commandLine, filepath);
    args.commandLine[args.length - 1] = '\0';
    args.argc = 1;
    args.argv = &args.commandLine;
    args.endARGV = args.argv + 1;
    */

    arg_init();
    arg_add(filepath); // argv[0] = filepath
    arg_add("sd:/apps/wiibrowser/shortcuts/Mail.www"); // argv[1] = url
    // arg_add(url); // argv[1] = url

    // load meta.xml
    // load_meta(filepath);
    // parse <arguments> in meta.xml
    // parse_meta();

    u8 * appboot_buff = (u8 *) malloc(app_booter_dol_size);
    if(!appboot_buff)
    {
        SDCard_deInit();
        USBDevice_deInit();
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
    }

    memcpy(appboot_buff, app_booter_dol, app_booter_dol_size);

    exeEntryPointAddress = load_dol_image(appboot_buff, &args);

    if(appboot_buff)
        free(appboot_buff);

    SDCard_deInit();
    USBDevice_deInit();

    if (exeEntryPointAddress == 0)
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);

    exeEntryPoint = (entrypoint) exeEntryPointAddress;
    /* cleaning up and load dol */
    SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
    _CPU_ISR_Disable (cookie);
    __exception_closeall ();
    exeEntryPoint ();
    _CPU_ISR_Restore (cookie);
    return 0;
}
