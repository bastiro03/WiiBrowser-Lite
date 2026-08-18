#ifndef MPLAYERWII_H
#define MPLAYERWII_H

#ifdef __cplusplus
extern "C" {
#endif

// MPlayer command IDs (matching external/mplayer/input/input.h enum values)
enum {
	MP_CMD_SEEK = 0,
	MP_CMD_AUDIO_DELAY,
	MP_CMD_QUIT,
	MP_CMD_PAUSE,
	MP_CMD_GRAB_FRAMES,
	MP_CMD_PLAY_TREE_STEP,
	MP_CMD_PLAY_TREE_UP_STEP,
	MP_CMD_PLAY_ALT_SRC_STEP,
	MP_CMD_SUB_DELAY,
	MP_CMD_OSD,
	MP_CMD_VOLUME,
	MP_CMD_MIXER_USEMASTER,
	MP_CMD_CONTRAST,
	MP_CMD_BRIGHTNESS,
	MP_CMD_HUE,
	MP_CMD_SATURATION,
	MP_CMD_FRAMEDROPPING,
	MP_CMD_TV_STEP_CHANNEL,
	MP_CMD_TV_STEP_NORM,
	MP_CMD_TV_STEP_CHANNEL_LIST,
	MP_CMD_CAPTURING,
	MP_CMD_VO_FULLSCREEN,
	MP_CMD_SUB_POS,
	MP_CMD_DVDNAV,
	MP_CMD_SCREENSHOT,
	MP_CMD_PANSCAN,
	MP_CMD_MUTE,
	MP_CMD_LOADFILE,
	MP_CMD_LOADLIST,
	MP_CMD_VF_CHANGE_RECTANGLE,
	MP_CMD_GAMMA,
	MP_CMD_SUB_VISIBILITY,
	MP_CMD_VOBSUB_LANG,
	MP_CMD_MENU,
	MP_CMD_SET_MENU,
	MP_CMD_GET_TIME_LENGTH,
	MP_CMD_GET_PERCENT_POS,
	MP_CMD_SUB_STEP,
	MP_CMD_TV_SET_CHANNEL,
	MP_CMD_EDL_LOADFILE,
	MP_CMD_EDL_MARK,
	MP_CMD_SUB_ALIGNMENT,
	MP_CMD_TV_LAST_CHANNEL,
	MP_CMD_OSD_SHOW_TEXT,
	MP_CMD_TV_SET_FREQ,
	MP_CMD_TV_SET_NORM,
	MP_CMD_TV_SET_BRIGHTNESS,
	MP_CMD_TV_SET_CONTRAST,
	MP_CMD_TV_SET_HUE,
	MP_CMD_TV_SET_SATURATION,
	MP_CMD_GET_VO_FULLSCREEN,
	MP_CMD_GET_SUB_VISIBILITY,
	MP_CMD_SUB_FORCED_ONLY,
	MP_CMD_VO_ONTOP,
	MP_CMD_SUB_SELECT,
	MP_CMD_VO_ROOTWIN,
	MP_CMD_SWITCH_VSYNC,
	MP_CMD_SWITCH_RATIO,
	MP_CMD_FRAME_STEP,
	MP_CMD_SPEED_INCR,
	MP_CMD_SPEED_MULT,
	MP_CMD_SPEED_SET,
	MP_CMD_RUN,
	MP_CMD_SUB_LOG,
	MP_CMD_SWITCH_AUDIO,
	MP_CMD_GET_TIME_POS,
	MP_CMD_SUB_LOAD,
	MP_CMD_SUB_REMOVE,
	MP_CMD_KEYDOWN_EVENTS,
	MP_CMD_VO_BORDER,
	MP_CMD_SET_PROPERTY,
	MP_CMD_GET_PROPERTY,
	MP_CMD_OSD_SHOW_PROPERTY_TEXT,
	MP_CMD_OSD_SHOW_PROGRESSION,
	MP_CMD_SEEK_CHAPTER,
	MP_CMD_FILE_FILTER,
	MP_CMD_GET_FILENAME,
	MP_CMD_GET_VIDEO_CODEC,
	MP_CMD_GET_VIDEO_BITRATE,
	MP_CMD_GET_VIDEO_RESOLUTION,
	MP_CMD_GET_AUDIO_CODEC,
	MP_CMD_GET_AUDIO_BITRATE,
	MP_CMD_GET_AUDIO_SAMPLES,
	MP_CMD_GET_META_TITLE,
	MP_CMD_GET_META_ARTIST,
	MP_CMD_GET_META_ALBUM,
	MP_CMD_GET_META_YEAR,
	MP_CMD_GET_META_COMMENT,
	MP_CMD_GET_META_TRACK,
	MP_CMD_GET_META_GENRE,
	MP_CMD_RADIO_STEP_CHANNEL,
	MP_CMD_RADIO_SET_CHANNEL,
	MP_CMD_RADIO_SET_FREQ,
	MP_CMD_SET_MOUSE_POS,
	MP_CMD_STEP_PROPERTY,
	MP_CMD_RADIO_STEP_FREQ,
	MP_CMD_TV_STEP_FREQ,
	MP_CMD_LOOP,
	MP_CMD_BALANCE,
	MP_CMD_SUB_SCALE,
	MP_CMD_TV_TELETEXT_ADD_DEC,
	MP_CMD_TV_TELETEXT_GO_LINK,
	MP_CMD_TV_START_SCAN,
	MP_CMD_SUB_SOURCE,
	MP_CMD_SUB_FILE,
	MP_CMD_SUB_VOB,
	MP_CMD_SUB_DEMUX,
	MP_CMD_SWITCH_ANGLE,
	MP_CMD_ASS_USE_MARGINS,
	MP_CMD_SWITCH_TITLE,
	MP_CMD_STOP,
	MP_CMD_OVERLAY_ADD,
	MP_CMD_OVERLAY_REMOVE,

	// DVDNAV commands
	MP_CMD_DVDNAV_UP = 1000,
	MP_CMD_DVDNAV_DOWN,
	MP_CMD_DVDNAV_LEFT,
	MP_CMD_DVDNAV_RIGHT,
	MP_CMD_DVDNAV_MENU,
	MP_CMD_DVDNAV_SELECT,
	MP_CMD_DVDNAV_PREVMENU,
	MP_CMD_DVDNAV_MOUSECLICK
};

// MPlayer main entry point (kept for compatibility with the real mplayer build)
int mplayer_main();

void wiiLoadFile(char *filename, char *partitionlabel);
void wiiGotoGui();
void wiiPause();
bool wiiIsPaused();
bool wiiIsPlaying();
void wiiMute();
void wiiSetSeekBackward(int sec);
void wiiSetSeekForward(int sec);
void wiiSeekPos(int sec);
void wiiFastForward();
void wiiRewind();
double wiiGetTimeLength();
double wiiGetTimePos();
void wiiGetTimeDisplay(char *buf);
void wiiSetDVDDevice(char *dev);
bool wiiAudioOnly();
char * wiiGetMetaTitle();
char * wiiGetMetaArtist();
char * wiiGetMetaAlbum();
char * wiiGetMetaYear();
void wiiDVDNav(int cmd);
void wiiUpdatePointer(int x, int y);
bool wiiPlayingDVD();
bool wiiInDVDMenu();
void wiiSetCacheFill(int fill);
void wiiSetOnlineCacheFill(int fill);
void wiiSetAutoResume(int enable);
void wiiSetVolume(int vol);
void wiiSetProperty(int cmd, float val);
void wiiSetCodepage(char *cp);
void wiiSetAudioLanguage(char *lang);
void wiiSetSubtitleLanguage(char *lang);
void wiiSetSubtitleColor(char *color);
void wiiSetSubtitleSize(float size);
bool wiiFindRestorePoint(char *filename, char *partitionlabel);
void wiiLoadRestorePoints(char *buffer, int size);
char * wiiSaveRestorePoints(char *path);

#ifdef __cplusplus
}
#endif

#endif // MPLAYERWII_H