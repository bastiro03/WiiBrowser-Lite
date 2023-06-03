// Translated by: Nick Kurshev <nickols_k@mail.ru>,
// Dmitry Baryshkov <mitya@school.ioffe.ru>
// Reworked by: Andrew Savchenko aka Bircoph <Bircoph[at]list[dot]ru>

// Synced with help_mp-en.h: r28860

// ========================= MPlayer help ===========================

static const char help_text[] =
	"И�?пользование:   mplayer [опции] [URL|путь/]им�?_файла\n"
	"\n"
	"Базовые опции: (полный �?пи�?ок �?м. на man-�?транице)\n"
	" -vo <drv[:dev]> выбор драйвера и у�?трой�?тва видеовывода (�?пи�?ок �?м. �? '-vo help')\n"
	" -ao <drv[:dev]> выбор драйвера и у�?трой�?тва аудиовывода (�?пи�?ок �?м. �? '-ao help')\n"
#ifdef CONFIG_VCD
" vcd://<номер трека> играть дорожку (S)VCD (Super Video CD) (указывайте у�?трой�?тво,\n                 не монтируйте его)\n"
#endif
#ifdef CONFIG_DVDREAD
" dvd://<номер ролика> играть DVD ролик �? у�?трой�?тва вме�?то файла\n"
#endif
	" -alang/-slang   выбрать �?зык аудио/�?убтитров DVD (двухбуквенный код �?траны)\n"
	" -ss <врем�?>     переме�?тить�?�? на заданную (�?екунды или ЧЧ:ММ:СС) позицию\n"
	" -nosound        без звука\n"
	" -fs             опции полно�?кранного проигрывани�? (или -vm, -zoom, подробно�?ти\n                 на man-�?транице)\n"
	" -x <x> -y <y>   у�?тановить разрешение ди�?пле�? (и�?пользовать �? -vm или -zoom)\n"
	" -sub <файл>     указать файл �?убтитров (�?м. также -subfps, -subdelay)\n"
	" -playlist <файл> указать �?пи�?ок во�?произведени�? (плейли�?т)\n"
	" -vid x -aid y   опции дл�? выбора видео (x) и аудио (y) потока дл�? во�?произведени�?\n"
	" -fps x -srate y опции дл�? изменени�? ча�?тоты видео (x, кадр/�?ек) и аудио (y, Гц)\n"
	" -pp <quality>   разрешить фильтр по�?тобработки (подробно�?ти на man-�?транице)\n"
	" -framedrop      включить отбра�?ывание кадров (дл�? медленных машин)\n"
	"\n"
	"О�?новные кнопки: (полный �?пи�?ок в �?транице man, также �?м. input.conf)\n"
	" <- или ->       перемещение вперёд/назад на 10 �?екунд\n"
	" up или down     перемещение вперёд/назад на  1 минуту\n"
	" pgup or pgdown  перемещение вперёд/назад на 10 минут\n"
	" < или >         перемещение вперёд/назад в �?пи�?ке во�?произведени�?\n"
	" p или ПРОБЕЛ    прио�?тановить фильм (люба�? клавиша - продолжить)\n"
	" q или ESC       о�?тановить во�?произведение и выйти\n"
	" + или -         регулировать задержку звука по +/- 0.1 �?екунде\n"
	" o               цикличный перебор OSD режимов:  нет / навигаци�? / навигаци�?+таймер\n"
	" * или /         прибавить или убавить PCM громко�?ть\n"
	" z или x         регулировать задержку �?убтитров по +/- 0.1 �?екунде\n"
	" r или t         регулировка вертикальной позиции �?убтитров, �?м. также -vf expand\n"
	"\n"
	" * * * ПОДРОБ�?ЕЕ СМ. ДОКУМЕ�?Т�?ЦИЮ, О ДОПОЛ�?ИТЕЛЬ�?ЫХ ОПЦИЯХ И КЛЮЧ�?Х! * * *\n"
	"\n";

// ========================= MPlayer messages ===========================

// mplayer.c
#define MSGTR_Exiting "\nВыходим...\n"
#define MSGTR_ExitingHow "\nВыходим... (%s)\n"
#define MSGTR_Exit_quit "Выход"
#define MSGTR_Exit_eof "Конец файла"
#define MSGTR_Exit_error "Фатальна�? ошибка"
#define MSGTR_IntBySignal "\nMPlayer прерван �?игналом %d в модуле: %s \n"
#define MSGTR_NoHomeDir "�?е могу найти ДОМ�?Ш�?ИЙ каталог\n"
#define MSGTR_GetpathProblem "проблемы в get_path(\"config\")\n"
#define MSGTR_CreatingCfgFile "Создание файла конфигурации: %s\n"
#define MSGTR_CantLoadFont "�?е могу загрузить побитовый шрифт: %s\n"
#define MSGTR_CantLoadSub "�?е могу загрузить �?убтитры: %s\n"
#define MSGTR_DumpSelectedStreamMissing "дамп: Ф�?Т�?ЛЬ�?�?Я ОШИБК�?: Выбранный поток потер�?н!\n"
#define MSGTR_CantOpenDumpfile "�?е могу открыть файл дампа!!!\n"
#define MSGTR_CoreDumped "Создан дамп �?дра ;)\n"
#define MSGTR_FPSnotspecified "В заголовке кадры/�?ек не указаны (или недопу�?тимые)! И�?пользуйте опцию -fps!\n"
#define MSGTR_TryForceAudioFmtStr "Попытка фор�?ировать �?емей�?тво аудиокодеков %s...\n"
#define MSGTR_CantFindAudioCodec "�?е могу найти кодек дл�? аудиоформата 0x%X!\n"
#define MSGTR_TryForceVideoFmtStr "Попытка фор�?ировать �?емей�?тво видеокодеков %s...\n"
#define MSGTR_CantFindVideoCodec "�?е могу найти кодек дл�? выбранного -vo и видеоформата 0x%X!\n"
#define MSGTR_CannotInitVO "Ф�?Т�?ЛЬ�?�?Я ОШИБК�?: �?е могу инициализировать видеодрайвер!\n"
#define MSGTR_CannotInitAO "�?е могу открыть/инициализировать аудиоу�?трой�?тво -> без звука.\n"
#define MSGTR_StartPlaying "�?ачало во�?произведени�?...\n"

#define MSGTR_SystemTooSlow "\n\n"\
"         *****************************************************************\n"\
"         **** Ваша �?и�?тема �?лишком МЕДЛЕ�?�?�? чтобы во�?производить �?то! ****\n"\
"         *****************************************************************\n"\
"Возможные причины, проблемы, обходы: \n"\
"- �?аиболее ча�?та�?: плохой/�?ырой _аудио_ драйвер\n"\
"  - Попробуйте -ao sdl или и�?пользуйте �?мул�?цию OSS на ALSA.\n"\
"  - По�?к�?периментируйте �? различными значени�?ми -autosync, начните �? 30.\n"\
"- Медленный видео вывод\n"\
"  - Попытайте�?ь другие -vo driver (�?пи�?ок: -vo help) или попытайте�?ь �? -framedrop!\n"\
"- Медленный проце�?�?ор\n"\
"  - �?е пытайте�?ь во�?производить большие DVD/DivX на медленных проце�?�?орах!\n" \
"    Попробуйте некоторые опции lavdopts, например:\n-vfm ffmpeg -lavdopts lowres=1:fast:skiploopfilter=all.\n"\
"- Битый файл\n"\
"  - Попробуйте различные комбинации: -nobps -ni -forceidx -mc 0\n"\
"- Медленный но�?итель (�?монтированные NFS/SMB, DVD, VCD и т.п.)\n"\
"  - И�?пользуйте -cache 8192.\n"\
"- И�?пользуете ли Вы -cache дл�? проигрывани�? нe�?лоёных [non-interleaved] AVI файлов?\n"\
"  - И�?пользуйте -nocache.\n"\
"Читайте DOCS/HTML/ru/video.html дл�? �?оветов по под�?тройке/у�?корению.\n"\
"Е�?ли ничего не помогло, тогда читайте DOCS/HTML/ru/bugreports.html!\n\n"

#define MSGTR_NoGui "MPlayer был �?компилирован БЕЗ поддержки GUI!\n"
#define MSGTR_GuiNeedsX "GUI MPlayer необходим X11!\n"
#define MSGTR_Playing "\nВо�?произведение %s.\n"
#define MSGTR_NoSound "�?удио: без звука\n"
#define MSGTR_FPSforced "Кадры/�?ек фор�?ированы в %5.3f (врем�? кадра: %5.3f).\n"
#define MSGTR_AvailableVideoOutputDrivers "До�?тупные драйвера вывода видео:\n"
#define MSGTR_AvailableAudioOutputDrivers "До�?тупные драйвера вывода звука:\n"
#define MSGTR_AvailableAudioCodecs "До�?тупные аудиокодеки:\n"
#define MSGTR_AvailableVideoCodecs "До�?тупные видеокодеки:\n"
#define MSGTR_AvailableAudioFm "До�?тупные (вкомпилированные) �?емей�?тва/драйверы аудиокодеков:\n"
#define MSGTR_AvailableVideoFm "До�?тупные (вкомпилированные) �?емей�?тва/драйверы видеокодеков:\n"
#define MSGTR_AvailableFsType "До�?тупные режимы изменени�? полно�?кранного �?ло�?:\n"
#define MSGTR_CannotReadVideoProperties "Видео: �?е могу прочитать �?вой�?тва.\n"
#define MSGTR_NoStreamFound "Поток не найден.\n"
#define MSGTR_ErrorInitializingVODevice "Ошибка при открытии/инициализации выбранного у�?трой�?тва видеовывода (-vo).\n"
#define MSGTR_ForcedVideoCodec "Фор�?ирован видеокодек: %s\n"
#define MSGTR_ForcedAudioCodec "Фор�?ирован аудиокодек: %s\n"
#define MSGTR_Video_NoVideo "Видео: нет видео\n"
#define MSGTR_NotInitializeVOPorVO "\nФ�?Т�?ЛЬ�?�?Я ОШИБК�?: �?е могу инициализировать видеофильтры (-vf) или видеовывод (-vo).\n"
#define MSGTR_Paused "=== ПРИОСТ�?�?ОВЛЕ�?О ===" // no more than 23 characters (status line for audio files)
#define MSGTR_PlaylistLoadUnable "\n�?е могу загрузить �?пи�?ок во�?произведени�? (плейли�?т) %s.\n"
#define MSGTR_Exit_SIGILL_RTCpuSel \
"- MPlayer рухнул из-за '�?еправильной Ин�?трукции'.\n"\
"  Это может быть ошибкой нашего нового кода динамиче�?кого определени�? типа CPU...\n"\
"  Пожалуй�?та, читайте DOCS/HTML/ru/bugreports.html.\n"
#define MSGTR_Exit_SIGILL \
"- MPlayer рухнул из-за '�?еправильной Ин�?трукции'.\n"\
"  Обычно, �?то прои�?ходит когда Вы его запу�?каете на CPU, отличном от того, дл�? которого\n"\
"  он был �?компилирован/оптимизирован.\n"\
"  Проверьте �?то!\n"
#define MSGTR_Exit_SIGSEGV_SIGFPE \
"- MPlayer рухнул из-за плохого и�?пользовани�? CPU/FPU/RAM.\n"\
"  Перекомпилируйте MPlayer �? --enable-debug и �?делайте 'gdb' backtrace и\n"\
"  диза�?�?емблирование. Дл�? подробно�?тей, �?м.\nDOCS/HTML/ru/bugreports_what.html#bugreports_crash\n"
#define MSGTR_Exit_SIGCRASH \
"- MPlayer рухнул. Это не должно прои�?ходить.\n"\
"  Это может быть ошибкой в коде MPlayer _или_ в Вашем драйвере, _или_\n"\
"  в Вашей вер�?ии gcc. Е�?ли Вы думаете, что в �?том виноват MPlayer, пожалуй�?та,\n"\
"  прочтите DOCS/HTML/ru/bugreports.html и �?ледуйте ин�?трукци�?м оттуда.\n"\
"  Мы не �?можем и не будем помогать, пока Вы не предо�?тавите �?ту информацию,\n�?ообща�? о возможной ошибке.\n"
#define MSGTR_LoadingConfig "Загружаю конфигурационный файл '%s'\n"
#define MSGTR_LoadingProtocolProfile "Загружаю профиль дл�? протокола '%s'\n"
#define MSGTR_LoadingExtensionProfile "Загружаю профиль дл�? ра�?ширени�? '%s'\n"
#define MSGTR_AddedSubtitleFile "СУБТИТРЫ: добавлен файл �?убтитров (%d): %s\n"
#define MSGTR_RemovedSubtitleFile "СУБТИТРЫ: Удалён файл �?убтитров (%d): %s\n"
#define MSGTR_ErrorOpeningOutputFile "Ошибка открыти�? файла [%s] дл�? запи�?и!\n"
#define MSGTR_RTCDeviceNotOpenable "�?е могу открыть %s: %s (пользователь должен обладать правом чтени�? на �?тот файл).\n"
#define MSGTR_LinuxRTCInitErrorIrqpSet "Ошибка инициализации Linux RTC в ioctl (rtc_irqp_set %lu): %s\n"
#define MSGTR_IncreaseRTCMaxUserFreq "Попробуйте добавить \"echo %lu > /proc/sys/dev/rtc/max-user-freq\" \nв загрузочные �?крипты Вашей �?и�?темы.\n"
#define MSGTR_LinuxRTCInitErrorPieOn "Ошибка инициализации Linux RTC в ioctl (rtc_pie_on): %s\n"
#define MSGTR_UsingTimingType "И�?пользует�?�? %s �?инхронизаци�?.\n"
#define MSGTR_Getch2InitializedTwice "ПРЕДУПРЕЖДЕ�?ИЕ: getch2_init вызван дважды!\n"
#define MSGTR_DumpstreamFdUnavailable "�?е могу �?оздать дамп �?того потока - нет до�?тупных файловых опи�?ателей.\n"
#define MSGTR_CantOpenLibmenuFilterWithThisRootMenu "�?е могу открыть видеофильтр libmenu �? �?тим корневым меню %s.\n"
#define MSGTR_AudioFilterChainPreinitError "Ошибка в цепочке pre-init аудиофильтра!\n"
#define MSGTR_LinuxRTCReadError "Ошибка чтени�? Linux RTC: %s\n"
#define MSGTR_SoftsleepUnderflow "Предупреждение! �?едопу�?тимо низкое значение программной задержки!\n"
#define MSGTR_DvdnavNullEvent "Событие DVDNAV NULL?!\n"
#define MSGTR_DvdnavHighlightEventBroken "Событие DVDNAV: Событие выделени�? �?ломано\n"
#define MSGTR_DvdnavEvent "Событие DVDNAV: %s\n"
#define MSGTR_DvdnavHighlightHide "Событие DVDNAV: Выделение �?крыто\n"
#define MSGTR_DvdnavStillFrame "######################################## Событие DVDNAV: Стоп-кадр: %d �?ек\n"
#define MSGTR_DvdnavNavStop "Событие DVDNAV: О�?тановка Nav \n"
#define MSGTR_DvdnavNavNOP "Событие DVDNAV: Nav NOP\n"
#define MSGTR_DvdnavNavSpuStreamChangeVerbose "Событие DVDNAV: Изменение SPU-потока Nav: физиче�?ки: %d/%d/%d логиче�?ки: %d\n"
#define MSGTR_DvdnavNavSpuStreamChange "Событие DVDNAV: Изменение SPU-потока Nav: физиче�?ки: %d логиче�?ки: %d\n"
#define MSGTR_DvdnavNavAudioStreamChange "Событие DVDNAV: Изменение аудиопотока Nav: физиче�?ки: %d логиче�?ки: %d\n"
#define MSGTR_DvdnavNavVTSChange "Событие DVDNAV: Изменение Nav VTS\n"
#define MSGTR_DvdnavNavCellChange "Событие DVDNAV: Изменение �?чейки Nav\n"
#define MSGTR_DvdnavNavSpuClutChange "Событие DVDNAV: Изменение Nav SPU CLUT\n"
#define MSGTR_DvdnavNavSeekDone "Событие DVDNAV: Завершено позиционирование Nav\n"
#define MSGTR_MenuCall "Вызов меню\n"

#define MSGTR_EdlOutOfMem "�?е могу выделить до�?таточный объём пам�?ти дл�? хранени�? данных EDL.\n"
#define MSGTR_EdlRecordsNo "Чтение %d EDL дей�?твий.\n"
#define MSGTR_EdlQueueEmpty "�?ет дей�?твий EDL, которые �?ледует и�?полнить (очередь пу�?та).\n"
#define MSGTR_EdlCantOpenForWrite "�?е могу открыть файл EDL [%s] дл�? запи�?и.\n"
#define MSGTR_EdlCantOpenForRead "�?е могу открыть файл EDL [%s] дл�? чтени�?.\n"
#define MSGTR_EdlNOsh_video "�?ельз�? и�?пользовать EDL без видео, отключаю.\n"
#define MSGTR_EdlNOValidLine "�?еверна�? �?трока EDL: %s\n"
#define MSGTR_EdlBadlyFormattedLine "Плохо форматированна�? �?трока EDL [%d]. Пропу�?каю.\n"
#define MSGTR_EdlBadLineOverlap "По�?ледн�?�? позици�? о�?танова была [%f]; �?ледующа�? �?тартова�? "\
"позици�? [%f]. Запи�?и должны быть в хронологиче�?ком пор�?дке, не могу перекрыть.\nПропу�?каю.\n"
#define MSGTR_EdlBadLineBadStop "Врем�? о�?танова должно быть по�?ле времени �?тарта.\n"
#define MSGTR_EdloutBadStop "EDL пропу�?к отменён, по�?ледний start > stop\n"
#define MSGTR_EdloutStartSkip "�?ачало EDL пропу�?ка, нажмите 'i' ещё раз дл�? завершени�? блока.\n"
#define MSGTR_EdloutEndSkip "Конец EDL пропу�?ка, �?трока запи�?ана.\n"

// mplayer.c OSD
#define MSGTR_OSDenabled "включено"
#define MSGTR_OSDdisabled "выключено"
#define MSGTR_OSDAudio "�?удио: %s"
#define MSGTR_OSDVideo "Видео: %s"
#define MSGTR_OSDChannel "Канал: %s"
#define MSGTR_OSDSubDelay "Задержка �?убтитров: %d м�?"
#define MSGTR_OSDSpeed "Скоро�?ть: x %6.2f"
#define MSGTR_OSDosd "OSD: %s"
#define MSGTR_OSDChapter "Раздел: (%d) %s"
#define MSGTR_OSDAngle "Угол: %d/%d"

// property values
#define MSGTR_Enabled "включено"
#define MSGTR_EnabledEdl "включено (EDL)"
#define MSGTR_Disabled "выключено"
#define MSGTR_HardFrameDrop "интен�?ивный"
#define MSGTR_Unknown "неизве�?тно"
#define MSGTR_Bottom "низ"
#define MSGTR_Center "центр"
#define MSGTR_Top "верх"
#define MSGTR_SubSourceFile "из файла"
#define MSGTR_SubSourceVobsub "vobsub"
#define MSGTR_SubSourceDemux "внедренные"

// OSD bar names
#define MSGTR_Volume "Громко�?ть"
#define MSGTR_Panscan "У�?ечение �?торон"
#define MSGTR_Gamma "Гамма"
#define MSGTR_Brightness "Ярко�?ть"
#define MSGTR_Contrast "Контра�?тно�?ть"
#define MSGTR_Saturation "�?а�?ыщенно�?ть"
#define MSGTR_Hue "Цвет"
#define MSGTR_Balance "Балан�?"

// property state
#define MSGTR_LoopStatus "Повтор�?ть: %s"
#define MSGTR_MuteStatus "Приглушить: %s"
#define MSGTR_AVDelayStatus "A-V задержка: %s"
#define MSGTR_OnTopStatus "Поверх о�?тальных: %s"
#define MSGTR_RootwinStatus "root-окно: %s"
#define MSGTR_BorderStatus "Рамка: %s"
#define MSGTR_FramedroppingStatus "Пропу�?к кадров: %s"
#define MSGTR_VSyncStatus "Вертикальна�? �?инхронизаци�?: %s"
#define MSGTR_SubSelectStatus "Субтитры: %s"
#define MSGTR_SubSourceStatus "И�?точник �?убтитров: %s"
#define MSGTR_SubPosStatus "Позици�? �?убтитров: %s/100"
#define MSGTR_SubAlignStatus "Выравнивание �?убтитров: %s"
#define MSGTR_SubDelayStatus "Задержка �?убтитров: %s"
#define MSGTR_SubScale "Ма�?штаб �?убтитров: %s"
#define MSGTR_SubVisibleStatus "Субтитры: %s"
#define MSGTR_SubForcedOnlyStatus "Фор�?ированы только �?убтитры: %s"

// mencoder.c
#define MSGTR_UsingPass3ControlFile "И�?пользую �?ледующий файл дл�? контрол�? 3-го прохода: %s\n"
#define MSGTR_MissingFilename "\nПропущено им�? файла.\n\n"
#define MSGTR_CannotOpenFile_Device "�?е могу открыть файл/у�?трой�?тво.\n"
#define MSGTR_CannotOpenDemuxer "�?е могу открыть демультиплек�?ор.\n"
#define MSGTR_NoAudioEncoderSelected "\nКодировщик аудио (-oac) не выбран.\nВыберете какой-нибудь (�?м. -oac help) или и�?пользуйте -nosound.\n"
#define MSGTR_NoVideoEncoderSelected "\nКодировщик видео (-ovc) не выбран. Выберете какой-нибудь (�?м. -ovc help).\n"
#define MSGTR_CannotOpenOutputFile "�?е могу открыть файл вывода '%s'.\n"
#define MSGTR_EncoderOpenFailed "�?е могу открыть кодировщик.\n"
#define MSGTR_MencoderWrongFormatAVI "\nПРЕДУПРЕЖДЕ�?ИЕ: ВЫХОД�?ОЙ ФОРМ�?Т Ф�?ЙЛ�? _AVI_. См. -of help.\n"
#define MSGTR_MencoderWrongFormatMPG "\nПРЕДУПРЕЖДЕ�?ИЕ: ВЫХОД�?ОЙ ФОРМ�?Т Ф�?ЙЛ�? _MPEG_. См. -of help.\n"
#define MSGTR_MissingOutputFilename "�?е указан выходной файл, и�?пользуйте опцию -o."
#define MSGTR_ForcingOutputFourcc "Выходной fourcc фор�?ирован в %x [%.4s]\n"
#define MSGTR_ForcingOutputAudiofmtTag "Фор�?ирую тег выходного аудиоформата в 0x%x.\n"
#define MSGTR_DuplicateFrames "\n%d повтор�?ющий�?�?(х�?�?) кадр(а/ов)!\n"
#define MSGTR_SkipFrame "\nПропу�?каю кадр!\n"
#define MSGTR_ResolutionDoesntMatch "\n�?овый видеофайл обладает иным разрешением или цветовым про�?тран�?твом, чем\nпредыдущий.\n"
#define MSGTR_FrameCopyFileMismatch "\nВ�?е видеофайлы должны обладать одинаковым fps, разрешением и кодеком дл�?\n-ovc copy.\n"
#define MSGTR_AudioCopyFileMismatch "\nВ�?е файлы должны обладать одинаковым аудиокодеком и форматом дл�? -oac copy.\n"
#define MSGTR_NoAudioFileMismatch "\n�?е могу �?мешивать файлы, �?одержащие только видео, �? аудио- и видеофайлами.\nПопробуйте -nosound.\n"
#define MSGTR_NoSpeedWithFrameCopy "ПРЕДУПРЕЖДЕ�?ИЕ: не гарантирует�?�? корректна�? работа -speed �? -oac copy!\n"\
"Ваше кодирование может быть и�?порчено!\n"
#define MSGTR_ErrorWritingFile "%s: Ошибка при запи�?и файла.\n"
#define MSGTR_FlushingVideoFrames "\nСбро�? видео кадров.\n"
#define MSGTR_FiltersHaveNotBeenConfiguredEmptyFile "Фильтры не на�?троены! Пу�?той файл?\n"
#define MSGTR_RecommendedVideoBitrate "Рекомендуемый битпоток дл�? %s CD: %d\n"
#define MSGTR_VideoStreamResult "\nПоток видео: %8.3f кбит/�?  (%d Б/�?)  размер: %"PRIu64" байт(а/ов)  %5.3f �?ек.  %d кадр(а/ов)\n"
#define MSGTR_AudioStreamResult "\nПоток аудио: %8.3f кбит/�?  (%d Б/�?)  размер: %"PRIu64" байт(а/ов)  %5.3f �?ек.\n"
#define MSGTR_EdlSkipStartEndCurrent "EDL SKIP: �?ачало: %.2f  Конец: %.2f   Текуща�?: V: %.2f  A: %.2f     \r"
#define MSGTR_OpenedStream "у�?пех: формат: %d  данные: 0x%X - 0x%x\n"
#define MSGTR_VCodecFramecopy "видеокодек: копирование кадров (%dx%d %dbpp fourcc=%x)\n"
#define MSGTR_ACodecFramecopy "аудиокодек: копирование кадров (формат=%x цепочек=%d �?коро�?ть=%d битов=%d Б/�?=%d образец=%d)\n"
#define MSGTR_CBRPCMAudioSelected "Выбрано CBR PCM аудио\n"
#define MSGTR_MP3AudioSelected "Выбрано MP3 аудио\n"
#define MSGTR_CannotAllocateBytes "�?е могу выделить пам�?ть дл�? %d байт\n"
#define MSGTR_SettingAudioDelay "У�?танавливаю задержку аудио в %5.3f\n"
#define MSGTR_SettingVideoDelay "У�?танавливаю задержку видео в %5.3fs.\n"
#define MSGTR_LimitingAudioPreload "Ограничиваю предварительную загрузку аудио до 0.4�?\n"
#define MSGTR_IncreasingAudioDensity "Увеличиваю плотно�?ть аудио до 4\n"
#define MSGTR_ZeroingAudioPreloadAndMaxPtsCorrection "Фор�?ирую предварительную загрузку аудио в 0, мак�?имальную коррекцию pts в 0\n"
#define MSGTR_LameVersion "Вер�?и�? LAME %s (%s)\n\n"
#define MSGTR_InvalidBitrateForLamePreset "Ошибка: Заданный битпоток вне допу�?тимого значени�? дл�? данной преду�?тановки.\n"\
"\n"\
"При и�?пользовании �?того режима Вы должны указать значение между \"8\" и \"320\"\n"\
"\n"\
"Дл�? дополнительной информации и�?пользуйте: \"-lameopts preset=help\"\n"
#define MSGTR_InvalidLamePresetOptions "Ошибка: Вы не указали верный профиль и/или опции преду�?тановки.\n"\
"\n"\
"До�?тупные профили:\n"\
"\n"\
"   <fast>        standard\n"\
"   <fast>        extreme\n"\
"                 insane\n"\
"   <cbr> (Режим ABR) - Подразумевает�?�? режим ABR. Дл�? и�?пользовани�?\n"\
"                       про�?то укажите битпоток. �?апример:\n"\
"                       \"preset=185\" активирует �?ту преду�?тановку (preset)\n"\
"                       и и�?пользует 185 как �?реднее значение кбит/�?ек.\n"\
"\n"\
"    �?е�?колько примеров:\n"\
"\n"\
"     \"-lameopts fast:preset=standard  \"\n"\
" или \"-lameopts  cbr:preset=192       \"\n"\
" или \"-lameopts      preset=172       \"\n"\
" или \"-lameopts      preset=extreme   \"\n"\
"\n"\
"Дл�? дополнительной информации и�?пользуйте: \"-lameopts preset=help\"\n"
#define MSGTR_LamePresetsLongInfo "\n"\
"Ключи преду�?тановок разработаны �? целью предо�?тавлени�? мак�?имально возможного\nкаче�?тва.\n"\
"\n"\
"Они были преимуще�?твенно разработаны и на�?троены �? помощью тщательных те�?тов\n"\
"двойного про�?лушивани�? дл�? проверки и до�?тижени�? �?той цели.\n"\
"\n"\
"Ключи преду�?тановок по�?то�?нно обновл�?ют�?�? дл�? �?оответ�?тви�? по�?ледним разработкам,\n"\
"в результате чего Вы должны получить практиче�?ки наилучшее каче�?тво, \n"\
"возможное на текущий момент при и�?пользовании LAME.\n"\
"\n"\
"Чтобы и�?пользовать �?ти преду�?тановки:\n"\
"\n"\
"   Дл�? VBR режимов (обычно лучшее каче�?тво):\n"\
"\n"\
"     \"preset=standard\" Обычно �?той преду�?тановки должно быть до�?таточно\n"\
"                             дл�? большин�?тва людей и большин�?тва музыки, и она\n"\
"                             уже предо�?тавл�?ет до�?таточно вы�?окое каче�?тво.\n"\
"\n"\
"     \"preset=extreme\" Е�?ли Вы обладаете чрезвычайно хорошим �?лухом и\n"\
"                             �?оответ�?твующим оборудованием, �?та преду�?тановка,\n"\
"                             как правило, предо�?тавит не�?колько лучшее каче�?тво,\n"\
"                             чем режим \"standard\".\n"\
"\n"\
"   Дл�? CBR 320kbps (мак�?имально возможное каче�?тво, получаемое\n                             при и�?пользовании ключей преду�?тановок):\n"\
"\n"\
"     \"preset=insane\"  И�?пользование �?той у�?тановки �?вл�?ет�?�? перебором дл�?\n"\
"                             большин�?тва людей и большин�?тва �?итуаций, но е�?ли\n"\
"                             Вам необходимо мак�?имально возможное каче�?тво,\n"\
"                             невзира�? на размер файла - �?то �?по�?об �?делать так.\n"\
"\n"\
"   Дл�? ABR режимов (вы�?окое каче�?тво дл�? заданного битпотока,\nно не такое вы�?окое, как VBR):\n"\
"\n"\
"     \"preset=<kbps>\"  И�?пользование �?той преду�?тановки обычно даёт хорошее\n"\
"                             каче�?тво дл�? заданного битпотока. О�?новыва�?�?ь на\n"\
"                             введённом битпотоке, �?та преду�?тановка определит\n"\
"                             оптимальные на�?тройки дл�? каждой конкретной �?итуации.\n"\
"                             �?е�?мотр�? на то, что �?тот подход работает, он далеко\n"\
"                             не такой гибкий как VBR и обычно не до�?тигает\n"\
"                             такого же уровн�? каче�?тва как VBR на вы�?оких битпотоках.\n"\
"\n"\
"Также до�?тупны �?ледующие опции дл�? �?оответ�?твующих профилей:\n"\
"\n"\
"   <fast>        standard\n"\
"   <fast>        extreme\n"\
"                 insane\n"\
"   <cbr> (Режим ABR) - Подразумевает�?�? режим ABR. Дл�? и�?пользовани�?\n"\
"                       про�?то укажите битпоток. �?апример:\n"\
"                       \"preset=185\" активирует �?ту преду�?тановку (preset)\n"\
"                       и и�?пользует 185 как �?реднее значение кбит/�?ек.\n"\
"\n"\
"   \"fast\" - Включает новый бы�?трый VBR дл�? конкретного профил�?.\n"\
"            �?едо�?татком �?того ключа �?вл�?ет�?�? то, что ча�?то\n"\
"            битпоток будет немного больше, чем в нормальном режиме;\n"\
"            также каче�?тво может быть не�?колько хуже.\n"\
"Предупреждение: В текущей вер�?ии бы�?трые преду�?тановки могут приве�?ти к �?лишком\n"\
"                вы�?окому битпотоку, по �?равнению �? обычными преду�?тановками.\n"\
"\n"\
"   \"cbr\"  - Е�?ли Вы и�?пользуете режим ABR (�?м. выше) �? таким \"кратным\""\
"            битпотоком как 80, 96, 112, 128, 160, 192, 224, 256, 320,\n"\
"            Вы можете и�?пользовать опцию \"cbr\" дл�? фор�?ировани�? кодировани�?\n"\
"            в режиме CBR вме�?то �?тандартного abr режима. ABR предо�?тавл�?ет\n"\
"            более вы�?окое каче�?тво, но CBR может быть полезным в таких\n"\
"            �?итуаци�?х, как передача потоков mp3 через интернет.\n"\
"\n"\
"    �?апример:\n"\
"\n"\
"     \"-lameopts fast:preset=standard  \"\n"\
" или \"-lameopts  cbr:preset=192       \"\n"\
" или \"-lameopts      preset=172       \"\n"\
" или \"-lameopts      preset=extreme   \"\n"\
"\n"\
"\n"\
"�?е�?колько п�?евдонимов до�?тупно дл�? режима ABR:\n"\
"phone => 16kbps/mono        phon+/lw/mw-eu/sw => 24kbps/mono\n"\
"mw-us => 40kbps/mono        voice => 56kbps/mono\n"\
"fm/radio/tape => 112kbps    hifi => 160kbps\n"\
"cd => 192kbps               studio => 256kbps"
#define MSGTR_LameCantInit \
"�?е могу задать опции LAME, проверьте битпоток/ча�?тоту_ди�?кретизации,\n"\
"некоторые очень малые битпотоки (<32) нуждают�?�? в меньших ча�?тотах\nди�?кретизации (например, -srate 8000).\n"\
"Е�?ли в�?ё другое не поможет, попробуйте преду�?тановку."
#define MSGTR_ConfigFileError "ошибка в конфигурационном файле"
#define MSGTR_ErrorParsingCommandLine "ошибка анализа командной �?троки"
#define MSGTR_VideoStreamRequired "�?аличие потока видео об�?зательно!\n"
#define MSGTR_ForcingInputFPS "входные кадры/�?ек будут заменены на %5.3f\n"
#define MSGTR_RawvideoDoesNotSupportAudio "Выходной формат файла RAWVIDEO не поддерживает аудио - отключаю аудио\n"
#define MSGTR_DemuxerDoesntSupportNosound "Этот демультиплек�?ор пока что не поддерживает -nosound.\n"
#define MSGTR_MemAllocFailed "не могу выделить пам�?ть"
#define MSGTR_NoMatchingFilter "�?е могу найти �?оответ�?твующий фильтр/формат_аудиовывода!\n"
#define MSGTR_MP3WaveFormatSizeNot30 "sizeof(MPEGLAYER3WAVEFORMAT)==%d!=30, возможно, �?ломанный компил�?тор C?\n"
#define MSGTR_NoLavcAudioCodecName "�?удио LAVC, пропущено им�? кодека!\n"
#define MSGTR_LavcAudioCodecNotFound "�?удио LAVC, не могу найти кодировщик дл�? кодека %s\n"
#define MSGTR_CouldntAllocateLavcContext "�?удио LAVC, не могу разме�?тить контек�?т!\n"
#define MSGTR_CouldntOpenCodec "�?е могу открыть кодек %s, br=%d\n"
#define MSGTR_CantCopyAudioFormat "�?удиоформат 0x%x не�?овме�?тим �? '-oac copy', попробуйте '-oac pcm'\nили и�?пользуйте '-fafmttag' дл�? его переопределени�?.\n"

// cfg-mencoder.h
#define MSGTR_MEncoderMP3LameHelp "\n\n"\
" vbr=<0-4>     метод кодировани�? �? переменным битпотоком\n"\
"                0: cbr (по�?то�?нный битпоток)\n"\
"                1: mt (VBR алгоритм Марка Тейлора [Mark Taylor])\n"\
"                2: rh (VBR алгоритм Роберта Гиджимана [Robert Hegemann]\n                   -- по умолчанию)\n"\
"                3: abr (у�?реднённый битпоток)\n"\
"                4: mtrh (VBR алгоритм Марка Тейлора и Роберта Гиджимана)\n"\
"\n"\
" abr           у�?реднённый битпоток\n"\
"\n"\
" cbr           по�?то�?нный битпоток\n"\
"               Также фор�?ирует режим CBR кодировани�? на по�?ледующих режимах\n               преду�?тановок ABR\n"\
"\n"\
" br=<0-1024>   укажите битпоток в кбит (только CBR и ABR)\n"\
"\n"\
" q=<0-9>       каче�?тво (0-вы�?шее, 9-наименьшее) (только дл�? VBR)\n"\
"\n"\
" aq=<0-9>      каче�?тво алгоритма (0-лучшее/�?амый медленный, 9-худшее/бы�?трейший)\n"\
"\n"\
" ratio=<1-100> ко�?ффициент �?жати�?\n"\
"\n"\
" vol=<0-10>    у�?тановите у�?иление вход�?щего аудио\n"\
"\n"\
" mode=<0-3>    (по-умолчанию: автоопределение)\n"\
"                0: �?терео\n"\
"                1: объединённое �?терео [joint-stereo]\n"\
"                2: двухканальный\n"\
"                3: моно\n"\
"\n"\
" padding=<0-2>\n"\
"                0: нет\n"\
"                1: в�?е\n"\
"                2: регулируемое\n"\
"\n"\
" fast          Переключение на бы�?трое кодирование на по�?ледующих режимах\n"\
"               преду�?тановок VBR; не�?колько худшее каче�?тво и завышенные битпотоки.\n"\
"\n"\
" preset=<value> Предо�?тавл�?ет у�?тановки наибольшего возможного каче�?тва.\n"\
"                 medium: VBR кодирование, хорошее каче�?тво\n"\
"                 (амплитуда битпотока 150-180 kbps)\n"\
"                 standard: VBR кодирование, вы�?окое каче�?тво\n"\
"                 (амплитуда битпотока 170-210 kbps)\n"\
"                 extreme: VBR кодирование, очень вы�?окое каче�?тво\n"\
"                 (амплитуда битпотока 200-240 kbps)\n"\
"                 insane:  CBR кодирование, лучшее преду�?тановленное каче�?тво\n"\
"                 (битпоток 320 kbps)\n"\
"                 <8-320>: ABR кодирование �? заданным в кбит �?редним битпотоком.\n\n"

//codec-cfg.c
#define MSGTR_DuplicateFourcc "повтор�?ющий�?�? FourCC"
#define MSGTR_TooManyFourccs "�?лишком много FourCCs/форматов..."
#define MSGTR_ParseError "ошибка разбора �?интак�?и�?а"
#define MSGTR_ParseErrorFIDNotNumber "ошибка разбора �?интак�?и�?а (ID формата не чи�?ло?)"
#define MSGTR_ParseErrorFIDAliasNotNumber "ошибка разбора �?интак�?и�?а (п�?евдоним ID формата не чи�?ло?)"
#define MSGTR_DuplicateFID "повтор�?ющее�?�? ID формата"
#define MSGTR_TooManyOut "�?лишком много выходных форматов..."
#define MSGTR_InvalidCodecName "\nим�? кодека '%s' не верно!\n"
#define MSGTR_CodecLacksFourcc "\nкодек '%s' не имеет FourCC/формат!\n"
#define MSGTR_CodecLacksDriver "\nкодек '%s' не имеет драйвера!\n"
#define MSGTR_CodecNeedsDLL "\nкодеку '%s' необходима 'dll'!\n"
#define MSGTR_CodecNeedsOutfmt "\nкодеку '%s' необходим 'outfmt'!\n"
#define MSGTR_CantAllocateComment "�?е могу выделить пам�?ть дл�? комментари�?. "
#define MSGTR_GetTokenMaxNotLessThanMAX_NR_TOKEN "get_token(): max >= MAX_MR_TOKEN!"
#define MSGTR_CantGetMemoryForLine "�?е могу выделить пам�?ть дл�? �?троки: %s\n"
#define MSGTR_CantReallocCodecsp "�?е могу выполнить realloc дл�? '*codecsp': %s\n"
#define MSGTR_CodecNameNotUnique "Им�? кодека '%s' не уникально."
#define MSGTR_CantStrdupName "�?е могу выполнить strdup -> 'name': %s\n"
#define MSGTR_CantStrdupInfo "�?е могу выполнить strdup -> 'info': %s\n"
#define MSGTR_CantStrdupDriver "�?е могу выполнить strdup -> 'driver': %s\n"
#define MSGTR_CantStrdupDLL "�?е могу выполнить strdup -> 'dll': %s"
#define MSGTR_AudioVideoCodecTotals "%d аудио & %d видео кодеков\n"
#define MSGTR_CodecDefinitionIncorrect "Кодек определён некорректно."
#define MSGTR_OutdatedCodecsConf "Этот codecs.conf �?лишком �?тар и не�?овме�?тим �? данным релизом MPlayer!"

// fifo.c
#define MSGTR_CannotMakePipe "�?е могу �?оздать канал!\n"

// parser-mecmd.c, parser-mpcmd.c
#define MSGTR_NoFileGivenOnCommandLine "'--' означает конец опций, но им�? файла в командной �?троке не указано.\n"
#define MSGTR_TheLoopOptionMustBeAnInteger "Опци�? loop должна быть целочи�?ленной: %s\n"
#define MSGTR_UnknownOptionOnCommandLine "�?еизве�?тна�? опци�? командной �?троки: -%s\n"
#define MSGTR_ErrorParsingOptionOnCommandLine "Ошибка анализа опции командной �?троки: -%s\n"
#define MSGTR_InvalidPlayEntry "�?еверный �?лемент во�?произведени�? %s\n"
#define MSGTR_NotAnMEncoderOption "-%s не �?вл�?ет�?�? опцией MEncoder\n"
#define MSGTR_NoFileGiven "�?е указан файл\n"

// m_config.c
#define MSGTR_SaveSlotTooOld "�?айден �?лишком �?тарый �?лот �?охранени�? из lvl %d: %d !!!\n"
#define MSGTR_InvalidCfgfileOption "Опци�? %s не может и�?пользовать�?�? в конфигурационном файле.\n"
#define MSGTR_InvalidCmdlineOption "Опци�? %s не может и�?пользовать�?�? в командной �?троке.\n"
#define MSGTR_InvalidSuboption "Ошибка: у опции '%s' нет �?убопции '%s'.\n"
#define MSGTR_MissingSuboptionParameter "Ошибка: у �?убопции '%s' опции '%s' должен быть параметр!\n"
#define MSGTR_MissingOptionParameter "Ошибка: у опции '%s' должен быть параметр!\n"
#define MSGTR_OptionListHeader "\n Им�?                  Тип             Минимум    Мак�?имум Общий   CL    Конф\n\n"
#define MSGTR_TotalOptions "\nВ�?его: %d опций(�?/и)\n"
#define MSGTR_ProfileInclusionTooDeep "ПРЕДУПРЕЖДЕ�?ИЕ: Включение профил�? �?лишком глубоко.\n"
#define MSGTR_NoProfileDefined "�?и один профиль не был определён.\n"
#define MSGTR_AvailableProfiles "До�?тупные профили:\n"
#define MSGTR_UnknownProfile "�?еизве�?тный профиль '%s'.\n"
#define MSGTR_Profile "Профиль %s: %s\n"

// m_property.c
#define MSGTR_PropertyListHeader "\n Им�?                  Тип             Минимум    Мак�?имум\n\n"
#define MSGTR_TotalProperties "\nВ�?его: %d �?вой�?тв\n"

// loader/ldt_keeper.c
#define MSGTR_LOADER_DYLD_Warning "В�?ИМ�?�?ИЕ: попытка и�?пользовани�? DLL кодеков без у�?тановленной переменной \n         окружени�? DYLD_BIND_AT_LAUNCH. Это �?корее в�?его приведет к краху.\n"

// ====================== GUI messages/buttons ========================

// --- labels ---
#define MSGTR_About "О �?ебе"
#define MSGTR_FileSelect "Выбрать файл..."
#define MSGTR_SubtitleSelect "Выбрать �?убтитры..."
#define MSGTR_OtherSelect "Выбор..."
#define MSGTR_AudioFileSelect "Выбор внешнего аудио канала..."
#define MSGTR_FontSelect "Выбор шрифта..."
// Note: If you change MSGTR_PlayList please see if it still fits MSGTR_MENU_PlayList
#define MSGTR_PlayList "Плейли�?т"
#define MSGTR_Equalizer "Эквалайзер"
#define MSGTR_ConfigureEqualizer "�?а�?тройка каналов"
#define MSGTR_SkinBrowser "Про�?мотрщик �?кинов"
#define MSGTR_Network "Сетевые потоки..."
// Note: If you change MSGTR_Preferences please see if it still fits MSGTR_MENU_Preferences
#define MSGTR_Preferences "�?а�?тройки"
#define MSGTR_AudioPreferences "Конфигураци�? аудио драйвера"
#define MSGTR_NoMediaOpened "�?о�?итель не открыт."
#define MSGTR_NoChapter "�?ет раздела"
#define MSGTR_Chapter "Раздел %d"
#define MSGTR_NoFileLoaded "Файл не загружен."

// --- buttons ---
#define MSGTR_Ok "Да"
#define MSGTR_Cancel "Отмена"
#define MSGTR_Add "Добавить"
#define MSGTR_Remove "Удалить"
#define MSGTR_Clear "Очи�?тить"
#define MSGTR_Config "�?а�?троить"
#define MSGTR_ConfigDriver "�?а�?троить драйвер"
#define MSGTR_Browse "Про�?мотреть"

// --- error messages ---
#define MSGTR_NEMDB "Извините, не хватает пам�?ти дл�? буфера прори�?овки."
#define MSGTR_NEMFMR "Извините, не хватает пам�?ти дл�? отображени�? меню."
#define MSGTR_IDFGCVD "Извините, не нашёл �?овме�?тимый �? GUI драйвер видео вывода."
#define MSGTR_NEEDLAVC "Извините, Вы не можете проигрывать не-MPEG файлы на Вашем DXR3/H+ у�?трой�?тве\nбез перекодировани�?. Пожалуй�?та, включите lavc при конфигурации DXR3/H+."

// --- skin loader error messages
#define MSGTR_SKIN_ERRORMESSAGE "[�?кин] ошибка в файле конфигурации шкуры на �?троке %d: %s"
#define MSGTR_SKIN_SkinFileNotFound "[�?кин] файл '%s' не найден.\n"
#define MSGTR_SKIN_SkinFileNotReadable "[�?кин] файл ( %s ) не читаем.\n"
#define MSGTR_SKIN_BITMAP_16bit  "Глубина битовой матрицы в 16 бит и меньше не поддерживает�?�? (%s).\n"
#define MSGTR_SKIN_BITMAP_FileNotFound  "Файл не найден (%s)\n"
#define MSGTR_SKIN_BITMAP_PNGReadError "ошибка чтени�? PNG (%s)\n"
#define MSGTR_SKIN_BITMAP_ConversionError "ошибка преобразовани�? 24-бит в 32-бит (%s)\n"
#define MSGTR_SKIN_UnknownMessage "неизве�?тное �?ообщение: %s\n"
#define MSGTR_SKIN_NotEnoughMemory "не хватает пам�?ти\n"
#define MSGTR_SKIN_FONT_TooManyFontsDeclared "Объ�?влено �?лишком много шрифтов.\n"
#define MSGTR_SKIN_FONT_FontFileNotFound "Файл шрифта не найден.\n"
#define MSGTR_SKIN_FONT_FontImageNotFound "Файл образов шрифта не найден.\n"
#define MSGTR_SKIN_FONT_NonExistentFont "не�?уще�?твующий идентификатор шрифта (%s)\n"
#define MSGTR_SKIN_UnknownParameter "неизве�?тный параметр (%s)\n"
#define MSGTR_SKIN_SKINCFG_SkinNotFound "Скин не найден (%s).\n"
#define MSGTR_SKIN_SKINCFG_SelectedSkinNotFound "Выбранный �?кин '%s' не найден, пробую 'default'...\n"
#define MSGTR_SKIN_LABEL "Шкуры:"

// --- GTK menus
#define MSGTR_MENU_AboutMPlayer "О MPlayer"
#define MSGTR_MENU_Open "Открыть..."
#define MSGTR_MENU_PlayFile "Играть файл..."
#define MSGTR_MENU_PlayVCD "Играть VCD..."
#define MSGTR_MENU_PlayDVD "Играть DVD..."
#define MSGTR_MENU_PlayURL "Играть URL..."
#define MSGTR_MENU_LoadSubtitle "Загрузить �?убтитры..."
#define MSGTR_MENU_DropSubtitle "Убрать �?убтитры..."
#define MSGTR_MENU_LoadExternAudioFile "Загрузить внешний аудио файл..."
#define MSGTR_MENU_Playing "Во�?произведение"
#define MSGTR_MENU_Play "Играть"
#define MSGTR_MENU_Pause "Пауза"
#define MSGTR_MENU_Stop "О�?танов"
#define MSGTR_MENU_NextStream "След. поток"
#define MSGTR_MENU_PrevStream "Пред. поток"
#define MSGTR_MENU_Size "Размер"
#define MSGTR_MENU_HalfSize   "Половинный размер"
#define MSGTR_MENU_NormalSize "�?ормальный размер"
#define MSGTR_MENU_DoubleSize "Двойной размер"
#define MSGTR_MENU_FullScreen "Полный �?кран"
#define MSGTR_MENU_DVD "DVD"
#define MSGTR_MENU_VCD "VCD"
#define MSGTR_MENU_PlayDisc "Играть ди�?к..."
#define MSGTR_MENU_ShowDVDMenu "Показать DVD меню"
#define MSGTR_MENU_Titles "Ролики"
#define MSGTR_MENU_Title "Ролик %2d"
#define MSGTR_MENU_None "(нет)"
#define MSGTR_MENU_Chapters "Разделы"
#define MSGTR_MENU_Chapter "Раздел %2d"
#define MSGTR_MENU_AudioLanguages "�?удио �?зыки"
#define MSGTR_MENU_SubtitleLanguages "Язык �?убтитров"
#define MSGTR_MENU_PlayList MSGTR_PlayList
#define MSGTR_MENU_SkinBrowser "Про�?мотр шкур"
#define MSGTR_MENU_Preferences MSGTR_Preferences
#define MSGTR_MENU_Exit "Выход"
#define MSGTR_MENU_Mute "Приглушить"
#define MSGTR_MENU_Original "И�?ходный"
#define MSGTR_MENU_AspectRatio "Соотношение �?торон"
#define MSGTR_MENU_AudioTrack "�?удио дорожка"
#define MSGTR_MENU_Track "Дорожка %d"
#define MSGTR_MENU_VideoTrack "Видео дорожка"
#define MSGTR_MENU_Subtitles "Субтитры"

// --- equalizer
// Note: If you change MSGTR_EQU_Audio please see if it still fits MSGTR_PREFERENCES_Audio
#define MSGTR_EQU_Audio "�?удио"
// Note: If you change MSGTR_EQU_Video please see if it still fits MSGTR_PREFERENCES_Video
#define MSGTR_EQU_Video "Видео"
#define MSGTR_EQU_Contrast "Контра�?т: "
#define MSGTR_EQU_Brightness "Ярко�?ть: "
#define MSGTR_EQU_Hue "Цвет: "
#define MSGTR_EQU_Saturation "�?а�?ыщенно�?ть: "
#define MSGTR_EQU_Front_Left "Передн�?�? Лева�?"
#define MSGTR_EQU_Front_Right "Передн�?�? Права�?"
#define MSGTR_EQU_Back_Left "Задн�?�? Лева�?"
#define MSGTR_EQU_Back_Right "Задн�?�? Права�?"
#define MSGTR_EQU_Center "Центральна�?"
#define MSGTR_EQU_Bass "Ба�?"
#define MSGTR_EQU_All "В�?е"
#define MSGTR_EQU_Channel1 "Канал 1:"
#define MSGTR_EQU_Channel2 "Канал 2:"
#define MSGTR_EQU_Channel3 "Канал 3:"
#define MSGTR_EQU_Channel4 "Канал 4:"
#define MSGTR_EQU_Channel5 "Канал 5:"
#define MSGTR_EQU_Channel6 "Канал 6:"

// --- playlist
#define MSGTR_PLAYLIST_Path "Путь"
#define MSGTR_PLAYLIST_Selected "Выбранные файлы"
#define MSGTR_PLAYLIST_Files "Файлы"
#define MSGTR_PLAYLIST_DirectoryTree "Дерево каталогов"

// --- preferences
#define MSGTR_PREFERENCES_Audio MSGTR_EQU_Audio
#define MSGTR_PREFERENCES_Video MSGTR_EQU_Video
#define MSGTR_PREFERENCES_SubtitleOSD "Субтитры и OSD"
#define MSGTR_PREFERENCES_Codecs "Кодеки и демультиплек�?ор"
// Note: If you change MSGTR_PREFERENCES_Misc see if it still fits MSGTR_PREFERENCES_FRAME_Misc
#define MSGTR_PREFERENCES_Misc "Разное"
#define MSGTR_PREFERENCES_None "�?ет"
#define MSGTR_PREFERENCES_DriverDefault "драйвер по умолчанию"
#define MSGTR_PREFERENCES_AvailableDrivers "До�?тупные драйверы:"
#define MSGTR_PREFERENCES_DoNotPlaySound "�?е проигрывать звук"
#define MSGTR_PREFERENCES_NormalizeSound "�?ормализовать звук"
#define MSGTR_PREFERENCES_EnableEqualizer "Включить �?квалайзер"
#define MSGTR_PREFERENCES_SoftwareMixer "Включить программный микшер"
#define MSGTR_PREFERENCES_ExtraStereo "Включить дополнительное �?терео"
#define MSGTR_PREFERENCES_Coefficient "Ко�?ффициент:"
#define MSGTR_PREFERENCES_AudioDelay "Задержка аудио"
#define MSGTR_PREFERENCES_DoubleBuffer "Включить двойную буферизацию"
#define MSGTR_PREFERENCES_DirectRender "Включить пр�?мое отображение"
#define MSGTR_PREFERENCES_FrameDrop "Включить пропу�?к кадров"
#define MSGTR_PREFERENCES_HFrameDrop "Включить И�?ТЕ�?СИВ�?ЫЙ пропу�?к кадров (опа�?но)"
#define MSGTR_PREFERENCES_Flip "Отобразить изображение вверх ногами"
#define MSGTR_PREFERENCES_Panscan "У�?ечение �?торон: "
#define MSGTR_PREFERENCES_Subtitle "Субтитры:"
#define MSGTR_PREFERENCES_SUB_Delay "Задержка: "
#define MSGTR_PREFERENCES_SUB_FPS "Кадр/�?ек:"
#define MSGTR_PREFERENCES_SUB_POS "Позици�?: "
#define MSGTR_PREFERENCES_SUB_AutoLoad "Выключить автозагрузку �?убтитров"
#define MSGTR_PREFERENCES_SUB_Unicode "Уникодовые �?убтитры"
#define MSGTR_PREFERENCES_SUB_MPSUB "Конвертировать данные �?убтитры в MPlayer'ов�?кий формат �?убтитров"
#define MSGTR_PREFERENCES_SUB_SRT "Конвертировать данные �?убтитры в о�?нованный на времени SubViewer (SRT) формат"
#define MSGTR_PREFERENCES_SUB_Overlap "Изменить перекрывание �?убтитров"
#define MSGTR_PREFERENCES_SUB_USE_ASS "SSA/ASS рендеринг �?убтитров"
#define MSGTR_PREFERENCES_SUB_ASS_USE_MARGINS "И�?польз. кра�?"
#define MSGTR_PREFERENCES_SUB_ASS_TOP_MARGIN "Верх: "
#define MSGTR_PREFERENCES_SUB_ASS_BOTTOM_MARGIN "�?из: "
#define MSGTR_PREFERENCES_Font "Шрифт:"
#define MSGTR_PREFERENCES_FontFactor "Ко�?ффициент шрифта:"
#define MSGTR_PREFERENCES_PostProcess "Включить по�?тобработку"
#define MSGTR_PREFERENCES_AutoQuality "�?вто каче�?тво: "
#define MSGTR_PREFERENCES_NI "И�?пользовать 'не�?лоёный' AVI пар�?ер"
#define MSGTR_PREFERENCES_IDX "Е�?ли требует�?�?, пере�?троить индек�?ную таблицу"
#define MSGTR_PREFERENCES_VideoCodecFamily "Семей�?тво видео кодеков:"
#define MSGTR_PREFERENCES_AudioCodecFamily "Семей�?тво аудио кодеков:"
#define MSGTR_PREFERENCES_FRAME_OSD_Level "уровень OSD"
#define MSGTR_PREFERENCES_FRAME_Subtitle "Субтитры"
#define MSGTR_PREFERENCES_FRAME_Font "Шрифт"
#define MSGTR_PREFERENCES_FRAME_PostProcess "По�?тобработка"
#define MSGTR_PREFERENCES_FRAME_CodecDemuxer "Кодек и демультиплек�?ор"
#define MSGTR_PREFERENCES_FRAME_Cache "К�?ш"
#define MSGTR_PREFERENCES_FRAME_Misc MSGTR_PREFERENCES_Misc
#define MSGTR_PREFERENCES_Audio_Device "У�?трой�?тво:"
#define MSGTR_PREFERENCES_Audio_Mixer "Микшер:"
#define MSGTR_PREFERENCES_Audio_MixerChannel "Канал микшера:"
#define MSGTR_PREFERENCES_Message "Пожалуй�?та, запомните, что Вам нужно перезапу�?тить проигрывание,\nчтобы некоторые изменени�? в�?тупили в �?илу!"
#define MSGTR_PREFERENCES_DXR3_VENC "Видео кодировщик:"
#define MSGTR_PREFERENCES_DXR3_LAVC "И�?пользовать LAVC (FFmpeg)"
#define MSGTR_PREFERENCES_FontEncoding1 "Уникод"
#define MSGTR_PREFERENCES_FontEncoding2 "Западноевропей�?кие �?зыки (ISO-8859-1)"
#define MSGTR_PREFERENCES_FontEncoding3 "Западноевропей�?кие �?зыки �? Евро (ISO-8859-15)"
#define MSGTR_PREFERENCES_FontEncoding4 "Слав�?н�?кие/Центрально-европей�?кие �?зыки (ISO-8859-2)"
#define MSGTR_PREFERENCES_FontEncoding5 "Э�?перанто, Галиций�?кий, Мальтий�?кий, Турецкий (ISO-8859-3)"
#define MSGTR_PREFERENCES_FontEncoding6 "Стара�? Балтий�?ка�? кодировка (ISO-8859-4)"
#define MSGTR_PREFERENCES_FontEncoding7 "Кириллица (ISO-8859-5)"
#define MSGTR_PREFERENCES_FontEncoding8 "�?раб�?ка�? (ISO-8859-6)"
#define MSGTR_PREFERENCES_FontEncoding9 "Современна�? Грече�?ка�? (ISO-8859-7)"
#define MSGTR_PREFERENCES_FontEncoding10 "Турецка�? (ISO-8859-9)"
#define MSGTR_PREFERENCES_FontEncoding11 "Балтий�?ка�? (ISO-8859-13)"
#define MSGTR_PREFERENCES_FontEncoding12 "Кельт�?ка�? (ISO-8859-14)"
#define MSGTR_PREFERENCES_FontEncoding13 "Еврей�?кие кодировки (ISO-8859-8)"
#define MSGTR_PREFERENCES_FontEncoding14 "Ру�?�?ка�? (KOI8-R)"
#define MSGTR_PREFERENCES_FontEncoding15 "Украин�?ка�?, Белору�?�?ка�? (KOI8-U/RU)"
#define MSGTR_PREFERENCES_FontEncoding16 "Упрощённа�? Китай�?ка�? кодировка (CP936)"
#define MSGTR_PREFERENCES_FontEncoding17 "Традиционна�? Китай�?ка�? кодировка (BIG5)"
#define MSGTR_PREFERENCES_FontEncoding18 "Япон�?кие кодировки (SHIFT-JIS)"
#define MSGTR_PREFERENCES_FontEncoding19 "Корей�?ка�? кодировка (CP949)"
#define MSGTR_PREFERENCES_FontEncoding20 "Тай�?ка�? кодировка (CP874)"
#define MSGTR_PREFERENCES_FontEncoding21 "Кириллица Window$ (CP1251)"
#define MSGTR_PREFERENCES_FontEncoding22 "Слав�?н�?кий/Центрально-европей�?кий Window$ (CP1250)"
#define MSGTR_PREFERENCES_FontEncoding23 "�?раб�?кий Windows (CP1256)"
#define MSGTR_PREFERENCES_FontNoAutoScale "�?е ма�?штабировать"
#define MSGTR_PREFERENCES_FontPropWidth "Пропорционально ширине фильма"
#define MSGTR_PREFERENCES_FontPropHeight "Пропорционально вы�?оте фильма"
#define MSGTR_PREFERENCES_FontPropDiagonal "Пропорционально диагонали фильма"
#define MSGTR_PREFERENCES_FontEncoding "Кодировка:"
#define MSGTR_PREFERENCES_FontBlur "�?ерезко�?ть:"
#define MSGTR_PREFERENCES_FontOutLine "Контуры:"
#define MSGTR_PREFERENCES_FontTextScale "Ма�?штаб тек�?та:"
#define MSGTR_PREFERENCES_FontOSDScale "Ма�?штаб OSD:"
#define MSGTR_PREFERENCES_Cache "К�?ш вкл/выкл"
#define MSGTR_PREFERENCES_CacheSize "Размер к�?ша: "
#define MSGTR_PREFERENCES_LoadFullscreen "Стартовать в полно�?кранном режиме"
#define MSGTR_PREFERENCES_SaveWinPos "Сохран�?ть позицию окна"
#define MSGTR_PREFERENCES_XSCREENSAVER "О�?танавливать XScreenSaver"
#define MSGTR_PREFERENCES_PlayBar "Включить поло�?у во�?произведени�?"
#define MSGTR_PREFERENCES_AutoSync "�?вто�?инхронизаци�? вкл/выкл"
#define MSGTR_PREFERENCES_AutoSyncValue "�?вто�?инхронизаци�?: "
#define MSGTR_PREFERENCES_CDROMDevice "CD-ROM:"
#define MSGTR_PREFERENCES_DVDDevice "DVD:"
#define MSGTR_PREFERENCES_FPS "FPS фильма:"
#define MSGTR_PREFERENCES_ShowVideoWindow "Показывать окно видео, когда неактивен"
#define MSGTR_PREFERENCES_ArtsBroken "�?овые вер�?ии aRts не�?овме�?тимы �? GTK 1.x "\
           "и приведут к фатальной ошибке GMPlayer!"

// --- aboutbox
#define MSGTR_ABOUT_UHU "Разработка GUI �?пон�?ирована UHU Linux\n"
#define MSGTR_ABOUT_Contributors "Разработчики кода и документации\n"
#define MSGTR_ABOUT_Codecs_libs_contributions "Кодеки и �?торонние библиотеки\n"
#define MSGTR_ABOUT_Translations "Переводы\n"
#define MSGTR_ABOUT_Skins "Шкуры\n"

// --- messagebox
#define MSGTR_MSGBOX_LABEL_FatalError "Фатальна�? ошибка!"
#define MSGTR_MSGBOX_LABEL_Error "Ошибка!"
#define MSGTR_MSGBOX_LABEL_Warning "Предупреждение!"

// cfg.c
#define MSGTR_UnableToSaveOption "[cfg] �?е могу �?охранить опцию '%s'.\n"

// interface.c
#define MSGTR_DeletingSubtitles "[GUI] Удал�?ю �?убтитры.\n"
#define MSGTR_LoadingSubtitles "[GUI] Загружаю �?убтитры: %s\n"
#define MSGTR_AddingVideoFilter "[GUI] Добавл�?ю видеофильтр: %s\n"

// mw.c
#define MSGTR_NotAFile "Это не похоже на файл: '%s' !\n"

// ws.c
#define MSGTR_WS_RemoteDisplay "[ws] Удалённый ди�?плей, отключаю XMITSHM.\n"
#define MSGTR_WS_NoXshm "[ws] Извините, ваша �?и�?тема не поддерживает ра�?ширение раздел�?емой пам�?ти X'ов.\n"
#define MSGTR_WS_NoXshape "[ws] Извините, ваша �?и�?тема не поддерживает ра�?ширение XShape.\n"
#define MSGTR_WS_ColorDepthTooLow "[ws] Извините, глубина цвета �?лишком мала.\n"
#define MSGTR_WS_TooManyOpenWindows "[ws] Слишком много открытых окон.\n"
#define MSGTR_WS_ShmError "[ws] ошибка ра�?ширени�? раздел�?емой пам�?ти\n"
#define MSGTR_WS_NotEnoughMemoryDrawBuffer "[ws] Извините, недо�?таточно пам�?ти дл�? буфера прори�?овки.\n"
#define MSGTR_WS_DpmsUnavailable "DPMS не до�?тупен?\n"
#define MSGTR_WS_DpmsNotEnabled "�?е могу включить DPMS.\n"

// wsxdnd.c
#define MSGTR_WS_NotAFile "Это не похоже на файл...\n"
#define MSGTR_WS_DDNothing "D&D: �?ичего не возвращено!\n"

// ======================= video output drivers ========================

#define MSGTR_VOincompCodec "Извините, выбранное у�?трой�?тво видеовывода не �?овме�?тимо �? �?тим кодеком.\n"\
                "Попробуйте добавить фильтр scale к вашему �?пи�?ку фильтров.\n"\
                "�?апример, -vf spp,scale вме�?то -vf spp.\n"
#define MSGTR_VO_GenericError "Произошла �?ледующа�? ошибка"
#define MSGTR_VO_UnableToAccess "�?е могу получить до�?туп"
#define MSGTR_VO_ExistsButNoDirectory "уже �?уще�?твует, но не �?вл�?ет�?�? директорией."
#define MSGTR_VO_DirExistsButNotWritable "Директори�? вывода уже �?уще�?твует, но не до�?тупна дл�? запи�?и."
#define MSGTR_VO_DirExistsAndIsWritable "Директори�? вывода уже �?уще�?твует и до�?тупна дл�? запи�?и."
#define MSGTR_VO_CantCreateDirectory "�?е могу �?оздать директорию вывода."
#define MSGTR_VO_CantCreateFile "�?е могу �?оздать выходной файл."
#define MSGTR_VO_DirectoryCreateSuccess "Директори�? вывода у�?пешно �?оздана."
#define MSGTR_VO_ValueOutOfRange "значение вне допу�?тимого диапазона"
#define MSGTR_VO_NoValueSpecified "Значение не указано."
#define MSGTR_VO_UnknownSuboptions "�?еизве�?тна�?(ые) �?убопци�?(и)"

// libvo/aspect.c
#define MSGTR_LIBVO_ASPECT_NoSuitableNewResFound "[ASPECT] Предупреждение: �?е найдено подход�?щего разрешени�?!\n"
#define MSGTR_LIBVO_ASPECT_NoNewSizeFoundThatFitsIntoRes "[ASPECT] Ошибка: �?е найден размер, умещающий�?�? в �?то разрешение!\n"

// font_load_ft.c
#define MSGTR_LIBVO_FONT_LOAD_FT_NewFaceFailed "Сбой New_Face. Возможно неверен путь к шрифту.\nУкажите, пожалуй�?та, файл шрифта дл�? тек�?та (~/.mplayer/subfont.ttf).\n"
#define MSGTR_LIBVO_FONT_LOAD_FT_NewMemoryFaceFailed "Сбой New_Memory_Face.\n"
#define MSGTR_LIBVO_FONT_LOAD_FT_SubFaceFailed "шрифт �?убтитров: �?бой load_sub_face.\n"
#define MSGTR_LIBVO_FONT_LOAD_FT_SubFontCharsetFailed "шрифт �?убтитров: �?бой prepare_charset.\n"
#define MSGTR_LIBVO_FONT_LOAD_FT_CannotPrepareSubtitleFont "�?евозможно подготовить шрифт �?убтитров.\n"
#define MSGTR_LIBVO_FONT_LOAD_FT_CannotPrepareOSDFont "�?евозможно подготовить шрифт OSD.\n"
#define MSGTR_LIBVO_FONT_LOAD_FT_CannotGenerateTables "�?евозможно �?генерировать таблицы.\n"
#define MSGTR_LIBVO_FONT_LOAD_FT_DoneFreeTypeFailed "Сбой FT_Done_FreeType.\n"

// sub.c
#define MSGTR_VO_SUB_Seekbar "�?авигаци�?"
#define MSGTR_VO_SUB_Play "Во�?произведение"
#define MSGTR_VO_SUB_Pause "Пауза"
#define MSGTR_VO_SUB_Stop "Стоп"
#define MSGTR_VO_SUB_Rewind "�?азад"
#define MSGTR_VO_SUB_Forward "Вперёд"
#define MSGTR_VO_SUB_Clock "Врем�?"
#define MSGTR_VO_SUB_Contrast "Контра�?т"
#define MSGTR_VO_SUB_Saturation "�?а�?ыщенно�?ть"
#define MSGTR_VO_SUB_Volume "Громко�?ть"
#define MSGTR_VO_SUB_Brightness "Ярко�?ть"
#define MSGTR_VO_SUB_Hue "Цвет"
#define MSGTR_VO_SUB_Balance "Балан�?"

// libvo/vo_3dfx.c
#define MSGTR_LIBVO_3DFX_Only16BppSupported "[VO_3DFX] Поддерживает�?�? только 16bpp!"
#define MSGTR_LIBVO_3DFX_VisualIdIs "[VO_3DFX] Визуальный ID  %lx.\n"
#define MSGTR_LIBVO_3DFX_UnableToOpenDevice "[VO_3DFX] �?е могу открыть /dev/3dfx.\n"
#define MSGTR_LIBVO_3DFX_Error "[VO_3DFX] Ошибка: %d.\n"
#define MSGTR_LIBVO_3DFX_CouldntMapMemoryArea "[VO_3DFX] �?е могу отобразить обла�?ти пам�?ти 3dfx: %p,%p,%d.\n"
#define MSGTR_LIBVO_3DFX_DisplayInitialized "[VO_3DFX] Инициализировано: %p.\n"
#define MSGTR_LIBVO_3DFX_UnknownSubdevice "[VO_3DFX] �?еизве�?тное поду�?трой�?тво: %s.\n"

// vo_aa.c
#define MSGTR_VO_AA_HelpHeader "\n\nСубопции vo_aa библиотеки aalib:\n"
#define MSGTR_VO_AA_AdditionalOptions "Дополнительные опции, предо�?тавл�?емые vo_aa:\n" \
"  help        выве�?ти �?то �?ообщение\n" \
"  osdcolor    у�?тановить цвет OSD\n  subcolor    у�?тановить цвет �?убтитров\n" \
"        параметры цвета:\n           0 : нормальный\n" \
"           1 : ту�?клый\n           2 : жирный\n           3 : жирный шрифт\n" \
"           4 : инвер�?ный\n           5 : �?пециальный\n\n\n"

// libvo/vo_dxr3.c
#define MSGTR_LIBVO_DXR3_UnableToLoadNewSPUPalette "[VO_DXR3] �?е могу загрузить новую палитру SPU!\n"
#define MSGTR_LIBVO_DXR3_UnableToSetPlaymode "[VO_DXR3] �?е могу задать режим во�?произведени�?!\n"
#define MSGTR_LIBVO_DXR3_UnableToSetSubpictureMode "[VO_DXR3] �?е могу задать режим �?убкартинки!\n"
#define MSGTR_LIBVO_DXR3_UnableToGetTVNorm "[VO_DXR3] �?е могу получить режим ТВ!\n"
#define MSGTR_LIBVO_DXR3_AutoSelectedTVNormByFrameRate "[VO_DXR3] �?втовыбор режима ТВ по ча�?тоте кадров: "
#define MSGTR_LIBVO_DXR3_UnableToSetTVNorm "[VO_DXR3] �?е могу у�?тановить режим ТВ!\n"
#define MSGTR_LIBVO_DXR3_SettingUpForNTSC "[VO_DXR3] У�?танавливаю дл�? NTSC.\n"
#define MSGTR_LIBVO_DXR3_SettingUpForPALSECAM "[VO_DXR3] У�?танавливаю дл�? PAL/SECAM.\n"
#define MSGTR_LIBVO_DXR3_SettingAspectRatioTo43 "[VO_DXR3] У�?танавливаю пропорции 4:3.\n"
#define MSGTR_LIBVO_DXR3_SettingAspectRatioTo169 "[VO_DXR3] У�?танавливаю пропорции 16:9.\n"
#define MSGTR_LIBVO_DXR3_OutOfMemory "[VO_DXR3] нехватка пам�?ти\n"
#define MSGTR_LIBVO_DXR3_UnableToAllocateKeycolor "[VO_DXR3] �?е могу назначить ключевой цвет!\n"
#define MSGTR_LIBVO_DXR3_UnableToAllocateExactKeycolor "[VO_DXR3] �?е могу назначить точный ключевой цвет,\nи�?пользую ближайшее приближение (0x%lx).\n"
#define MSGTR_LIBVO_DXR3_Uninitializing "[VO_DXR3] Деинициализаци�?.\n"
#define MSGTR_LIBVO_DXR3_FailedRestoringTVNorm "[VO_DXR3] �?е могу во�?�?тановить режим ТВ!\n"
#define MSGTR_LIBVO_DXR3_EnablingPrebuffering "[VO_DXR3] Включаю предварительную буферизацию.\n"
#define MSGTR_LIBVO_DXR3_UsingNewSyncEngine "[VO_DXR3] И�?пользую новый механизм �?инхронизации.\n"
#define MSGTR_LIBVO_DXR3_UsingOverlay "[VO_DXR3] И�?пользую оверлей.\n"
#define MSGTR_LIBVO_DXR3_ErrorYouNeedToCompileMplayerWithX11 "[VO_DXR3] Ошибка: Дл�? оверле�? необходима компил�?ци�? �? у�?тановленными\nбиблиотеками/заголовками X11.\n"
#define MSGTR_LIBVO_DXR3_WillSetTVNormTo "[VO_DXR3] У�?тановлю режим ТВ в: "
#define MSGTR_LIBVO_DXR3_AutoAdjustToMovieFrameRatePALPAL60 "авторегулировка к ча�?тоте кадров фильма (PAL/PAL-60)"
#define MSGTR_LIBVO_DXR3_AutoAdjustToMovieFrameRatePALNTSC "авторегулировка к ча�?тоте кадров фильма (PAL/NTSC)"
#define MSGTR_LIBVO_DXR3_UseCurrentNorm "И�?пользую текущий режим."
#define MSGTR_LIBVO_DXR3_UseUnknownNormSuppliedCurrentNorm "Предложен неизве�?тный режим. И�?пользую текущий режим."
#define MSGTR_LIBVO_DXR3_ErrorOpeningForWritingTrying "[VO_DXR3] Ошибка открыти�? %s дл�? запи�?и, пробую /dev/em8300 взамен.\n"
#define MSGTR_LIBVO_DXR3_ErrorOpeningForWritingTryingMV "[VO_DXR3] Ошибка открыти�? %s дл�? запи�?и, пробую /dev/em8300_mv взамен.\n"
#define MSGTR_LIBVO_DXR3_ErrorOpeningForWritingAsWell "[VO_DXR3] Также ошибка открыти�? /dev/em8300 дл�? запи�?и!\nВыхожу.\n"
#define MSGTR_LIBVO_DXR3_ErrorOpeningForWritingAsWellMV "[VO_DXR3] Также ошибка открыти�? /dev/em8300_mv дл�? запи�?и!\nВыхожу.\n"
#define MSGTR_LIBVO_DXR3_Opened "[VO_DXR3] Открыто: %s.\n"
#define MSGTR_LIBVO_DXR3_ErrorOpeningForWritingTryingSP "[VO_DXR3] Ошибка открыти�? %s дл�? запи�?и, пробую /dev/em8300_sp взамен.\n"
#define MSGTR_LIBVO_DXR3_ErrorOpeningForWritingAsWellSP "[VO_DXR3] Также ошибка открыти�? /dev/em8300_sp дл�? запи�?и!\nВыхожу.\n"
#define MSGTR_LIBVO_DXR3_UnableToOpenDisplayDuringHackSetup "[VO_DXR3] �?е могу открыть ди�?плей в проце�?�?е у�?тановки хака оверле�?!\n"
#define MSGTR_LIBVO_DXR3_UnableToInitX11 "[VO_DXR3] �?е могу инициализировать X11!\n"
#define MSGTR_LIBVO_DXR3_FailedSettingOverlayAttribute "[VO_DXR3] �?е могу у�?тановить атрибут оверле�?.\n"
#define MSGTR_LIBVO_DXR3_FailedSettingOverlayScreen "[VO_DXR3] �?е могу у�?тановить �?кран оверле�?!\nВыхожу.\n"
#define MSGTR_LIBVO_DXR3_FailedEnablingOverlay "[VO_DXR3] �?е могу активировать оверлей!\nВыхожу.\n"
#define MSGTR_LIBVO_DXR3_FailedResizingOverlayWindow "[VO_DXR3] �?е могу изменить размер окна оверле�?!\n"
#define MSGTR_LIBVO_DXR3_FailedSettingOverlayBcs "[VO_DXR3] �?е могу у�?тановить bcs оверле�?!\n"
#define MSGTR_LIBVO_DXR3_FailedGettingOverlayYOffsetValues "[VO_DXR3] �?е могу получить значени�? Y-�?мещений оверле�?!\nВыхожу.\n"
#define MSGTR_LIBVO_DXR3_FailedGettingOverlayXOffsetValues "[VO_DXR3] �?е могу получить значени�? X-�?мещений оверле�?!\nВыхожу.\n"
#define MSGTR_LIBVO_DXR3_FailedGettingOverlayXScaleCorrection "[VO_DXR3] �?е могу получить коррекцию на ма�?штабирование по X оверле�?!\nВыхожу.\n"
#define MSGTR_LIBVO_DXR3_YOffset "[VO_DXR3] �?мещение по Y: %d.\n"
#define MSGTR_LIBVO_DXR3_XOffset "[VO_DXR3] �?мещение по X: %d.\n"
#define MSGTR_LIBVO_DXR3_XCorrection "[VO_DXR3] коррекци�? по X: %d.\n"
#define MSGTR_LIBVO_DXR3_FailedSetSignalMix "[VO_DXR3] �?е могу у�?тановить �?игнал mix!\n"

// vo_jpeg.c
#define MSGTR_VO_JPEG_ProgressiveJPEG "Прогре�?�?ивный JPEG включен."
#define MSGTR_VO_JPEG_NoProgressiveJPEG "Прогре�?�?ивный JPEG выключен."
#define MSGTR_VO_JPEG_BaselineJPEG "Базовый JPEG включен."
#define MSGTR_VO_JPEG_NoBaselineJPEG "Базовый JPEG выключен."

// libvo/vo_mga.c
#define MSGTR_LIBVO_MGA_AspectResized "[VO_MGA] aspect(): изменён размер в %dx%d.\n"
#define MSGTR_LIBVO_MGA_Uninit "[VO] деинициализаци�?!\n"

// mga_template.c
#define MSGTR_LIBVO_MGA_ErrorInConfigIoctl "[MGA] ошибка в mga_vid_config ioctl (неверна�? вер�?и�? mga_vid.o?)"
#define MSGTR_LIBVO_MGA_CouldNotGetLumaValuesFromTheKernelModule "[MGA] �?е могу получить значени�? люмы из модул�? �?дра!\n"
#define MSGTR_LIBVO_MGA_CouldNotSetLumaValuesFromTheKernelModule "[MGA] �?е могу у�?тановить значени�? люмы из модул�? �?дра!\n"
#define MSGTR_LIBVO_MGA_ScreenWidthHeightUnknown "[MGA] Ширина/вы�?ота �?крана неизве�?тны!\n"
#define MSGTR_LIBVO_MGA_InvalidOutputFormat "[MGA] неверный выходной формат %0X\n"
#define MSGTR_LIBVO_MGA_IncompatibleDriverVersion "[MGA] Вер�?и�? вашего драйвера mga_vid не�?овме�?тима �? �?той вер�?ией MPlayer!\n"
#define MSGTR_LIBVO_MGA_CouldntOpen "[MGA] �?е могу открыть: %s\n"
#define MSGTR_LIBVO_MGA_ResolutionTooHigh "[MGA] Разрешение и�?точника, по крайней мере в одном измерении, больше чем\n1023x1023. Перема�?штабируйте программно или и�?пользуйте -lavdopts lowres=1\n"
#define MSGTR_LIBVO_MGA_mgavidVersionMismatch "[MGA] не�?оответ�?твие вер�?ий драйвера mga_vid �?дра (%u) и MPlayer (%u)\n"

// libvo/vo_null.c
#define MSGTR_LIBVO_NULL_UnknownSubdevice "[VO_NULL] �?еизве�?тное поду�?трой�?тво: %s.\n"

// libvo/vo_png.c
#define MSGTR_LIBVO_PNG_Warning1 "[VO_PNG] Предупреждение: уровень �?жати�? у�?тановлен в 0, �?жатие отключено!\n"
#define MSGTR_LIBVO_PNG_Warning2 "[VO_PNG] Информаци�?: И�?пользуйте -vo png:z=<n> дл�? у�?тановки уровн�?\n�?жати�? от 0 до 9.\n"
#define MSGTR_LIBVO_PNG_Warning3 "[VO_PNG] Информаци�?: (0 = нет �?жати�?, 1 = бы�?трое, худшее - 9 лучшее,\n�?амое медленное �?жатие)\n"
#define MSGTR_LIBVO_PNG_ErrorOpeningForWriting "\n[VO_PNG] Ошибка открыти�? '%s' дл�? запи�?и!\n"
#define MSGTR_LIBVO_PNG_ErrorInCreatePng "[VO_PNG] Ошибка в create_png.\n"

// vo_pnm.c
#define MSGTR_VO_PNM_ASCIIMode "Режим ASCII включен."
#define MSGTR_VO_PNM_RawMode "'Сырой' режим включен."
#define MSGTR_VO_PNM_PPMType "Будут запи�?аны PPM файлы."
#define MSGTR_VO_PNM_PGMType "Будут запи�?аны PGM файлы."
#define MSGTR_VO_PNM_PGMYUVType "Будут запи�?аны PGMYUV файлы."

// libvo/vo_sdl.c
#define MSGTR_LIBVO_SDL_CouldntGetAnyAcceptableSDLModeForOutput "[VO_SDL] �?е могу получить какой-либо приемлемый режим SDL дл�? вывода.\n"
#define MSGTR_LIBVO_SDL_SetVideoModeFailed "[VO_SDL] set_video_mode: Сбой SDL_SetVideoMode: %s.\n"
#define MSGTR_LIBVO_SDL_MappingI420ToIYUV "[VO_SDL] Отображение I420 в IYUV.\n"
#define MSGTR_LIBVO_SDL_UnsupportedImageFormat "[VO_SDL] �?еподдерживаемый формат изображени�? (0x%X).\n"
#define MSGTR_LIBVO_SDL_InfoPleaseUseVmOrZoom "[VO_SDL] Информаци�?: и�?пользуйте -vm или -zoom дл�? переключени�?\nв лучшее разрешение.\n"
#define MSGTR_LIBVO_SDL_FailedToSetVideoMode "[VO_SDL] �?е могу у�?тановить видеорежим: %s.\n"
#define MSGTR_LIBVO_SDL_CouldntCreateAYUVOverlay "[VO_SDL] �?е могу �?оздать оверлей YUV: %s.\n"
#define MSGTR_LIBVO_SDL_CouldntCreateARGBSurface "[VO_SDL] �?е могу �?оздать поверхно�?ть RGB: %s.\n"
#define MSGTR_LIBVO_SDL_UsingDepthColorspaceConversion "[VO_SDL] И�?пользую преобразование глубины/цветового про�?тран�?тва,\n�?то замедлит работу (%ibpp -> %ibpp).\n"
#define MSGTR_LIBVO_SDL_UnsupportedImageFormatInDrawslice "[VO_SDL] �?еподдерживаемый формат изображени�? в draw_slice,\n�?в�?жите�?ь �? разработчиками MPlayer!\n"
#define MSGTR_LIBVO_SDL_BlitFailed "[VO_SDL] Сбой blit: %s.\n"
#define MSGTR_LIBVO_SDL_InitializationFailed "[VO_SDL] Сбой инициализации SDL: %s.\n"
#define MSGTR_LIBVO_SDL_UsingDriver "[VO_SDL] И�?пользую драйвер: %s.\n"

// libvo/vo_svga.c
#define MSGTR_LIBVO_SVGA_ForcedVidmodeNotAvailable "[VO_SVGA] Фор�?ированный vid_mode %d (%s) не до�?тупен.\n"
#define MSGTR_LIBVO_SVGA_ForcedVidmodeTooSmall "[VO_SVGA] Фор�?ированный vid_mode %d (%s) �?лишком мал.\n"
#define MSGTR_LIBVO_SVGA_Vidmode "[VO_SVGA] Vid_mode: %d, %dx%d %dbpp.\n"
#define MSGTR_LIBVO_SVGA_VgasetmodeFailed "[VO_SVGA] Сбой Vga_setmode(%d).\n"
#define MSGTR_LIBVO_SVGA_VideoModeIsLinearAndMemcpyCouldBeUsed "[VO_SVGA] Видеорежим линейный и дл�? передачи изображени�? может быть\nи�?пользован memcpy.\n"
#define MSGTR_LIBVO_SVGA_VideoModeHasHardwareAcceleration "[VO_SVGA] Видеорежим обладает аппаратным у�?корением и может быть\nи�?пользован put_image.\n"
#define MSGTR_LIBVO_SVGA_IfItWorksForYouIWouldLikeToKnow "[VO_SVGA] Е�?ли �?то работает у Ва�?, мне хотело�?ь бы знать.\n[VO_SVGA] (отправьте лог �? помощью `mplayer test.avi -v -v -v -v &> svga.log`).\nСпа�?ибо!\n"
#define MSGTR_LIBVO_SVGA_VideoModeHas "[VO_SVGA] У видеорежима %d �?траниц(а/ы).\n"
#define MSGTR_LIBVO_SVGA_CenteringImageStartAt "[VO_SVGA] Центрирую изображение. �?ачинаю в (%d,%d)\n"
#define MSGTR_LIBVO_SVGA_UsingVidix "[VO_SVGA] И�?пользую VIDIX. w=%i h=%i  mw=%i mh=%i\n"

// libvo/vo_tdfx_vid.c
#define MSGTR_LIBVO_TDFXVID_Move "[VO_TDXVID] Шаг %d(%d) x %d => %d.\n"
#define MSGTR_LIBVO_TDFXVID_AGPMoveFailedToClearTheScreen "[VO_TDFXVID] Сбой очи�?тки �?крана шагом AGP.\n"
#define MSGTR_LIBVO_TDFXVID_BlitFailed "[VO_TDFXVID] Сбой blit.\n"
#define MSGTR_LIBVO_TDFXVID_NonNativeOverlayFormatNeedConversion "[VO_TDFXVID] Дл�? незаложенных форматов оверле�? нужна конвер�?и�?.\n"
#define MSGTR_LIBVO_TDFXVID_UnsupportedInputFormat "[VO_TDFXVID] �?еподдерживаемый входной формат 0x%x.\n"
#define MSGTR_LIBVO_TDFXVID_OverlaySetupFailed "[VO_TDFXVID] Сбой у�?тановки оверле�?.\n"
#define MSGTR_LIBVO_TDFXVID_OverlayOnFailed "[VO_TDFXVID] Сбой включени�? оверле�?.\n"
#define MSGTR_LIBVO_TDFXVID_OverlayReady "[VO_TDFXVID] Оверлей готов: %d(%d) x %d @ %d => %d(%d) x %d @ %d.\n"
#define MSGTR_LIBVO_TDFXVID_TextureBlitReady "[VO_TDFXVID] Готов blit тек�?тур: %d(%d) x %d @ %d => %d(%d) x %d @ %d.\n"
#define MSGTR_LIBVO_TDFXVID_OverlayOffFailed "[VO_TDFXVID] Сбой выключени�? оверле�?\n"
#define MSGTR_LIBVO_TDFXVID_CantOpen "[VO_TDFXVID] �?е могу открыть %s: %s.\n"
#define MSGTR_LIBVO_TDFXVID_CantGetCurrentCfg "[VO_TDFXVID] �?е могу получить текущую конфигурацию: %s.\n"
#define MSGTR_LIBVO_TDFXVID_MemmapFailed "[VO_TDFXVID] Сбой memmap !!!!!\n"
#define MSGTR_LIBVO_TDFXVID_GetImageTodo "�?ужно доделать получение изображени�?.\n"
#define MSGTR_LIBVO_TDFXVID_AgpMoveFailed "[VO_TDFXVID] Сбой шага AGP.\n"
#define MSGTR_LIBVO_TDFXVID_SetYuvFailed "[VO_TDFXVID] Сбой у�?тановки YUV.\n"
#define MSGTR_LIBVO_TDFXVID_AgpMoveFailedOnYPlane "[VO_TDFXVID] Сбой шага AGP на проекции Y.\n"
#define MSGTR_LIBVO_TDFXVID_AgpMoveFailedOnUPlane "[VO_TDFXVID] Сбой шага AGP на проекции U.\n"
#define MSGTR_LIBVO_TDFXVID_AgpMoveFailedOnVPlane "[VO_TDFXVID] Сбой шага AGP на проекции V.\n"
#define MSGTR_LIBVO_TDFXVID_UnknownFormat "[VO_TDFXVID] неизве�?тный формат: 0x%x.\n"

// libvo/vo_tdfxfb.c
#define MSGTR_LIBVO_TDFXFB_CantOpen "[VO_TDFXFB] �?е могу открыть %s: %s.\n"
#define MSGTR_LIBVO_TDFXFB_ProblemWithFbitgetFscreenInfo "[VO_TDFXFB] Проблема �? ioctl FBITGET_FSCREENINFO: %s.\n"
#define MSGTR_LIBVO_TDFXFB_ProblemWithFbitgetVscreenInfo "[VO_TDFXFB] Проблема �? ioctl FBITGET_VSCREENINFO: %s.\n"
#define MSGTR_LIBVO_TDFXFB_ThisDriverOnlySupports "[VO_TDFXFB] Этот драйвер поддерживает только 3Dfx Banshee, Voodoo3 и Voodoo 5.\n"
#define MSGTR_LIBVO_TDFXFB_OutputIsNotSupported "[VO_TDFXFB] %d bpp вывод не поддерживает�?�?.\n"
#define MSGTR_LIBVO_TDFXFB_CouldntMapMemoryAreas "[VO_TDFXFB] �?е могу отобразить обла�?ти пам�?ти: %s.\n"
#define MSGTR_LIBVO_TDFXFB_BppOutputIsNotSupported "[VO_TDFXFB] %d bpp вывод не поддерживает�?�?\n(Этого никогда не должно прои�?ходить).\n"
#define MSGTR_LIBVO_TDFXFB_SomethingIsWrongWithControl "[VO_TDFXFB] Ой! Что-то не в пор�?дке �? control().\n"
#define MSGTR_LIBVO_TDFXFB_NotEnoughVideoMemoryToPlay "[VO_TDFXFB] �?едо�?таточно пам�?ти дл�? во�?произведени�? �?того фильма.\nПопробуйте меньшее разрешение.\n"
#define MSGTR_LIBVO_TDFXFB_ScreenIs "[VO_TDFXFB] Экран %dx%d и %d bpp, входные данные %dx%d и %d bpp, режим %dx%d.\n"

// libvo/vo_tga.c
#define MSGTR_LIBVO_TGA_UnknownSubdevice "[VO_TGA] �?еизве�?тное поду�?трой�?тво: %s.\n"

// libvo/vo_vesa.c
#define MSGTR_LIBVO_VESA_FatalErrorOccurred "[VO_VESA] Возникла фатальна�? ошибка! �?е могу продолжить.\n"
#define MSGTR_LIBVO_VESA_UnknownSubdevice "[VO_VESA] неизве�?тное поду�?трой�?тво: '%s'.\n"
#define MSGTR_LIBVO_VESA_YouHaveTooLittleVideoMemory "[VO_VESA] У Ва�? �?лишком мало видеопам�?ти дл�? �?того режима:\n[VO_VESA] �?еобходимо: %08lX имеет�?�?: %08lX.\n"
#define MSGTR_LIBVO_VESA_YouHaveToSpecifyTheCapabilitiesOfTheMonitor "[VO_VESA] Вам нужно указать возможно�?ти монитора. �?е измен�?ю ча�?тоту обновлени�?.\n"
#define MSGTR_LIBVO_VESA_UnableToFitTheMode "[VO_VESA] Режим не �?оответ�?твует ограничени�?м монитора. �?е измен�?ю ча�?тоту обновлени�?.\n"
#define MSGTR_LIBVO_VESA_DetectedInternalFatalError "[VO_VESA] Обнаружена внутренн�?�? фатальна�? ошибка: init вызван до preinit.\n"
#define MSGTR_LIBVO_VESA_SwitchFlipIsNotSupported "[VO_VESA] Опци�? -flip не поддерживает�?�?.\n"
#define MSGTR_LIBVO_VESA_PossibleReasonNoVbe2BiosFound "[VO_VESA] Возможна�? причина: VBE2 BIOS не найден.\n"
#define MSGTR_LIBVO_VESA_FoundVesaVbeBiosVersion "[VO_VESA] �?айден VESA VBE BIOS, вер�?и�? %x.%x, ревизи�?: %x.\n"
#define MSGTR_LIBVO_VESA_VideoMemory "[VO_VESA] Видеопам�?ть: %u Кб.\n"
#define MSGTR_LIBVO_VESA_Capabilites "[VO_VESA] Возможно�?ти VESA: %s %s %s %s %s.\n"
#define MSGTR_LIBVO_VESA_BelowWillBePrintedOemInfo "[VO_VESA] !!! �?иже будет выведена информаци�? OEM !!!\n"
#define MSGTR_LIBVO_VESA_YouShouldSee5OemRelatedLines "[VO_VESA] Вы должны ниже видеть 5 �?трок информации OEM.\nЕ�?ли нет, у ва�? неи�?правна�? vm86.\n"
#define MSGTR_LIBVO_VESA_OemInfo "[VO_VESA] Информаци�? OEM: %s.\n"
#define MSGTR_LIBVO_VESA_OemRevision "[VO_VESA] Ревизи�? OEM: %x.\n"
#define MSGTR_LIBVO_VESA_OemVendor "[VO_VESA] По�?тавщик OEM: %s.\n"
#define MSGTR_LIBVO_VESA_OemProductName "[VO_VESA] �?аименование продукта OEM: %s.\n"
#define MSGTR_LIBVO_VESA_OemProductRev "[VO_VESA] Ревизи�? продукта OEM: %s.\n"
#define MSGTR_LIBVO_VESA_Hint "[VO_VESA] Под�?казка: Дл�? работы ТВ-выхода Вам необходимо подключить разъём ТВ\n"\
"[VO_VESA] до загрузки, т.к. VESA BIOS инициализирует�?�? только в проце�?�?е POST.\n"
#define MSGTR_LIBVO_VESA_UsingVesaMode "[VO_VESA] И�?пользую режим VESA (%u) = %x [%ux%u@%u]\n"
#define MSGTR_LIBVO_VESA_CantInitializeSwscaler "[VO_VESA] �?е могу инициализировать программное ма�?штабирование.\n"
#define MSGTR_LIBVO_VESA_CantUseDga "[VO_VESA] �?е могу и�?пользовать DGA. Фор�?ирую режим коммутации �?егментов. :(\n"
#define MSGTR_LIBVO_VESA_UsingDga "[VO_VESA] И�?пользую DGA (физиче�?кие ре�?ур�?ы: %08lXh, %08lXh)"
#define MSGTR_LIBVO_VESA_CantUseDoubleBuffering "[VO_VESA] �?е могу и�?пользовать двойную буферизацию: недо�?таточно видеопам�?ти.\n"
#define MSGTR_LIBVO_VESA_CantFindNeitherDga "[VO_VESA] �?е могу найти ни DGA ни переме�?тимого фрейма окна.\n"
#define MSGTR_LIBVO_VESA_YouveForcedDga "[VO_VESA] Вы фор�?ировали DGA. Выхожу\n"
#define MSGTR_LIBVO_VESA_CantFindValidWindowAddress "[VO_VESA] �?е могу найти допу�?тимый адре�? окна.\n"
#define MSGTR_LIBVO_VESA_UsingBankSwitchingMode "[VO_VESA] И�?пользую режим коммутации �?егментов\n(физиче�?кие ре�?ур�?ы: %08lXh, %08lXh).\n"
#define MSGTR_LIBVO_VESA_CantAllocateTemporaryBuffer "[VO_VESA] �?е могу выделить временный буфер.\n"
#define MSGTR_LIBVO_VESA_SorryUnsupportedMode "[VO_VESA] Извините, неподдерживаемый режим -- попробуйте -x 640 -zoom.\n"
#define MSGTR_LIBVO_VESA_OhYouReallyHavePictureOnTv "[VO_VESA] О, у Ва�? дей�?твительно е�?ть изображение на ТВ!\n"
#define MSGTR_LIBVO_VESA_CantInitialozeLinuxVideoOverlay "[VO_VESA] �?е могу инициализировать Видеоверлей Linux.\n"
#define MSGTR_LIBVO_VESA_UsingVideoOverlay "[VO_VESA] И�?пользую видеооверлей: %s.\n"
#define MSGTR_LIBVO_VESA_CantInitializeVidixDriver "[VO_VESA] �?е могу инициализировать драйвер VIDIX.\n"
#define MSGTR_LIBVO_VESA_UsingVidix "[VO_VESA] И�?пользую VIDIX.\n"
#define MSGTR_LIBVO_VESA_CantFindModeFor "[VO_VESA] �?е могу найти режим дл�?: %ux%u@%u.\n"
#define MSGTR_LIBVO_VESA_InitializationComplete "[VO_VESA] Инициализаци�? VESA завершена.\n"

// libvo/vesa_lvo.c
#define MSGTR_LIBVO_VESA_ThisBranchIsNoLongerSupported "[VESA_LVO] Эта ветка больше не поддерживает�?�?.\n[VESA_LVO] И�?пользуйте -vo vesa:vidix взамен.\n"
#define MSGTR_LIBVO_VESA_CouldntOpen "[VESA_LVO] �?е могу открыть: '%s'\n"
#define MSGTR_LIBVO_VESA_InvalidOutputFormat "[VESA_LVI] �?еверный выходной формат: %s(%0X)\n"
#define MSGTR_LIBVO_VESA_IncompatibleDriverVersion "[VESA_LVO] Вер�?и�? вашего драйвера fb_vid не�?овме�?тима �? �?той вер�?ией MPlayer!\n"

// libvo/vo_x11.c
#define MSGTR_LIBVO_X11_DrawFrameCalled "[VO_X11] Вызван draw_frame()!!!!!!\n"

// libvo/vo_xv.c
#define MSGTR_LIBVO_XV_DrawFrameCalled "[VO_XV] Вызван draw_frame()!!!!!!\n"
#define MSGTR_LIBVO_XV_SharedMemoryNotSupported "[VO_XV] Раздел�?ема�? пам�?ть не поддерживает�?�?\nВозврат к обычному Xv.\n"
#define MSGTR_LIBVO_XV_XvNotSupportedByX11 "[VO_XV] Извините, Xv не поддерживает�?�? �?той вер�?ией/драйвером X11\n[VO_XV] ******** Попробуйте -vo x11  или  -vo sdl  *********\n"
#define MSGTR_LIBVO_XV_XvQueryAdaptorsFailed  "[VO_XV] Ошибка XvQueryAdaptors.\n"
#define MSGTR_LIBVO_XV_InvalidPortParameter "[VO_XV] �?еверный параметр port, переопределение параметром port 0.\n"
#define MSGTR_LIBVO_XV_CouldNotGrabPort "[VO_XV] �?евозможно захватить порт %i.\n"
#define MSGTR_LIBVO_XV_CouldNotFindFreePort "[VO_XV] �?евозможно найти �?вободный Xvideo порт - возможно другое приложение\n"\
"[VO_XV] уже его и�?пользует. Закройте в�?е программы видео, и попытайте�?ь �?нова. Е�?ли �?то\n"\
"[VO_XV] не помогает, �?мотрите другие (не-xv) драйверы в выводе 'mplayer -vo help'.\n"
#define MSGTR_LIBVO_XV_NoXvideoSupport "[VO_XV] Похоже, ваша карта не имеет поддержки Xvideo.\n"\
"[VO_XV] Запу�?тите 'xvinfo', чтобы проверить поддержку Xv и прочтите \n"\
"[VO_XV] DOCS/HTML/ru/video.html#xv!\n"\
"[VO_XV] Другие (не-xv) драйверы ищите в выводе 'mplayer -vo help'.\n"\
"[VO_XV] Попробуйте -vo x11.\n"
#define MSGTR_VO_XV_ImagedimTooHigh "Размеры изображени�? и�?точника �?лишком велики: %ux%u (мак�?имум %ux%u)\n"

// vo_yuv4mpeg.c
#define MSGTR_VO_YUV4MPEG_InterlacedHeightDivisibleBy4 "Дл�? режима c чере�?�?трочной развёрткой необходимо, чтобы вы�?ота изображени�?\nделила�?ь на 4."
#define MSGTR_VO_YUV4MPEG_InterlacedLineBufAllocFail "�?е могу выделить пам�?ть дл�? линейного буфера в режиме чере�?�?трочной развёртки."
#define MSGTR_VO_YUV4MPEG_WidthDivisibleBy2 "Ширина изображени�? должна делить�?�? на 2."
#define MSGTR_VO_YUV4MPEG_OutFileOpenError "�?е могу выделить пам�?ть или файловый опи�?атель дл�? запи�?и \"%s\"!"
#define MSGTR_VO_YUV4MPEG_OutFileWriteError "Ошибка запи�?и изображени�? в вывод!"
#define MSGTR_VO_YUV4MPEG_UnknownSubDev "�?еизве�?тное поду�?трой�?тво: %s"
#define MSGTR_VO_YUV4MPEG_InterlacedTFFMode "И�?пользую режим вывода �? чере�?�?трочной развёрткой, верхнее поле первое."
#define MSGTR_VO_YUV4MPEG_InterlacedBFFMode "И�?пользую режим вывода �? чере�?�?трочной развёрткой, нижнее поле первое."
#define MSGTR_VO_YUV4MPEG_ProgressiveMode "И�?пользую (по умолчанию) режим по�?трочной развёртки кадров."

// vosub_vidix.c
#define MSGTR_LIBVO_SUB_VIDIX_CantStartPlayback "[VO_SUB_VIDIX] �?е могу начать во�?произведение: %s\n"
#define MSGTR_LIBVO_SUB_VIDIX_CantStopPlayback "[VO_SUB_VIDIX] �?е могу о�?тановить во�?произведение: %s\n"
#define MSGTR_LIBVO_SUB_VIDIX_InterleavedUvForYuv410pNotSupported "[VO_SUB_VIDIX] Слоёный UV дл�? YUV410P не поддерживает�?�?.\n"
#define MSGTR_LIBVO_SUB_VIDIX_DummyVidixdrawsliceWasCalled "[VO_SUB_VIDIX] Был вызван фиктивный vidix_draw_slice().\n"
#define MSGTR_LIBVO_SUB_VIDIX_DummyVidixdrawframeWasCalled "[VO_SUB_VIDIX] Был вызван фиктивный vidix_draw_frame().\n"
#define MSGTR_LIBVO_SUB_VIDIX_UnsupportedFourccForThisVidixDriver "[VO_SUB_VIDIX] �?еподдерживаемый FourCC дл�? �?того драйвера VIDIX: %x (%s).\n"
#define MSGTR_LIBVO_SUB_VIDIX_VideoServerHasUnsupportedResolution "[VO_SUB_VIDIX] У �?ервера видео разрешение (%dx%d) не поддерживает�?�?,\nподдерживаемое: %dx%d-%dx%d.\n"
#define MSGTR_LIBVO_SUB_VIDIX_VideoServerHasUnsupportedColorDepth "[VO_SUB_VIDIX] Сервер видео не поддерживает глубину цвета vidix (%d).\n"
#define MSGTR_LIBVO_SUB_VIDIX_DriverCantUpscaleImage "[VO_SUB_VIDIX] Драйвер VIDIX не может увеличить изображение (%d%d -> %d%d).\n"
#define MSGTR_LIBVO_SUB_VIDIX_DriverCantDownscaleImage "[VO_SUB_VIDIX] Драйвер VIDIX не может уменьшить изображение (%d%d -> %d%d).\n"
#define MSGTR_LIBVO_SUB_VIDIX_CantConfigurePlayback "[VO_SUB_VIDIX] �?е могу на�?троить во�?произведение: %s.\n"
#define MSGTR_LIBVO_SUB_VIDIX_YouHaveWrongVersionOfVidixLibrary "[VO_SUB_VIDIX] У Ва�? неверна�? вер�?и�? библиотеки VIDIX.\n"
#define MSGTR_LIBVO_SUB_VIDIX_CouldntFindWorkingVidixDriver "[VO_SUB_VIDIX] �?е могу найти работающий драйвер VIDIX.\n"
#define MSGTR_LIBVO_SUB_VIDIX_CouldntGetCapability "[VO_SUB_VIDIX] �?е могу получить возможно�?ть: %s.\n"

// x11_common.c
#define MSGTR_EwmhFullscreenStateFailed "\nX11: �?е могу по�?лать �?обытие EWMH fullscreen!\n"
#define MSGTR_CouldNotFindXScreenSaver "xscreensaver_disable: �?е могу найти окно XScreenSaver'а.\n"
#define MSGTR_SelectedVideoMode "XF86VM: Выбран видеорежим %dx%d дл�? размера изображени�? %dx%d.\n"

#define MSGTR_InsertingAfVolume "[Микшер] �?ет аппаратного микшировани�?, в�?тавл�?ю фильтр громко�?ти.\n"
#define MSGTR_NoVolume "[Микшер] Контроль громко�?ти не до�?тупен.\n"
#define MSGTR_NoBalance "[Микшер] Управление балан�?ом недо�?тупно.\n"

// old vo drivers that have been replaced
#define MSGTR_VO_PGM_HasBeenReplaced "Драйвер видеовывода pgm был заменён -vo pnm:pgmyuv.\n"
#define MSGTR_VO_MD5_HasBeenReplaced "Драйвер видеовывода md5 был заменён -vo md5sum.\n"

// ======================= audio output drivers ========================

// audio_out.c
#define MSGTR_AO_ALSA9_1x_Removed "аудиовывод: модули alsa9 и alsa1x были удалены, и�?пользуйте -ao alsa взамен.\n"
#define MSGTR_AO_NoSuchDriver "От�?ут�?твует аудио драйвер '%.*s'\n"
#define MSGTR_AO_FailedInit "�?е могу инициализировать аудио драйвер '%s'\n"

// ao_oss.c
#define MSGTR_AO_OSS_CantOpenMixer "[AO OSS] инициализаци�? аудио: �?е могу открыть у�?трой�?тво микшера %s: %s\n"
#define MSGTR_AO_OSS_ChanNotFound "[AO OSS] инициализаци�? аудио: У микшера аудиокарты от�?ут�?твует канал '%s',\nи�?пользую канал по умолчанию.\n"
#define MSGTR_AO_OSS_CantOpenDev "[AO OSS] инициализаци�? аудио: �?е могу открыть аудиоу�?трой�?тво %s: %s\n"
#define MSGTR_AO_OSS_CantMakeFd "[AO OSS] инициализаци�? аудио: �?е могу заблокировать файловый опи�?атель: %s\n"
#define MSGTR_AO_OSS_CantSet "[AO OSS] �?е могу у�?тановить аудиоу�?трой�?тво %s в %s вывод, пробую %s...\n"
#define MSGTR_AO_OSS_CantSetChans "[AO OSS] инициализаци�? аудио: �?е могу у�?тановить аудиоу�?трой�?тво\nв %d-канальный режим.\n"
#define MSGTR_AO_OSS_CantUseGetospace "[AO OSS] инициализаци�? аудио: драйвер не поддерживает SNDCTL_DSP_GETOSPACE :-(\n"
#define MSGTR_AO_OSS_CantUseSelect "[AO OSS]\n   ***  Ваш аудиодрайвер �?Е поддерживает select()  ***\n Перекомпилируйте MPlayer �? #undef HAVE_AUDIO_SELECT в config.h !\n\n"
#define MSGTR_AO_OSS_CantReopen "[AO OSS] Фатальна�? ошибка:\n*** �?Е МОГУ ПОВТОР�?О ОТКРЫТЬ / СБРОСИТЬ �?УДИОУСТРОЙСТВО (%s) ***\n"
#define MSGTR_AO_OSS_UnknownUnsupportedFormat "[AO OSS] �?еизве�?тный/�?еподдерживаемый формат OSS: %x.\n"

// ao_arts.c
#define MSGTR_AO_ARTS_CantInit "[AO ARTS] %s\n"
#define MSGTR_AO_ARTS_ServerConnect "[AO ARTS] Соединил�?�? �? звуковым �?ервером.\n"
#define MSGTR_AO_ARTS_CantOpenStream "[AO ARTS] �?е могу открыть поток.\n"
#define MSGTR_AO_ARTS_StreamOpen "[AO ARTS] Поток открыт.\n"
#define MSGTR_AO_ARTS_BufferSize "[AO ARTS] размер буфера: %d\n"

// ao_dxr2.c
#define MSGTR_AO_DXR2_SetVolFailed "[AO DXR2] �?е могу у�?тановить громко�?ть в %d.\n"
#define MSGTR_AO_DXR2_UnsupSamplerate "[AO DXR2] dxr2: %d Гц не поддерживает�?�?, попробуйте изменить\nча�?тоту ди�?кретизации.\n"

// ao_esd.c
#define MSGTR_AO_ESD_CantOpenSound "[AO ESD] Выполнить esd_open_sound не удало�?ь: %s\n"
#define MSGTR_AO_ESD_LatencyInfo "[AO ESD] задержка: [�?ервер: %0.2fs, �?еть: %0.2fs] (под�?тройка %0.2fs)\n"
#define MSGTR_AO_ESD_CantOpenPBStream "[AO ESD] не могу открыть поток во�?произведени�? esd: %s\n"

// ao_mpegpes.c
#define MSGTR_AO_MPEGPES_CantSetMixer "[AO MPEGPES] DVB аудио: не могу у�?тановить микшер: %s\n"
#define MSGTR_AO_MPEGPES_UnsupSamplerate "[AO MPEGPES] %d Гц не поддерживает�?�?, попробуйте изменить\nча�?тоту ди�?кретизации.\n"

// ao_pcm.c
#define MSGTR_AO_PCM_FileInfo "[AO PCM] Файл: %s (%s)\nPCM: Ча�?тота ди�?кретизации: %i Гц Каналы: %s Формат %s\n"
#define MSGTR_AO_PCM_HintInfo "[AO PCM] Информаци�?: наиболее бы�?трый дамп до�?тигает�?�? �? -vc null -vo null\n[AO PCM]: Информаци�?: -ao pcm:fast. Дл�? запи�?и WAVE файлов и�?пользуйте\n[AO PCM]: Информаци�?: -ao pcm:waveheader (по умолчанию).\n"
#define MSGTR_AO_PCM_CantOpenOutputFile "[AO PCM] �?е могу открыть %s дл�? запи�?и!\n"

// ao_sdl.c
#define MSGTR_AO_SDL_INFO "[AO SDL] Ча�?тота ди�?кретизации: %i Гц Каналы: %s Формат %s\n"
#define MSGTR_AO_SDL_DriverInfo "[AO SDL] и�?пользую %s аудиодрайвер.\n"
#define MSGTR_AO_SDL_UnsupportedAudioFmt "[AO SDL] �?еподдерживаемый аудиоформат: 0x%x.\n"
#define MSGTR_AO_SDL_CantInit "[AO SDL] �?е могу инициализировать SDL аудио: %s\n"
#define MSGTR_AO_SDL_CantOpenAudio "[AO SDL] �?е могу открыть аудио: %s\n"

// ao_sgi.c
#define MSGTR_AO_SGI_INFO "[AO SGI] управление.\n"
#define MSGTR_AO_SGI_InitInfo "[AO SGI] инициализаци�?: Ча�?тота ди�?кретизации: %i Гц Каналы: %s Формат %s\n"
#define MSGTR_AO_SGI_InvalidDevice "[AO SGI] во�?произведение: неверное у�?трой�?тво.\n"
#define MSGTR_AO_SGI_CantSetParms_Samplerate "[AO SGI] инициализаци�?: ошибка у�?тановки параметров: %s\n�?е могу у�?тановить требуемую ча�?тоту ди�?кретизации.\n"
#define MSGTR_AO_SGI_CantSetAlRate "[AO SGI] инициализаци�?: AL_RATE не до�?тупен на заданном ре�?ур�?е.\n"
#define MSGTR_AO_SGI_CantGetParms "[AO SGI] инициализаци�?: ошибка получени�? параметров: %s\n"
#define MSGTR_AO_SGI_SampleRateInfo "[AO SGI] инициализаци�?: ча�?тота ди�?кретизации теперь %f (требуема�? ча�?тота %f)\n"
#define MSGTR_AO_SGI_InitConfigError "[AO SGI] инициализаци�?: %s\n"
#define MSGTR_AO_SGI_InitOpenAudioFailed "[AO SGI] инициализаци�?: �?е могу открыть канал аудио: %s\n"
#define MSGTR_AO_SGI_Uninit "[AO SGI] деинициализаци�?: ...\n"
#define MSGTR_AO_SGI_Reset "[AO SGI] �?бро�?: ...\n"
#define MSGTR_AO_SGI_PauseInfo "[AO SGI] пауза аудио: ...\n"
#define MSGTR_AO_SGI_ResumeInfo "[AO SGI] возобновление аудио: ...\n"

// ao_sun.c
#define MSGTR_AO_SUN_RtscSetinfoFailed "[AO SUN] rtsc: Выполнить SETINFO не удало�?ь.\n"
#define MSGTR_AO_SUN_RtscWriteFailed "[AO SUN] rtsc: запи�?ь не удала�?ь."
#define MSGTR_AO_SUN_CantOpenAudioDev "[AO SUN] �?е могу открыть аудиоу�?трой�?тво %s, %s -> нет звука.\n"
#define MSGTR_AO_SUN_UnsupSampleRate "[AO SUN] инициализаци�? аудио: ваша карта не поддерживает канал %d,\n%s, ча�?тоту ди�?кретизации %d Гц.\n"
#define MSGTR_AO_SUN_CantUseSelect "[AO SUN]\n   ***  Ваш аудиодрайвер �?Е поддерживает select()  ***\nПерекомпилируйте MPlayer �? #undef HAVE_AUDIO_SELECT в config.h !\n\n"
#define MSGTR_AO_SUN_CantReopenReset "[AO SUN] Фатальна�? ошибка:\n*** �?Е МОГУ ПОВТОР�?О ОТКРЫТЬ / СБРОСИТЬ �?УДИОУСТРОЙСТВО (%s) ***\n"

// ao_alsa.c
#define MSGTR_AO_ALSA_InvalidMixerIndexDefaultingToZero "[AO_ALSA] �?еверный индек�? микшера. Возврат к 0.\n"
#define MSGTR_AO_ALSA_MixerOpenError "[AO_ALSA] Ошибка открыти�? микшера: %s\n"
#define MSGTR_AO_ALSA_MixerAttachError "[AO_ALSA] Ошибка при�?оединени�? %s к микшеру: %s\n"
#define MSGTR_AO_ALSA_MixerRegisterError "[AO_ALSA] Ошибка реги�?трации микшера: %s\n"
#define MSGTR_AO_ALSA_MixerLoadError "[AO_ALSA] Ошибка загрузки микшера: %s\n"
#define MSGTR_AO_ALSA_UnableToFindSimpleControl "[AO_ALSA] �?евозможно найти про�?той �?лемент управлени�? '%s',%i.\n"
#define MSGTR_AO_ALSA_ErrorSettingLeftChannel "[AO_ALSA] Ошибка у�?тановки левого канала, %s\n"
#define MSGTR_AO_ALSA_ErrorSettingRightChannel "[AO_ALSA] Ошибка у�?тановки правого канала, %s\n"
#define MSGTR_AO_ALSA_CommandlineHelp "\n[AO_ALSA] под�?казка командной �?троки -ao alsa:\n"\
"[AO_ALSA] Пример: mplayer -ao alsa:device=hw=0.3\n"\
"[AO_ALSA]   У�?танавливает четвертое у�?трой�?тво первой карты.\n\n"\
"[AO_ALSA] Опции:\n"\
"[AO_ALSA]   noblock\n"\
"[AO_ALSA]     Открывает у�?трой�?тво в неблокирующем режиме.\n"\
"[AO_ALSA]   device=<им�?-у�?трой�?тва>\n"\
"[AO_ALSA]     У�?танавливает у�?трой�?тво (замените , на . и : на =)\n"
#define MSGTR_AO_ALSA_ChannelsNotSupported "[AO_ALSA] Количе�?тво каналов %d не поддерживает�?�?.\n"
#define MSGTR_AO_ALSA_OpenInNonblockModeFailed "[AO_ALSA] Ошибка открыти�? в неблокирующем режиме, попытка открыть в режиме блокировани�?.\n"
#define MSGTR_AO_ALSA_PlaybackOpenError "[AO_ALSA] Ошибка начала во�?произведени�?: %s\n"
#define MSGTR_AO_ALSA_ErrorSetBlockMode "[AL_ALSA] Ошибка у�?тановки блокирующего режима: %s.\n"
#define MSGTR_AO_ALSA_UnableToGetInitialParameters "[AO_ALSA] �?евозможно получить начальные параметры: %s\n"
#define MSGTR_AO_ALSA_UnableToSetAccessType "[AO_ALSA] �?евозможно у�?тановить тип до�?тупа: %s\n"
#define MSGTR_AO_ALSA_FormatNotSupportedByHardware "[AO_ALSA] Формат %s не поддерживает�?�? оборудованием, пробуем значение по-умолчанию.\n"
#define MSGTR_AO_ALSA_UnableToSetFormat "[AO_ALSA] �?евозможно у�?тановить формат: %s\n"
#define MSGTR_AO_ALSA_UnableToSetChannels "[AO_ALSA] �?евозможно у�?тановить количе�?тво каналов: %s\n"
#define MSGTR_AO_ALSA_UnableToDisableResampling "[AO_ALSA] �?евозможно отключить ре�?�?мплинг: %s\n"
#define MSGTR_AO_ALSA_UnableToSetSamplerate2 "[AO_ALSA] �?евозможно у�?тановить ча�?тоту ди�?кретизации-2: %s\n"
#define MSGTR_AO_ALSA_UnableToSetBufferTimeNear "[AO_ALSA] �?евозможно у�?тановить примерное врем�? буфера: %s\n"
#define MSGTR_AO_ALSA_UnableToGetPeriodSize "[AO ALSA] �?евозможно получить врем�? периода: %s\n"
#define MSGTR_AO_ALSA_UnableToSetPeriods "[AO_ALSA] �?евозможно у�?тановить периоды: %s\n"
#define MSGTR_AO_ALSA_UnableToSetHwParameters "[AO_ALSA] �?евозможно у�?тановить hw-параметры: %s\n"
#define MSGTR_AO_ALSA_UnableToGetBufferSize "[AO_ALSA] �?евозможно получить размер буфера: %s\n"
#define MSGTR_AO_ALSA_UnableToGetSwParameters "[AO_ALSA] �?евозможно получить sw-параметры: %s\n"
#define MSGTR_AO_ALSA_UnableToSetSwParameters "[AO_ALSA] �?евозможно у�?тановить sw-параметры: %s\n"
#define MSGTR_AO_ALSA_UnableToGetBoundary "[AO_ALSA] �?евозможно получить границу: %s\n"
#define MSGTR_AO_ALSA_UnableToSetStartThreshold "[AO_ALSA] �?евозможно у�?тановить порог запу�?ка: %s\n"
#define MSGTR_AO_ALSA_UnableToSetStopThreshold "[AO_ALSA] �?евозможно у�?тановить порог о�?танова: %s\n"
#define MSGTR_AO_ALSA_UnableToSetSilenceSize "[AO_ALSA] �?евозможно у�?тановить величину тишины: %s\n"
#define MSGTR_AO_ALSA_PcmCloseError "[AO_ALSA] ошибка pcm close: %s\n"
#define MSGTR_AO_ALSA_NoHandlerDefined "[AO_ALSA] �?е определен обработчик!\n"
#define MSGTR_AO_ALSA_PcmPrepareError "[AO_ALSA] ошибка pcm prepare: %s\n"
#define MSGTR_AO_ALSA_PcmPauseError "[AO_ALSA] ошибка pcm pause: %s\n"
#define MSGTR_AO_ALSA_PcmDropError "[AO_ALSA] ошибка pcm drop: %s\n"
#define MSGTR_AO_ALSA_PcmResumeError "[AO_ALSA] ошибка pcm resume: %s\n"
#define MSGTR_AO_ALSA_DeviceConfigurationError "[AO_ALSA] Ошибка на�?тройки у�?трой�?тва."
#define MSGTR_AO_ALSA_PcmInSuspendModeTryingResume "[AO_ALSA] Pcm в ждущем режиме, попытка продолжить.\n"
#define MSGTR_AO_ALSA_WriteError "[AO_ALSA] ошибка запи�?и: %s\n"
#define MSGTR_AO_ALSA_TryingToResetSoundcard "[AO_ALSA] попытка �?бро�?ить звуковую карту.\n"
#define MSGTR_AO_ALSA_CannotGetPcmStatus "[AO_ALSA] �?евозможно получить pcm �?тату�?: %s\n"

// ao_plugin.c
#define MSGTR_AO_PLUGIN_InvalidPlugin "[AO ПЛ�?ГИ�?] неверный плагин: %s\n"

// ======================= audio filters ================================

// af_scaletempo.c
#define MSGTR_AF_ValueOutOfRange MSGTR_VO_ValueOutOfRange

// af_ladspa.c
#define MSGTR_AF_LADSPA_AvailableLabels "до�?тупные метки в"
#define MSGTR_AF_LADSPA_WarnNoInputs "ПРЕДУПРЕЖДЕ�?ИЕ! У �?того LADSPA плагина от�?ут�?твуют аудиовходы.\n  Вход�?щий �?игнал аудио будет потер�?н."
#define MSGTR_AF_LADSPA_ErrMultiChannel "Мультиканальные (>2) плагины пока что не поддерживают�?�?.\n  И�?пользуйте только моно- и �?тереоплагины."
#define MSGTR_AF_LADSPA_ErrNoOutputs "У �?того LADSPA плагина от�?ут�?твуют выходы аудио."
#define MSGTR_AF_LADSPA_ErrInOutDiff "Чи�?ло аудиовходов и аудиовыходов у LADSPA плагина отличает�?�?."
#define MSGTR_AF_LADSPA_ErrFailedToLoad "не могу загрузить"
#define MSGTR_AF_LADSPA_ErrNoDescriptor "�?е могу найти функцию ladspa_descriptor() в указанном файле библиотеки."
#define MSGTR_AF_LADSPA_ErrLabelNotFound "�?е могу найти метку в библиотеке плагина."
#define MSGTR_AF_LADSPA_ErrNoSuboptions "�?е указаны �?убопции."
#define MSGTR_AF_LADSPA_ErrNoLibFile "�?е указан файл библиотеки."
#define MSGTR_AF_LADSPA_ErrNoLabel "�?е указана метка фильтра."
#define MSGTR_AF_LADSPA_ErrNotEnoughControls "�?едо�?таточно на�?троек указано в командной �?троке."
#define MSGTR_AF_LADSPA_ErrControlBelow "%s: Входной параметр #%d меньше нижней границы %0.4f.\n"
#define MSGTR_AF_LADSPA_ErrControlAbove "%s: Входной параметр #%d больше верхней границы %0.4f.\n"

// format.c
#define MSGTR_AF_FORMAT_UnknownFormat "неизве�?тный формат "

// ========================== INPUT =========================================

// joystick.c
#define MSGTR_INPUT_JOYSTICK_CantOpen "�?е могу отрыть у�?трой�?тво джой�?тика %s: %s\n"
#define MSGTR_INPUT_JOYSTICK_ErrReading "Ошибка чтени�? у�?трой�?тва джой�?тика: %s\n"
#define MSGTR_INPUT_JOYSTICK_LoosingBytes "Джой�?тик: Потер�?но %d байт(а/ов) данных\n"
#define MSGTR_INPUT_JOYSTICK_WarnLostSync "Джой�?тик: предупреждение о �?обытии инициализации,\nпотер�?на �?инхронизаци�? �? драйвером.\n"
#define MSGTR_INPUT_JOYSTICK_WarnUnknownEvent "Джой�?тик: предупреждение о неизве�?тном типе �?обыти�? %d\n"

// appleir.c
#define MSGTR_INPUT_APPLE_IR_CantOpen "�?евозможно открыть у�?трой�?тво Apple IR: %s\n"

// input.c
#define MSGTR_INPUT_INPUT_ErrCantRegister2ManyCmdFds "Слишком много опи�?ателей файлов команд,\nне могу зареги�?трировать файловый опи�?атель %d.\n"
#define MSGTR_INPUT_INPUT_ErrCantRegister2ManyKeyFds "Слишком много опи�?ателей файлов клавиш,\nне могу зареги�?трировать файловый опи�?атель %d.\n"
#define MSGTR_INPUT_INPUT_ErrArgMustBeInt "Команда %s: аргумент %d не целое чи�?ло.\n"
#define MSGTR_INPUT_INPUT_ErrArgMustBeFloat "Команда %s: аргумент %d не веще�?твенный.\n"
#define MSGTR_INPUT_INPUT_ErrUnterminatedArg "Команда %s: аргумент %d не определён.\n"
#define MSGTR_INPUT_INPUT_ErrUnknownArg "�?еизве�?тный аргумент %d\n"
#define MSGTR_INPUT_INPUT_Err2FewArgs "Команда %s требует не менее %d аргументов, мы нашли пока только %d.\n"
#define MSGTR_INPUT_INPUT_ErrReadingCmdFd "Ошибка чтени�? опи�?ател�? %d файла команд: %s\n"
#define MSGTR_INPUT_INPUT_ErrCmdBufferFullDroppingContent "Командный буфер файлового опи�?ател�? %d полон: пропу�?каю �?одержимое.\n"
#define MSGTR_INPUT_INPUT_ErrInvalidCommandForKey "�?еверна�? команда дл�? прив�?зки к клавише %s"
#define MSGTR_INPUT_INPUT_ErrSelect "Ошибка вызова select: %s\n"
#define MSGTR_INPUT_INPUT_ErrOnKeyInFd "Ошибка в файловом опи�?ателе %d клавиш ввода\n"
#define MSGTR_INPUT_INPUT_ErrDeadKeyOnFd "Фатальна�? ошибка клавиши ввода в файловом опи�?ателе %d\n"
#define MSGTR_INPUT_INPUT_Err2ManyKeyDowns "Слишком много �?обытий одновременного нажати�? клавиш\n"
#define MSGTR_INPUT_INPUT_ErrOnCmdFd "Ошибка в опи�?ателе %d файла команд\n"
#define MSGTR_INPUT_INPUT_ErrReadingInputConfig "Ошибка чтени�? конфигурационного файла ввода %s: %s\n"
#define MSGTR_INPUT_INPUT_ErrUnknownKey "�?еизве�?тна�? клавиша '%s'\n"
#define MSGTR_INPUT_INPUT_ErrUnfinishedBinding "�?еоконченна�? прив�?зка %s\n"
#define MSGTR_INPUT_INPUT_ErrBuffer2SmallForKeyName "Буфер �?лишком мал дл�? названи�? �?той клавиши: %s\n"
#define MSGTR_INPUT_INPUT_ErrNoCmdForKey "�?е найдено команды дл�? клавиши %s"
#define MSGTR_INPUT_INPUT_ErrBuffer2SmallForCmd "Буфер �?лишком мал дл�? команды %s\n"
#define MSGTR_INPUT_INPUT_ErrWhyHere "Что мы зде�?ь делаем?\n"
#define MSGTR_INPUT_INPUT_ErrCantInitJoystick "�?е могу инициализировать джой�?тик ввода\n"
#define MSGTR_INPUT_INPUT_ErrCantOpenFile "�?е могу выполнить open %s: %s\n"
#define MSGTR_INPUT_INPUT_ErrCantInitAppleRemote "�?е могу инициализировать Пульт ДУ Apple Remote.\n"

// lirc.c
#define MSGTR_LIRCopenfailed "�?еудачное открытие поддержки LIRC.\nВы не �?можете и�?пользовать Ваш пульт управлени�?.\n"
#define MSGTR_LIRCcfgerr "�?еудачна�? попытка чтени�? файла конфигурации LIRC '%s'!\n"

// ========================== LIBMPDEMUX ===================================

// muxer.c, muxer_*.c
#define MSGTR_TooManyStreams "Слишком много потоков!"
#define MSGTR_RawMuxerOnlyOneStream "Мультиплек�?ор rawaudio поддерживает только один аудиопоток!\n"
#define MSGTR_IgnoringVideoStream "Игнорирую видеопоток!\n"
#define MSGTR_UnknownStreamType "Предупреждение, неизве�?тный тип потока: %d\n"
#define MSGTR_WarningLenIsntDivisible "Предупреждение: длина не кратна размеру образца!\n"
#define MSGTR_MuxbufMallocErr "Мультиплек�?ор фреймбуфера не может выделить пам�?ть (malloc)!\n"
#define MSGTR_MuxbufReallocErr "Мультиплек�?ор фреймбуфера не может перера�?пределить пам�?ть (realloc)!\n"
#define MSGTR_WritingHeader "Запи�?ь заголовка...\n"
#define MSGTR_WritingTrailer "Запи�?ь индек�?а...\n"

// demuxer.c, demux_*.c
#define MSGTR_AudioStreamRedefined "ПРЕДУПРЕЖДЕ�?ИЕ: Заголовок аудио потока %d переопределён!\n"
#define MSGTR_VideoStreamRedefined "ПРЕДУПРЕЖДЕ�?ИЕ: Заголовок видео потока %d переопределён!\n"
#define MSGTR_TooManyAudioInBuffer "\nСлишком много (%d в %d байтах) аудиопакетов в буфере!\n"
#define MSGTR_TooManyVideoInBuffer "\nСлишком много (%d в %d байтах) видеопакетов в буфере!\n"
#define MSGTR_MaybeNI "Возможно Вы проигрываете 'не�?лоёный' поток/файл или неудачный кодек?\n" \
                      "Дл�? AVI файлов попробуйте фор�?ировать 'не�?лоёный' режим опцией -ni.\n"
#define MSGTR_WorkAroundBlockAlignHeaderBug "AVI: Обход ошибки CBR-MP3 nBlockAlign заголовка!\n"
#define MSGTR_SwitchToNi "\nОбнаружен плохо '�?лоёный' AVI файл - переключаю�?ь в -ni режим...\n"
#define MSGTR_InvalidAudioStreamNosound "AVI: неверный ID аудио потока: %d - игнорирование (без звука)\n"
#define MSGTR_InvalidAudioStreamUsingDefault "AVI: неверный ID видео потока: %d - игнорирование (и�?пользует�?�? значение по-умолчанию)\n"
#define MSGTR_ON2AviFormat "ON2 AVI формат"
#define MSGTR_Detected_XXX_FileFormat "Обнаружен %s формат файла!\n"
#define MSGTR_DetectedAudiofile "Обнаружен аудиофайл.\n"
#define MSGTR_InvalidMPEGES "�?едопу�?тимый MPEG-ES поток??? �?в�?жите�?ь �? автором, �?то может быть ошибкой :(\n"
#define MSGTR_FormatNotRecognized "======= Извините, формат �?того файла не ра�?познан/не поддерживает�?�? ==========\n"\
                                  "===== Е�?ли �?то AVI, ASF или MPEG поток, пожалуй�?та �?в�?жите�?ь �? автором! ======\n"
#define MSGTR_SettingProcessPriority "У�?танавливаю приоритет проце�?�?а: %s\n"
#define MSGTR_FilefmtFourccSizeFpsFtime "[V] формат:%d  fourcc:0x%X  размер:%dx%d  fps:%5.3f  ftime:=%6.4f\n"
#define MSGTR_CannotInitializeMuxer "�?евозможно инициализировать мультиплек�?ор."
#define MSGTR_MissingVideoStream "Видеопоток не найден!\n"
#define MSGTR_MissingAudioStream "�?удиопоток не найден -> без звука\n"
#define MSGTR_MissingVideoStreamBug "Видеопоток потер�?н!? �?в�?жите�?ь �? автором, �?то может быть ошибкой :(\n"

#define MSGTR_DoesntContainSelectedStream "demux: в файле нет выбранного аудио или видеопотока\n"

#define MSGTR_NI_Forced "Фор�?ирован"
#define MSGTR_NI_Detected "Обнаружен"
#define MSGTR_NI_Message "%s '�?ЕСЛО�?�?ЫЙ' формат AVI файла!\n"

#define MSGTR_UsingNINI "И�?пользование '�?ЕСЛО�?�?ОГО' и�?порченного формата AVI файла!\n"
#define MSGTR_CouldntDetFNo "�?е �?мог определить чи�?ло кадров (дл�? аб�?олютного перемещени�?).\n"
#define MSGTR_CantSeekRawAVI "�?е могу переме�?тить�?�? в �?ыром потоке AVI!\n(требует�?�? индек�?, попробуйте �? ключом -idx!)\n"
#define MSGTR_CantSeekFile "�?е могу перемещать�?�? в �?том файле!\n"

#define MSGTR_MOVcomprhdr "MOV: Дл�? поддержки �?жатых заголовков необходим zlib!\n"
#define MSGTR_MOVvariableFourCC "MOV: Предупреждение! Обнаружен переменный FOURCC!?\n"
#define MSGTR_MOVtooManyTrk "MOV: Предупреждение! �?лишком много треков!"
#define MSGTR_DetectedTV "�?айден ТВ! ;-)\n"
#define MSGTR_ErrorOpeningOGGDemuxer "�?е могу открыть демультиплек�?ор ogg.\n"
#define MSGTR_CannotOpenAudioStream "�?е могу открыть аудиопоток: %s\n"
#define MSGTR_CannotOpenSubtitlesStream "�?е могу открыть поток �?убтитров: %s\n"
#define MSGTR_OpeningAudioDemuxerFailed "�?е могу открыть демультиплек�?ор аудио: %s\n"
#define MSGTR_OpeningSubtitlesDemuxerFailed "�?е могу открыть демультиплек�?ор �?убтитров: %s\n"
#define MSGTR_TVInputNotSeekable "По ТВ входу нельз�? перемещать�?�?! (Возможно, перемещение будет дл�? �?мены каналов ;)\n"
#define MSGTR_DemuxerInfoChanged "Информаци�? демультиплек�?ора %s изменена в %s\n"
#define MSGTR_ClipInfo "Информаци�? о клипе:\n"

#define MSGTR_LeaveTelecineMode "\ndemux_mpg: обнаружено 30000/1001 кадров/�?ек NTSC �?одержимое, переключаю ча�?тоту кадров.\n"
#define MSGTR_EnterTelecineMode "\ndemux_mpg: обнаружено 24000/1001 кадра/�?ек NTSC �?одержимое �? по�?трочной развёрткой,\nпереключаю ча�?тоту кадров.\n"

#define MSGTR_CacheFill "\rЗаполнение к�?ша: %5.2f%% (%"PRId64" байт(а))   "
#define MSGTR_NoBindFound "�?е найдена прив�?зка к клавише '%s'.\n"
#define MSGTR_FailedToOpen "�?е могу открыть %s.\n"

#define MSGTR_VideoID "[%s] �?айден видео поток, -vid %d\n"
#define MSGTR_AudioID "[%s] �?айден аудио поток, -aid %d\n"
#define MSGTR_SubtitleID "[%s] �?айден поток �?убтитров, -sid %d\n"

// asfheader.c
#define MSGTR_MPDEMUX_ASFHDR_HeaderSizeOver1MB "Ф�?Т�?ЛЬ�?�?Я ОШИБК�?: размер заголовка более 1 MB (%d)!\nПожалуй�?та, обратите�?ь к разработчикам MPlayer и загрузите/отошлите �?тот файл.\n"
#define MSGTR_MPDEMUX_ASFHDR_HeaderMallocFailed "�?е могу выделить %d байт(а/ов) дл�? заголовка.\n"
#define MSGTR_MPDEMUX_ASFHDR_EOFWhileReadingHeader "EOF при чтении заголовка ASF, и�?порченный/неполный файл?\n"
#define MSGTR_MPDEMUX_ASFHDR_DVRWantsLibavformat "DVR, веро�?тно, будет работать только �? libavformat,\nпопробуйте -demuxer 35, е�?ли у Ва�? е�?ть проблемы\n"
#define MSGTR_MPDEMUX_ASFHDR_NoDataChunkAfterHeader "�?ет звена данных, �?ледующих за заголовком!\n"
#define MSGTR_MPDEMUX_ASFHDR_AudioVideoHeaderNotFound "ASF: не найден аудио или видео заголовок - и�?порченный файл?\n"
#define MSGTR_MPDEMUX_ASFHDR_InvalidLengthInASFHeader "�?еверна�? длина в заголовке ASF!\n"
#define MSGTR_MPDEMUX_ASFHDR_DRMLicenseURL "URL DRM лицензии: %s\n"
#define MSGTR_MPDEMUX_ASFHDR_DRMProtected "Файл был обременен DRM шифрованием, он не будет во�?производить�?�? в MPlayer!\n"

// aviheader.c
#define MSGTR_MPDEMUX_AVIHDR_EmptyList "** пу�?той �?пи�?ок?!\n"
#define MSGTR_MPDEMUX_AVIHDR_WarnNotExtendedAVIHdr "** Предупреждение: Это не ра�?ширенный заголовок AVI..\n"
#define MSGTR_MPDEMUX_AVIHDR_BuildingODMLidx "AVI: ODML: По�?троение индек�?а ODML (%d звеньев �?упериндек�?а).\n"
#define MSGTR_MPDEMUX_AVIHDR_BrokenODMLfile "AVI: ODML: Обнаружен плохой (неполный?) файл. И�?пользую традиционный индек�?.\n"
#define MSGTR_MPDEMUX_AVIHDR_CantReadIdxFile "�?е могу прочитать файл индек�?а %s: %s\n"
#define MSGTR_MPDEMUX_AVIHDR_NotValidMPidxFile "%s не �?вл�?ет�?�? корректным файлом индек�?а MPlayer.\n"
#define MSGTR_MPDEMUX_AVIHDR_FailedMallocForIdxFile "�?е могу выделить пам�?ть дл�? данных индек�?а из %s.\n"
#define MSGTR_MPDEMUX_AVIHDR_PrematureEOF "преждевременный конец индек�?ного файла %s\n"
#define MSGTR_MPDEMUX_AVIHDR_IdxFileLoaded "Загружаю индек�?ный файл: %s\n"
#define MSGTR_MPDEMUX_AVIHDR_GeneratingIdx "Создаю Индек�?: %3lu %s     \r"
#define MSGTR_MPDEMUX_AVIHDR_IdxGeneratedForHowManyChunks "AVI: Создана индек�?на�? таблица дл�? %d звеньев!\n"
#define MSGTR_MPDEMUX_AVIHDR_Failed2WriteIdxFile "�?е могу запи�?ать файл индек�?а %s: %s\n"
#define MSGTR_MPDEMUX_AVIHDR_IdxFileSaved "Сохранён индек�?ный файл: %s\n"

// open.c, stream.c
#define MSGTR_CdDevNotfound "CD-ROM '%s' не найден!\n"
#define MSGTR_ErrTrackSelect "Ошибка выбора дорожки VCD!"
#define MSGTR_ReadSTDIN "Чтение из stdin (�?о �?тандартного входа)...\n"
#define MSGTR_UnableOpenURL "�?е могу открыть URL: %s\n"
#define MSGTR_ConnToServer "Соединение �? �?ервером: %s\n"
#define MSGTR_FileNotFound "Файл не найден: '%s'\n"

#define MSGTR_SMBInitError "�?е могу инициализировать библиотеку libsmbclient: %d\n"
#define MSGTR_SMBFileNotFound "�?е могу открыть по �?ети: '%s'\n"
#define MSGTR_SMBNotCompiled "MPlayer не был �?компилирован �? поддержкой чтени�? SMB.\n"

#define MSGTR_CantOpenDVD "�?е могу открыть DVD: %s (%s)\n"

// network.c
#define MSGTR_MPDEMUX_NW_UnknownAF "�?еизве�?тное �?емей�?тво адре�?ов %d\n"
#define MSGTR_MPDEMUX_NW_ResolvingHostForAF "Разрешаю %s дл�? %s...\n"
#define MSGTR_MPDEMUX_NW_CantResolv "�?е могу разрешить им�? дл�? %s: %s\n"
#define MSGTR_MPDEMUX_NW_ConnectingToServer "Соедин�?ю�?ь �? �?ервером %s[%s]: %d...\n"
#define MSGTR_MPDEMUX_NW_CantConnect2Server "�?е могу �?оединит�?�? �? �?ервером: %s\n"
#define MSGTR_MPDEMUX_NW_SelectFailed "Select не удал�?�?.\n"
#define MSGTR_MPDEMUX_NW_ConnTimeout "таймаут �?оединени�?\n"
#define MSGTR_MPDEMUX_NW_GetSockOptFailed "getsockopt не удал�?�?: %s\n"
#define MSGTR_MPDEMUX_NW_ConnectError "ошибка �?оединени�?: %s\n"
#define MSGTR_MPDEMUX_NW_InvalidProxySettingTryingWithout "�?еверные на�?тройки прок�?и... пробую без прок�?и.\n"
#define MSGTR_MPDEMUX_NW_CantResolvTryingWithoutProxy "�?е могу разрешить удалённое им�? дл�? AF_INET. Пробую без прок�?и.\n"
#define MSGTR_MPDEMUX_NW_ErrSendingHTTPRequest "Ошибка отправки HTTP запро�?а: По�?лан не ве�?ь запро�?.\n"
#define MSGTR_MPDEMUX_NW_ReadFailed "Чтение не удало�?ь.\n"
#define MSGTR_MPDEMUX_NW_Read0CouldBeEOF "http_read_response прочитано 0 (т.е. EOF).\n"
#define MSGTR_MPDEMUX_NW_AuthFailed "Ошибка аутентификации. И�?пользуйте опции -user и -passwd чтобы предо�?тавить ваши\n"\
"логин/пароль дл�? �?пи�?ка URL, или �?формируйте URL �?ледующим образом:\n"\
"http://логин:пароль@им�?_хо�?та/файл\n"
#define MSGTR_MPDEMUX_NW_AuthRequiredFor "Дл�? %s требует�?�? аутентификаци�?\n"
#define MSGTR_MPDEMUX_NW_AuthRequired "Требует�?�? аутентификаци�?.\n"
#define MSGTR_MPDEMUX_NW_NoPasswdProvidedTryingBlank "Пароль не указан, пробую пу�?той пароль.\n"
#define MSGTR_MPDEMUX_NW_ErrServerReturned "Сервер вернул %d: %s\n"
#define MSGTR_MPDEMUX_NW_CacheSizeSetTo "У�?тановлен размер к�?ша %d КБайт(а/ов)\n"

// demux_audio.c
#define MSGTR_MPDEMUX_AUDIO_UnknownFormat "Демультиплек�?ор: неизве�?тный формат %d.\n"

// demux_demuxers.c
#define MSGTR_MPDEMUX_DEMUXERS_FillBufferError "ошибка заполнени�?_буфера: плохой демультиплек�?ор: не vd, ad или sd.\n"

// demux_mkv.c
#define MSGTR_MPDEMUX_MKV_ZlibInitializationFailed "[mkv] ошибка инициализации zlib.\n"
#define MSGTR_MPDEMUX_MKV_ZlibDecompressionFailed "[mkv] ошибка zlib ра�?паковки.\n"
#define MSGTR_MPDEMUX_MKV_LzoInitializationFailed "[mkv] ошибка инициализации lzo.\n"
#define MSGTR_MPDEMUX_MKV_LzoDecompressionFailed "[mkv] ошибка lzo ра�?паковки.\n"
#define MSGTR_MPDEMUX_MKV_TrackEncrypted "[mkv] Дорожка номер %u зашифрована, а ра�?шифровка еще не \n[mkv] реализована. Пропу�?к дорожки.\n"
#define MSGTR_MPDEMUX_MKV_UnknownContentEncoding "[mkv] �?еизве�?тный тип шифровани�? �?одержимого дл�? дорожки %u. Пропу�?к дорожки.\n"
#define MSGTR_MPDEMUX_MKV_UnknownCompression "[mkv] Дорожка %u �?жата неизве�?тным/неподдерживаемым \n[mkv] алгоритмом (%u). Пропу�?к дорожки.\n"
#define MSGTR_MPDEMUX_MKV_ZlibCompressionUnsupported "[mkv] Дорожка %u �?жата zlib, но mplayer �?компилирован \n[mkv] без поддержки �?жати�? zlib. Пропу�?к дорожки.\n"
#define MSGTR_MPDEMUX_MKV_TrackIDName "[mkv] ID дорожки %u: %s (%s) \"%s\", %s\n"
#define MSGTR_MPDEMUX_MKV_TrackID "[mkv] ID дорожки %u: %s (%s), %s\n"
#define MSGTR_MPDEMUX_MKV_UnknownCodecID "[mkv] �?еизве�?тный/неподдерживаемый CodecID (%s) или от�?ут�?твующие/плохие\n[mkv] данные CodecPrivate (дорожка %u).\n"
#define MSGTR_MPDEMUX_MKV_FlacTrackDoesNotContainValidHeaders "[mkv] Дорожка FLAC не �?одержит правильных заголовков.\n"
#define MSGTR_MPDEMUX_MKV_UnknownAudioCodec "[mkv] �?еизве�?тный/неподдерживаемый ID аудио кодека '%s' дл�? дорожки %u или от�?ут�?твующие/неверные\n[mkv] ча�?тные данные кодека.\n"
#define MSGTR_MPDEMUX_MKV_SubtitleTypeNotSupported "[mkv] Тип �?убтитров '%s' не поддерживает�?�?.\n"
#define MSGTR_MPDEMUX_MKV_WillPlayVideoTrack "[mkv] Будет во�?производить�?�? дорожка %u.\n"
#define MSGTR_MPDEMUX_MKV_NoVideoTrackFound "[mkv] �?е найдена/не требует�?�? видео дорожка.\n"
#define MSGTR_MPDEMUX_MKV_NoAudioTrackFound "[mkv] �?е найдена/не требует�?�? аудио дорожка.\n"
#define MSGTR_MPDEMUX_MKV_WillDisplaySubtitleTrack "[mkv] Будет отображать�?�? дорожка �?убтитров %u.\n"
#define MSGTR_MPDEMUX_MKV_NoBlockDurationForSubtitleTrackFound "[mkv] Предупреждение: �?е найдена длительно�?ть блока трека �?убтитров.\n"
#define MSGTR_MPDEMUX_MKV_TooManySublines "[mkv] Внимание: �?лишком много �?трок дл�? рендеринга, пропущены.\n"
#define MSGTR_MPDEMUX_MKV_TooManySublinesSkippingAfterFirst "\n[mkv] Предупреждение: �?лишком много �?трок дл�? рендеринга,\nпропущены в�?е кроме первой %i.\n"

// demux_nuv.c
#define MSGTR_MPDEMUX_NUV_NoVideoBlocksInFile "В файле нет блоков видео.\n"

// demux_xmms.c
#define MSGTR_MPDEMUX_XMMS_FoundPlugin "�?айден плагин: %s (%s).\n"
#define MSGTR_MPDEMUX_XMMS_ClosingPlugin "Закрываю плагин: %s.\n"
#define MSGTR_MPDEMUX_XMMS_WaitForStart "Ожидание начала во�?произведени�? '%s' плагином XMMS...\n"

// ========================== LIBMENU ===================================

// common
#define MSGTR_LIBMENU_NoEntryFoundInTheMenuDefinition "[МЕ�?Ю] �?е найдено вхождений в опи�?ании меню.\n"

// libmenu/menu.c
#define MSGTR_LIBMENU_SyntaxErrorAtLine "[МЕ�?Ю] ошибка �?интак�?и�?а в �?троке: %d\n"
#define MSGTR_LIBMENU_MenuDefinitionsNeedANameAttrib "[МЕ�?Ю] Опи�?ани�? меню требуют именного атрибута (�?трока %d).\n"
#define MSGTR_LIBMENU_BadAttrib "[МЕ�?Ю] плохой атрибут %s=%s в меню '%s' в �?троке %d\n"
#define MSGTR_LIBMENU_UnknownMenuType "[МЕ�?Ю] неизве�?тный тип меню '%s' в �?троке %d\n"
#define MSGTR_LIBMENU_CantOpenConfigFile "[МЕ�?Ю] �?е могу открыть конфигурационный файл меню: %s\n"
#define MSGTR_LIBMENU_ConfigFileIsTooBig "[МЕ�?Ю] Конфигурационный файл �?лишком велик (> %d KB)\n"
#define MSGTR_LIBMENU_ConfigFileIsEmpty "[МЕ�?Ю] Конфигурационный файл пу�?т.\n"
#define MSGTR_LIBMENU_MenuNotFound "[МЕ�?Ю] Меню %s не найдено.\n"
#define MSGTR_LIBMENU_MenuInitFailed "[МЕ�?Ю] Меню '%s': Ошибка инициализации.\n"
#define MSGTR_LIBMENU_UnsupportedOutformat "[МЕ�?Ю] �?еподдерживаемый выходной формат!!!!\n"

// libmenu/menu_cmdlist.c
#define MSGTR_LIBMENU_ListMenuEntryDefinitionsNeedAName "[МЕ�?Ю] Опи�?ани�?м вхождений меню �?пи�?ка нужно им�? (�?трока %d).\n"
#define MSGTR_LIBMENU_ListMenuNeedsAnArgument "[МЕ�?Ю] Меню �?пи�?ка необходим аргумент.\n"

// libmenu/menu_console.c
#define MSGTR_LIBMENU_WaitPidError "[МЕ�?Ю] Ошибка вызова waitpid: %s.\n"
#define MSGTR_LIBMENU_SelectError "[МЕ�?Ю] Ошибка вызова select.\n"
#define MSGTR_LIBMENU_ReadErrorOnChildFD "[МЕ�?Ю] Ошибка чтени�? дочернего файлового опи�?ател�?: %s.\n"
#define MSGTR_LIBMENU_ConsoleRun "[МЕ�?Ю] Запу�?к кон�?оли: %s ...\n"
#define MSGTR_LIBMENU_AChildIsAlreadyRunning "[МЕ�?Ю] Дочерний проце�?�? уже запущен.\n"
#define MSGTR_LIBMENU_ForkFailed "[МЕ�?Ю] Вызов fork не удал�?�? !!!\n"
#define MSGTR_LIBMENU_WriteError "[МЕ�?Ю] ошибка запи�?и\n"

// libmenu/menu_filesel.c
#define MSGTR_LIBMENU_OpendirError "[МЕ�?Ю] ошибка открыти�? каталога: %s\n"
#define MSGTR_LIBMENU_ReallocError "[МЕ�?Ю] ошибка перера�?пределени�? пам�?ти: %s\n"
#define MSGTR_LIBMENU_MallocError "[МЕ�?Ю] ошибка выделени�? пам�?ти: %s\n"
#define MSGTR_LIBMENU_ReaddirError "[МЕ�?Ю] ошибка чтени�? каталога: %s\n"
#define MSGTR_LIBMENU_CantOpenDirectory "[МЕ�?Ю] �?е могу открыть каталог %s.\n"

// libmenu/menu_param.c
#define MSGTR_LIBMENU_SubmenuDefinitionNeedAMenuAttribut "[МЕ�?Ю] Опи�?ани�?м подменю нужен атрибут 'menu'.\n"
#define MSGTR_LIBMENU_InvalidProperty "[MENU] �?еверное �?вой�?тво '%s' в �?лементе меню предпочтений. (�?трока %d).\n"
#define MSGTR_LIBMENU_PrefMenuEntryDefinitionsNeed "[МЕ�?Ю] Опи�?ани�?м �?лемента меню предпочтений нужен допу�?тимый \nатрибут 'property' или 'txt' (�?трока %d).\n"
#define MSGTR_LIBMENU_PrefMenuNeedsAnArgument "[МЕ�?Ю] Меню предпочтений нужен аргумент.\n"

// libmenu/menu_pt.c
#define MSGTR_LIBMENU_CantfindTheTargetItem "[МЕ�?Ю] �?е могу найти целевой пункт ????\n"
#define MSGTR_LIBMENU_FailedToBuildCommand "[МЕ�?Ю] �?е могу по�?троить команду: %s.\n"

// libmenu/menu_txt.c
#define MSGTR_LIBMENU_MenuTxtNeedATxtFileName "[МЕ�?Ю] Тек�?товому меню нужно им�? тек�?тового файла (параметр file).\n"
#define MSGTR_LIBMENU_MenuTxtCantOpen "[МЕ�?Ю] �?е могу открыть %s.\n"
#define MSGTR_LIBMENU_WarningTooLongLineSplitting "[МЕ�?Ю] Предупреждение, �?трока �?лишком длинна�?. Разбиваю.\n"
#define MSGTR_LIBMENU_ParsedLines "[МЕ�?Ю] Проанализировано %d �?трок.\n"

// libmenu/vf_menu.c
#define MSGTR_LIBMENU_UnknownMenuCommand "[МЕ�?Ю] �?еизве�?тна�? команда: '%s'.\n"
#define MSGTR_LIBMENU_FailedToOpenMenu "[МЕ�?Ю] �?е могу открыть меню: '%s'.\n"

// ========================== LIBMPCODECS ===================================

// dec_video.c & dec_audio.c
#define MSGTR_CantOpenCodec "�?е могу открыть кодек.\n"
#define MSGTR_CantCloseCodec "�?е могу закрыть кодек.\n"

#define MSGTR_MissingDLLcodec "ОШИБК�?: �?е �?мог открыть требующий�?�? DirectShow кодек: %s\n"
#define MSGTR_ACMiniterror "�?е �?мог загрузить/инициализировать Win32/ACM аудиокодек (потер�?н DLL файл?)\n"
#define MSGTR_MissingLAVCcodec "�?е могу найти кодек '%s' в libavcodec...\n"

#define MSGTR_MpegNoSequHdr "MPEG: Ф�?Т�?ЛЬ�?�?Я ОШИБК�?: КО�?ЕЦ Ф�?ЙЛ�? при пои�?ке по�?ледовательно�?ти заголовков.\n"
#define MSGTR_CannotReadMpegSequHdr "Ф�?Т�?ЛЬ�?�?Я ОШИБК�?: �?е могу �?читать по�?ледовательно�?ть заголовков.\n"
#define MSGTR_CannotReadMpegSequHdrEx "Ф�?Т�?ЛЬ�?�?Я ОШИБК�?: �?е мочу �?читать ра�?ширение по�?ледовательно�?ти заголовков.\n"
#define MSGTR_BadMpegSequHdr "MPEG: Плоха�? по�?ледовательно�?ть заголовков.\n"
#define MSGTR_BadMpegSequHdrEx "MPEG: Плохое ра�?ширение по�?ледовательно�?ти заголовков.\n"

#define MSGTR_ShMemAllocFail "�?е могу зарезервировать раздел�?емую пам�?ть.\n"
#define MSGTR_CantAllocAudioBuf "�?е могу зарезервировать выходной аудио буфер.\n"

#define MSGTR_UnknownAudio "�?еизве�?тный/потер�?нный аудио формат -> без звука\n"

#define MSGTR_UsingExternalPP "[PP] И�?пользую внешний фильтр по�?тобработки, max q = %d.\n"
#define MSGTR_UsingCodecPP "[PP] И�?пользую по�?тобработку из кодека, max q = %d.\n"
#define MSGTR_VideoCodecFamilyNotAvailableStr "Запрошенное �?емей�?тво видеокодеков [%s] (vfm=%s) не до�?тупно.\nВключите его во врем�? компил�?ции.\n"
#define MSGTR_AudioCodecFamilyNotAvailableStr "Запрошенное �?емей�?тво аудиокодеков [%s] (afm=%s) не до�?тупно.\nВключите его во врем�? компил�?ции.\n"
#define MSGTR_OpeningVideoDecoder "Открываю декодер видео: [%s] %s\n"
#define MSGTR_SelectedVideoCodec "Выбран видеокодек: [%s] vfm: %s (%s)\n"
#define MSGTR_OpeningAudioDecoder "Открываю декодер аудио: [%s] %s\n"
#define MSGTR_SelectedAudioCodec "Выбран аудиокодек: [%s] afm: %s (%s)\n"
#define MSGTR_VDecoderInitFailed "Ошибка инициализации Декодера Видео :(\n"
#define MSGTR_ADecoderInitFailed "Ошибка инициализации Декодера �?удио :(\n"
#define MSGTR_ADecoderPreinitFailed "Ошибка предварительной инициализации Декодера �?удио :(\n"

// libmpcodecs/ad_dvdpcm.c
#define MSGTR_SamplesWanted "Дл�? улучшени�? поддержки необходимы образцы �?того формата.\nПожалуй�?та, �?в�?жите�?ь �? разработчиками.\n"

// libmpcodecs/ad_libdv.c
#define MSGTR_MPCODECS_AudioFramesizeDiffers "[AD_LIBDV] Предупреждение! Размер фрейма аудио отличает�?�?! read=%d  hdr=%d.\n"

// vd.c
#define MSGTR_CodecDidNotSet "VDec: Кодек не у�?тановил sh->disp_w и sh->disp_h, пытаю�?ь обойти.\n"
#define MSGTR_CouldNotFindColorspace "�?е могу найти подход�?щее цветовое про�?тран�?тво - попытаю�?ь �? -vf scale...\n"
#define MSGTR_MovieAspectIsSet "Movie-Aspect - %.2f:1 - выполн�?ю предварительное ма�?штабирование\nдл�? коррекции �?оотношени�? �?торон фильма.\n"
#define MSGTR_MovieAspectUndefined "Movie-Aspect не определён - предварительное ма�?штабирование не примен�?ет�?�?.\n"

// vd_dshow.c, vd_dmo.c
#define MSGTR_DownloadCodecPackage "Вам нужно обновить/у�?тановить пакет бинарных кодеков.\nЗайдите на http://www.mplayerhq.hu/dload.html\n"

// libmpcodecs/vd_dmo.c vd_dshow.c vd_vfw.c
#define MSGTR_MPCODECS_CouldntAllocateImageForCinepakCodec "[VD_DMO] �?е могу выделить изображение дл�? кодека cinepak.\n"

// libmpcodecs/vd_ffmpeg.c
#define MSGTR_MPCODECS_XVMCAcceleratedCodec "[VD_FFMPEG] XVMC у�?коренный кодек.\n"
#define MSGTR_MPCODECS_ArithmeticMeanOfQP "[VD_FFMPEG] �?рифметиче�?кое �?реднее QP: %2.4f, Гармониче�?кое �?реднее QP: %2.4f\n"
#define MSGTR_MPCODECS_DRIFailure "[VD_FFMPEG] Сбой DRI.\n"
#define MSGTR_MPCODECS_CouldntAllocateImageForCodec "[VD_FFMPEG] �?е могу выделить изображени�? дл�? кодека.\n"
#define MSGTR_MPCODECS_XVMCAcceleratedMPEG2 "[VD_FFMPEG] XVMC-у�?коренный MPEG-2.\n"
#define MSGTR_MPCODECS_TryingPixfmt "[VD_FFMPEG] Пробую pixfmt=%d.\n"
#define MSGTR_MPCODECS_McGetBufferShouldWorkOnlyWithXVMC "[VD_FFMPEG] Буфер mc_get_buffer должен и�?пользовать�?�? только �? XVMC у�?корением!!"
#define MSGTR_MPCODECS_UnexpectedInitVoError "[VD_FFMPEG] �?еожиданна�? ошибка init_vo.\n"
#define MSGTR_MPCODECS_UnrecoverableErrorRenderBuffersNotTaken "[VD_FFMPEG] �?ево�?�?тановима�? ошибка: не получены буферы рендеринга.\n"
#define MSGTR_MPCODECS_OnlyBuffersAllocatedByVoXvmcAllowed "[VD_FFMPEG] Разрешены только буферы, выделенные vo_xvmc.\n"

// libmpcodecs/ve_lavc.c
#define MSGTR_MPCODECS_HighQualityEncodingSelected "[VE_LAVC] Выбрано вы�?ококаче�?твенное кодирование (не в реальном времени)!\n"
#define MSGTR_MPCODECS_UsingConstantQscale "[VE_LAVC] И�?пользую по�?то�?нный qscale = %f (VBR).\n"

// libmpcodecs/ve_raw.c
#define MSGTR_MPCODECS_OutputWithFourccNotSupported "[VE_RAW] Сырой вывод �? FourCC [%x] не поддерживает�?�?!\n"
#define MSGTR_MPCODECS_NoVfwCodecSpecified "[VE_RAW] �?еобходимый VfW кодек не определён!!\n"

// vf.c
#define MSGTR_CouldNotFindVideoFilter "�?е могу найти видео фильтр '%s'.\n"
#define MSGTR_CouldNotOpenVideoFilter "�?е могу открыть видео фильтр '%s'.\n"
#define MSGTR_OpeningVideoFilter "Открываю видео фильтр: "
#define MSGTR_CannotFindColorspace "�?е могу найти подход�?щее цветовое про�?тран�?тво, даже в�?тавив 'scale' :(\n"

// libmpcodecs/vf_crop.c
#define MSGTR_MPCODECS_CropBadPositionWidthHeight "[CROP] Плоха�? позици�?/ширина/вы�?ота - урезанна�? обла�?ть вне оригинала!\n"

// libmpcodecs/vf_cropdetect.c
#define MSGTR_MPCODECS_CropArea "[CROP] Обла�?ть урезани�?: X: %d..%d  Y: %d..%d  (-vf crop=%d:%d:%d:%d).\n"

// libmpcodecs/vf_format.c, vf_palette.c, vf_noformat.c
#define MSGTR_MPCODECS_UnknownFormatName "[VF_FORMAT] �?еизве�?тное им�? формата: '%s'.\n"

// libmpcodecs/vf_framestep.c vf_noformat.c vf_palette.c vf_tile.c
#define MSGTR_MPCODECS_ErrorParsingArgument "[VF_FRAMESTEP] Ошибка анализа аргумента.\n"

// libmpcodecs/ve_vfw.c
#define MSGTR_MPCODECS_CompressorType "Тип компре�?�?ора: %.4lx\n"
#define MSGTR_MPCODECS_CompressorSubtype "Подтип компре�?�?ора: %.4lx\n"
#define MSGTR_MPCODECS_CompressorFlags "Флаги компре�?�?ора: %lu, вер�?и�? %lu, вер�?и�? ICM: %lu\n"
#define MSGTR_MPCODECS_Flags "Флаги:"
#define MSGTR_MPCODECS_Quality " каче�?тво"

// libmpcodecs/vf_expand.c
#define MSGTR_MPCODECS_FullDRNotPossible "Полный DR невозможен, пробую SLICES взамен!\n"
#define MSGTR_MPCODECS_WarnNextFilterDoesntSupportSlices  "ПРЕДУПРЕЖДЕ�?ИЕ! Следующий фильтр не поддерживает SLICES,\nприготовьте�?ь к ошибке �?егментации пам�?ти (sig11)...\n"
#define MSGTR_MPCODECS_FunWhydowegetNULL "Почему мы получили NULL??\n"

// libmpcodecs/vf_test.c, vf_yuy2.c, vf_yvu9.c
#define MSGTR_MPCODECS_WarnNextFilterDoesntSupport "%s не поддерживает�?�? �?ледующим фильтром/видеовыводом :(\n"

// ================================== LIBASS ====================================

// ass_bitmap.c
#define MSGTR_LIBASS_FT_Glyph_To_BitmapError "[ass] Сбой FT_Glyph_To_Bitmap %d \n"
#define MSGTR_LIBASS_UnsupportedPixelMode "[ass] �?еподдерживаемый пик�?ельный режим: %d\n"
#define MSGTR_LIBASS_GlyphBBoxTooLarge "[ass] Рамка, ограничивающа�? �?имвол, �?лишком велика: %dx%dpx\n"

// ass.c
#define MSGTR_LIBASS_NoStyleNamedXFoundUsingY "[ass] [%p] Внимание: �?тиль '%s' не найден, и�?пользую '%s'\n"
#define MSGTR_LIBASS_BadTimestamp "[ass] неверна�? временна�? метка\n"
#define MSGTR_LIBASS_BadEncodedDataSize "[ass] неверный размер кодированный данных\n"
#define MSGTR_LIBASS_FontLineTooLong "[ass] Лини�? шрифта �?лишком длинна: %d, %s\n"
#define MSGTR_LIBASS_EventFormatHeaderMissing "[ass] От�?ут�?твует заголовок формата �?обытий\n"
#define MSGTR_LIBASS_ErrorOpeningIconvDescriptor "[ass] ошибка открыти�? де�?криптора iconv\n"
#define MSGTR_LIBASS_ErrorRecodingFile "[ass] ошибка запи�?и файла.\n"
#define MSGTR_LIBASS_FopenFailed "[ass] ass_read_file(%s): �?бой fopen\n"
#define MSGTR_LIBASS_FseekFailed "[ass] ass_read_file(%s): �?бой fseek\n"
#define MSGTR_LIBASS_RefusingToLoadSubtitlesLargerThan100M "[ass] ass_read_file(%s): Отклонение загрузки �?убтитров больше 100M\n"
#define MSGTR_LIBASS_ReadFailed "Ошибка чтени�?, %d: %s\n"
#define MSGTR_LIBASS_AddedSubtitleFileMemory "[ass] Добавлен файл �?убтитров: <пам�?ть> (�?тилей: %d, �?обытий: %d)\n"
#define MSGTR_LIBASS_AddedSubtitleFileFname "[ass] Добавлен файл �?убтитров: %s (�?тилей: %d, �?обытий: %d)\n"
#define MSGTR_LIBASS_FailedToCreateDirectory "[ass] Ошибка �?оздани�? каталога %s\n"
#define MSGTR_LIBASS_NotADirectory "[ass] �?е каталог: %s\n"

// ass_cache.c
#define MSGTR_LIBASS_TooManyFonts "[ass] Слишком много шрифтов\n"
#define MSGTR_LIBASS_ErrorOpeningFont "[ass] Ошибка открыти�? шрифта: %s, %d\n"

// ass_fontconfig.c
#define MSGTR_LIBASS_SelectedFontFamilyIsNotTheRequestedOne "[ass] fontconfig: Выбранный шрифт не �?овпадает �? запрошенным: '%s' != '%s'\n"
#define MSGTR_LIBASS_UsingDefaultFontFamily "[ass] fontconfig_select: И�?пользует�?�? �?емей�?тво шрифтов по-умолчанию: (%s, %d, %d) -> %s, %d\n"
#define MSGTR_LIBASS_UsingDefaultFont "[ass] fontconfig_select: И�?пользует�?�? шрифт по-умолчанию: (%s, %d, %d) -> %s, %d\n"
#define MSGTR_LIBASS_UsingArialFontFamily "[ass] fontconfig_select: И�?пользует�?�? �?емей�?тво шрифтов 'Arial': (%s, %d, %d) -> %s, %d\n"
#define MSGTR_LIBASS_FcInitLoadConfigAndFontsFailed "[ass] Сбой FcInitLoadConfigAndFonts.\n"
#define MSGTR_LIBASS_UpdatingFontCache "[ass] Обновление кеша шрифтов.\n"
#define MSGTR_LIBASS_BetaVersionsOfFontconfigAreNotSupported "[ass] Бета вер�?ии fontconfig не поддерживают�?�?.\n[ass] Обновите�?ь прежде чем от�?ылать отчеты о любых ошибках.\n"
#define MSGTR_LIBASS_FcStrSetAddFailed "[ass] Сбой FcStrSetAdd.\n"
#define MSGTR_LIBASS_FcDirScanFailed "[ass] Сбой FcDirScan.\n"
#define MSGTR_LIBASS_FcDirSave "[ass] Сбой FcDirSave.\n"
#define MSGTR_LIBASS_FcConfigAppFontAddDirFailed "[ass] Сбой FcConfigAppFontAddDir\n"
#define MSGTR_LIBASS_FontconfigDisabledDefaultFontWillBeUsed "[ass] Fontconfig отключен, будет и�?пользовать�?�? только шрифт по-умолчанию.\n"
#define MSGTR_LIBASS_FunctionCallFailed "[ass] Сбой %s\n"

// ass_render.c
#define MSGTR_LIBASS_NeitherPlayResXNorPlayResYDefined "[ass] �?е определены ни PlayResX, ни PlayResY. Предполагаю 384x288.\n"
#define MSGTR_LIBASS_PlayResYUndefinedSettingY "[ass] PlayResY не определено, у�?танавливаю %d.\n"
#define MSGTR_LIBASS_PlayResXUndefinedSettingX "[ass] PlayResX не определено, у�?танавливаю %d.\n"
#define MSGTR_LIBASS_FT_Init_FreeTypeFailed "[ass] Сбой FT_Init_FreeType.\n"
#define MSGTR_LIBASS_Init "[ass] Инициализаци�?.\n"
#define MSGTR_LIBASS_InitFailed "[ass] Сбой инициализации.\n"
#define MSGTR_LIBASS_BadCommand "[ass] �?еверна�? команда: %c%c\n"
//FIXME glyph
#define MSGTR_LIBASS_ErrorLoadingGlyph  "[ass] Ошибка загрузки глифа [glyph].\n"
#define MSGTR_LIBASS_FT_Glyph_Stroke_Error "[ass] Ошибка FT_Glyph_Stroke %d \n"
#define MSGTR_LIBASS_UnknownEffectType_InternalError "[ass] �?еизве�?тный тип �?ффекта (внутренн�?�? ошибка)\n"
#define MSGTR_LIBASS_NoStyleFound "[ass] Стили не найдены!\n"
#define MSGTR_LIBASS_EmptyEvent "[ass] Пу�?тое �?обытие!\n"
#define MSGTR_LIBASS_MAX_GLYPHS_Reached "[ass] До�?тигнуто значение MAX_GLYPHS: �?обытие %d, начало = %llu, длительно�?ть = %llu\n Тек�?т = %s\n"
#define MSGTR_LIBASS_EventHeightHasChanged "[ass] Внимание! Вы�?ота �?обыти�? изменила�?ь!  \n"

// ass_font.c
#define MSGTR_LIBASS_GlyphNotFoundReselectingFont "[ass] Глиф 0x%X не найден, выбираю еще один шрифт дл�? (%s, %d, %d)\n"
#define MSGTR_LIBASS_GlyphNotFound "[ass] В шрифте не найден глиф 0x%X дл�? (%s, %d, %d)\n"
#define MSGTR_LIBASS_ErrorOpeningMemoryFont "[ass] Ошибка открыти�? шрифта в пам�?ти: %s\n"
#define MSGTR_LIBASS_NoCharmaps "[ass] шрифт без таблиц �?имволов\n"
#define MSGTR_LIBASS_NoCharmapAutodetected "[ass] автоматиче�?ки определить таблицу �?имволов не удало�?ь, пробует�?�? перва�? до�?тупна�?\n"

// ================================== stream ====================================

// ai_alsa.c
#define MSGTR_MPDEMUX_AIALSA_CannotSetSamplerate "�?е могу задать ча�?тоту ди�?кретизации.\n"
#define MSGTR_MPDEMUX_AIALSA_CannotSetBufferTime "�?е могу задать врем�? буферизации.\n"
#define MSGTR_MPDEMUX_AIALSA_CannotSetPeriodTime "�?е могу задать врем�? периода.\n"

// ai_alsa.c
#define MSGTR_MPDEMUX_AIALSA_PcmBrokenConfig "�?екорректна�? конфигураци�? дл�? данного PCM: нет до�?тупных конфигураций.\n"
#define MSGTR_MPDEMUX_AIALSA_UnavailableAccessType "Тип до�?тупа не до�?тупен.\n"
#define MSGTR_MPDEMUX_AIALSA_UnavailableSampleFmt "Формат образца не до�?тупен.\n"
#define MSGTR_MPDEMUX_AIALSA_UnavailableChanCount "Чи�?ло каналов не до�?тупно - возвращаю�?ь к умолчанию: %d\n"
#define MSGTR_MPDEMUX_AIALSA_CannotInstallHWParams "�?е могу у�?тановить аппаратные параметры: %s"
#define MSGTR_MPDEMUX_AIALSA_PeriodEqualsBufferSize "�?е могу и�?пользовать период, равный размеру буфера (%u == %lu)\n"
#define MSGTR_MPDEMUX_AIALSA_CannotInstallSWParams "�?е могу у�?тановить программные параметры:\n"
#define MSGTR_MPDEMUX_AIALSA_ErrorOpeningAudio "Ошибка открыти�? аудио: %s\n"
#define MSGTR_MPDEMUX_AIALSA_AlsaXRUN "ALSA xrun!!! (как минимум длительно�?тью %.3f м�?)\n"
#define MSGTR_MPDEMUX_AIALSA_AlsaXRUNPrepareError "ALSA xrun: ошибка подготовки: %s"
#define MSGTR_MPDEMUX_AIALSA_AlsaReadWriteError "Ошибка чтени�?/запи�?и ALSA"

// ai_oss.c
#define MSGTR_MPDEMUX_AIOSS_Unable2SetChanCount "�?е могу задать чи�?ло каналов: %d\n"
#define MSGTR_MPDEMUX_AIOSS_Unable2SetStereo "�?е могу включить �?терео: %d\n"
#define MSGTR_MPDEMUX_AIOSS_Unable2Open "�?е могу открыть '%s': %s\n"
#define MSGTR_MPDEMUX_AIOSS_UnsupportedFmt "неподдерживаемый формат\n"
#define MSGTR_MPDEMUX_AIOSS_Unable2SetAudioFmt "�?е могу задать аудиоформат."
#define MSGTR_MPDEMUX_AIOSS_Unable2SetSamplerate "�?е могу задать ча�?тоту ди�?кретизации: %d\n"
#define MSGTR_MPDEMUX_AIOSS_Unable2SetTrigger "�?е могу у�?тановить триггер: %d\n"
#define MSGTR_MPDEMUX_AIOSS_Unable2GetBlockSize "�?е могу получить размер блока!\n"
#define MSGTR_MPDEMUX_AIOSS_AudioBlockSizeZero "Размер блока аудио нулевой, у�?танавливаю в %d!\n"
#define MSGTR_MPDEMUX_AIOSS_AudioBlockSize2Low "Размер блока аудио �?лишком мал, у�?танавливаю в %d!\n"

// asf_mmst_streaming.c
#define MSGTR_MPDEMUX_MMST_WriteError "ошибка запи�?и\n"
#define MSGTR_MPDEMUX_MMST_EOFAlert "\nТревога! EOF\n"
#define MSGTR_MPDEMUX_MMST_PreHeaderReadFailed "чтение предварительного заголовка не удало�?ь\n"
#define MSGTR_MPDEMUX_MMST_InvalidHeaderSize "�?еверный размер заголовка, безнадёжно.\n"
#define MSGTR_MPDEMUX_MMST_HeaderDataReadFailed "�?е могу прочитать данные заголовка.\n"
#define MSGTR_MPDEMUX_MMST_packet_lenReadFailed "не могу прочитать packet_len.\n"
#define MSGTR_MPDEMUX_MMST_InvalidRTSPPacketSize "�?еверный размер пакета RTSP, безнадёжно.\n"
#define MSGTR_MPDEMUX_MMST_CmdDataReadFailed "�?е могу прочитать управл�?ющие данные.\n"
#define MSGTR_MPDEMUX_MMST_HeaderObject "объект заголовка\n"
#define MSGTR_MPDEMUX_MMST_DataObject "объект данных\n"
#define MSGTR_MPDEMUX_MMST_FileObjectPacketLen "файловый объект, длина пакета = %d (%d)\n"
#define MSGTR_MPDEMUX_MMST_StreamObjectStreamID "потоковый объект, ID потока: %d\n"
#define MSGTR_MPDEMUX_MMST_2ManyStreamID "Слишком много ID, поток пропущен."
#define MSGTR_MPDEMUX_MMST_UnknownObject "неизве�?тный объект\n"
#define MSGTR_MPDEMUX_MMST_MediaDataReadFailed "�?е могу прочитать медиаданные.\n"
#define MSGTR_MPDEMUX_MMST_MissingSignature "пропущена подпи�?ь\n"
#define MSGTR_MPDEMUX_MMST_PatentedTechnologyJoke "В�?ё �?делано. Спа�?ибо за загрузку медиафайла,\n�?одержащего проприетарную и запатентованную технологию =).\n"
#define MSGTR_MPDEMUX_MMST_UnknownCmd "неизве�?тна�? команда %02x\n"
#define MSGTR_MPDEMUX_MMST_GetMediaPacketErr "ошибка get_media_packet : %s\n"
#define MSGTR_MPDEMUX_MMST_Connected "Соединили�?ь\n"

// asf_streaming.c
#define MSGTR_MPDEMUX_ASF_StreamChunkSize2Small "Эх, размер stream_chunck �?лишком мал: %d\n"
#define MSGTR_MPDEMUX_ASF_SizeConfirmMismatch "не�?оответ�?твие size_confirm!: %d %d\n"
#define MSGTR_MPDEMUX_ASF_WarnDropHeader "Предупреждение: пропу�?к заголовка ????\n"
#define MSGTR_MPDEMUX_ASF_ErrorParsingChunkHeader "Ошибка разбора заголовка звена\n"
#define MSGTR_MPDEMUX_ASF_NoHeaderAtFirstChunk "�?е получил заголовок как первое звено !!!!\n"
#define MSGTR_MPDEMUX_ASF_BufferMallocFailed "Ошибка: �?е могу выделить буфер в %d байт(а/ов).\n"
#define MSGTR_MPDEMUX_ASF_ErrReadingNetworkStream "Ошибка чтени�? �?етевого потока.\n"
#define MSGTR_MPDEMUX_ASF_ErrChunk2Small "Ошибка: Звено �?лишком мало.\n"
#define MSGTR_MPDEMUX_ASF_ErrSubChunkNumberInvalid "Ошибка: �?омер подцепочки неверен.\n"
#define MSGTR_MPDEMUX_ASF_Bandwidth2SmallCannotPlay "Скоро�?ть передачи �?лишком мала, файл не может быть проигран!\n"
#define MSGTR_MPDEMUX_ASF_Bandwidth2SmallDeselectedAudio "Скоро�?ть передачи �?лишком мала, отключаю аудиопоток.\n"
#define MSGTR_MPDEMUX_ASF_Bandwidth2SmallDeselectedVideo "Скоро�?ть передачи �?лишком мала, отключаю видеопоток.\n"
#define MSGTR_MPDEMUX_ASF_InvalidLenInHeader "�?еверна�? длина в заголовке ASF!\n"
#define MSGTR_MPDEMUX_ASF_ErrReadingChunkHeader "Ошибка чтени�? заголовка звена.\n"
#define MSGTR_MPDEMUX_ASF_ErrChunkBiggerThanPacket "Ошибка: chunk_size > packet_size\n"
#define MSGTR_MPDEMUX_ASF_ErrReadingChunk "Ошибка чтени�? звена.\n"
#define MSGTR_MPDEMUX_ASF_ASFRedirector "=====> Перенаправитель ASF\n"
#define MSGTR_MPDEMUX_ASF_InvalidProxyURL "неверный URL прок�?и\n"
#define MSGTR_MPDEMUX_ASF_UnknownASFStreamType "неизве�?тный тип потока ASF\n"
#define MSGTR_MPDEMUX_ASF_Failed2ParseHTTPResponse "�?е могу проанализировать ответ HTTP.\n"
#define MSGTR_MPDEMUX_ASF_ServerReturn "Сервер вернул %d:%s\n"
#define MSGTR_MPDEMUX_ASF_ASFHTTPParseWarnCuttedPragma "ПРЕДУПРЕЖДЕ�?ИЕ �?�?�?ЛИЗ�?ТОР�? HTTP ASF : Pragma %s урезана от %zu байт до %zu\n"
#define MSGTR_MPDEMUX_ASF_SocketWriteError "ошибка запи�?и �?окета: %s\n"
#define MSGTR_MPDEMUX_ASF_HeaderParseFailed "�?е могу разобрать заголовок.\n"
#define MSGTR_MPDEMUX_ASF_NoStreamFound "Поток не найден.\n"
#define MSGTR_MPDEMUX_ASF_UnknownASFStreamingType "неизве�?тный тип потока ASF\n"
#define MSGTR_MPDEMUX_ASF_InfoStreamASFURL "STREAM_ASF, URL: %s\n"
#define MSGTR_MPDEMUX_ASF_StreamingFailed "Сбой, выхожу.\n"

// audio_in.c
#define MSGTR_MPDEMUX_AUDIOIN_ErrReadingAudio "\nОшибка чтени�? аудио: %s\n"
#define MSGTR_MPDEMUX_AUDIOIN_XRUNSomeFramesMayBeLeftOut "Во�?�?тановление по�?ле пере�?кока, некоторые кадры могут быть потер�?ны!\n"
#define MSGTR_MPDEMUX_AUDIOIN_ErrFatalCannotRecover "Фатальна�? ошибка, не могу во�?�?тановить�?�?!\n"
#define MSGTR_MPDEMUX_AUDIOIN_NotEnoughSamples "\n�?едо�?таточна�? выборка аудио!\n"

// cache2.c
#define MSGTR_MPDEMUX_CACHE2_NonCacheableStream "\rЭтот поток не к�?шируем.\n"
#define MSGTR_MPDEMUX_CACHE2_ReadFileposDiffers "!!! read_filepos различают�?�?!!! Сообщите об �?той ошибке...\n"

// stream_cdda.c
#define MSGTR_MPDEMUX_CDDA_CantOpenCDDADevice "�?е могу открыть у�?трой�?тво CDDA.\n"
#define MSGTR_MPDEMUX_CDDA_CantOpenDisc "�?е могу открыть ди�?к.\n"
#define MSGTR_MPDEMUX_CDDA_AudioCDFoundWithNTracks "�?айден аудио CD �? %d дорожками.\n"

// stream_cddb.c
#define MSGTR_MPDEMUX_CDDB_FailedToReadTOC "�?е могу прочитать TOC.\n"
#define MSGTR_MPDEMUX_CDDB_FailedToOpenDevice "�?е могу открыть у�?трой�?тво %s.\n"
#define MSGTR_MPDEMUX_CDDB_NotAValidURL "неверный URL\n"
#define MSGTR_MPDEMUX_CDDB_FailedToSendHTTPRequest "�?е могу отправить HTTP запро�?.\n"
#define MSGTR_MPDEMUX_CDDB_FailedToReadHTTPResponse "�?е могу проче�?ть HTTP ответ.\n"
#define MSGTR_MPDEMUX_CDDB_HTTPErrorNOTFOUND "�?е найден.\n"
#define MSGTR_MPDEMUX_CDDB_HTTPErrorUnknown "неизве�?тный код ошибки\n"
#define MSGTR_MPDEMUX_CDDB_NoCacheFound "К�?ш не найден.\n"
#define MSGTR_MPDEMUX_CDDB_NotAllXMCDFileHasBeenRead "�?е в�?е xmcd файлы были прочитаны.\n"
#define MSGTR_MPDEMUX_CDDB_FailedToCreateDirectory "�?е могу �?оздать каталог %s.\n"
#define MSGTR_MPDEMUX_CDDB_NotAllXMCDFileHasBeenWritten "�?е в�?е xmcd файлы были запи�?аны.\n"
#define MSGTR_MPDEMUX_CDDB_InvalidXMCDDatabaseReturned "Возвращён неверный файл базы данных xmcd.\n"
#define MSGTR_MPDEMUX_CDDB_UnexpectedFIXME "неожиданное FIXME\n"
#define MSGTR_MPDEMUX_CDDB_UnhandledCode "необработанный код\n"
#define MSGTR_MPDEMUX_CDDB_UnableToFindEOL "�?евозможно найти конец �?троки.\n"
#define MSGTR_MPDEMUX_CDDB_ParseOKFoundAlbumTitle "�?нализ у�?пешен, найдено: %s\n"
#define MSGTR_MPDEMUX_CDDB_AlbumNotFound "�?льбом не найден.\n"
#define MSGTR_MPDEMUX_CDDB_ServerReturnsCommandSyntaxErr "Сервер вернул: ошибка �?интак�?и�?а команды\n"
#define MSGTR_MPDEMUX_CDDB_NoSitesInfoAvailable "�?едо�?тупна информаци�? о �?айте.\n"
#define MSGTR_MPDEMUX_CDDB_FailedToGetProtocolLevel "�?е могу получить уровень протокола.\n"
#define MSGTR_MPDEMUX_CDDB_NoCDInDrive "�?ет CD в приводе.\n"

// stream_cue.c
#define MSGTR_MPDEMUX_CUEREAD_UnexpectedCuefileLine "[bincue] �?еожиданна�? �?трока файла cue: %s\n"
#define MSGTR_MPDEMUX_CUEREAD_BinFilenameTested "[bincue] проверенное им�? бинарного файла: %s\n"
#define MSGTR_MPDEMUX_CUEREAD_CannotFindBinFile "[bincue] �?е могу найти бинарный файл - безнадёжно.\n"
#define MSGTR_MPDEMUX_CUEREAD_UsingBinFile "[bincue] И�?пользую бинарный файл %s.\n"
#define MSGTR_MPDEMUX_CUEREAD_UnknownModeForBinfile "[bincue] неизве�?тный режим дл�? бинарного файла.\nЭтого не должно прои�?ходить. Выхожу.\n"
#define MSGTR_MPDEMUX_CUEREAD_CannotOpenCueFile "[bincue] �?е могу открыть %s.\n"
#define MSGTR_MPDEMUX_CUEREAD_ErrReadingFromCueFile "[bincue] Ошибка чтени�? из %s\n"
#define MSGTR_MPDEMUX_CUEREAD_ErrGettingBinFileSize "[bincue] Ошибка получени�? размера бинарного файла.\n"
#define MSGTR_MPDEMUX_CUEREAD_InfoTrackFormat "дорожка %02d:  формат=%d  %02d:%02d:%02d\n"
#define MSGTR_MPDEMUX_CUEREAD_UnexpectedBinFileEOF "[bincue] неожиданный конец бинарного файла\n"
#define MSGTR_MPDEMUX_CUEREAD_CannotReadNBytesOfPayload "[bincue] �?е могу прочитать %d байт(а/ов) полезной нагрузки.\n"
#define MSGTR_MPDEMUX_CUEREAD_CueStreamInfo_FilenameTrackTracksavail "CUE открытие_потока, им�? файла=%s, дорожка=%d, до�?тупные дорожки: %d -> %d\n"

// stream_dvd.c
#define MSGTR_DVDspeedCantOpen "�?евозможно открыть DVD у�?трой�?тво дл�? запи�?и, изменение �?коро�?ти DVD требует до�?туп на запи�?ь.\n"
#define MSGTR_DVDrestoreSpeed "Во�?�?тановление �?коро�?ти DVD... "
#define MSGTR_DVDlimitSpeed "Ограничение �?коро�?ти DVD до %dКБ/�?... "
#define MSGTR_DVDlimitFail "ошибка\n"
#define MSGTR_DVDlimitOk "у�?пешно\n"
#define MSGTR_NoDVDSupport "MPlayer был �?компилирован без поддержки DVD, выходим.\n"
#define MSGTR_DVDnumTitles "�?а �?том DVD %d роликов.\n"
#define MSGTR_DVDinvalidTitle "�?едопу�?тимый номер DVD ролика: %d\n"
#define MSGTR_DVDnumChapters "В �?том DVD ролике %d раздел[а/ов].\n"
#define MSGTR_DVDinvalidChapter "�?едопу�?тимый номер раздела DVD: %d\n"
#define MSGTR_DVDinvalidChapterRange "�?еверное опи�?ание диапазона раздела %s\n"
#define MSGTR_DVDinvalidLastChapter "�?еверный номер по�?леднего раздела DVD: %d\n"
#define MSGTR_DVDnumAngles "В �?том DVD ролике %d углов.\n"
#define MSGTR_DVDinvalidAngle "�?едопу�?тимый номер DVD угла: %d\n"
#define MSGTR_DVDnoIFO "�?е могу открыть IFO файл дл�? DVD ролика %d.\n"
#define MSGTR_DVDnoVMG "�?е могу открыть VMG информацию!\n"
#define MSGTR_DVDnoVOBs "�?е могу открыть VOBS ролика (VTS_%02d_1.VOB).\n"
#define MSGTR_DVDnoMatchingAudio "�?е найден подход�?щий аудио �?зык DVD!\n"
#define MSGTR_DVDaudioChannel "Выбранный аудиоканал DVD: %d �?зык: %c%c\n"
#define MSGTR_DVDaudioStreamInfo "аудиопоток: %d формат: %s (%s) �?зык: %s aid: %d.\n"
#define MSGTR_DVDnumAudioChannels "чи�?ло аудиоканалов на ди�?ке: %d.\n"
#define MSGTR_DVDnoMatchingSubtitle "�?е найден подход�?щий �?зык �?убтитров DVD!\n"
#define MSGTR_DVDsubtitleChannel "Выбранный канал �?убтитров DVD: %d �?зык: %c%c\n"
#define MSGTR_DVDsubtitleLanguage "�?убтитры ( sid ): %d �?зык: %s\n"
#define MSGTR_DVDnumSubtitles "чи�?ло �?убтитров на ди�?ке: %d\n"

// stream/stream_radio.c
#define MSGTR_RADIO_ChannelNamesDetected "[radio] Обнаружены имена радио�?танций.\n"
#define MSGTR_RADIO_WrongFreqForChannel "[radio] �?еверна�? ча�?тота дл�? �?танции %s\n"
#define MSGTR_RADIO_WrongChannelNumberFloat "[radio] �?еверный номер �?танции: %.2f\n"
#define MSGTR_RADIO_WrongChannelNumberInt "[radio] �?еверный номер �?танции: %d\n"
#define MSGTR_RADIO_WrongChannelName "[radio] �?еверное название �?танции: %s\n"
#define MSGTR_RADIO_FreqParameterDetected "[radio] В параметрах обнаружена ча�?тота.\n"
#define MSGTR_RADIO_GetTunerFailed "[radio] Предупреждение: �?бой вызова ioctl get tuner : %s. frac у�?тановлен в %d.\n"
#define MSGTR_RADIO_NotRadioDevice "[radio] %s не �?вл�?ет�?�? у�?трой�?твом радио!\n"
#define MSGTR_RADIO_SetFreqFailed "[radio] �?бой вызова ioctl set frequency 0x%x (%.2f): %s\n"
#define MSGTR_RADIO_GetFreqFailed "[radio] �?бой вызова ioctl get frequency: %s\n"
#define MSGTR_RADIO_SetMuteFailed "[radio] �?бой вызова ioctl set mute: %s\n"
#define MSGTR_RADIO_QueryControlFailed "[radio] �?бой вызова ioctl query control: %s\n"
#define MSGTR_RADIO_GetVolumeFailed "[radio] �?бой вызова ioctl get volume: %s\n"
#define MSGTR_RADIO_SetVolumeFailed "[radio] �?бой вызова ioctl set volume: %s\n"
#define MSGTR_RADIO_DroppingFrame "\n[radio] потер�?н аудио фрейм (байт: %d)!\n"
#define MSGTR_RADIO_BufferEmpty "[radio] grab_audio_frame: буфер пу�?т, ожидание данных. байт: %d.\n"
#define MSGTR_RADIO_AudioInitFailed "[radio] �?бой вызова audio_in_init: %s\n"
#define MSGTR_RADIO_AllocateBufferFailed "[radio] �?евозможно �?оздать аудио буфер (блок=%d,размер=%d): %s\n"
#define MSGTR_RADIO_CurrentFreq "[radio] Текуща�? ча�?тота: %.2f\n"
#define MSGTR_RADIO_SelectedChannel "[radio] Выбрана �?танци�?: %d - %s (ча�?тота: %.2f)\n"
#define MSGTR_RADIO_ChangeChannelNoChannelList "[radio] �?евозможно изменить �?танцию: не передан �?пи�?ок радио�?танций.\n"
#define MSGTR_RADIO_UnableOpenDevice "[radio] �?евозможно открыть '%s': %s\n"
#define MSGTR_RADIO_InitFracFailed "[radio] �?бой вызова init_frac\n"
#define MSGTR_RADIO_WrongFreq "[radio] �?еверна�? ча�?тота: %.2f\n"
#define MSGTR_RADIO_UsingFreq "[radio] И�?пользует�?�? ча�?тота: %.2f.\n"
#define MSGTR_RADIO_AudioInInitFailed "[radio] �?бой вызова audio_in_init\n"
#define MSGTR_RADIO_AudioInSetupFailed "[radio] �?бой вызова audio_in_setup: %s\n"
#define MSGTR_RADIO_ClearBufferFailed "[radio] Ошибка очи�?тки буфера: %s\n"
#define MSGTR_RADIO_StreamEnableCacheFailed "[radio] Ошибка вызова stream_enable_cache: %s\n"
#define MSGTR_RADIO_DriverUnknownStr "[radio] �?еизве�?тный драйвер: %s\n"
#define MSGTR_RADIO_DriverV4L "[radio] И�?пользует�?�? V4Lv1 радио интерфей�?.\n"
#define MSGTR_RADIO_DriverV4L2 "[radio] И�?пользует�?�? V4Lv2 радио интерфей�?.\n"
#define MSGTR_RADIO_DriverBSDBT848 "[radio] И�?пользует�?�? *BSD BT848 радио интерфей�?.\n"

//tv.c
#define MSGTR_TV_BogusNormParameter "tv.c: norm_from_string(%s): �?еизве�?тный параметр norm, у�?танавливает�?�? %s.\n"
#define MSGTR_TV_NoVideoInputPresent "Ошибка: Видео вход от�?ут�?твует!\n"
#define MSGTR_TV_UnknownImageFormat ""\
"==================================================================\n"\
" В�?ИМ�?�?ИЕ:\n"\
" З�?ПРОШЕ�? �?ЕПРОТЕСТИРОВ�?�?�?ЫЙ ИЛИ �?ЕИЗВЕСТ�?ЫЙ ФОРМ�?Т ИЗОБР�?ЖЕ�?ИЯ (0x%x)\n"\
" Это может приве�?ти к неверному во�?произведению или краху программы!\n"\
" Отчеты об ошибках принимать�?�? не будут! Вам �?ледует попытать�?�? еще раз \n"\
" �? YV12 (про�?тран�?тво цветов по умолчанию) и проче�?ть документацию!\n"\
"==================================================================\n"
#define MSGTR_TV_CannotSetNorm "Ошибка: �?евозможно у�?тановить norm!\n"
#define MSGTR_TV_MJP_WidthHeight "  MJP: ширина %d вы�?ота %d\n"
#define MSGTR_TV_UnableToSetWidth "�?евозможно у�?тановить запрошенную ширину: %d\n"
#define MSGTR_TV_UnableToSetHeight "�?евозможно у�?тановить запрошенную вы�?оту: %d\n"
#define MSGTR_TV_NoTuner "Выбранный вход не имеет тюнера!\n"
#define MSGTR_TV_UnableFindChanlist "�?евозможно найти выбранный �?пи�?ок каналов! (%s)\n"
#define MSGTR_TV_ChannelFreqParamConflict "Вы не можете указать ча�?тоту и канал одновременно!\n"
#define MSGTR_TV_ChannelNamesDetected "Обнаружены названи�? TV каналов.\n"
#define MSGTR_TV_NoFreqForChannel "�?евозможно найти ча�?тоту дл�? канала %s (%s)\n"
#define MSGTR_TV_SelectedChannel3 "Выбран канал: %s - %s (ча�?тота: %.3f)\n"
#define MSGTR_TV_SelectedChannel2 "Выбран канал: %s (ча�?тота: %.3f)\n"
#define MSGTR_TV_UnsupportedAudioType "Тип аудио '%s (%x)' не поддерживает�?�?!\n"
#define MSGTR_TV_AvailableDrivers "До�?тупные драйверы:\n"
#define MSGTR_TV_DriverInfo "Выбран драйвер: %s\n название: %s\n автор: %s\n комментарий %s\n"
#define MSGTR_TV_NoSuchDriver "�?ет такого драйвера: %s\n"
#define MSGTR_TV_DriverAutoDetectionFailed "�?втоматиче�?ки определить TV драйвер не удало�?ь.\n"
#define MSGTR_TV_UnknownColorOption "Указана неизве�?тна�? опци�? цвета (%d)!\n"
#define MSGTR_TV_NoTeletext "�?ет телетек�?та"
#define MSGTR_TV_Bt848IoctlFailed "tvi_bsdbt848: Сбой %s ioctl. Ошибка: %s\n"
#define MSGTR_TV_Bt848InvalidAudioRate "tvi_bsdbt848: �?еверна�? величина аудио потока. Ошибка: %s\n"
#define MSGTR_TV_Bt848ErrorOpeningBktrDev "tvi_bsdbt848: �?евозможно открыть у�?трой�?тво bktr. Ошибка: %s\n"
#define MSGTR_TV_Bt848ErrorOpeningTunerDev "tvi_bsdbt848: �?евозможно открыть у�?трой�?тво tuner. Ошибка: %s\n"
#define MSGTR_TV_Bt848ErrorOpeningDspDev "tvi_bsdbt848: �?евозможно открыть у�?трой�?тво dsp. Ошибка: %s\n"
#define MSGTR_TV_Bt848ErrorConfiguringDsp "tvi_bsdbt848: Сбой конфигурировани�? dsp. Ошибка: %s\n"
#define MSGTR_TV_Bt848ErrorReadingAudio "tvi_bsdbt848: Ошибка чтени�? аудио данных. Ошибка: %s\n"
#define MSGTR_TV_Bt848MmapFailed "tvi_bsdbt848: Сбой mmap. Ошибка: %s\n"
#define MSGTR_TV_Bt848FrameBufAllocFailed "tvi_bsdbt848: Ошибка выделени�? пам�?ти дл�? кадрового буфера. Ошибка: %s\n"
#define MSGTR_TV_Bt848ErrorSettingWidth "tvi_bsdbt848: �?евозможно у�?тановить ширину изображени�?. Ошибка: %s\n"
#define MSGTR_TV_Bt848ErrorSettingHeight "tvi_bsdbt848: �?евозможно у�?тановить вы�?оту изображени�?. Ошибка: %s\n"
#define MSGTR_TV_Bt848UnableToStopCapture "tvi_bsdbt848: �?евозможно о�?тановить захват. Ошибка: %s\n"
#define MSGTR_TV_TTSupportedLanguages "Поддерживаемые �?зыки Телетек�?та:\n"
#define MSGTR_TV_TTSelectedLanguage "Выбран �?зык Телетек�?та по умолчанию: %s\n"
#define MSGTR_TV_ScannerNotAvailableWithoutTuner "Сканер каналов недо�?тупен, е�?ли нет тюнера\n"

//tvi_dshow.c
#define MSGTR_TVI_DS_UnableConnectInputVideoDecoder  "�?евозможно при�?оединить данный вход к видео декодеру. Ошибка:0x%x\n"
#define MSGTR_TVI_DS_UnableConnectInputAudioDecoder  "�?евозможно при�?оединить данный вход к аудио декодеру. Ошибка:0x%x\n"
#define MSGTR_TVI_DS_UnableSelectVideoFormat "tvi_dshow: �?евозможно выбрать видео формат. Ошибка:0x%x\n"
#define MSGTR_TVI_DS_UnableSelectAudioFormat "tvi_dshow: �?евозможно выбрать аудио формат. Ошибка:0x%x\n"
#define MSGTR_TVI_DS_UnableGetMediaControlInterface "tvi_dshow: �?евозможно получить IMediaControl интерфей�?. Ошибка:0x%x\n"
#define MSGTR_TVI_DS_UnableStartGraph "tvi_dshow: �?евозможно запу�?тить граф! Ошибка:0x%x\n"
#define MSGTR_TVI_DS_DeviceNotFound "tvi_dshow: У�?трой�?тво #%d не найдено\n"
#define MSGTR_TVI_DS_UnableGetDeviceName "tvi_dshow: �?евозможно получить название у�?трой�?тва #%d\n"
#define MSGTR_TVI_DS_UsingDevice "tvi_dshow: И�?пользует�?�? у�?трой�?тво #%d: %s\n"
#define MSGTR_TVI_DS_DirectGetFreqFailed "tvi_dshow: �?евозможно получить ча�?тоту напр�?мую. Будет и�?пользовать�?�? таблица каналов ОС.\n"
//following phrase will be printed near the selected audio/video input
#define MSGTR_TVI_DS_UnableExtractFreqTable "tvi_dshow: �?евозможно загрузить таблицу ча�?тот из kstvtune.ax\n"
#define MSGTR_TVI_DS_WrongDeviceParam "tvi_dshow: �?еверный параметр device: %s\n"
#define MSGTR_TVI_DS_WrongDeviceIndex "tvi_dshow: �?еверный индек�? device: %d\n"
#define MSGTR_TVI_DS_WrongADeviceParam "tvi_dshow: �?еверный параметр adevice: %s\n"
#define MSGTR_TVI_DS_WrongADeviceIndex "tvi_dshow: �?еверный индек�? adevice: %d\n"

#define MSGTR_TVI_DS_SamplerateNotsupported "tvi_dshow: Ча�?тота ди�?кретизации %d не поддерживает�?�? у�?трой�?твом. У�?танавливает�?�? перва�? до�?тупна�?.\n"
#define MSGTR_TVI_DS_VideoAdjustigNotSupported "tvi_dshow: Под�?тройка �?рко�?ти/цветно�?ти/на�?ыщенно�?ти/контра�?та не поддерживает�?�? у�?трой�?твом\n"

#define MSGTR_TVI_DS_ChangingWidthHeightNotSupported "tvi_dshow: Изменение ширины/вы�?оты видео не поддерживает�?�? у�?трой�?твом.\n"
#define MSGTR_TVI_DS_SelectingInputNotSupported  "tvi_dshow: Выбор и�?точника захвата не поддерживает�?�? у�?трой�?твом\n"
#define MSGTR_TVI_DS_ErrorParsingAudioFormatStruct "tvi_dshow: �?евозможно разобрать �?труктуру аудио формата.\n"
#define MSGTR_TVI_DS_ErrorParsingVideoFormatStruct "tvi_dshow: �?евозможно разобрать �?труктуру видео формата.\n"
#define MSGTR_TVI_DS_UnableSetAudioMode "tvi_dshow: �?евозможно у�?тановить аудио режим %d. Ошибка:0x%x\n"
#define MSGTR_TVI_DS_UnsupportedMediaType "tvi_dshow: �?еподдерживаемый медиа тип передан в %s\n"
#define MSGTR_TVI_DS_UnableGetsupportedVideoFormats "tvi_dshow: �?евозможно получить поддерживаемые медиа форматы у видео коннектора. Ошибка:0x%x\n"
#define MSGTR_TVI_DS_UnableGetsupportedAudioFormats "tvi_dshow: �?евозможно получить поддерживаемые медиа форматы у аудио коннектора. Ошибка:0x%x Отключение аудио.\n"
#define MSGTR_TVI_DS_UnableFindNearestChannel "tvi_dshow: �?евозможно найти ближайший канал в �?и�?темной таблице ча�?тот\n"
#define MSGTR_TVI_DS_UnableToSetChannel "tvi_dshow: �?евозможно переключить�?�? на ближайший канал в �?и�?темой таблице ча�?тот. Ошибка:0x%x\n"
#define MSGTR_TVI_DS_UnableTerminateVPPin "tvi_dshow: �?евозможно под�?оединить к VideoPort коннектору ни один фильтр из графа. Ошибка:0x%x\n"
#define MSGTR_TVI_DS_UnableBuildVideoSubGraph "tvi_dshow: �?евозможно вы�?троить цепочку дл�? видео в графе. Ошибка:0x%x\n"
#define MSGTR_TVI_DS_UnableBuildAudioSubGraph "tvi_dshow: �?евозможно вы�?троить цепочку дл�? аудио в графе. Ошибка:0x%x\n"
#define MSGTR_TVI_DS_UnableBuildVBISubGraph "tvi_dshow: �?евозможно вы�?троить VBI цепочку в графе. Ошибка:0x%x\n"
#define MSGTR_TVI_DS_GraphInitFailure "tvi_dshow: Ошибка инициализации Directshow графа.\n"
#define MSGTR_TVI_DS_NoVideoCaptureDevice "tvi_dshow: �?евозможно найти у�?трой�?тво захвата видео\n"
#define MSGTR_TVI_DS_NoAudioCaptureDevice "tvi_dshow: �?евозможно найти у�?трой�?тво захвата аудио\n"
#define MSGTR_TVI_DS_GetActualMediatypeFailed "tvi_dshow: �?евозможно получить текущий медиа формат (Ошибка:0x%x). Предполагает�?�? �?овпадающий �? запрошенным.\n"

// url.c
#define MSGTR_MPDEMUX_URL_StringAlreadyEscaped "Похоже, что �?трока уже пропущена в url_escape %c%c1%c2\n"
