/*
 * This file is part of MPlayer CE.
 *
 * MPlayer CE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer CE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer CE; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <malloc.h>
#include <sys/iosupport.h>
#include <errno.h>

#include <ogcsys.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/lwp.h>

#include "config.h"
#include "parser-cfg.h"
#include "mpbswap.h"
#include "mp_osd.h"
#include "m_option.h"

#include "log_console.h"
#include "gx_supp.h"
#include "plat_gekko.h"


#ifdef HW_RVL
#include "mload.h"
#include "ehcmodule_elf.h"

static bool load_ehci_module()
{
	data_elf my_data_elf;
	
	if (mload_elf(ehcmodule_elf, &my_data_elf) < 0)
		return false;
	
	if (mload_run_thread(my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, my_data_elf.prio) < 0) {
		usleep(1000);
		if (mload_run_thread(my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, 0x47) < 0)
			return false;
	}
	
	return true;
}

static bool FindIOS(u32 ios)
{
	u32 num = 0;
	
	if (ES_GetNumTitles(&num) < 0)
		return false;
	
	if (num < 1) 
		return false;
	
	u64 *titles = memalign(32, num * sizeof(u64) + 32);
	
	if (titles == NULL)
		return false;
	
	if (ES_GetTitles(titles, num) < 0) {
		free(titles);
		return false;
	}
	
	for (int i = 0; i < num; i++) {
		if ((titles[i] & 0xFFFFFFFF) == ios) {
			free(titles); 
			return true;
		}
	}
	
	free(titles); 
	return false;
}
#endif


bool reset_pressed = false;
bool power_pressed = false;

static void power_cb(void) {
	power_pressed = true;
}

static void reset_cb(void) {
	reset_pressed = true;
}


#define MOUNT_STACKSIZE (8 * 1024)
static u8 mountstack[MOUNT_STACKSIZE] ATTRIBUTE_ALIGN(32);
static lwp_t mountthread = LWP_THREAD_NULL;
static bool exit_automount_thread = false;

#include <sdcard/wiisd_io.h>
#include <sdcard/gcsd.h>
#include <ogc/usbstorage.h>
#include <ogc/dvd.h>
#include <di/di.h>

#include <fat.h>
#include <ntfs.h>
#include <ext2.h>
#include "iso.h"
#include "fst.h"
#include "gcfst.h"

#ifdef HW_RVL
static const DISC_INTERFACE *sd = &__io_wiisd;
static const DISC_INTERFACE *usb = &__io_usbstorage;
static const DISC_INTERFACE *dvd = &__io_wiidvd;
#else
static const DISC_INTERFACE *dvd = &__io_gcdvd;
#endif
static const DISC_INTERFACE *carda = &__io_gcsda;
static const DISC_INTERFACE *cardb = &__io_gcsdb;

typedef struct _PARTITION_RECORD {
	u8 status;
	u8 chs_start[3];
	u8 type;
	u8 chs_end[3];
	u32 lba_start;
	u32 block_count;
} ATTRIBUTE_PACKED PARTITION_RECORD;

typedef struct _MASTER_BOOT_RECORD {
	u8 code_area[446];
	PARTITION_RECORD partitions[4];
	u16 signature;
} ATTRIBUTE_PACKED MASTER_BOOT_RECORD;

#define MBR_SIGNATURE	0x55AA

#define PARTITION_TYPE_FREE				0x00
#define PARTITION_TYPE_FAT12			0x01
#define PARTITION_TYPE_FAT16_32MB		0x04
#define PARTITION_TYPE_FAT16			0x06
#define PARTITION_TYPE_NTFS				0x07
#define PARTITION_TYPE_FAT32			0x0b
#define PARTITION_TYPE_FAT32_LBA		0x0c
#define PARTITION_TYPE_FAT16_LBA		0x0e
#define PARTITION_TYPE_LINUX			0x83

enum {
	DEVICE_CARDA = 0,
	DEVICE_CARDB,
	DEVICE_DVD,
#ifdef HW_RVL
	DEVICE_SD,
	DEVICE_USB,
#endif
	DEVICE_MAX,
};

static bool isInserted[DEVICE_MAX];

static mutex_t dvd_mutex = LWP_MUTEX_NULL;
static u64 dvd_lasttick = 0;

static void mountproc()
{
#ifdef HW_RVL
	if (isInserted[DEVICE_SD]) {
		if (!sd->isInserted()) {
			fatUnmount("sd:");
			sd->shutdown();
			isInserted[DEVICE_SD] = false;
		}
	} else if (sd->startup() && sd->isInserted()) {
		fatMount("sd", sd, 0, 2, 128);
		isInserted[DEVICE_SD] = true;
	}
	
	if (isInserted[DEVICE_USB]) {
		if (!usb->isInserted()) {
			fatUnmount("usb:");
			ntfsUnmount("ntfs", true);
			ext2Unmount("ext2");
			usb->shutdown();
			isInserted[DEVICE_USB] = false;
		}
	} else if (usb->startup() && usb->isInserted()) {
		MASTER_BOOT_RECORD mbr;
		
		if (usb->readSectors(0, 1, &mbr) && (mbr.signature == MBR_SIGNATURE)) {
			for (int i = 0; i < 4; i++) {
				PARTITION_RECORD *partition = &mbr.partitions[i];
				sec_t sector = le2me_32(partition->lba_start);
				
				switch (partition->type) {
					case PARTITION_TYPE_FREE:
						continue;
					case PARTITION_TYPE_FAT12:
					case PARTITION_TYPE_FAT16_32MB:
					case PARTITION_TYPE_FAT16:
					case PARTITION_TYPE_FAT32:
					case PARTITION_TYPE_FAT32_LBA:
					case PARTITION_TYPE_FAT16_LBA:
					{
						fatMount("usb", usb, sector, 2, 128);
					}
					case PARTITION_TYPE_NTFS:
						ntfsMount("ntfs", usb, sector, 2, 128, NTFS_DEFAULT | NTFS_RECOVER);
					case PARTITION_TYPE_LINUX:
						ext2Mount("ext2", usb, sector, 2, 128, EXT2_FLAG_DEFAULT);
				}
			}
		} else fatMount("usb", usb, 0, 2, 128);
		
		isInserted[DEVICE_USB] = true;
	}
#endif
	
	if (isInserted[DEVICE_CARDA]) {
		if (!carda->isInserted()) {
			fatUnmount("carda:");
			carda->shutdown();
			isInserted[DEVICE_CARDA] = false;
		}
	} else if (carda->startup() && carda->isInserted()) {
		fatMount("carda", carda, 0, 4, 64);
		isInserted[DEVICE_CARDA] = true;
	}
	
	if (isInserted[DEVICE_CARDB]) {
		if (!cardb->isInserted()) {
			fatUnmount("cardb:");
			cardb->shutdown();
			isInserted[DEVICE_CARDB] = false;
		}
	} else if (cardb->startup() && cardb->isInserted()) {
		fatMount("cardb", cardb, 0, 4, 64);
		isInserted[DEVICE_CARDB] = true;
	}
}

#ifdef HW_RVL
static void *mountloop(void *arg)
{
	usleep(200 * TB_MSPERSEC);
	
	while (!exit_automount_thread) {
		if (!LWP_MutexTryLock(dvd_mutex)) {
			LWP_MutexLock(dvd_mutex);
			
			if (isInserted[DEVICE_DVD]) {
				if (!dvd->isInserted()) {
					FST_Unmount();
					GCFST_Unmount();
					ISO9660_Unmount();
					isInserted[DEVICE_DVD] = false;
					dvd_lasttick = 0;
				} else if (dvd_lasttick > 0) {
					if (diff_sec(dvd_lasttick, gettime()) > 60) {
						DI_StopMotor();
						dvd_lasttick = 0;
					}
				}
			} else if (dvd->isInserted())
				isInserted[DEVICE_DVD] = true;
			
			LWP_MutexUnlock(dvd_mutex);
		}
		
		mountproc();
		if (exit_automount_thread) break;
		usleep(2 * TB_USPERSEC);
	}
	
	return NULL;
}

void DVDGekkoTick(bool silent)
{
	if (!dvd_lasttick) {
		LWP_MutexLock(dvd_mutex);
		
		if (dvd->isInserted()) {
			if (!silent) {
				set_osd_msg(OSD_MSG_TEXT, 1, 5000, "Mounting DVD, please wait");
				force_osd();
			}
			
			if (dvd->startup()) {
				dvd_lasttick = gettime();
				LWP_MutexUnlock(dvd_mutex);
				return;
			}
		}
		
		if (!silent) {
			set_osd_msg(OSD_MSG_TEXT, 1, 2000, "Error mounting DVD");
			force_osd();
		}
		
		LWP_MutexUnlock(dvd_mutex);
	} else dvd_lasttick = gettime();
}

bool DVDGekkoMount()
{
	LWP_MutexLock(dvd_mutex);
	
	if (dvd->isInserted()) {
		if (!dvd_lasttick) {
			set_osd_msg(OSD_MSG_TEXT, 1, 5000, "Mounting DVD, please wait");
			force_osd();
			
			if (!dvd->startup()) {
				LWP_MutexUnlock(dvd_mutex);
				return false;
			} else dvd_lasttick = gettime();
		}
		
		if (ISO9660_Mount() || FST_Mount() || GCFST_Mount()) {
			LWP_MutexUnlock(dvd_mutex);
			return true;
		}
	}
	
	LWP_MutexUnlock(dvd_mutex);
	return false;
}
#endif


#define NET_STACKSIZE (8 * 1024)
static u8 netstack[NET_STACKSIZE] ATTRIBUTE_ALIGN(32);
static lwp_t netthread = LWP_THREAD_NULL;

#include <network.h>
#include <smb.h>
#include "ftp_devoptab.h"

int network_initied = 0;

typedef struct {
	char *ip;
	char *share;
	char *user;
	char *pass;
	bool init;
} t_smb_conf;

typedef struct {
	char *ip;
	char *share;
	char *user;
	char *pass;
	int passive;
	bool init;
} t_ftp_conf;

static t_smb_conf smb_conf[5];
static t_ftp_conf ftp_conf[5];

static s32 initialise_network() 
{
	s32 result = 0;
#ifdef HW_RVL
	while ((result = net_init()) == -EAGAIN) 
	{
		usleep(1000);
	}
#endif
	return result;
}

static int wait_for_network_initialisation() 
{
	int i;
	if(network_initied) return 1;
	
	for(i=0;i<5;i++)
	{
		if (initialise_network() >= 0) {
			char myIP[16];
			if (if_config(myIP, NULL, NULL, true) < 0)
			{
				sleep(5);
				continue;
			}
			else
			{
				network_initied=1;
				usleep(1000);
				return 1;
			}
		}
		sleep(5);
	}
	
	return 0;
}

static void read_net_config()
{
	int i;
	char file[100];
	m_config_t *conf;
	
	static const m_option_t smb_opts[] = {
		{"ip1", &smb_conf[0].ip, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"share1", &smb_conf[0].share, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"user1", &smb_conf[0].user, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"pass1", &smb_conf[0].pass, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"ip2", &smb_conf[1].ip, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"share2", &smb_conf[1].share, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"user2", &smb_conf[1].user, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"pass2", &smb_conf[1].pass, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"ip3", &smb_conf[2].ip, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"share3", &smb_conf[2].share, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"user3", &smb_conf[2].user, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"pass3", &smb_conf[2].pass, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"ip4", &smb_conf[3].ip, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"share4", &smb_conf[3].share, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"user4", &smb_conf[3].user, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"pass4", &smb_conf[3].pass, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"ip5", &smb_conf[4].ip, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"share5", &smb_conf[4].share, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"user5", &smb_conf[4].user, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"pass5", &smb_conf[4].pass, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{NULL, NULL, 0, 0, 0, 0, NULL}
	};
	
	for (i = 0; i < 5; i++) {
		smb_conf[i].ip = NULL;
		smb_conf[i].init = false;
	}
	
	sprintf(file, "%s/smb.conf", MPLAYER_CONFDIR);
	conf = m_config_new();
	m_config_register_options(conf, smb_opts);
	m_config_parse_config_file(conf, file);
	m_config_free(conf);
	
#ifdef HW_RVL
	static const m_option_t ftp_opts[] = {
		{"ip1", &ftp_conf[0].ip, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"share1", &ftp_conf[0].share, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"user1", &ftp_conf[0].user, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"pass1", &ftp_conf[0].pass, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"passive1", &ftp_conf[0].passive, CONF_TYPE_FLAG, 0, 0, 1, NULL},
		{"ip2", &ftp_conf[1].ip, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"share2", &ftp_conf[1].share, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"user2", &ftp_conf[1].user, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"pass2", &ftp_conf[1].pass, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"passive2", &ftp_conf[1].passive, CONF_TYPE_FLAG, 0, 0, 1, NULL},
		{"ip3", &ftp_conf[2].ip, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"share3", &ftp_conf[2].share, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"user3", &ftp_conf[2].user, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"pass3", &ftp_conf[2].pass, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"passive3", &ftp_conf[2].passive, CONF_TYPE_FLAG, 0, 0, 1, NULL},
		{"ip4", &ftp_conf[3].ip, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"share4", &ftp_conf[3].share, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"user4", &ftp_conf[3].user, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"pass4", &ftp_conf[3].pass, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"passive4", &ftp_conf[3].passive, CONF_TYPE_FLAG, 0, 0, 1, NULL},
		{"ip5", &ftp_conf[4].ip, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"share5", &ftp_conf[4].share, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"user5", &ftp_conf[4].user, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"pass5", &ftp_conf[4].pass, CONF_TYPE_STRING, 0, 0, 0, NULL},
		{"passive5", &ftp_conf[4].passive, CONF_TYPE_FLAG, 0, 0, 1, NULL},
		{NULL, NULL, 0, 0, 0, 0, NULL}
	};
	
	for (i = 0; i < 5; i++) {
		ftp_conf[i].ip = NULL;
		ftp_conf[i].init = false;
	}
	
	sprintf(file, "%s/ftp.conf", MPLAYER_CONFDIR);
	conf = m_config_new();
	m_config_register_options(conf, ftp_opts);
	m_config_parse_config_file(conf, file);
	m_config_free(conf);
#endif
}

#ifdef HW_RVL
static void *networkthreadfunc(void *arg)
{
	wait_for_network_initialisation();
	LWP_SuspendThread(netthread);
	net_deinit();
	
	return NULL;
}
#endif

bool smbConnect(char *device)
{
	if (network_initied) {
		int num = (device[3] - '0') - 1;
		
		if (smb_conf[num].init) {
			if (!(smb_conf[num].init = smbCheckConnection(device)))
				smbClose(device);
		} else if (smb_conf[num].ip != NULL && smb_conf[num].share != NULL) {
			if (smb_conf[num].user == NULL)
				smb_conf[num].user = strdup("");
			
			if (smb_conf[num].pass == NULL)
				smb_conf[num].pass = strdup("");
			
			smb_conf[num].init = smbInitDevice(device, smb_conf[num].user, smb_conf[num].pass,
														smb_conf[num].share, smb_conf[num].ip);
		}
		
		return smb_conf[num].init;
	}
	
	return false;
}

#ifdef HW_RVL
bool ftpConnect(char *device)
{
	if (network_initied) {
		int num = (device[3] - '0') - 1;
		
		if (!ftp_conf[num].init && (ftp_conf[num].ip != NULL && ftp_conf[num].share != NULL)) {
			if (ftp_conf[num].user == NULL)
				ftp_conf[num].user = strdup("anonymous");
			
			if (ftp_conf[num].pass == NULL)
				ftp_conf[num].pass = strdup("anonymous");
			
			ftp_conf[num].init = ftpInitDevice(device, ftp_conf[num].user, ftp_conf[num].pass,
														ftp_conf[num].share, ftp_conf[num].ip,
														21, ftp_conf[num].passive > 0);
		}
		
		return ftp_conf[num].init;
	}
	
	return false;
}
#endif


extern int enable_restore_points;
static int video_mode = 0;
static int overscan = 1;

static void LoadParams()
{
	char file[100];
	m_config_t *conf;
	
	static const m_option_t opts[] = {
		{"restore_points", &enable_restore_points, CONF_TYPE_FLAG, 0, 0, 1, NULL},
		{"video_mode", &video_mode, CONF_TYPE_INT, CONF_RANGE, 0, 4, NULL},
		{"overscan", &overscan, CONF_TYPE_FLAG, 0, 0, 1, NULL},
		{NULL, NULL, 0, 0, 0, 0, NULL}
	};
	
	conf = m_config_new();
	m_config_register_options(conf, opts);
	
	sprintf(file, "%s/mplayer.conf", MPLAYER_CONFDIR);
	m_config_parse_config_file(conf, file); 
	m_config_free(conf);
}

static bool CheckPath(char *path)
{
	char *file = malloc(sizeof(char) * (strlen(path) + 15));
	strcpy(file, path);
	strcat(file, "/mplayer.conf");
	
	FILE *handle = fopen(file, "r");
	free(file);
	
	if (!handle)
		return false;
	
	fclose(handle);
	
	sprintf(MPLAYER_DATADIR, "%s", path);
	sprintf(MPLAYER_CONFDIR, "%s", path);
	sprintf(MPLAYER_LIBDIR, "%s", path);
	
	chdir(path);
	setenv("HOME", path, 1);
	return true;
}

static bool DetectValidPath()
{
#ifdef HW_RVL
	if (isInserted[DEVICE_SD] && !(FindDevice("sd:") < 0)) {
		if (CheckPath("sd:/apps/mplayer-ce")) return true;
		if (CheckPath("sd:/apps/mplayer_ce")) return true;
		if (CheckPath("sd:/mplayer")) return true;
	}
	
	if (isInserted[DEVICE_USB] && !(FindDevice("usb:") < 0)) {
		if (CheckPath("usb:/apps/mplayer-ce")) return true;
		if (CheckPath("usb:/apps/mplayer_ce")) return true;
		if (CheckPath("usb:/mplayer")) return true;
	}
#endif
	
	if (isInserted[DEVICE_CARDA] && !(FindDevice("carda:") < 0)) {
		if (CheckPath("carda:/apps/mplayer-ce")) return true;
		if (CheckPath("carda:/mplayer")) return true;
	}
	
	if (isInserted[DEVICE_CARDB] && !(FindDevice("cardb:") < 0)) {
		if (CheckPath("cardb:/apps/mplayer-ce")) return true;
		if (CheckPath("cardb:/mplayer")) return true;
	}
	
	return false;
}

static char *default_args[] = {
	"sd:/apps/mplayer-ce/mplayer.dol",
	"-bgvideo", NULL,
	"-idle", NULL,
	"-menu", "-menu-startup",
	"-quiet"
};

extern void __exception_setreload(int t);
extern u32 __di_check_ahbprot(void);

void plat_init(int *argc, char **argv[])
{
	__exception_setreload(8);
	
#if defined(HW_DOL)
	AR_Init(NULL, 0);
	AR_Reset();
	
	ARQ_Init();
	ARQ_Reset();
#elif defined(HW_RVL)
	L2Enhance();
	bool ehci = false;
	
	if (FindIOS(202)) {
		IOS_ReloadIOS(202);
		if (mload_init())
			ehci = load_ehci_module();
	} else if (!__di_check_ahbprot()) {
		s32 preferred = IOS_GetPreferredVersion();
		if (preferred == 58 || preferred == 61)
			IOS_ReloadIOS(preferred);
		else DI_LoadDVDX(true);
	}
	
	DI_Init();
	USB2Enable(ehci);
#endif
	
	mpviSetup(0, true);
	log_console_init(vmode, 0);
	mountproc();
	
	if (!DetectValidPath()) {
		printf("\nSD/USB access failed\n");
		printf("Please check that you have installed MPlayer CE in the right folder\n");
		printf("Valid folders:\n");
		printf(" sd:/apps/mplayer-ce\n sd:/mplayer\n usb:/apps/mplayer-ce\n usb:/mplayer\n");
		
		sleep(10);
		mpviClear();
		exit(0);
	}
	
	LoadParams();
	read_net_config();
	
#ifndef HW_DOL
	if (*argc < 2)
#endif
	{
		default_args[2] = malloc(sizeof(char) * strlen(MPLAYER_DATADIR) + 16);
		strcpy(default_args[2], MPLAYER_DATADIR);
		
		default_args[4] = malloc(sizeof(char) * strlen(MPLAYER_DATADIR) + 16);
		strcpy(default_args[4], MPLAYER_DATADIR);
		
#ifdef HW_RVL
		if (CONF_GetAspectRatio() == CONF_ASPECT_16_9)
		{	// 16:9
			strcat(default_args[2], "/loop-wide.avi");
			strcat(default_args[4], "/loop-wide.avi");
		} else
#endif
		{	// 4:3
			strcat(default_args[2], "/loop.avi");
			strcat(default_args[4], "/loop.avi");
		}
		
		*argc = sizeof(default_args) / sizeof(char *);
		*argv = default_args;
	}
	
#ifdef HW_RVL
	SYS_SetPowerCallback(power_cb);
	SYS_SetResetCallback(reset_cb);
	
	LWP_MutexInit(&dvd_mutex, false);
	LWP_CreateThread(&mountthread, mountloop, NULL, mountstack, MOUNT_STACKSIZE, 40);
	
	LWP_CreateThread(&netthread, networkthreadfunc, NULL, netstack, NET_STACKSIZE, 40);
#else
	wait_for_network_initialisation();
#endif
	
	log_console_enable_video(false);
	
	if ((video_mode > 0) || !overscan) {
		log_console_deinit();
		mpviClear();
		mpviSetup(video_mode, overscan);
		log_console_init(vmode, 0);
		log_console_enable_video(false);
	}
}

void plat_deinit(int rc) 
{
	log_console_deinit();
	mpviClear();
	
#ifdef HW_RVL
	exit_automount_thread = true;
	LWP_JoinThread(mountthread, NULL);
	
	if (power_pressed)
		SYS_ResetSystem(SYS_POWEROFF, 0, 0);
#endif
}
