// Translated by:  Volodymyr M. Lisivka <lvm@mystery.lviv.net>,
// Andriy Gritsenko <andrej@lucky.net>
// sevenfourk <sevenfourk@gmail.com>

// Was synced with help_mp-en.h: r28450

// ========================= MPlayer help ===========================

static const char help_text[] =
	"Запу�?к:   mplayer [опції] [path/]filename\n"
	"\n"
	"Опції:\n"
	" -vo <drv[:dev]> вибір драйвера і при�?трою відео виводу (�?пи�?ок див. з '-vo help')\n"
	" -ao <drv[:dev]> вибір драйвера і при�?трою аудіо виводу (�?пи�?ок див. з '-ao help')\n"
#ifdef CONFIG_VCD
" vcd://<номер треку> грати VCD (video cd) трек з при�?трою замі�?ть файлу\n"
#endif
#ifdef CONFIG_DVDREAD
" dvd://<номер титрів> грати DVD титри/трек з при�?трою замі�?ть файлу\n"
#endif
	" -alang/-slang   вибрати мову DVD аудіо/�?убтитрів (дво�?имвольний код країни)\n"
	" -ss <ча�?>       перемі�?тити�?�? на задану (�?екунди або ГГ:ХХ:СС) позицію\n"
	" -nosound        без звуку\n"
	" -fs -vm -zoom   повноекранне програванн�? (повноекр.,зміна відео,ма�?штабуванн�?\n"
	" -x <x> -y <y>   маштабувати картинку до <x> * <y> [�?кщо -vo драйвер підтримує!]\n"
	" -sub <file>     вказати файл �?убтитрів (див. також -subfps, -subdelay)\n"
	" -playlist <file> вказати playlist\n"
	" -vid x -aid y   опції дл�? вибору відео (x) і аудіо (y) потоку дл�? програванн�?\n"
	" -fps x -srate y опції дл�? зміни відео (x кадр/�?ек) і аудіо (y Hz) швидко�?ті\n"
	" -pp <quality>   дозволити фільтр (0-4 дл�? DivX, 0-63 дл�? mpegs)\n"
	" -framedrop      дозволити втрату кадрів (дл�? повільних машин)\n"
	"\n"
	"Клавіші:\n"
	" <-  або ->      перемотуванн�? вперед/назад на 10 �?екунд\n"
	" вверх або вниз  перемотуванн�? вперед/назад на  1 хвилину\n"
	" pgup або pgdown перемотуванн�? вперед/назад на 10 хвилин\n"
	" < або >         перемотуванн�? вперед/назад у �?пи�?ку програванн�?\n"
	" p або ПРОБІЛ    зупинити фільм (будь-�?ка клавіша - продовжити)\n"
	" q або ESC       зупинити відтворенн�? і вихід\n"
	" + або -         регулювати затримку звуку по +/- 0.1 �?екунді\n"
	" o               циклічний перебір OSD режимів:  нема / навігаці�? / навігаці�?+таймер\n"
	" * або /         додати або зменшити гучні�?ть (нати�?канн�? 'm' вибирає master/pcm)\n"
	" z або x         регулювати затримку �?убтитрів по +/- 0.1 �?екунді\n"
	" r or t          змінити положенн�? �?убтитрів вгору/вниз, також див. -vf expand\n"
	"\n"
	" * * * ДЕТ�?ЛЬ�?ІШЕ ДИВ. ДОКУМЕ�?Т�?ЦІЮ, ПРО ДОД�?ТКОВІ ОПЦІЇ І КЛЮЧІ! * * *\n"
	"\n";

// ========================= MPlayer messages ===========================

// mplayer.c:
#define MSGTR_Exiting "\nВиходимо...\n"
#define MSGTR_ExitingHow "\nВиходимо... (%s)\n"
#define MSGTR_Exit_quit "Вихід"
#define MSGTR_Exit_eof "Кінець файлу"
#define MSGTR_Exit_error "Фатальна помилка"
#define MSGTR_IntBySignal "\nMPlayer перерваний �?игналом %d у модулі: %s \n"
#define MSGTR_NoHomeDir "�?е можу знайти домашній каталог\n"
#define MSGTR_GetpathProblem "проблеми у get_path(\"config\")\n"
#define MSGTR_CreatingCfgFile "Створенн�? файлу конфігурації: %s\n"
#define MSGTR_CantLoadFont "�?е можу завантажити шрифт: %s\n"
#define MSGTR_CantLoadSub "�?е можу завантажити �?убтитри: %s\n"
#define MSGTR_DumpSelectedStreamMissing "dump: FATAL: обраний потік загублений!\n"
#define MSGTR_CantOpenDumpfile "�?е можу відкрити файл дампу!!!\n"
#define MSGTR_CoreDumped "Створено дамп �?дра :)\n"
#define MSGTR_FPSnotspecified "�?е вказано чи невірна кількі�?ть кадрів, за�?то�?уйте опцію -fps.\n"
#define MSGTR_TryForceAudioFmtStr "�?амагаю�?ь фор�?увати групу аудіо кодеків %s...\n"
#define MSGTR_CantFindVideoCodec "�?е можу знайти кодек дл�? відео формату 0x%X!\n"
#define MSGTR_TryForceVideoFmtStr "�?амагаю�?ь фор�?увати групу відео кодеків %s...\n"
#define MSGTR_CannotInitVO "Ф�?Т�?ЛЬ�?О: �?е можу ініціалізувати відео драйвер!\n"
#define MSGTR_CannotInitAO "не можу відкрити/ініціалізувати аудіо при�?трій -> ГР�?Ю БЕЗ ЗВУКУ\n"
#define MSGTR_StartPlaying "Початок програванн�?...\n"

#define MSGTR_SystemTooSlow "\n\n"\
"         ********************************************************\n"\
"         **** Ваша �?и�?тема надто ПОВІЛЬ�?�? щоб відтворити це! ****\n"\
"         ********************************************************\n"\
"!!! Можливі причини, проблеми, обхідні шл�?хи: \n"\
"- �?айбільш загальні: поганий/�?ирий _аудіо_ драйвер :\n"\
"  - �?пробуйте -ao sdl або викори�?товуйте ALSA 0.5 або емул�?цію oss на ALSA 0.9.\n"\
"  - Ек�?перимент з різними значенн�?ми дл�? -autosync, �?пробуйте 30 .\n"\
"- Повільний відео вивід.\n"\
"  - �?пробуйте інший -vo драйвер (�?пи�?ок: -vo help) або �?пробуйте з -framedrop!\n"\
"- Повільний ЦП. �?е намагайте�?�? відтворювати великі dvd/divx на повільних\n"\
"  проце�?орах! �?пробуйте -hardframedrop\n"\
"- Битий файл. Спробуйте різні комбінації: -nobps  -ni  -mc 0  -forceidx\n"\
"- Повільний но�?ій (ди�?ки NFS/SMB, DVD, VCD та ін.). Спробуйте -cache 8192.\n"\
"- Ви викори�?товуєте -cache дл�? програванн�? неперемеженого AVI файлу?\n"\
"  - �?пробуйте -nocache.\n"\
"Читайте поради в файлах DOCS/HTML/en/video.html .\n"\
"Якщо нічого не допомогло, тоді читайте DOCS/HTML/en/bugreports.html!\n\n"

#define MSGTR_NoGui "MPlayer був �?компільований БЕЗ підтримки GUI!\n"
#define MSGTR_GuiNeedsX "MPlayer GUI вимагає X11!\n"
#define MSGTR_Playing "\nПрограванн�? %s\n"
#define MSGTR_NoSound "�?удіо: без звуку!!!\n"
#define MSGTR_FPSforced "Приму�?ово змінена кількі�?ть кадрів на �?екунду на %5.3f (ftime: %5.3f)\n"
#define MSGTR_AvailableVideoOutputDrivers "До�?тупні модулі відео виводу:\n"
#define MSGTR_AvailableAudioOutputDrivers "До�?тупні модулі аудіо виводу:\n"
#define MSGTR_AvailableAudioCodecs "До�?тупні аудіо кодеки:\n"
#define MSGTR_AvailableVideoCodecs "До�?тупні відео кодеки:\n"
#define MSGTR_AvailableAudioFm "До�?тупні (вбудовані) групи/драйвера аудіо кодеків:\n"
#define MSGTR_AvailableVideoFm "До�?тупні (вбудовані) групи/драйвера відео кодеків:\n"
#define MSGTR_AvailableFsType "До�?тупні варіанти повноекранного відеорежиму:\n"
#define MSGTR_CannotReadVideoProperties "Відео: �?еможливо отримати вла�?тиво�?ті.\n"
#define MSGTR_NoStreamFound "Потік не знайдено.\n"
#define MSGTR_ErrorInitializingVODevice "Помилка відкритт�?/ініціалізації вибраного video_out (-vo) при�?трою.\n"
#define MSGTR_ForcedVideoCodec "Приму�?овий відео кодек: %s\n"
#define MSGTR_ForcedAudioCodec "Приму�?овий аудіо кодек: %s\n"
#define MSGTR_Video_NoVideo "Відео: без відео\n"
#define MSGTR_NotInitializeVOPorVO "\nFATAL: �?еможливо ініціалізувати відео фільтри (-vf) або відео вивід (-vo).\n"
#define MSGTR_Paused "  =====  П�?УЗ�?  ====="
#define MSGTR_PlaylistLoadUnable "\n�?еможливо завантажити playlist %s.\n"
#define MSGTR_Exit_SIGILL_RTCpuSel \
"- MPlayer зламав�?�? через '�?евірні ін�?трукції'.\n"\
"  Може бути помилка у вашому новому коду визначенн�? типу CPU...\n"\
"  Будь-ла�?ка перегл�?ньте DOCS/HTML/en/bugreports.html.\n"
#define MSGTR_Exit_SIGILL \
"- MPlayer зламав�?�? через '�?евірні ін�?трукції'.\n"\
"  Іноді таке трапл�?єть�?�? під ча�? запу�?ку програвача на CPU що відрізн�?єть�?�? від того, на �?кому він\n"\
"  був зібраний/оптимізований.\n  Перевірте!\n"
#define MSGTR_Exit_SIGSEGV_SIGFPE \
"- MPlayer зламав�?�? через невірне викори�?танн�? CPU/FPU/RAM.\n"\
"  Зберіть знову MPlayer з --enable-debug а зробіть 'gdb' backtrace та\n"\
"  диза�?емблюванн�?. Дл�? довідок, перегл�?ньте DOCS/HTML/en/bugreports_what.html#bugreports_crash\n"
#define MSGTR_Exit_SIGCRASH \
"- MPlayer зламав�?�?. Цього не повинно було трапити�?�?.\n"\
"  Може бути помилка у коді MPlayer _або_ ваших драйверах _або_ через\n"\
"  вер�?ію gcc. Якщо важаєте що, це помилка MPlayer, будь-ла�?ка читайте\n"\
"  DOCS/HTML/en/bugreports.html та �?лідкуєте ін�?трукці�?м. Ми можемо\n"\
"  допомогти лише у разі забезпеченн�? інформаці�? коли доповідаєте про помилку.\n"
#define MSGTR_LoadingConfig "Завантаженн�? конфігурації '%s'\n"
#define MSGTR_LoadingProtocolProfile "Завантаженн�? профілю дл�? протоколу '%s'\n"
#define MSGTR_LoadingExtensionProfile "Завантаженн�? профілю дл�? розширенн�? '%s'\n"
#define MSGTR_AddedSubtitleFile "СУБТИТРИ: Додано файл �?убтитрів (%d): %s\n"
#define MSGTR_RemovedSubtitleFile "СУБТИТРИ: Видалено файл �?убтитрів (%d): %s\n"
#define MSGTR_ErrorOpeningOutputFile "Помилка при відкритті файлу [%s] дл�? запи�?у!\n"
#define MSGTR_RTCDeviceNotOpenable "�?е можу відкрити %s: %s (кори�?тувач повинен мати права читанн�? дл�? файлу.)\n"
#define MSGTR_LinuxRTCInitErrorIrqpSet "Помилка ініцілізації Linux RTC у ioctl (rtc_irqp_set %lu): %s\n"
#define MSGTR_IncreaseRTCMaxUserFreq "Спробуйте додати \"echo %lu > /proc/sys/dev/rtc/max-user-freq\" до �?криптів запу�?ку �?и�?теми.\n"
#define MSGTR_LinuxRTCInitErrorPieOn "Помилка ініціалізації Linux RTC у ioctl (rtc_pie_on): %s\n"
#define MSGTR_UsingTimingType "Викори�?товую %s �?инхронізацію.\n"
#define MSGTR_Getch2InitializedTwice "ПОПЕРЕДЖЕ�?�?Я: getch2_init визвано двічі!\n"
#define MSGTR_DumpstreamFdUnavailable "�?е можу �?творити дамп цього потоку - не має до�?тупного дек�?риптору.\n"
#define MSGTR_CantOpenLibmenuFilterWithThisRootMenu "�?е можу відкрити відео фільтр libmenu з цим кореневим меню %s.\n"
#define MSGTR_AudioFilterChainPreinitError "Помилка у ланці pre-init аудіо фільтру!\n"
#define MSGTR_LinuxRTCReadError "Помилка читанн�? Linux RTC: %s\n"
#define MSGTR_SoftsleepUnderflow "Попередженн�?! �?едупу�?тиме низьке значенн�? затримки програми!\n"
#define MSGTR_DvdnavNullEvent "Поді�? DVDNAV NULL?!\n"
#define MSGTR_DvdnavHighlightEventBroken "Поді�? DVDNAV: Поді�? виділенн�? зламана\n"
#define MSGTR_DvdnavEvent "Поді�? DVDNAV: %s\n"
#define MSGTR_DvdnavHighlightHide "Поді�? DVDNAV: Виділенн�? �?ховано\n"
#define MSGTR_DvdnavStillFrame "######################################## Поді�? DVDNAV: Стоп-кадр: %d �?ек\n"
#define MSGTR_DvdnavNavStop "Поді�? DVDNAV: Зупинка Nav\n"
#define MSGTR_DvdnavNavNOP "Поді�? DVDNAV: Nav NOP\n"
#define MSGTR_DvdnavNavSpuStreamChangeVerbose "Поді�? DVDNAV: Зміна SPU потоку Nav: фізично: %d/%d/%d логічно: %d\n"
#define MSGTR_DvdnavNavSpuStreamChange "Поді�? DVDNAV: Зміна SPU потоку Nav: фізично: %d логічно: %d\n"
#define MSGTR_DvdnavNavAudioStreamChange "Поді�? DVDNAV: Зміна �?удіо потоку Nav: фізично: %d логічно: %d\n"
#define MSGTR_DvdnavNavVTSChange "Поді�? DVDNAV: Зміна Nav VTS\n"
#define MSGTR_DvdnavNavCellChange "Поді�? DVDNAV: Зміна Nav Cell\n"
#define MSGTR_DvdnavNavSpuClutChange "Поді�? DVDNAV: Зміна Nav SPU CLUT\n"
#define MSGTR_DvdnavNavSeekDone "Поді�? DVDNAV: Nav Seek зроблено\n"
#define MSGTR_MenuCall "Виклик меню\n"

// --- edit decision lists
#define MSGTR_EdlOutOfMem "�?е можу виділити до�?татньо пам'�?ті дл�? збереженн�? даних EDL.\n"
#define MSGTR_EdlRecordsNo "Читанн�? %d EDL дій.\n"
#define MSGTR_EdlQueueEmpty "�?емає дій EDL �?кі треба виконати.\n"
#define MSGTR_EdlCantOpenForWrite "�?е може відкрити EDL файл [%s] дл�? запи�?у.\n"
#define MSGTR_EdlCantOpenForRead "�?е може відкрити EDL файл [%s] дл�? читанн�?.\n"
#define MSGTR_EdlNOsh_video "�?е можу викори�?тати EDL без відео, вимикаю.\n"
#define MSGTR_EdlNOValidLine "�?евірний р�?док EDL: %s\n"
#define MSGTR_EdlBadlyFormattedLine "Погано відформатований EDL р�?док [%d], пропу�?каю.\n"
#define MSGTR_EdlBadLineOverlap "О�?танн�? зупинка була [%f]; на�?тупний �?тарт [%f].\n"\
"Запи�?и повинні бути у хронологічному пор�?дку, не можу перекрити. Пропу�?каю.\n"
#define MSGTR_EdlBadLineBadStop "Ча�? зупинки повинен бути пі�?л�? ча�?у �?тарту.\n"
#define MSGTR_EdloutBadStop "Ігноруванн�? EDL відмінено, о�?танній start > stop\n"
#define MSGTR_EdloutStartSkip "Старт EDL пропу�?ку, нати�?ніть 'i' знов, щоб завершити блок.\n"
#define MSGTR_EdloutEndSkip "Кінець EDL пропу�?ку, р�?док запи�?ано.\n"

// mplayer.c OSD
#define MSGTR_OSDenabled "увімкнено"
#define MSGTR_OSDdisabled "вимкнено"
#define MSGTR_OSDAudio "�?удіо: %s"
#define MSGTR_OSDVideo "Відео: %s"
#define MSGTR_OSDChannel "Канал: %s"
#define MSGTR_OSDSubDelay "Затримка �?убтитрыв: %d м�?"
#define MSGTR_OSDSpeed "Швидкі�?ть: x %6.2f"
#define MSGTR_OSDosd "OSD: %s"
#define MSGTR_OSDChapter "Розділ: (%d) %s"
#define MSGTR_OSDAngle "Кут: %d/%d"

// property values
#define MSGTR_Enabled "увімкнено"
#define MSGTR_EnabledEdl "увімкнено (EDL)"
#define MSGTR_Disabled "вимкнено"
#define MSGTR_HardFrameDrop "інтен�?ивний"
#define MSGTR_Unknown "невідомий"
#define MSGTR_Bottom "низ"
#define MSGTR_Center "центр"
#define MSGTR_Top "верх"
#define MSGTR_SubSourceFile "файл"
#define MSGTR_SubSourceVobsub "vobsub"
#define MSGTR_SubSourceDemux "вкладено"

// OSD bar names
#define MSGTR_Volume "Гучні�?ть"
#define MSGTR_Panscan "Зріз �?торін"
#define MSGTR_Gamma "Гамма"
#define MSGTR_Brightness "Я�?краві�?ть"
#define MSGTR_Contrast "Контра�?т"
#define MSGTR_Saturation "�?а�?иченні�?ть"
#define MSGTR_Hue "Колір"
#define MSGTR_Balance "Балан�?"

// property state
#define MSGTR_LoopStatus "Повтор: %s"
#define MSGTR_MuteStatus "Вимкнути звук: %s"
#define MSGTR_AVDelayStatus "A-V затримка: %s"
#define MSGTR_OnTopStatus "Звурху інших: %s"
#define MSGTR_RootwinStatus "Вікно-root: %s"
#define MSGTR_BorderStatus "Рамка: %s"
#define MSGTR_FramedroppingStatus "Пропу�?к кадрів: %s"
#define MSGTR_VSyncStatus "Вертикальна �?инхронізаці�?: %s"
#define MSGTR_SubSelectStatus "Субтитри: %s"
#define MSGTR_SubSourceStatus "Субтитри з: %s"
#define MSGTR_SubPosStatus "Позиці�? �?убтитрів: %s/100"
#define MSGTR_SubAlignStatus "Вирівнюванн�? �?убтитрів: %s"
#define MSGTR_SubDelayStatus "Затримка �?убтитрів: %s"
#define MSGTR_SubScale "Ма�?штаб �?убтитрів: %s"
#define MSGTR_SubVisibleStatus "Субтитри: %s"
#define MSGTR_SubForcedOnlyStatus "Фор�?увати тільки �?убтитри: %s"

// mencoder.c:
#define MSGTR_UsingPass3ControlFile "Викори�?товую pass3 файл: %s\n"
#define MSGTR_MissingFilename "\n�?евизначений файл.\n\n"
#define MSGTR_CannotOpenFile_Device "�?еможливо відкрити файл/при�?трій.\n"
#define MSGTR_CannotOpenDemuxer "�?еможливо відкрити demuxer.\n"
#define MSGTR_NoAudioEncoderSelected "\n�?е вибраний аудіо кодек (-oac). Виберіть або викори�?товуйте -nosound. Спробуйте -oac help!\n"
#define MSGTR_NoVideoEncoderSelected "\n�?е вибраний відео кодек (-ovc). Виберіть, �?пробуйте -ovc help!\n"
#define MSGTR_CannotOpenOutputFile "�?еможливо �?творити файл '%s'.\n"
#define MSGTR_EncoderOpenFailed "�?еможливо відкрити кодек.\n"
#define MSGTR_MencoderWrongFormatAVI "\nПОПЕРЕДЖЕ�?�?Я: ФОРМ�?Т Ф�?ЙЛУ �?�? ВИХОДІ _AVI_. Погл�?ньте -of help.\n"
#define MSGTR_MencoderWrongFormatMPG "\nПОПЕРЕДЖЕ�?�?Я: ФОРМ�?Т Ф�?ЙЛУ �?�? ВИХОДІ _MPEG_. Погл�?ньте -of help.\n"
#define MSGTR_MissingOutputFilename "�?е вказано файлу на виході, будь-ла�?ка подивіть�?�? опцію -o."
#define MSGTR_ForcingOutputFourcc "В�?тановлюю вихідний fourcc в %x [%.4s]\n"
#define MSGTR_ForcingOutputAudiofmtTag "Фор�?ую таг аудіо фармату на виході до 0x%x.\n"
#define MSGTR_DuplicateFrames "\n%d повторних кадрів!\n"
#define MSGTR_SkipFrame "\nКадр пропущено!\n"
#define MSGTR_ResolutionDoesntMatch "\n�?овий та попередній відео файл має різне розширенн�? та кольорову гаму.\n"
#define MSGTR_FrameCopyFileMismatch "\nУ�?і відео файли повинні мати однакові кадр/�?ек, розширенн�?, та кодек дл�? -ovc copy.\n"
#define MSGTR_AudioCopyFileMismatch "\nУ�?і відео файли повинні мати однакові аудіо кодек та формат дл�? -oac copy.\n"
#define MSGTR_NoAudioFileMismatch "\n�?е можу поєднати файли відео з файлами аудіо та відео. Спробуйте -nosound.\n"
#define MSGTR_NoSpeedWithFrameCopy "ПОПЕРЕДЖЕ�?�?Я: опці�? -speed не гарантує коректну роботу з -oac copy!\n"\
"Ваше кодуванн�? може бути невдалим!\n"
#define MSGTR_ErrorWritingFile "%s: Помилка запи�?у файлу.\n"
#define MSGTR_FlushingVideoFrames "\nЗкидую кадри відео.\n"
#define MSGTR_FiltersHaveNotBeenConfiguredEmptyFile "Фільтри не було налаштовано! Порожній файл?\n"
#define MSGTR_RecommendedVideoBitrate "Рекомендований бітрейт дл�? %s CD: %d\n"
#define MSGTR_VideoStreamResult "\nВідео потік: %8.3f кбіт/�?  (%d Б/�?)  розмір: %"PRIu64" байт(ів)  %5.3f �?ек  %d кадрів\n"
#define MSGTR_AudioStreamResult "\n�?удіо потік: %8.3f кбіт/�?  (%d Б/�?)  розмір: %"PRIu64" байт(ів)  %5.3f �?ек\n"
#define MSGTR_EdlSkipStartEndCurrent "EDL SKIP: Початок: %.2f  Кінець: %.2f   Поточна: V: %.2f  A: %.2f     \r"
#define MSGTR_OpenedStream "вдало: формат: %d  дані: 0x%X - 0x%x\n"
#define MSGTR_VCodecFramecopy "відеокодек: копі�? кадрів (%dx%d %dbpp fourcc=%x)\n"
#define MSGTR_ACodecFramecopy "аудіокодек: копі�? кадрів (формат=%x каналів=%d швидкі�?ть=%d бітів=%d Б/�?=%d приклад-%d)\n"
#define MSGTR_CBRPCMAudioSelected "Вибрано CBR PCM аудіо.\n"
#define MSGTR_MP3AudioSelected "Вибрано MP3 аудіо.\n"
#define MSGTR_CannotAllocateBytes "�?е можу виділити пам'�?ть дл�? %d байтів.\n"
#define MSGTR_SettingAudioDelay "В�?тановлюю аудіо затримку у %5.3f�?.\n"
#define MSGTR_SettingVideoDelay "В�?тановлюю відео затримку у %5.3f�?.\n"
#define MSGTR_LimitingAudioPreload "Обмежити підвантаженн�? аудіо до 0.4�?.\n"
#define MSGTR_IncreasingAudioDensity "Збільшую гу�?тину аудіо до 4.\n"
#define MSGTR_ZeroingAudioPreloadAndMaxPtsCorrection "Фор�?ую аудіо підвантаженн�? до 0, мак�?имальну корекцію pts у 0.\n"
#define MSGTR_LameVersion "Вер�?і�? LAME %s (%s)\n\n"
#define MSGTR_InvalidBitrateForLamePreset "Помилка: Вказаний бітрейт не є вірним дл�? даного в�?тановленн�?.\n"\
"\n"\
"Викори�?товуючи цей режим ви повинні вве�?ти значенн�? між \"8\" та \"320\".\n"\
"\n"\
"Дл�? подальшої інформації �?пробуйте: \"-lameopts preset=help\"\n"
#define MSGTR_InvalidLamePresetOptions "Помилка: Ви не ввели дій�?ний профайл та/чи опції з в�?тановленн�?.\n"\
"\n"\
"До�?тупні профайли:\n"\
"\n"\
"   <fast>        standard\n"\
"   <fast>        extreme\n"\
"                 insane\n"\
"   <cbr> (Режим ABR) - Маєть�?�? на увазі режим ABR. Дл�? викори�?танн�?,\n"\
"                       про�?то вкажіть бітрейт. �?априклад:\n"\
"                       \"preset=185\" активує це\n"\
"                       в�?тановленн�? та викори�?товує 185 �?к �?ереднє значенн�? кбіт/�?.\n"\
"\n"\
"    Декілька прикладів:\n"\
"\n"\
"    \"-lameopts fast:preset=standard  \"\n"\
" or \"-lameopts  cbr:preset=192       \"\n"\
" or \"-lameopts      preset=172       \"\n"\
" or \"-lameopts      preset=extreme   \"\n"\
"\n"\
"Дл�? подальшої інформації �?пробуйте: \"-lameopts preset=help\"\n"
#define MSGTR_LamePresetsLongInfo "\n"\
"В�?тановленн�? розроблені так, щоб отримати �?кнайкращу �?кі�?ть.\n"\
"\n"\
"Вони були розроблені та налаштовані у результаті ретельних те�?тів\n"\
"те�?ти подвійного про�?луховуванн�?, щоб до�?�?гти цього результату.\n"\
"\n"\
"Ключі в�?тановлень по�?тійно поновлюють�?�?, щоб відповідати о�?таннім розробленн�?м.\n"\
"в результаті чого ви повинні отримати практично найкращу �?кі�?ть\n"\
"на даний момент можливо при викори�?танні LAME.\n"\
"\n"\
"Щоб активувати ці в�?тановленн�?:\n"\
"\n"\
"   Дл�? VBR режимів (найкраща �?кі�?ть звичайно):\n"\
"\n"\
"     \"preset=standard\" Звичайно цього в�?тановленн�? повинно бути до�?татньо\n"\
"                             дл�? більшо�?ті людей та більшо�?ті музики, та воно\n"\
"                             �?вл�?є �?обою до�?ить ви�?оку �?кі�?ть.\n"\
"\n"\
"     \"preset=extreme\" Якщо у ва�? хороший �?лух та добра музича апаратура,\n"\
"                             це в�?тановленн�? �?к правило забезпечить кращу �?кі�?ть\n"\
"                             ніж режим \"standard\"\n"\
"                             mode.\n"\
"\n"\
"   Дл�? CBR 320kbps (мак�?имально можлива �?кі�?ть, �?ку можна тримати з в�?тановлень):\n"\
"\n"\
"     \"preset=insane\"  Це в�?тановленн�? звичайно буде занадто дл�? більшо�?ті людей\n"\
"                             та �?итуацій, але �?кщо ви му�?ите отримати найкращу\n"\
"                             мак�?имально можливу �?кі�?ть, не дивл�?чи�?ь на\n"\
"                             розмір файлу, це ваш вибір.\n"\
"\n"\
"   Дл�? ABR режимів (ви�?ока �?кі�?ть дл�? заданого бітрейта, але така ви�?ока �?к VBR):\n"\
"\n"\
"     \"preset=<kbps>\"  Викори�?товуючи це в�?тановленн�? звичайно дає добру �?кі�?ть\n"\
"                             дл�? заданого бітрейта. Базуючи�?ь на введеному\n"\
"                             бітрейті, це в�?тановленн�? визначить оптимальні\n"\
"                             налаштуванн�? дл�? кожной конкретного випадку.\n"\
"                             �?е дивл�?чи�?ь на то, що цей підхід працює, він\n"\
"                             далеко не такий гнучкий �?к VBR, та звичайно не до�?�?гає\n"\
"                             такого рівн�? �?ко�?ті �?к VBR на ви�?оких бітрейтах.\n"\
"\n"\
"�?а�?тупні опції також до�?тупні дл�? і�?нуючих профілей:\n"\
"\n"\
"   <fast>        standard\n"\
"   <fast>        extreme\n"\
"                 insane\n"\
"   <cbr> (Режим ABR) - Маєть�?�? на увазі режим ABR. Дл�? викори�?танн�?\n"\
"                       про�?то вкажіть бітрейт. �?априклад:\n"\
"                       \"preset=185\" активує це в�?тановленн�?\n"\
"                       та викори�?та�? 185 �?к �?ереднє значенн�? кбіт/�?ек.\n"\
"\n"\
"   \"fast\" - Вмикає новий швидкий VBR дл�? конкретного профілю.\n"\
"            �?едо�?татком цього ключа є те, що ча�?то бітрейт буде\n"\
"            набагато більше ніж у нормальному режимі;\n"\
"            а �?кі�?ть може буте дещо гірше.\n"\
"Попередженн�?: У теперешній вер�?ії швидкі в�?тановленн�? можуть приве�?ти до\n"\
"              ви�?окому бітрейту, у порівн�?нні з звичайними в�?тановленн�?ми.\n"\
"\n"\
"   \"cbr\"  - Якщо ви викори�?товуєте режим ABR (див. вище) з бітрейтом кратним\n"\
"            80, 96, 112, 128, 160, 192, 224, 256, 320,\n"\
"            ви можете за�?то�?увати опцію \"cbr\" щоб фор�?увати кодуванн�? у режимі\n"\
"            CBR замі�?ть �?тандартного режиму abr. ABR забезпечує кращу �?кі�?ть,\n"\
"            але CBR може бути кори�?ним у таких �?итуаці�?х,\n"\
"            �?к передача потоків mp3 через інтернет.\n"\
"\n"\
"    �?априклад:\n"\
"\n"\
"    \"-lameopts fast:preset=standard  \"\n"\
" or \"-lameopts  cbr:preset=192       \"\n"\
" or \"-lameopts      preset=172       \"\n"\
" or \"-lameopts      preset=extreme   \"\n"\
"\n"\
"\n"\
"Декілька п�?евдонімів до�?тупні дл�? режима ABR:\n"\
"phone => 16kbps/mono        phon+/lw/mw-eu/sw => 24kbps/mono\n"\
"mw-us => 40kbps/mono        voice => 56kbps/mono\n"\
"fm/radio/tape => 112kbps    hifi => 160kbps\n"\
"cd => 192kbps               studio => 256kbps"
#define MSGTR_LameCantInit \
"�?е можу в�?тановити опції LAME, перевірте бітрейт/ча�?тому_ди�?кретизації, де�?кі\n"\
"дуже низькі бітрейти (<32) потребують менші ча�?тоти\nди�?кретизації(наприклад, -srate 8000).\n"\
"Якщо в�?е це не допоможе, �?пробуйте в�?тановленн�?."
#define MSGTR_ConfigFileError "помилка у файлі налаштувань"
#define MSGTR_ErrorParsingCommandLine "помилка аналізу командного р�?дка"
#define MSGTR_VideoStreamRequired "Вивід відео обов'�?зковий!\n"
#define MSGTR_ForcingInputFPS "Вхідні кадри/�?ек будуть замінені на %5.3f.\n"
#define MSGTR_RawvideoDoesNotSupportAudio "Вихідний формат файлу RAWVIDEO не підтримує аудіо - вимикаю відео.\n"
#define MSGTR_DemuxerDoesntSupportNosound "Цей демультиплек�?ор поки не підтримуєть�?�? -nosound.\n"
#define MSGTR_MemAllocFailed "�?е можу виділити пам'�?ть.\n"
#define MSGTR_NoMatchingFilter "�?е можу знайти потрібний фільтр/формат аудіовиводу!\n"
#define MSGTR_MP3WaveFormatSizeNot30 "sizeof(MPEGLAYER3WAVEFORMAT)==%d!=30, можливо зламаний компіл�?тор C?\n"
#define MSGTR_NoLavcAudioCodecName "�?удіо LAVC, від�?утнє назва кодека!\n"
#define MSGTR_LavcAudioCodecNotFound "�?удіо LAVC, не можу знайти кодувальщик дл�? кодека %s.\n"
#define MSGTR_CouldntAllocateLavcContext "�?удіо LAVC, не можу розмі�?тити контек�?т!\n"
#define MSGTR_CouldntOpenCodec "�?е можу відкрити кодек %s, br=%d.\n"
#define MSGTR_CantCopyAudioFormat "�?удіо формат 0x%x не викори�?товуєть�?�? з '-oac copy', �?пробуйте\n'-oac pcm' замі�?ть чи викори�?тайте '-fafmttag' дл�? його перевизначенн�?.\n"

// cfg-mencoder.h:
#define MSGTR_MEncoderMP3LameHelp "\n\n"\
" vbr=<0-4>     метод змінного бітрейту\n"\
"                0: cbr (по�?тійний бітрейт)\n"\
"                1: mt (Mark Taylor VBR алгоритм)\n"\
"                2: rh (Robert Hegemann VBR алгоритм - зомовчуванн�?)\n"\
"                3: abr (�?ередній бітрейт)\n"\
"                4: mtrh (Mark Taylor Robert Hegemann VBR алгоритм)\n"\
"\n"\
" abr           �?ередній бітрейт\n"\
"\n"\
" cbr           по�?тійний бітрейт\n"\
"               Також фор�?ує CBR режим кодуванн�? на �?лідуючих ABR режимах\n"\
"\n"\
" br=<0-1024>   вказати бітрейт в kBit (тільки дл�? CBR та ABR)\n"\
"\n"\
" q=<0-9>       �?кі�?ть (0-найвища, 9-найнижча) (тільки дл�? VBR)\n"\
"\n"\
" aq=<0-9>      алгорітмична �?кі�?ть (0-краща/повільніша 9-гірша/швидкіша)\n"\
"\n"\
" ratio=<1-100> рівень �?ти�?ненн�?\n"\
"\n"\
" vol=<0-10>    в�?тановити по�?иленн�? вхідного аудіо\n"\
"\n"\
" mode=<0-3>    (замовчуванн�?: auto)\n"\
"                0: stereo\n"\
"                1: joint-stereo\n"\
"                2: dualchannel\n"\
"                3: mono\n"\
"\n"\
" padding=<0-2>\n"\
"                0: no\n"\
"                1: all\n"\
"                2: adjust\n"\
"\n"\
" fast          Переходити на швидке кодуванн�? при по�?лідовних VBR presets modes,\n"\
"               трохи менша �?кі�?ть та більші бітрейти.\n"\
"\n"\
" preset=<value> запровадити найбільші у�?тановки �?ко�?ті.\n"\
"                 �?ередн�?: VBR кодуванн�?, добра �?кі�?ть\n"\
"                 (150-180 kbps бітрейт)\n"\
"                 �?тандарт: VBR кодуванн�?, ви�?ока �?кі�?ть\n"\
"                 (170-210 kbps бітрейт)\n"\
"                 ви�?ока: VBR кодуванн�?, дуже ви�?ока �?кі�?ть\n"\
"                 (200-240 kbps бітрейт)\n"\
"                 найкраща: CBR кодуванн�?, найвища �?ко�?ть\n"\
"                 (320 kbps бітрейт)\n"\
"                 <8-320>: ABR кодуванн�? з вказаним приблизним бітрейтом.\n\n"

// codec-cfg.c
#define MSGTR_DuplicateFourcc "подвоєні FourCC"
#define MSGTR_TooManyFourccs "забагато FourCCs/форматів..."
#define MSGTR_ParseError "помилка у �?интак�?ичному розборі"
#define MSGTR_ParseErrorFIDNotNumber "помилка у �?интак�?ичному розборі (ID формату не є номером?)"
#define MSGTR_ParseErrorFIDAliasNotNumber "помилка у �?интак�?ичному розборі (ID ім'�? формату не є номером?)"
#define MSGTR_DuplicateFID "подвоєний ID формату"
#define MSGTR_TooManyOut "забагато вихідних форматів..."
#define MSGTR_InvalidCodecName "\nкодек(%s) ім'�? невірне!\n"
#define MSGTR_CodecLacksFourcc "\nкодек(%s) не має FourCC/фармат!\n"
#define MSGTR_CodecLacksDriver "\nкодек(%s) не має драйверу!\n"
#define MSGTR_CodecNeedsDLL "\nкодек(%s) потребує 'dll'!\n"
#define MSGTR_CodecNeedsOutfmt "\nкодек(%s) потребує 'outfmt'!\n"
#define MSGTR_CantAllocateComment "�?е можу виділити пам'�?ть дл�? коментар�?. "
#define MSGTR_GetTokenMaxNotLessThanMAX_NR_TOKEN "get_token(): max >= MAX_MR_TOKEN!"
#define MSGTR_CantGetMemoryForLine "�?е можу виділити пам'�?ть дл�? 'line': %s\n"
#define MSGTR_CantReallocCodecsp "�?е можу виконати realloc дл�? '*codecsp': %s\n"
#define MSGTR_CodecNameNotUnique "�?азва кодеку '%s' не унікальна."
#define MSGTR_CantStrdupName "�?е можу виконати strdup -> 'name': %s\n"
#define MSGTR_CantStrdupInfo "�?е можу виконати strdup -> 'info': %s\n"
#define MSGTR_CantStrdupDriver "�?е можу виконати strdup -> 'driver': %s\n"
#define MSGTR_CantStrdupDLL "�?е можу виконати strdup -> 'dll': %s"
#define MSGTR_AudioVideoCodecTotals "%d аудіо & %d відео кодеки\n"
#define MSGTR_CodecDefinitionIncorrect "�?еправильно визначено кодек."
#define MSGTR_OutdatedCodecsConf "Цей codecs.conf за�?тарий та не�?умі�?ний із цим релізом MPlayer!"

// fifo.c
#define MSGTR_CannotMakePipe "�?е можу �?творити канал!\n"

// parser-mecmd.c, parser-mpcmd.c
#define MSGTR_NoFileGivenOnCommandLine "'--' означає кінець опцій, але не було вказано назви файлу у команд. р�?дку.\n"
#define MSGTR_TheLoopOptionMustBeAnInteger "Опці�? loop має бути цілим чи�?лом: %s\n"
#define MSGTR_UnknownOptionOnCommandLine "�?евідома опці�? команд. р�?дку: -%s\n"
#define MSGTR_ErrorParsingOptionOnCommandLine "Помилка аналізу опції команд. р�?дку: -%s\n"
#define MSGTR_InvalidPlayEntry "�?евірний елемент програванн�? %s\n"
#define MSGTR_NotAnMEncoderOption "-%s не є опцією MEncoder \n"
#define MSGTR_NoFileGiven "Файл не вказано\n"

// m_config.c
#define MSGTR_SaveSlotTooOld "Знайдений �?лот збереженн�? за�?тарий з lvl %d: %d !!!\n"
#define MSGTR_InvalidCfgfileOption "Опці�? %s не може бути викори�?тана у файлі конфігурації.\n"
#define MSGTR_InvalidCmdlineOption "Опці�? %s не може бути викори�?тана у команд. р�?дку.\n"
#define MSGTR_InvalidSuboption "Помилка: опці�? '%s' не має �?убопцій '%s'.\n"
#define MSGTR_MissingSuboptionParameter "Помилка: в �?убопції '%s' опції '%s' повинен бути параметр!\n"
#define MSGTR_MissingOptionParameter "Помилка: опці�? '%s' повинна мати параметр!\n"
#define MSGTR_OptionListHeader "\n І'м�?                 Тип            Мін        Мак�?      Загальн  CL    Конф\3n\n"
#define MSGTR_TotalOptions "\nЗагалом: %d опцій\n"
#define MSGTR_ProfileInclusionTooDeep "ПОПЕРЕДЖЕ�?�?Я: Включенн�? профайлу дуже глибоко.\n"
#define MSGTR_NoProfileDefined "�?е визначено профайлів.\n"
#define MSGTR_AvailableProfiles "До�?тупні профайлиs:\n"
#define MSGTR_UnknownProfile "�?евідомий профайл '%s'.\n"
#define MSGTR_Profile "Профайл %s: %s\n"

// m_property.c
#define MSGTR_PropertyListHeader "\n �?азва                 Тип            Мін        Мак�?\n\n"
#define MSGTR_TotalProperties "\nЗагалом: %d вла�?тиво�?тей\n"

// loader/ldt_keeper.c
#define MSGTR_LOADER_DYLD_Warning "ПОПЕРЕДЖЕ�?�?Я: �?амагаю�?ь викори�?тати DLL кодеки але змінна �?ередовища\n         DYLD_BIND_AT_LAUNCH не в�?тановлена. Це здаєть�?�? поламка.\n"

// ====================== GUI messages/buttons ========================

// --- labels ---
#define MSGTR_About "Про"
#define MSGTR_FileSelect "Вибрати файл..."
#define MSGTR_SubtitleSelect "Вибрати �?убтитри..."
#define MSGTR_OtherSelect "Вибрати..."
#define MSGTR_AudioFileSelect "Вибрати іншу аудіо доріжку..."
#define MSGTR_FontSelect "Вибрати шрифт..."
// Note: If you change MSGTR_PlayList please see if it still fits MSGTR_MENU_PlayList
#define MSGTR_PlayList "Спи�?ок програванн�?"
#define MSGTR_Equalizer "�?квалайзер"
#define MSGTR_ConfigureEqualizer "�?алаштувати �?квалайзер"
#define MSGTR_SkinBrowser "Перегл�?дач �?кінів"
#define MSGTR_Network "Передача потоку..."
// Note: If you change MSGTR_Preferences please see if it still fits MSGTR_MENU_Preferences
#define MSGTR_Preferences "Шалаштуванн�?"
#define MSGTR_AudioPreferences "�?алаштуванн�? аудіо драйверу"
#define MSGTR_NoMediaOpened "�?о�?ій не відкритий."
#define MSGTR_NoChapter "Без розділу"
#define MSGTR_Chapter "Розділ %d"
#define MSGTR_NoFileLoaded "Файл не завантжено."

// --- buttons ---
#define MSGTR_Ok "OK"
#define MSGTR_Cancel "Відміна"
#define MSGTR_Add "Додати"
#define MSGTR_Remove "Видалити"
#define MSGTR_Clear "Очи�?тити"
#define MSGTR_Config "�?алаштунки"
#define MSGTR_ConfigDriver "�?алаштувати драйвер"
#define MSGTR_Browse "Дивити�?ь"

// --- error messages ---
#define MSGTR_NEMDB "Вибачте, не до�?татньо пам'�?ті дл�? прори�?овки буферу."
#define MSGTR_NEMFMR "Вибачте, не до�?татньо пам'�?ті дл�? рендеренн�? меню."
#define MSGTR_IDFGCVD "Вибачте, не знаходжу відео драйвер дл�? підтримки GUI."
#define MSGTR_NEEDLAVC "Вибачте, ви не можете програвати не MPEG файли з вашим DXR3/H+ при�?троєм без перекодуванн�?.\nВключіть lavc у нашалтунки DXR3/H+."

// --- skin loader error messages
#define MSGTR_SKIN_ERRORMESSAGE "[�?кін] помилка у налаштунках �?кіна у р�?дку %d: %s"
#define MSGTR_SKIN_SkinFileNotFound "[skin] файл ( %s ) не знайдено.\n"
#define MSGTR_SKIN_SkinFileNotReadable "[skin] файл ( %s ) не прочитати.\n"
#define MSGTR_SKIN_BITMAP_16bit  "Глибина матриці у 16 біт і менше не підтримуєть�?�? (%s).\n"
#define MSGTR_SKIN_BITMAP_FileNotFound  "Файл не знайдено (%s)\n"
#define MSGTR_SKIN_BITMAP_PNGReadError "Помилка читанн�? PNG (%s)\n"
#define MSGTR_SKIN_BITMAP_ConversionError "помилка конвертуванн�? з 24 до 32 біт (%s)\n"
#define MSGTR_SKIN_UnknownMessage "невідоме повідомленн�?: %s\n"
#define MSGTR_SKIN_NotEnoughMemory "недо�?татньо пам'�?ті\n"
#define MSGTR_SKIN_FONT_TooManyFontsDeclared "Вказано забагато шрифтів.\n"
#define MSGTR_SKIN_FONT_FontFileNotFound "Файл шрифту не знайдено.\n"
#define MSGTR_SKIN_FONT_FontImageNotFound "Font image file not found.\n"
#define MSGTR_SKIN_FONT_NonExistentFont "ідентифікатор шрифту не і�?нує (%s)\n"
#define MSGTR_SKIN_UnknownParameter "невідомий параметр (%s)\n"
#define MSGTR_SKIN_SKINCFG_SkinNotFound "Скін не знайдено (%s).\n"
#define MSGTR_SKIN_SKINCFG_SelectedSkinNotFound "Обраний �?кін ( %s ) не знайдено, обираю 'default'...\n"
#define MSGTR_SKIN_LABEL "Скіни:"

// --- GTK menus
#define MSGTR_MENU_AboutMPlayer "Про програму"
#define MSGTR_MENU_Open "Відкрити..."
#define MSGTR_MENU_PlayFile "Грати файл..."
#define MSGTR_MENU_PlayVCD "Грати VCD..."
#define MSGTR_MENU_PlayDVD "Грати DVD..."
#define MSGTR_MENU_PlayURL "Грати URL..."
#define MSGTR_MENU_LoadSubtitle "Завантажити �?убтитри..."
#define MSGTR_MENU_DropSubtitle "Викинути �?убтитри..."
#define MSGTR_MENU_LoadExternAudioFile "Завантажити зовнішній аудіо файл..."
#define MSGTR_MENU_Playing "Відтворенн�?"
#define MSGTR_MENU_Play "Грати"
#define MSGTR_MENU_Pause "Пауза"
#define MSGTR_MENU_Stop "Зупинити"
#define MSGTR_MENU_NextStream "�?а�?тупний потік"
#define MSGTR_MENU_PrevStream "Попередній потік"
#define MSGTR_MENU_Size "Розмір"
#define MSGTR_MENU_HalfSize   "Half size"
#define MSGTR_MENU_NormalSize "�?ормальний розмір"
#define MSGTR_MENU_DoubleSize "Подвійний розмір"
#define MSGTR_MENU_FullScreen "Повний екран"
#define MSGTR_MENU_DVD "DVD"
#define MSGTR_MENU_VCD "VCD"
#define MSGTR_MENU_PlayDisc "Грати ди�?к..."
#define MSGTR_MENU_ShowDVDMenu "Показати DVD меню"
#define MSGTR_MENU_Titles "Титри"
#define MSGTR_MENU_Title "Титр %2d"
#define MSGTR_MENU_None "(нема)"
#define MSGTR_MENU_Chapters "Розділи"
#define MSGTR_MENU_Chapter "Розділ %2d"
#define MSGTR_MENU_AudioLanguages "�?удіо мови"
#define MSGTR_MENU_SubtitleLanguages "Мови �?убтитрів"
#define MSGTR_MENU_PlayList MSGTR_PlayList
#define MSGTR_MENU_SkinBrowser "Перегл�?дач жупанів"
#define MSGTR_MENU_Preferences MSGTR_Preferences
#define MSGTR_MENU_Exit "Вихід"
#define MSGTR_MENU_Mute "Тиша"
#define MSGTR_MENU_Original "Вихідний"
#define MSGTR_MENU_AspectRatio "Відношенн�? �?торін"
#define MSGTR_MENU_AudioTrack "�?удіо доріжка"
#define MSGTR_MENU_Track "Доріжка %d"
#define MSGTR_MENU_VideoTrack "Відео доріжка"
#define MSGTR_MENU_Subtitles "Субтитри"

// --- equalizer
// Note: If you change MSGTR_EQU_Audio please see if it still fits MSGTR_PREFERENCES_Audio
#define MSGTR_EQU_Audio "�?удіо"
// Note: If you change MSGTR_EQU_Video please see if it still fits MSGTR_PREFERENCES_Video
#define MSGTR_EQU_Video "Відео"
#define MSGTR_EQU_Contrast "Контра�?т: "
#define MSGTR_EQU_Brightness "Я�?краві�?ть: "
#define MSGTR_EQU_Hue "Тон: "
#define MSGTR_EQU_Saturation "�?а�?ичені�?ть: "
#define MSGTR_EQU_Front_Left "Передній Лівий"
#define MSGTR_EQU_Front_Right "Передній Правий"
#define MSGTR_EQU_Back_Left "Задній Лівий"
#define MSGTR_EQU_Back_Right "Задній Правий"
#define MSGTR_EQU_Center "Центральний"
#define MSGTR_EQU_Bass "Ба�?"
#define MSGTR_EQU_All "У�?і"
#define MSGTR_EQU_Channel1 "Канал 1:"
#define MSGTR_EQU_Channel2 "Канал 2:"
#define MSGTR_EQU_Channel3 "Канал 3:"
#define MSGTR_EQU_Channel4 "Канал 4:"
#define MSGTR_EQU_Channel5 "Канал 5:"
#define MSGTR_EQU_Channel6 "Канал 6:"

// --- playlist
#define MSGTR_PLAYLIST_Path "Шл�?х"
#define MSGTR_PLAYLIST_Selected "Вибрані файли"
#define MSGTR_PLAYLIST_Files "Файли"
#define MSGTR_PLAYLIST_DirectoryTree "Дерево каталогу"

// --- preferences
#define MSGTR_PREFERENCES_Audio MSGTR_EQU_Audio
#define MSGTR_PREFERENCES_Video MSGTR_EQU_Video
#define MSGTR_PREFERENCES_SubtitleOSD "Субтитри й OSD"
#define MSGTR_PREFERENCES_Codecs "Кодеки й demuxer"
// Note: If you change MSGTR_PREFERENCES_Misc see if it still fits MSGTR_PREFERENCES_FRAME_Misc
#define MSGTR_PREFERENCES_Misc "Різне"
#define MSGTR_PREFERENCES_None "�?емає"
#define MSGTR_PREFERENCES_DriverDefault "звичайний драйвер"
#define MSGTR_PREFERENCES_AvailableDrivers "До�?тупні драйвери:"
#define MSGTR_PREFERENCES_DoNotPlaySound "�?е грати звук"
#define MSGTR_PREFERENCES_NormalizeSound "�?ормалізувати звук"
#define MSGTR_PREFERENCES_EnableEqualizer "Дозволити еквалайзер"
#define MSGTR_PREFERENCES_ExtraStereo "Дозволити додаткове �?терео"
#define MSGTR_PREFERENCES_Coefficient "Коефіціент:"
#define MSGTR_PREFERENCES_AudioDelay "Затримка аудіо"
#define MSGTR_PREFERENCES_DoubleBuffer "Дозволити подвійне буферуванн�?"
#define MSGTR_PREFERENCES_DirectRender "Дозволити пр�?мий вивід"
#define MSGTR_PREFERENCES_FrameDrop "Дозволити пропу�?к кадрів"
#define MSGTR_PREFERENCES_HFrameDrop "Дозволити викидуванн�? кадрів (небезпечно)"
#define MSGTR_PREFERENCES_Flip "Перегорнути зображенн�? догори ногами"
#define MSGTR_PREFERENCES_Panscan "Panscan: "
#define MSGTR_PREFERENCES_Subtitle "Субтитри:"
#define MSGTR_PREFERENCES_SUB_Delay "Затримка: "
#define MSGTR_PREFERENCES_SUB_FPS "к/c:"
#define MSGTR_PREFERENCES_SUB_POS "Положенн�?: "
#define MSGTR_PREFERENCES_SUB_AutoLoad "Заборонити автозавантаженн�? �?убтитрів"
#define MSGTR_PREFERENCES_SUB_Unicode "Unicode �?убтитри"
#define MSGTR_PREFERENCES_SUB_MPSUB "Перетворити вказані �?убтитри до формату MPlayer"
#define MSGTR_PREFERENCES_SUB_SRT "Перетворити вказані �?убтитри до формату SubViewer (SRT)"
#define MSGTR_PREFERENCES_SUB_Overlap "Дозволити/заборонити перекритт�? �?убтитрів"
#define MSGTR_PREFERENCES_SUB_USE_ASS "SSA/ASS вивід �?убтитрів"
#define MSGTR_PREFERENCES_SUB_ASS_USE_MARGINS "Викори�?товувати кордони"
#define MSGTR_PREFERENCES_SUB_ASS_TOP_MARGIN "Угорі: "
#define MSGTR_PREFERENCES_SUB_ASS_BOTTOM_MARGIN "Знизу: "
#define MSGTR_PREFERENCES_Font "Шрифт:"
#define MSGTR_PREFERENCES_FontFactor "Фактор шрифту:"
#define MSGTR_PREFERENCES_PostProcess "Дозволити postprocessing"
#define MSGTR_PREFERENCES_AutoQuality "�?вто �?кі�?ть: "
#define MSGTR_PREFERENCES_NI "Викори�?товувати неперемежений AVI пар�?ер"
#define MSGTR_PREFERENCES_IDX "Перебудувати індек�?, �?кщо треба"
#define MSGTR_PREFERENCES_VideoCodecFamily "Драйвер відео �?одеку:"
#define MSGTR_PREFERENCES_AudioCodecFamily "Драйвер аудіо кодеку:"
#define MSGTR_PREFERENCES_FRAME_OSD_Level "Рівень OSD"
#define MSGTR_PREFERENCES_FRAME_Subtitle "Субтитри"
#define MSGTR_PREFERENCES_FRAME_Font "Шрифт"
#define MSGTR_PREFERENCES_FRAME_PostProcess "Postprocessing"
#define MSGTR_PREFERENCES_FRAME_CodecDemuxer "Кодек й demuxer"
#define MSGTR_PREFERENCES_FRAME_Cache "Кеш"
#define MSGTR_PREFERENCES_FRAME_Misc MSGTR_PREFERENCES_Misc
#define MSGTR_PREFERENCES_Audio_Device "При�?трій:"
#define MSGTR_PREFERENCES_Audio_Mixer "Мікшер:"
#define MSGTR_PREFERENCES_Audio_MixerChannel "Канал мікшеру:"
#define MSGTR_PREFERENCES_Message "�?е забудьте, що вам треба перезапу�?тити програванн�? дл�? набутт�? чинно�?ті де�?ких параметрів!"
#define MSGTR_PREFERENCES_DXR3_VENC "Відео кодек:"
#define MSGTR_PREFERENCES_DXR3_LAVC "Викори�?товувати LAVC (FFmpeg)"
#define MSGTR_PREFERENCES_FontEncoding1 "Unicode"
#define MSGTR_PREFERENCES_FontEncoding2 "Western European Languages (ISO-8859-1)"
#define MSGTR_PREFERENCES_FontEncoding3 "Western European Languages with Euro (ISO-8859-15)"
#define MSGTR_PREFERENCES_FontEncoding4 "Slavic/Central European Languages (ISO-8859-2)"
#define MSGTR_PREFERENCES_FontEncoding5 "Esperanto, Galician, Maltese, Turkish (ISO-8859-3)"
#define MSGTR_PREFERENCES_FontEncoding6 "Old Baltic charset (ISO-8859-4)"
#define MSGTR_PREFERENCES_FontEncoding7 "Cyrillic (ISO-8859-5)"
#define MSGTR_PREFERENCES_FontEncoding8 "Arabic (ISO-8859-6)"
#define MSGTR_PREFERENCES_FontEncoding9 "Modern Greek (ISO-8859-7)"
#define MSGTR_PREFERENCES_FontEncoding10 "Turkish (ISO-8859-9)"
#define MSGTR_PREFERENCES_FontEncoding11 "Baltic (ISO-8859-13)"
#define MSGTR_PREFERENCES_FontEncoding12 "Celtic (ISO-8859-14)"
#define MSGTR_PREFERENCES_FontEncoding13 "Hebrew charsets (ISO-8859-8)"
#define MSGTR_PREFERENCES_FontEncoding14 "Russian (KOI8-R)"
#define MSGTR_PREFERENCES_FontEncoding15 "Ukrainian, Belarusian (KOI8-U/RU)"
#define MSGTR_PREFERENCES_FontEncoding16 "Simplified Chinese charset (CP936)"
#define MSGTR_PREFERENCES_FontEncoding17 "Traditional Chinese charset (BIG5)"
#define MSGTR_PREFERENCES_FontEncoding18 "Japanese charsets (SHIFT-JIS)"
#define MSGTR_PREFERENCES_FontEncoding19 "Korean charset (CP949)"
#define MSGTR_PREFERENCES_FontEncoding20 "Thai charset (CP874)"
#define MSGTR_PREFERENCES_FontEncoding21 "Cyrillic Windows (CP1251)"
#define MSGTR_PREFERENCES_FontEncoding22 "Slavic/Central European Windows (CP1250)"
#define MSGTR_PREFERENCES_FontEncoding23 "Arabic Windows (CP1256)"
#define MSGTR_PREFERENCES_FontNoAutoScale "Без автома�?штабуванн�?"
#define MSGTR_PREFERENCES_FontPropWidth "Пропорційно ширині кадру"
#define MSGTR_PREFERENCES_FontPropHeight "Пропорційно ви�?оті кадру"
#define MSGTR_PREFERENCES_FontPropDiagonal "Пропорційно діагоналі кадру"
#define MSGTR_PREFERENCES_FontEncoding "Кодуванн�?:"
#define MSGTR_PREFERENCES_FontBlur "Розпливанн�?:"
#define MSGTR_PREFERENCES_FontOutLine "Обведенн�?:"
#define MSGTR_PREFERENCES_FontTextScale "Ма�?штаб тек�?ту:"
#define MSGTR_PREFERENCES_FontOSDScale "Ма�?штаб OSD:"
#define MSGTR_PREFERENCES_Cache "Кеш on/off"
#define MSGTR_PREFERENCES_CacheSize "Розмір кешу: "
#define MSGTR_PREFERENCES_LoadFullscreen "Стартувати в полний екран"
#define MSGTR_PREFERENCES_SaveWinPos "Зберігати положенн�? вікна"
#define MSGTR_PREFERENCES_XSCREENSAVER "Зупинити XScreenSaver"
#define MSGTR_PREFERENCES_PlayBar "Дозволити лінійку програванн�?"
#define MSGTR_PREFERENCES_AutoSync "AutoSync ув/вимк"
#define MSGTR_PREFERENCES_AutoSyncValue "Autosync: "
#define MSGTR_PREFERENCES_CDROMDevice "CD-ROM при�?трій:"
#define MSGTR_PREFERENCES_DVDDevice "DVD при�?трій:"
#define MSGTR_PREFERENCES_FPS "Кадрів на �?екунду:"
#define MSGTR_PREFERENCES_ShowVideoWindow "Показувати неактивне вікно зображенн�?"
#define MSGTR_PREFERENCES_ArtsBroken "�?овіші вер�?ії aRts не �?умі�?ні"\
           "з GTK 1.x та �?причин�?ть помилку GMPlayer!"

// -- aboutbox
#define MSGTR_ABOUT_UHU "GUI розробку �?пон�?овано UHU Linux\n"
#define MSGTR_ABOUT_Contributors "Розробники коду та документації\n"
#define MSGTR_ABOUT_Codecs_libs_contributions "Кодеки та �?торонні бібліотеки\n"
#define MSGTR_ABOUT_Translations "Переклади\n"
#define MSGTR_ABOUT_Skins "Жупани\n"

// --- messagebox
#define MSGTR_MSGBOX_LABEL_FatalError "Фатальна помилка!"
#define MSGTR_MSGBOX_LABEL_Error "Помилка!"
#define MSGTR_MSGBOX_LABEL_Warning "Попередженн�?!"

// cfg.c
#define MSGTR_UnableToSaveOption "[cfg] �?е можу зберегти '%s' опцію.\n"

// interface.c
#define MSGTR_DeletingSubtitles "[GUI] Видал�?ю �?убтитри.\n"
#define MSGTR_LoadingSubtitles "[GUI] Вантажу �?убтитри: %s\n"
#define MSGTR_AddingVideoFilter "[GUI] Додаю відео фільтр: %s\n"

// mw.c
#define MSGTR_NotAFile "Здаєть�?�?, це не файл: %s !\n"

// ws.c
#define MSGTR_WS_RemoteDisplay "[ws] Віддалений ди�?плей, вимикаю XMITSHM.\n"
#define MSGTR_WS_NoXshm "[ws] Вибачте, ваша �?и�?тема не підтримує розширенн�? загальної пам'�?ті X.\n"
#define MSGTR_WS_NoXshape "[ws] Вибачте, здаєть�?�?, ваша �?и�?тема не підтримує розширенн�? XShape.\n"
#define MSGTR_WS_ColorDepthTooLow "[ws] Вибачте, глибина кольору занизька.\n"
#define MSGTR_WS_TooManyOpenWindows "[ws] Забагато відкритих вікон.\n"
#define MSGTR_WS_ShmError "[ws] помилка розширенн�? загальної пам'�?ті\n"
#define MSGTR_WS_NotEnoughMemoryDrawBuffer "[ws] Вибачте, не до�?татньо пам'�?ті дл�? прори�?уванн�? буферу.\n"
#define MSGTR_WS_DpmsUnavailable "DPMS не до�?тупний?\n"
#define MSGTR_WS_DpmsNotEnabled "�?е можу увімкнути DPMS.\n"

// wsxdnd.c
#define MSGTR_WS_NotAFile "Здаєть�?�?, це не файл...\n"
#define MSGTR_WS_DDNothing "D&D: �?ічого не повернено!\n"

// ======================= video output drivers ========================

#define MSGTR_VOincompCodec "Обраний при�?трій виводу відео не�?умі�?ний з цим кодеком.\n"\
                "Спробуйте додати фільтр scale до �?пи�?ку вашого �?пи�?ку фільтрів,\n"\
                "наприклад. -vf spp,scale замі�?ть -vf spp.\n"
#define MSGTR_VO_GenericError "Виникла �?лідуюча помилка"
#define MSGTR_VO_UnableToAccess "�?еможлово отримати до�?туп"
#define MSGTR_VO_ExistsButNoDirectory "вже і�?нує, але це не директорі�?."
#define MSGTR_VO_DirExistsButNotWritable "Директорі�? виводу вже і�?нує, але не до�?тупна дл�? запи�?у."
#define MSGTR_VO_DirExistsAndIsWritable "Директорі�? виводу вже і�?нує та до�?тупна дл�? запи�?у."
#define MSGTR_VO_CantCreateDirectory "�?е можу �?творити директорію виводу."
#define MSGTR_VO_CantCreateFile "�?е можу �?творити файл виводу."
#define MSGTR_VO_DirectoryCreateSuccess "Директорі�? виводу у�?пішно �?творена."
#define MSGTR_VO_ValueOutOfRange "значенн�? за межами до�?тупного діапазону"
#define MSGTR_VO_NoValueSpecified "�?е вказано значенн�?."
#define MSGTR_VO_UnknownSuboptions "невідома(і) �?убопці�?(ї)"

// aspect.c
#define MSGTR_LIBVO_ASPECT_NoSuitableNewResFound "[ASPECT] Попередженн�?: �?е знайдено потрібного розширенн�?!\n"
#define MSGTR_LIBVO_ASPECT_NoNewSizeFoundThatFitsIntoRes "[ASPECT] Помилка: �?е знайдено розмір, що помі�?тив�?�? б у дане розширенн�?!\n"

// font_load_ft.c
#define MSGTR_LIBVO_FONT_LOAD_FT_NewFaceFailed "Помилка New_Face. Можливо шл�?х до шрифту невірний.\nВкажіть файл шрифту (~/.mplayer/subfont.ttf).\n"
#define MSGTR_LIBVO_FONT_LOAD_FT_NewMemoryFaceFailed "Помилка New_Memory_Face..\n"
#define MSGTR_LIBVO_FONT_LOAD_FT_SubFaceFailed "шрифт �?убтитрів: помилка load_sub_face.\n"
#define MSGTR_LIBVO_FONT_LOAD_FT_SubFontCharsetFailed "шрифт �?убтитрів: помилка prepare_charset.\n"
#define MSGTR_LIBVO_FONT_LOAD_FT_CannotPrepareSubtitleFont "�?е можу підготувати шрифт �?убтитрів.\n"
#define MSGTR_LIBVO_FONT_LOAD_FT_CannotPrepareOSDFont "�?е можу підготувати шрифт OSD.\n"
#define MSGTR_LIBVO_FONT_LOAD_FT_CannotGenerateTables "�?е можу генерувати таблиці.\n"
#define MSGTR_LIBVO_FONT_LOAD_FT_DoneFreeTypeFailed "Помилка FT_Done_FreeType.\n"

// sub.c
#define MSGTR_VO_SUB_Seekbar "�?авігаці�?"
#define MSGTR_VO_SUB_Play "Грати"
#define MSGTR_VO_SUB_Pause "Пауза"
#define MSGTR_VO_SUB_Stop "Стоп"
#define MSGTR_VO_SUB_Rewind "�?азад"
#define MSGTR_VO_SUB_Forward "Уперед"
#define MSGTR_VO_SUB_Clock "Ча�?"
#define MSGTR_VO_SUB_Contrast "Контра�?т"
#define MSGTR_VO_SUB_Saturation "�?а�?ичені�?ть"
#define MSGTR_VO_SUB_Volume "Гучні�?ть"
#define MSGTR_VO_SUB_Brightness "Бли�?кучі�?ть"
#define MSGTR_VO_SUB_Hue "Колір"
#define MSGTR_VO_SUB_Balance "Балан�?"

// vo_3dfx.c
#define MSGTR_LIBVO_3DFX_Only16BppSupported "[VO_3DFX] Підтримуєть�?�? тільки 16bpp!"
#define MSGTR_LIBVO_3DFX_VisualIdIs "[VO_3DFX] Візуальний ID  %lx.\n"
#define MSGTR_LIBVO_3DFX_UnableToOpenDevice "[VO_3DFX] �?е можу відкрити /dev/3dfx.\n"
#define MSGTR_LIBVO_3DFX_Error "[VO_3DFX] Помилка: %d.\n"
#define MSGTR_LIBVO_3DFX_CouldntMapMemoryArea "[VO_3DFX] �?е можу показати обла�?ті пам'�?ті 3dfx: %p,%p,%d.\n"
#define MSGTR_LIBVO_3DFX_DisplayInitialized "[VO_3DFX] Ініціалізовано: %p.\n"
#define MSGTR_LIBVO_3DFX_UnknownSubdevice "[VO_3DFX] �?евідомий підпри�?трій: %s.\n"

// vo_aa.c
#define MSGTR_VO_AA_HelpHeader "\n\nСубопції vo_aa бібліотеки aalib:\n"
#define MSGTR_VO_AA_AdditionalOptions "Додаткові опції, що забезпечує vo_aa:\n" \
"  help        показати це повідомленн�?\n" \
"  osdcolor    в�?тановити колір OSD\n  subcolor    в�?тановити колір �?убтитрівr\n" \
"        параметри кольору:\n           0 : �?тандартний\n" \
"           1 : дим\n           2 : тов�?тий\n           3 : тов�?тий шрифт\n" \
"           4 : ревер�?\n           5 : �?пеці�?льний\n\n\n"

// vo_dxr3.c
#define MSGTR_LIBVO_DXR3_UnableToLoadNewSPUPalette "[VO_DXR3] �?е можу завантажити нову палітру SPU!\n"
#define MSGTR_LIBVO_DXR3_UnableToSetPlaymode "[VO_DXR3] �?е можу в�?тановити режим програванн�?!\n"
#define MSGTR_LIBVO_DXR3_UnableToSetSubpictureMode "[VO_DXR3] �?е можу в�?тановити режим �?убкартинки!\n"
#define MSGTR_LIBVO_DXR3_UnableToGetTVNorm "[VO_DXR3] �?е можу отримати режим ТБ!\n"
#define MSGTR_LIBVO_DXR3_AutoSelectedTVNormByFrameRate "[VO_DXR3] �?вто-вибір режиму ТБ за ча�?тотою кадрів: "
#define MSGTR_LIBVO_DXR3_UnableToSetTVNorm "[VO_DXR3] �?е можу отримати режим ТБ!\n"
#define MSGTR_LIBVO_DXR3_SettingUpForNTSC "[VO_DXR3] В�?тановлюю дл�? NTSC.\n"
#define MSGTR_LIBVO_DXR3_SettingUpForPALSECAM "[VO_DXR3] В�?тановлюю дл�? PAL/SECAM.\n"
#define MSGTR_LIBVO_DXR3_SettingAspectRatioTo43 "[VO_DXR3] В�?тановлюю пропорції 4:3.\n"
#define MSGTR_LIBVO_DXR3_SettingAspectRatioTo169 "[VO_DXR3] В�?тановлюю пропорції 16:9.\n"
#define MSGTR_LIBVO_DXR3_OutOfMemory "[VO_DXR3] не ви�?тачає пам'�?ті\n"
#define MSGTR_LIBVO_DXR3_UnableToAllocateKeycolor "[VO_DXR3] �?е можу знайти головний колір!\n"
#define MSGTR_LIBVO_DXR3_UnableToAllocateExactKeycolor "[VO_DXR3] �?е можу знайти точний головний колір, викори�?товую найбільш �?хоже (0x%lx).\n"
#define MSGTR_LIBVO_DXR3_Uninitializing "[VO_DXR3] Ініці�?лізуванн�?.\n"
#define MSGTR_LIBVO_DXR3_FailedRestoringTVNorm "[VO_DXR3] �?е можу в�?тановити режим ТБ!\n"
#define MSGTR_LIBVO_DXR3_EnablingPrebuffering "[VO_DXR3] Дозвол�?ю попередню буферизацію.\n"
#define MSGTR_LIBVO_DXR3_UsingNewSyncEngine "[VO_DXR3] Викори�?товую новий механізм �?инхронізації.\n"
#define MSGTR_LIBVO_DXR3_UsingOverlay "[VO_DXR3] Викори�?товую оверлей.\n"
#define MSGTR_LIBVO_DXR3_ErrorYouNeedToCompileMplayerWithX11 "[VO_DXR3] Помилка: Оверлей потребує збиранн�? з в�?тановленими бібліотеками/допоміжними файлами X11.\n"
#define MSGTR_LIBVO_DXR3_WillSetTVNormTo "[VO_DXR3] В�?тановлюю режим ТБ у: "
#define MSGTR_LIBVO_DXR3_AutoAdjustToMovieFrameRatePALPAL60 "авто-регулюванн�? за ча�?тотою кадрів фільма (PAL/PAL-60)"
#define MSGTR_LIBVO_DXR3_AutoAdjustToMovieFrameRatePALNTSC "авто-регулюванн�? за ча�?тотою кадрів фільма (PAL/NTSC)"
#define MSGTR_LIBVO_DXR3_UseCurrentNorm "Викори�?товую поточний режим."
#define MSGTR_LIBVO_DXR3_UseUnknownNormSuppliedCurrentNorm "Запропонований невідомий режим. Спробуйте поточний."
#define MSGTR_LIBVO_DXR3_ErrorOpeningForWritingTrying "[VO_DXR3] Помилка при відкритті %s дл�? запи�?у, пробую /dev/em8300 замі�?ть.\n"
#define MSGTR_LIBVO_DXR3_ErrorOpeningForWritingTryingMV "[VO_DXR3] Помилка при відкритті %s дл�? запи�?у, пробую /dev/em8300_mv замі�?ть.\n"
#define MSGTR_LIBVO_DXR3_ErrorOpeningForWritingAsWell "[VO_DXR3] Також помилка при відкритті /dev/em8300 дл�? запи�?у!\nВиходжу.\n"
#define MSGTR_LIBVO_DXR3_ErrorOpeningForWritingAsWellMV "[VO_DXR3] Також помилка при відкритті /dev/em8300_дл�? запи�?у!\nВиходжу.\n"
#define MSGTR_LIBVO_DXR3_Opened "[VO_DXR3] Відкрито: %s.\n"
#define MSGTR_LIBVO_DXR3_ErrorOpeningForWritingTryingSP "[VO_DXR3] Помилка при відкритті %s дл�? запи�?у, пробую /dev/em8300_sp замі�?ть.\n"
#define MSGTR_LIBVO_DXR3_ErrorOpeningForWritingAsWellSP "[VO_DXR3] Також помилка при відкритті /dev/em8300_sp дл�? запи�?у!\nВиходжу.\n"
#define MSGTR_LIBVO_DXR3_UnableToOpenDisplayDuringHackSetup "[VO_DXR3] �?е можу відкрити ди�?плей у ча�? в�?тановленн�? хаку оверле�?!\n"
#define MSGTR_LIBVO_DXR3_UnableToInitX11 "[VO_DXR3] �?е можу ініці�?лізувати X11!\n"
#define MSGTR_LIBVO_DXR3_FailedSettingOverlayAttribute "[VO_DXR3] �?евдало�?ь в�?тановити атрибут оверле�?.\n"
#define MSGTR_LIBVO_DXR3_FailedSettingOverlayScreen "[VO_DXR3] �?евдало�?ь в�?тановити екран оверлею!\nВиходжу.\n"
#define MSGTR_LIBVO_DXR3_FailedEnablingOverlay "[VO_DXR3] �?еадало�?ь увімкнути оверлей!\nВиходжу.\n"
#define MSGTR_LIBVO_DXR3_FailedResizingOverlayWindow "[VO_DXR3] �?евдало�?ь зімнити розмір вікна оверлею!\n"
#define MSGTR_LIBVO_DXR3_FailedSettingOverlayBcs "[VO_DXR3] �?евдало�?�? в�?тановити bcs оверлею!\n"
#define MSGTR_LIBVO_DXR3_FailedGettingOverlayYOffsetValues "[VO_DXR3] �?е можу отримати значенн�? Y-зміщенн�? оверлею!\nВиходжу.\n"
#define MSGTR_LIBVO_DXR3_FailedGettingOverlayXOffsetValues "[VO_DXR3] �?е можу отримати значенн�? X-зміщенн�? оверлею!\nВиходжу.\n"
#define MSGTR_LIBVO_DXR3_FailedGettingOverlayXScaleCorrection "[VO_DXR3] �?е можу отримати корекцію оверле�? ма�?штабуванн�? X!\nВиходжу.\n"
#define MSGTR_LIBVO_DXR3_YOffset "[VO_DXR3] Зміщенн�? за Y: %d.\n"
#define MSGTR_LIBVO_DXR3_XOffset "[VO_DXR3] Зміщенн�? за X: %d.\n"
#define MSGTR_LIBVO_DXR3_XCorrection "[VO_DXR3] Корекці�? за X: %d.\n"
#define MSGTR_LIBVO_DXR3_FailedSetSignalMix "[VO_DXR3] �?е можу в�?тановити �?игнал mix!\n"

// vo_jpeg.c
#define MSGTR_VO_JPEG_ProgressiveJPEG "Увімкнено прогре�?ивний JPEG."
#define MSGTR_VO_JPEG_NoProgressiveJPEG "Ввимкнено прогре�?ивний JPEG."
#define MSGTR_VO_JPEG_BaselineJPEG "Увімкнено о�?новний JPEG."
#define MSGTR_VO_JPEG_NoBaselineJPEG "Ввимкнено о�?новний JPEG."

// vo_mga.c
#define MSGTR_LIBVO_MGA_AspectResized "[VO_MGA] aspect(): розмір змінений до %dx%d.\n"
#define MSGTR_LIBVO_MGA_Uninit "[VO] деініці�?лізаці�?!\n"

// mga_template.c
#define MSGTR_LIBVO_MGA_ErrorInConfigIoctl "[MGA] помилка у mga_vid_config ioctl (неправильна вер�?і�? mga_vid.o?)"
#define MSGTR_LIBVO_MGA_CouldNotGetLumaValuesFromTheKernelModule "[MGA] �?е можу отримати значенн�? luma з модул�? �?дра!\n"
#define MSGTR_LIBVO_MGA_CouldNotSetLumaValuesFromTheKernelModule "[MGA] �?е можу в�?тановити значенн�? luma з модул�? �?дра!\n"
#define MSGTR_LIBVO_MGA_ScreenWidthHeightUnknown "[MGA] �?евідома ширина/ви�?ота екрану!\n"
#define MSGTR_LIBVO_MGA_InvalidOutputFormat "[MGA] невірний вихідний формат %0X\n"
#define MSGTR_LIBVO_MGA_IncompatibleDriverVersion "[MGA] Вер�?і�? вашого mga_vid драйверу не�?умі�?на із цією вер�?ією MPlayer!\n"
#define MSGTR_LIBVO_MGA_CouldntOpen "[MGA] �?е можу відкрити: %s\n"
#define MSGTR_LIBVO_MGA_ResolutionTooHigh "[MGA] Розширенн�? джерела, у крайньому випадку в одному вимірі, більше ніж 1023x1023.\n[MGA] Перема�?штабуйте програмно або викор�?тайте -lavdopts lowres=1.\n"
#define MSGTR_LIBVO_MGA_mgavidVersionMismatch "[MGA] вер�?і�? драйверу mga_vid �?дра (%u) та MPlayer (%u) не �?півпадають\n"

// vo_null.c
#define MSGTR_LIBVO_NULL_UnknownSubdevice "[VO_NULL] �?евідомий підпри�?трій: %s.\n"

// vo_png.c
#define MSGTR_LIBVO_PNG_Warning1 "[VO_PNG] Попередженн�?: рівень �?ти�?ненн�? в�?тановлено 0, �?ти�?ненн�? вимкнено!\n"
#define MSGTR_LIBVO_PNG_Warning2 "[VO_PNG] Інфо: Викори�?тайте -vo png:z=<n> щоб в�?тановити рівень �?ти�?ненн�? з 0 до 9.\n"
#define MSGTR_LIBVO_PNG_Warning3 "[VO_PNG] Інфо: (0 = без �?ти�?ненн�?, 1 = найшвидша, найповільніша - 9 але найкраще �?ти�?ненн�?)\n"
#define MSGTR_LIBVO_PNG_ErrorOpeningForWriting "\n[VO_PNG] Помилка при відкритті '%s' дл�? запи�?у!\n"
#define MSGTR_LIBVO_PNG_ErrorInCreatePng "[VO_PNG] Помилка при �?творенні png.\n"

// vo_pnm.c
#define MSGTR_VO_PNM_ASCIIMode "Режим ASCII увімкнено."
#define MSGTR_VO_PNM_RawMode "Режим Raw увімкнено."
#define MSGTR_VO_PNM_PPMType "Запише файли PPM."
#define MSGTR_VO_PNM_PGMType "Запише файли PGM."
#define MSGTR_VO_PNM_PGMYUVType "Запише файли PGMYUV."

// vo_sdl.c
#define MSGTR_LIBVO_SDL_CouldntGetAnyAcceptableSDLModeForOutput "[VO_SDL] �?е можу виве�?ти прийн�?тний SDL Mode.\n"
#define MSGTR_LIBVO_SDL_SetVideoModeFailed "[VO_SDL] set_video_mode: Збій SDL_SetVideoMode: %s.\n"
#define MSGTR_LIBVO_SDL_MappingI420ToIYUV "[VO_SDL] Показую I420 у IYUV.\n"
#define MSGTR_LIBVO_SDL_UnsupportedImageFormat "[VO_SDL] Формат картинки не підтримуєть�?�? (0x%X).\n"
#define MSGTR_LIBVO_SDL_InfoPleaseUseVmOrZoom "[VO_SDL] Інфо - �?пробуйте -vm чи -zoom щоб перейти до найкращої роздільної здатно�?ті.\n"
#define MSGTR_LIBVO_SDL_FailedToSetVideoMode "[VO_SDL] �?е вдало�?�? в�?тановити відео режим: %s.\n"
#define MSGTR_LIBVO_SDL_CouldntCreateAYUVOverlay "[VO_SDL] �?е вдало�?�? �?творити оверлей YUV: %s.\n"
#define MSGTR_LIBVO_SDL_CouldntCreateARGBSurface "[VO_SDL] �?е вдало�?�? �?творити поверхню RGB: %s.\n"
#define MSGTR_LIBVO_SDL_UsingDepthColorspaceConversion "[VO_SDL] Викори�?товую перетворенн�? глибини/кольорового про�?тору, це уповільнить протіканн�? проце�?у (%ibpp -> %ibpp).\n"
#define MSGTR_LIBVO_SDL_UnsupportedImageFormatInDrawslice "[VO_SDL] Формат картинки не підтримуєть�?�? у draw_slice, повідомте розробників MPlayer!\n"
#define MSGTR_LIBVO_SDL_BlitFailed "[VO_SDL] Збій Blit: %s.\n"
#define MSGTR_LIBVO_SDL_InitializationFailed "[VO_SDL] Ініці�?лізаці�? SDL не вдала�?�?: %s.\n"
#define MSGTR_LIBVO_SDL_UsingDriver "[VO_SDL] Викори�?товую драйвер: %s.\n"

// vo_svga.c
#define MSGTR_LIBVO_SVGA_ForcedVidmodeNotAvailable "[VO_SVGA] Фор�?ований vid_mode %d (%s) не до�?тупний.\n"
#define MSGTR_LIBVO_SVGA_ForcedVidmodeTooSmall "[VO_SVGA] Фор�?ований vid_mode %d (%s) замалий.\n"
#define MSGTR_LIBVO_SVGA_Vidmode "[VO_SVGA] Vid_mode: %d, %dx%d %dbpp.\n"
#define MSGTR_LIBVO_SVGA_VgasetmodeFailed "[VO_SVGA] Збій Vga_setmode(%d).\n"
#define MSGTR_LIBVO_SVGA_VideoModeIsLinearAndMemcpyCouldBeUsed "[VO_SVGA] Режим Відео лінійний та дл�? передачі зображенн�? може бути\nвикори�?таний memcpy.\n"
#define MSGTR_LIBVO_SVGA_VideoModeHasHardwareAcceleration "[VO_SVGA] Режим Відео має апаратне при�?коренн�? тому може бути кокори�?таний put_image.\n"
#define MSGTR_LIBVO_SVGA_IfItWorksForYouIWouldLikeToKnow "[VO_SVGA] Якщо це працює у ва�?, дайте мені знати.\n[VO_SVGA] (надішліть лоґ з `mplayer test.avi -v -v -v -v &> svga.log`). Д�?кую!\n"
#define MSGTR_LIBVO_SVGA_VideoModeHas "[VO_SVGA] Режим Відео має %d �?торінку(ок/ки).\n"
#define MSGTR_LIBVO_SVGA_CenteringImageStartAt "[VO_SVGA] Centering image. Starting at (%d,%d)\n"
#define MSGTR_LIBVO_SVGA_UsingVidix "[VO_SVGA] Викори�?товую VIDIX. w=%i h=%i  mw=%i mh=%i\n"

// vo_tdfx_vid.c
#define MSGTR_LIBVO_TDFXVID_Move "[VO_TDXVID] Переміщую %d(%d) x %d => %d.\n"
#define MSGTR_LIBVO_TDFXVID_AGPMoveFailedToClearTheScreen "[VO_TDFXVID] Переміщенн�? AGP не вдало�?�? очи�?тити екран.\n"
#define MSGTR_LIBVO_TDFXVID_BlitFailed "[VO_TDFXVID] Збій Blit.\n"
#define MSGTR_LIBVO_TDFXVID_NonNativeOverlayFormatNeedConversion "[VO_TDFXVID] �?е�?тандартний формат оверлею потребує перетворенн�?.\n"
#define MSGTR_LIBVO_TDFXVID_UnsupportedInputFormat "[VO_TDFXVID] Формат вводу 0x%x не підтримуєть�?�?.\n"
#define MSGTR_LIBVO_TDFXVID_OverlaySetupFailed "[VO_TDFXVID] В�?тановленн�? оверлею не вдало�?�?.\n"
#define MSGTR_LIBVO_TDFXVID_OverlayOnFailed "[VO_TDFXVID] Помилка оверлею.\n"
#define MSGTR_LIBVO_TDFXVID_OverlayReady "[VO_TDFXVID] Оверлей готовий: %d(%d) x %d @ %d => %d(%d) x %d @ %d.\n"
#define MSGTR_LIBVO_TDFXVID_TextureBlitReady "[VO_TDFXVID] Blit тек�?тур готовий: %d(%d) x %d @ %d => %d(%d) x %d @ %d.\n"
#define MSGTR_LIBVO_TDFXVID_OverlayOffFailed "[VO_TDFXVID] Ввимкненн�? оверлею не вдоло�?�?\n"
#define MSGTR_LIBVO_TDFXVID_CantOpen "[VO_TDFXVID] �?е можу відкрити %s: %s.\n"
#define MSGTR_LIBVO_TDFXVID_CantGetCurrentCfg "[VO_TDFXVID] �?е можу отримати поточну конфігурацію: %s.\n"
#define MSGTR_LIBVO_TDFXVID_MemmapFailed "[VO_TDFXVID] Збій memmap !!!!!\n"
#define MSGTR_LIBVO_TDFXVID_GetImageTodo "Треба доробити картинку.\n"
#define MSGTR_LIBVO_TDFXVID_AgpMoveFailed "[VO_TDFXVID] Переміщенн�? AGP не вдало�?�?.\n"
#define MSGTR_LIBVO_TDFXVID_SetYuvFailed "[VO_TDFXVID] В�?тановленн�? YUV не вдало�?�?.\n"
#define MSGTR_LIBVO_TDFXVID_AgpMoveFailedOnYPlane "[VO_TDFXVID] Переміщенн�? AGP не вдало�?�? на проєкції Y.\n"
#define MSGTR_LIBVO_TDFXVID_AgpMoveFailedOnUPlane "[VO_TDFXVID] Переміщенн�? AGP не вдало�?�? на проєкції U.\n"
#define MSGTR_LIBVO_TDFXVID_AgpMoveFailedOnVPlane "[VO_TDFXVID] Переміщенн�? AGP не вдало�?�? на проєкції V.\n"
#define MSGTR_LIBVO_TDFXVID_UnknownFormat "[VO_TDFXVID] невідомий формат: 0x%x.\n"

// vo_tdfxfb.c
#define MSGTR_LIBVO_TDFXFB_CantOpen "[VO_TDFXFB] �?е можу відкрити %s: %s.\n"
#define MSGTR_LIBVO_TDFXFB_ProblemWithFbitgetFscreenInfo "[VO_TDFXFB] Проблема з FBITGET_FSCREENINFO ioctl: %s.\n"
#define MSGTR_LIBVO_TDFXFB_ProblemWithFbitgetVscreenInfo "[VO_TDFXFB] Проблема з FBITGET_VSCREENINFO ioctl: %s.\n"
#define MSGTR_LIBVO_TDFXFB_ThisDriverOnlySupports "[VO_TDFXFB] Це драйвер підтримує тільки 3Dfx Banshee, Voodoo3 та Voodoo 5.\n"
#define MSGTR_LIBVO_TDFXFB_OutputIsNotSupported "[VO_TDFXFB] %d вивід bpp не підтримуєть�?�?.\n"
#define MSGTR_LIBVO_TDFXFB_CouldntMapMemoryAreas "[VO_TDFXFB] �?е можу показати ча�?тини пам'�?ті: %s.\n"
#define MSGTR_LIBVO_TDFXFB_BppOutputIsNotSupported "[VO_TDFXFB] %d вивід bpp не підтримуєть�?�? (Цього зов�?ім не повинно трапл�?ти�?�?).\n"
#define MSGTR_LIBVO_TDFXFB_SomethingIsWrongWithControl "[VO_TDFXFB] Ой! Що�?ь �?тало�?�? з control().\n"
#define MSGTR_LIBVO_TDFXFB_NotEnoughVideoMemoryToPlay "[VO_TDFXFB] �?е до�?татньо пам'�?ті щоб програти це відео. Спробуйте меншу роздільну здатні�?ть.\n"
#define MSGTR_LIBVO_TDFXFB_ScreenIs "[VO_TDFXFB] Екран %dx%d на %d bpp, у %dx%d на %d bpp, режим %dx%d.\n"

// vo_tga.c
#define MSGTR_LIBVO_TGA_UnknownSubdevice "[VO_TGA] �?евідомий підпри�?трій: %s.\n"

// vo_vesa.c
#define MSGTR_LIBVO_VESA_FatalErrorOccurred "[VO_VESA] Виникла фатально помилка! �?е можу продовжити.\n"
#define MSGTR_LIBVO_VESA_UnknownSubdevice "[VO_VESA] невідомий підпри�?трій: '%s'.\n"
#define MSGTR_LIBVO_VESA_YouHaveTooLittleVideoMemory "[VO_VESA] Замало відео пам'�?ті дл�? цього режиму:\n[VO_VESA] Потребує: %08lX є: %08lX.\n"
#define MSGTR_LIBVO_VESA_YouHaveToSpecifyTheCapabilitiesOfTheMonitor "[VO_VESA] Ви маєте вказати можливо�?ті монітора. �?е змінюю ча�?тому оновленн�?.\n"
#define MSGTR_LIBVO_VESA_UnableToFitTheMode "[VO_VESA] Режим не відповідає обмеженн�?м монітора. �?е змінюю ча�?тому оновленн�?.\n"
#define MSGTR_LIBVO_VESA_DetectedInternalFatalError "[VO_VESA] Ви�?влена фатальна внутрішн�? помилка: init викликаний перед preinit.\n"
#define MSGTR_LIBVO_VESA_SwitchFlipIsNotSupported "[VO_VESA] Опці�? -flip не підтримуєть�?�?.\n"
#define MSGTR_LIBVO_VESA_PossibleReasonNoVbe2BiosFound "[VO_VESA] Можлива причина: �?е знайдено VBE2 BIOS.\n"
#define MSGTR_LIBVO_VESA_FoundVesaVbeBiosVersion "[VO_VESA] Знайдено VESA VBE BIOS, вер�?і�? %x.%x ревізі�?: %x.\n"
#define MSGTR_LIBVO_VESA_VideoMemory "[VO_VESA] Відео пам'�?ть: %u Кб.\n"
#define MSGTR_LIBVO_VESA_Capabilites "[VO_VESA] Можливо�?ті VESA: %s %s %s %s %s.\n"
#define MSGTR_LIBVO_VESA_BelowWillBePrintedOemInfo "[VO_VESA] !!! OEM інформаці�? буде виведена нище !!!\n"
#define MSGTR_LIBVO_VESA_YouShouldSee5OemRelatedLines "[VO_VESA] Ви повинні бачити 5 р�?дків інфо про OEM нище; Якщо ні, у ва�? поламана vm86.\n"
#define MSGTR_LIBVO_VESA_OemInfo "[VO_VESA] OEM інфо: %s.\n"
#define MSGTR_LIBVO_VESA_OemRevision "[VO_VESA] OEM ревізі�?: %x.\n"
#define MSGTR_LIBVO_VESA_OemVendor "[VO_VESA] OEM по�?тачальник: %s.\n"
#define MSGTR_LIBVO_VESA_OemProductName "[VO_VESA] �?азва продукту OEM: %s.\n"
#define MSGTR_LIBVO_VESA_OemProductRev "[VO_VESA] Ревізі�? продукту OEM: %s.\n"
#define MSGTR_LIBVO_VESA_Hint "[VO_VESA] Підказка: Дл�? роботи ТБ-виходу вам необхідно підключити ТБ роз'єм\n"\
"[VO_VESA] перед завантаженн�?м �?к VESA BIOS ініці�?лізує �?ебе прот�?гом POST.\n"
#define MSGTR_LIBVO_VESA_UsingVesaMode "[VO_VESA] Викори�?товую режим VESA (%u) = %x [%ux%u@%u]\n"
#define MSGTR_LIBVO_VESA_CantInitializeSwscaler "[VO_VESA] �?е можу ініці�?лізувати програмне ма�?штабуванн�?.\n"
#define MSGTR_LIBVO_VESA_CantUseDga "[VO_VESA] �?е можу викори�?товувати DGA. Фор�?ую режим комутації �?егментів. :(\n"
#define MSGTR_LIBVO_VESA_UsingDga "[VO_VESA] Викори�?товую DGA (фізичні ре�?ур�?и: %08lXh, %08lXh)"
#define MSGTR_LIBVO_VESA_CantUseDoubleBuffering "[VO_VESA] �?е можу викори�?тати подвійну буферизацію: не до�?татньо відео пам'�?ті.\n"
#define MSGTR_LIBVO_VESA_CantFindNeitherDga "[VO_VESA] �?е можу знайти ні DGA ні переміщуваного фрейму вікна.\n"
#define MSGTR_LIBVO_VESA_YouveForcedDga "[VO_VESA] Ви фор�?ували DGA. Виходжу\n"
#define MSGTR_LIBVO_VESA_CantFindValidWindowAddress "[VO_VESA] �?е можу знайти правильну адре�?у вікна.\n"
#define MSGTR_LIBVO_VESA_UsingBankSwitchingMode "[VO_VESA] Викори�?товую режим комутації �?егментів (фізичні ре�?ур�?и: %08lXh, %08lXh).\n"
#define MSGTR_LIBVO_VESA_CantAllocateTemporaryBuffer "[VO_VESA] �?е можу виділити тимча�?овий буфер.\n"
#define MSGTR_LIBVO_VESA_SorryUnsupportedMode "[VO_VESA] Вибачте, режим не підтримуєть�?�? -- �?пробуйте -x 640 -zoom.\n"
#define MSGTR_LIBVO_VESA_OhYouReallyHavePictureOnTv "[VO_VESA] О, ви �?правді маєте картинку на ТБ!\n"
#define MSGTR_LIBVO_VESA_CantInitialozeLinuxVideoOverlay "[VO_VESA] �?е можу ініці�?лізувати Відео оверлей Linux.\n"
#define MSGTR_LIBVO_VESA_UsingVideoOverlay "[VO_VESA] Викори�?товую відео оверлей: %s.\n"
#define MSGTR_LIBVO_VESA_CantInitializeVidixDriver "[VO_VESA] �?е можу ініці�?лізувати драйвер VIDIX.\n"
#define MSGTR_LIBVO_VESA_UsingVidix "[VO_VESA] Викори�?товую VIDIX.\n"
#define MSGTR_LIBVO_VESA_CantFindModeFor "[VO_VESA] �?е можу знайти режим дл�?: %ux%u@%u.\n"
#define MSGTR_LIBVO_VESA_InitializationComplete "[VO_VESA] Ініці�?лізаці�? VESA завершена.\n"

// open.c, stream.c:
#define MSGTR_CdDevNotfound "Компактовід \"%s\" не знайдений!\n"
#define MSGTR_ErrTrackSelect "Помилка вибору треку на VCD!"
#define MSGTR_ReadSTDIN "Читанн�? з stdin...\n"
#define MSGTR_UnableOpenURL "�?е можу відкрити URL: %s\n"
#define MSGTR_ConnToServer "З'єднанн�? з �?ервером: %s\n"
#define MSGTR_FileNotFound "Файл не знайдений: '%s'\n"

#define MSGTR_SMBFileNotFound "Помилка відкритт�? з мережі: '%s'\n"
#define MSGTR_SMBNotCompiled "MPlayer не має вкомпільованої підтримки SMB\n"

#define MSGTR_CantOpenDVD "�?е зміг відкрити DVD: %s (%s)\n"
#define MSGTR_DVDnumTitles "Є %d доріжок з титрами на цьому DVD.\n"
#define MSGTR_DVDinvalidTitle "�?еприпу�?тимий номер доріжки титрів на DVD: %d\n"
#define MSGTR_DVDnumChapters "Є %d розділів на цій доріжці з DVD титрами.\n"
#define MSGTR_DVDinvalidChapter "�?еприпу�?тимий номер DVD розділу: %d\n"
#define MSGTR_DVDnumAngles "Є %d кутів на цій доріжці з DVD титрами.\n"
#define MSGTR_DVDinvalidAngle "�?еприпу�?тимий номер DVD кута: %d\n"
#define MSGTR_DVDnoIFO "�?е можу відкрити IFO файл дл�? DVD титрів %d.\n"
#define MSGTR_DVDnoVOBs "�?е можу відкрити титри VOBS (VTS_%02d_1.VOB).\n"

// demuxer.c, demux_*.c:
#define MSGTR_AudioStreamRedefined "Попередженн�?! Заголовок аудіо потоку %d перевизначений!\n"
#define MSGTR_VideoStreamRedefined "Попередженн�?! Заголовок відео потоку %d перевизначений!\n"
#define MSGTR_TooManyAudioInBuffer "\nDEMUXER: �?адто багато (%d, %d байтів) аудіо пакетів у буфері!\n"
#define MSGTR_TooManyVideoInBuffer "\nDEMUXER: �?адто багато (%d, %d байтів) відео пакетів у буфері!\n"
#define MSGTR_MaybeNI "(можливо ви програєте неперемежений потік/файл або невдалий кодек)\n"
#define MSGTR_SwitchToNi "\nДетектовано погано перемежений AVI файл - переходжу в -ni режим...\n"
#define MSGTR_Detected_XXX_FileFormat "Знайдений %s формат файлу!\n"
#define MSGTR_DetectedAudiofile "�?удіо файл детектовано.\n"
#define MSGTR_FormatNotRecognized "========= Вибачте, формат цього файлу не розпізнаний чи не підтримуєть�?�? ===========\n"\
                                  "===== Якщо це AVI, ASF або MPEG потік, будь ла�?ка зв'�?жіть�?�? з автором! ======\n"
#define MSGTR_MissingVideoStream "Відео потік не знайдений!\n"
#define MSGTR_MissingAudioStream "�?удіо потік не знайдений...  -> програю без звуку\n"
#define MSGTR_MissingVideoStreamBug "Відео потік загублений!? Зв'�?жіть�?�? з автором, це мабуть помилка :(\n"

#define MSGTR_DoesntContainSelectedStream "demux: файл не мі�?тить обраний аудіо або відео потік\n"

#define MSGTR_NI_Forced "Приму�?ово вибраний"
#define MSGTR_NI_Detected "Знайдений"
#define MSGTR_NI_Message "%s �?ЕПЕРЕМЕЖЕ�?ИЙ формат AVI файлу!\n"

#define MSGTR_UsingNINI "Викори�?танн�? �?ЕПЕРЕМЕЖЕ�?ОГО або пошкодженого формату AVI файлу!\n"
#define MSGTR_CouldntDetFNo "�?е зміг визначити чи�?ло кадрів (дл�? аб�?олютного перене�?енн�?)\n"
#define MSGTR_CantSeekRawAVI "�?е можу перемі�?тити�?�? у непроіндек�?ованому потоці .AVI! (вимагаєть�?�? індек�?, �?пробуйте з ключом -idx!)\n"
#define MSGTR_CantSeekFile "�?е можу переміщувати�?�? у цьому файлі!\n"

#define MSGTR_MOVcomprhdr "MOV: Сти�?нуті заголовки (поки що) не підтримують�?�?!\n"
#define MSGTR_MOVvariableFourCC "MOV: Попередженн�?! Знайдено перемінний FOURCC!?\n"
#define MSGTR_MOVtooManyTrk "MOV: Попередженн�?! надто багато треків!"
#define MSGTR_DetectedTV "Детектовано ТВ! ;-)\n"
#define MSGTR_ErrorOpeningOGGDemuxer "�?еможливо відкрити ogg demuxer.\n"
#define MSGTR_CannotOpenAudioStream "�?еможливо відкрити аудіо потік: %s\n"
#define MSGTR_CannotOpenSubtitlesStream "�?еможливо відкрити потік �?убтитрів: %s\n"
#define MSGTR_OpeningAudioDemuxerFailed "�?е вдало�?�? відкрити аудіо demuxer: %s\n"
#define MSGTR_OpeningSubtitlesDemuxerFailed "�?е вдало�?�? відкрити demuxer �?убтитрів: %s\n"
#define MSGTR_TVInputNotSeekable "TV input is not seekable! (Seeking will probably be for changing channels ;)\n"
#define MSGTR_ClipInfo "Інформаці�? кліпу:\n"

// dec_video.c & dec_audio.c:
#define MSGTR_CantOpenCodec "�?е зміг відкрити кодек\n"
#define MSGTR_CantCloseCodec "�?е зміг закрити кодек\n"

#define MSGTR_MissingDLLcodec "ПОМИЛК�?: �?е зміг відкрити необхідний DirectShow кодек: %s\n"
#define MSGTR_ACMiniterror "�?е зміг завантажити чи ініціалізувати Win32/ACM AUDIO кодек (загублений DLL файл?)\n"
#define MSGTR_MissingLAVCcodec "�?е можу знайти кодек \"%s\" у libavcodec...\n"

#define MSGTR_MpegNoSequHdr "MPEG: FATAL: КІ�?ЕЦЬ Ф�?ЙЛУ при пошуку по�?лідовно�?ті заголовків\n"
#define MSGTR_CannotReadMpegSequHdr "FATAL: �?е можу читати по�?лідовні�?ть заголовків!\n"
#define MSGTR_CannotReadMpegSequHdrEx "FATAL: �?е мочу читати розширенн�? по�?лідовно�?ті заголовків!\n"
#define MSGTR_BadMpegSequHdr "MPEG: Погана по�?лідовні�?ть заголовків!\n"
#define MSGTR_BadMpegSequHdrEx "MPEG: Погане розширенн�? по�?лідовно�?ті заголовків!\n"

#define MSGTR_ShMemAllocFail "�?е можу захопити загальну пам'�?ть\n"
#define MSGTR_CantAllocAudioBuf "�?е можу захопити вихідний буфер аудіо\n"

#define MSGTR_UnknownAudio "�?евідомий чи загублений аудіо формат, програю без звуку\n"

#define MSGTR_UsingExternalPP "[PP] Викори�?товую зовнішній фільтр обробки, мак�? q = %d.\n"
#define MSGTR_UsingCodecPP "[PP] Викори�?товую обробку кодека, мак�? q = %d.\n"
#define MSGTR_VideoCodecFamilyNotAvailableStr "Запрошений драйвер відео кодеку [%s] (vfm=%s) недо�?�?жний (ввімкніть його під ча�? компіл�?ції)\n"
#define MSGTR_AudioCodecFamilyNotAvailableStr "Запрошений драйвер аудіо кодеку [%s] (afm=%s) недо�?�?жний (ввімкніть його під ча�? компіл�?ції)\n"
#define MSGTR_OpeningVideoDecoder "Відкриваю відео декодер: [%s] %s\n"
#define MSGTR_OpeningAudioDecoder "Відкриваю аудіо декодер: [%s] %s\n"
#define MSGTR_VDecoderInitFailed "Збій ініціалізації VDecoder :(\n"
#define MSGTR_ADecoderInitFailed "Збій ініціалізації ADecoder :(\n"
#define MSGTR_ADecoderPreinitFailed "Збій підготуванн�? ADecoder :(\n"

// LIRC:
#define MSGTR_LIRCopenfailed "�?евдале відкритт�? підтримки lirc!\n"
#define MSGTR_LIRCcfgerr "�?евдале читанн�? файлу конфігурації LIRC %s!\n"

// vf.c
#define MSGTR_CouldNotFindVideoFilter "�?еможливо знайти відео фільтр '%s'\n"
#define MSGTR_CouldNotOpenVideoFilter "�?еможливо відкрити відео фільтр '%s'\n"
#define MSGTR_OpeningVideoFilter "Відкриваю відео фільтр: "
//-----------------------------
#define MSGTR_CannotFindColorspace "�?е можу підібрати загальну �?хему кольорів, навіть додавши 'scale' :(\n"

// vd.c
#define MSGTR_CodecDidNotSet "VDec: Кодек не в�?тановив sh->disp_w та sh->disp_h, �?пробую обійти це.\n"
#define MSGTR_CouldNotFindColorspace "�?е можу підібрати підход�?щу �?хему кольорів - повтор з -vf scale...\n"
#define MSGTR_MovieAspectIsSet "Відношенн�? �?торін %.2f:1 - ма�?штабую аби �?коректувати.\n"
#define MSGTR_MovieAspectUndefined "Відношенн�? �?торін не вказано - ма�?штабуванн�? не викори�?товуєть�?�?.\n"
