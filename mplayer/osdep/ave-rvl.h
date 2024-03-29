

// http://www.wiibrew.org/wiki/Hardware/AV_Encoder


#define AVE_VI_COMB 0x03
#define AVE_VI_GAMMA 0x10
#define AVE_VI_TEST 0x67

#define AVE_AI_MUTE 0x6D
#define AVE_AI_VOLUME 0x71


static const u8 VI_GM_0_0[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const u8 VI_GM_0_1[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x03, 0x97, 0x3B, 0x49,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x80, 0x1B, 0x80, 0xEB, 0x00
};

static const u8 VI_GM_0_2[] = {
	0x00, 0x00, 0x00, 0x28, 0x00, 0x5A, 0x02, 0xDB, 0x0D, 0x8D, 0x30, 0x49,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x10, 0x00, 0x10, 0x40, 0x11, 0x00, 0x18, 0x80, 0x42, 0x00, 0xEB, 0x00
};

static const u8 VI_GM_0_3[] = {
	0x00, 0x00, 0x00, 0x7A, 0x02, 0x3C, 0x07, 0x6D, 0x12, 0x9C, 0x27, 0x24,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x10, 0x00, 0x10, 0xC0, 0x15, 0x80, 0x29, 0x00, 0x62, 0x00, 0xEB, 0x00
};

static const u8 VI_GM_0_4[] = {
	0x00, 0x4E, 0x01, 0x99, 0x05, 0x2D, 0x0B, 0x24, 0x14, 0x29, 0x20, 0xA4,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x00, 0x10, 0x10, 0x40, 0x12, 0xC0, 0x1D, 0xC0, 0x3B, 0x00, 0x78, 0xC0, 0xEB, 0x00
};

static const u8 VI_GM_0_5[] = {
	0x00, 0xEC, 0x03, 0xD7, 0x08, 0x00, 0x0D, 0x9E, 0x14, 0x3E, 0x1B, 0xDB,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x10, 0xC0, 0x16, 0xC0, 0x27, 0xC0, 0x4B, 0x80, 0x89, 0x80, 0xEB, 0x00
};

static const u8 VI_GM_0_6[] = {
	0x02, 0x76, 0x06, 0x66, 0x0A, 0x96, 0x0E, 0xF3, 0x13, 0xAC, 0x18, 0x49,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB, 
	0x10, 0x00, 0x12, 0x00, 0x1C, 0x00, 0x32, 0x80, 0x59, 0xC0, 0x96, 0x00, 0xEB, 0x00
};

static const u8 VI_GM_0_7[] = {
	0x04, 0xEC, 0x08, 0xF5, 0x0C, 0x96, 0x0F, 0xCF, 0x12, 0xC6, 0x15, 0x80,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB, 
	0x10, 0x00, 0x14, 0x00, 0x22, 0x00, 0x3C, 0xC0, 0x66, 0x40, 0x9F, 0xC0, 0xEB, 0x00
};

static const u8 VI_GM_0_8[] = {
	0x08, 0x00, 0x0B, 0xAE, 0x0E, 0x00, 0x10, 0x30, 0x11, 0xCB, 0x13, 0x49,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB, 
	0x10, 0x00, 0x16, 0x80, 0x28, 0xC0, 0x46, 0x80, 0x71, 0x00, 0xA7, 0x80, 0xEB, 0x00
};

static const u8 VI_GM_0_9[] = {
	0x0B, 0xB1, 0x0E, 0x14, 0x0F, 0x2D, 0x10, 0x18, 0x10, 0xE5, 0x11, 0x80,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x19, 0x80, 0x2F, 0x80, 0x4F, 0xC0, 0x7A, 0x00, 0xAD, 0xC0, 0xEB, 0x00
};

static const u8 VI_GM_1_0[] = {
	0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00,
	0x10, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xEB,
	0x10, 0x00, 0x20, 0x00, 0x40, 0x00, 0x60, 0x00, 0x80, 0x00, 0xA0, 0x00, 0xEB, 0x00
};

static const u8 VI_GM_1_0_DIM[] = {
	0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00,
	0x10, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xEB,
	0x08, 0x00, 0x10, 0x00, 0x20, 0x00, 0x30, 0x00, 0x40, 0x00, 0x50, 0x00, 0x75, 0x00
};

static const u8 VI_GM_1_1[] = {
	0x14, 0xEC, 0x11, 0xC2, 0x10, 0x78, 0x0F, 0xB6, 0x0F, 0x2F, 0x0E, 0xB6,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x21, 0x00, 0x3C, 0xC0, 0x5F, 0xC0, 0x89, 0x00, 0xB7, 0x80, 0xEB, 0x00
};

static const u8 VI_GM_1_2[] = {
	0x19, 0xD8, 0x13, 0x33, 0x10, 0xD2, 0x0F, 0x6D, 0x0E, 0x5E, 0x0D, 0xA4,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x25, 0x00, 0x43, 0x00, 0x66, 0xC0, 0x8F, 0x40, 0xBB, 0x40, 0xEB, 0x00
};
 
static const u8 VI_GM_1_3[] = {
	0x1E, 0xC4, 0x14, 0x7A, 0x11, 0x0F, 0xF, 0x0C, 0x0D, 0xA1, 0x0C, 0xB6,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x29, 0x00, 0x49, 0x00, 0x6D, 0x40, 0x94, 0xC0, 0xBE, 0x80, 0xEB, 0x00
};

static const u8 VI_GM_1_4[] = {
	0x24, 0x00, 0x15, 0x70, 0x11, 0x0F, 0x0E, 0xAA, 0x0D, 0x0F, 0x0B, 0xDB,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x2D, 0x40, 0x4E, 0xC0, 0x73, 0x00, 0x99, 0x80, 0xC1, 0x80, 0xEB, 0x00
};

static const u8 VI_GM_1_5[] = {
	0x29, 0x3B, 0x16, 0x3D, 0x11, 0x0F, 0x0E, 0x30, 0x0C, 0x7D, 0x0B, 0x24,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x31, 0x80, 0x54, 0x40, 0x78, 0x80, 0x9D, 0xC0, 0xC4, 0x00, 0xEB, 0x00
};

static const u8 VI_GM_1_6[] = {
	0x2E, 0x27, 0x17, 0x0A, 0x10, 0xD2, 0x0D, 0xE7, 0x0B, 0xEB, 0x0A, 0x80,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x35, 0x80, 0x59, 0x80, 0x7D, 0x40, 0xA1, 0xC0, 0xC6, 0x40, 0xEB, 0x00
};

static const u8 VI_GM_1_7[] = {
	0x33, 0x62, 0x17, 0x5C, 0x10, 0xD2, 0x0D, 0x6D, 0x0B, 0x6D, 0x09, 0xED,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x39, 0xC0, 0x5E, 0x40, 0x82, 0x00, 0xA5, 0x40, 0xC8, 0x40, 0xEB, 0x00
};

static const u8 VI_GM_1_8[] = {
	0x38, 0x4E, 0x17, 0xAE, 0x10, 0xB4, 0x0D, 0x0C, 0x0A, 0xF0, 0x09, 0x6D,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x3D, 0xC0, 0x62, 0xC0, 0x86, 0x40, 0xA8, 0x80, 0xCA, 0x00, 0xEB, 0x00
};

static const u8 VI_GM_1_9[] = {
	0x3D, 0x3B, 0x18, 0x00, 0x10, 0x5A, 0x0C, 0xC3, 0x0A, 0x72, 0x09, 0x00,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x41, 0xC0, 0x67, 0x40, 0x8A, 0x00, 0xAB, 0x80, 0xCB, 0x80, 0xEB, 0x00
};

static const u8 VI_GM_2_0[] = {
	0x41, 0xD8, 0x18, 0x28, 0x10, 0x3C, 0x0C, 0x49, 0x0A, 0x1F, 0x08, 0x92,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x45, 0x80, 0x6B, 0x40, 0x8D, 0xC0, 0xAE, 0x00, 0xCD, 0x00, 0xEB, 0x00
};

static const u8 VI_GM_2_1[] = {
	0x46, 0x76, 0x18, 0x51, 0x0F, 0xE1, 0x0C, 0x00, 0x09, 0xB6, 0x08, 0x36,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x49, 0x40, 0x6F, 0x40, 0x91, 0x00, 0xB0, 0x80, 0xCE, 0x40, 0xEB, 0x00
};

static const u8 VI_GM_2_2[] = {
	0x4A, 0xC4, 0x18, 0x7A, 0x0F, 0xA5, 0x0B, 0x9E, 0x09, 0x63, 0x07, 0xDB,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x4C, 0xC0, 0x73, 0x00, 0x94, 0x40, 0xB2, 0xC0, 0xCF, 0x80, 0xEB, 0x00
};

static const u8 VI_GM_2_3[] = {
	0x4F, 0x13, 0x18, 0x51, 0x0F, 0x69, 0x0B, 0x6D, 0x09, 0x0F, 0x07, 0x80,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x50, 0x40, 0x76, 0x40, 0x97, 0x00, 0xB5, 0x00, 0xD0, 0xC0, 0xEB, 0x00
};

static const u8 VI_GM_2_4[] = {
	0x53, 0x13, 0x18, 0x7A, 0x0F, 0x0F, 0x0B, 0x24, 0x08, 0xBC, 0x07, 0x36,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x53, 0x80, 0x79, 0xC0, 0x99, 0xC0, 0xB7, 0x00, 0xD1, 0xC0, 0xEB, 0x00
};

static const u8 VI_GM_2_5[] = {
	0x57, 0x13, 0x18, 0x51, 0x0E, 0xF0, 0x0A, 0xC3, 0x08, 0x7D, 0x06, 0xED,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x56, 0xC0, 0x7C, 0xC0, 0x9C, 0x80, 0xB8, 0xC0, 0xD2, 0xC0, 0xEB, 0x00
};
 
static const u8 VI_GM_2_6[] = {
	0x5B, 0x13, 0x18, 0x28, 0x0E, 0x96, 0x0A, 0x92, 0x08, 0x29, 0x06, 0xB6,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x5A, 0x00, 0x7F, 0xC0, 0x9E, 0xC0, 0xBA, 0x80, 0xD3, 0x80, 0xEB, 0x00
};	

static const u8 VI_GM_2_7[] = {
	0x5E, 0xC4, 0x18, 0x00, 0x0E, 0x78, 0x0A, 0x30, 0x08, 0x00, 0x06, 0x6D,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x5D, 0x00, 0x82, 0x80, 0xA1, 0x40, 0xBC, 0x00, 0xD4, 0x80, 0xEB, 0x00
};

static const u8 VI_GM_2_8[] = {
	0x62, 0x76, 0x17, 0xD7, 0x0E, 0x1E, 0x0A, 0x00, 0x07, 0xC1, 0x06, 0x36,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x60, 0x00, 0x85, 0x40, 0xA3, 0x40, 0xBD, 0x80, 0xD5, 0x40, 0xEB, 0x00
};

static const u8 VI_GM_2_9[] = {
	0x65, 0xD8, 0x17, 0xAE, 0x0D, 0xE1, 0x09, 0xCF, 0x07, 0x82, 0x06, 0x00,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x62, 0xC0, 0x87, 0xC0, 0xA5, 0x40, 0xBF, 0x00, 0xD6, 0x00, 0xEB, 0x00
};

static const u8 VI_GM_3_0[] = {
	0x69, 0x3B, 0x17, 0x85, 0x0D, 0xA5, 0x09, 0x86, 0x07, 0x43, 0x05, 0xDB,
	0x10, 0x1D, 0x36, 0x58, 0x82, 0xB3, 0xEB,
	0x10, 0x00, 0x65, 0x80, 0x8A, 0x40, 0xA7, 0x40, 0xC0, 0x40, 0xD6, 0x80, 0xEB, 0x00
};


void AVE_GetVolume(u8 *left, u8 *right);
void AVE_SetVolume(u8 left, u8 right);
