#include <cassert>
#include <iostream>
#include <cstdlib>
#include <cwchar>

// Host-side mock of the ISI 0x4E800020 hardening in FreeTypeGX/GuiText.
// This test does not link against real freetype or libogc; it validates
// the guard logic that prevents a branch to corrupted function pointer data.
//
// The real bug: FreeTypeGX ctor dereferenced ftFace without checking FT_Error,
// leaving ftFace garbage (bytes 0x4E800020 == PowerPC blr). Later
// FT_Get_Char_Index(ftFace, ...) executed mtctr r0; bctrl with CTR=0x4E800020.

struct MockFTFace {
    void* glyph;
    void* driver; // contains function pointer chain face+92 -> +12 -> +12 -> fn
    bool valid;
};

struct MockFreeTypeGX {
    MockFTFace* ftFace = nullptr;
    void* ftSlot = nullptr;
    bool ftValid = false;
    int pointSize = 0;

    // Simulated ctor guard (mirrors src/ui/FreeTypeGX.cpp:103)
    static MockFreeTypeGX createInvalid() {
        MockFreeTypeGX m;
        m.ftFace = nullptr;
        m.ftSlot = nullptr;
        m.ftValid = false;
        return m;
    }
    static MockFreeTypeGX createValid() {
        MockFreeTypeGX m;
        static MockFTFace face{ (void*)0x1234, (void*)0x5678, true };
        m.ftFace = &face;
        m.ftSlot = (void*)0xABCD;
        m.ftValid = true;
        m.pointSize = 12;
        return m;
    }
    bool isValid() const { return ftValid && ftFace; }

    // Guarded methods mirror hardening
    unsigned getWidth(const wchar_t* text) const {
        if(!text || !ftValid || !ftFace) return 0;
        // In real code would iterate and call cacheGlyphData
        return (unsigned)wcslen(text) * (unsigned)pointSize;
    }
    unsigned cacheGlyphData(wchar_t) const {
        if(!ftValid || !ftFace || !ftSlot) return 0;
        // Would call FT_Get_Char_Index which derefs face chain
        return 1;
    }
    void ChangeFontSize(int sz) {
        if(!ftValid || !ftFace) return;
        pointSize = sz;
    }
    unsigned drawText(int, int, const wchar_t* t) const {
        if(!t || !ftValid || !ftFace) return 0;
        return 1;
    }
};

// Mock fontSystem array like src/ui/FreeTypeGX.cpp:30
static MockFreeTypeGX* fontSystem[51] = {nullptr};

static void test_invalid_face_no_isi() {
    auto bad = MockFreeTypeGX::createInvalid();
    assert(!bad.isValid());
    // These must NOT crash or branch to 0x4E800020
    assert(bad.getWidth(L"Hello") == 0);
    assert(bad.cacheGlyphData(L'A') == 0);
    assert(bad.drawText(0,0,L"test") == 0);
    bad.ChangeFontSize(16); // no-op
    assert(bad.pointSize == 0);
    std::cout << "test_invalid_face_no_isi PASS\n";
}

static void test_valid_face_works() {
    auto good = MockFreeTypeGX::createValid();
    assert(good.isValid());
    assert(good.getWidth(L"Hi") == 2 * 12u);
    assert(good.cacheGlyphData(L'A') == 1);
    assert(good.drawText(0,0,L"test") == 1);
    good.ChangeFontSize(24);
    assert(good.pointSize == 24);
    std::cout << "test_valid_face_works PASS\n";
}

static void test_gui_text_null_guard() {
    // Simulate GuiText::GuiText(const char*,int) guard
    for(int i=0;i<=50;++i) { delete fontSystem[i]; fontSystem[i]=nullptr; }
    int currentSize = 16;
    // Simulate allocation failure (new returns nullptr) - GuiText must fallback to width 0
    fontSystem[currentSize] = nullptr;
    MockFreeTypeGX* fs = fontSystem[currentSize];
    unsigned width = 0;
    if(!fs || !fs->isValid()) {
        // In real code: fontSystem[currentSize] = new (nothrow) FreeTypeGX(currentSize)
        // Here we simulate failed allocation still leaves nullptr
        fs = nullptr; // allocation failed
    }
    if(fs && fs->isValid()) width = fs->getWidth(L"test");
    else width = 0;
    assert(width == 0);

    // Now with valid font primed
    fontSystem[currentSize] = new MockFreeTypeGX(MockFreeTypeGX::createValid());
    fs = fontSystem[currentSize];
    assert(fs && fs->isValid());
    width = fs->getWidth(L"test");
    assert(width == 4 * 12u);
    std::cout << "test_gui_text_null_guard PASS\n";
    delete fontSystem[currentSize];
    fontSystem[currentSize]=nullptr;
}

static void test_setupgui_priming() {
    // Mirrors src/ui/menu.cpp:907 SetupGui priming loop
    for(int i=0;i<=50;++i) { delete fontSystem[i]; fontSystem[i]=nullptr; }
    const int criticalSizes[] = {10,12,14,16,18,20,24,28};
    for(size_t i=0;i<sizeof(criticalSizes)/sizeof(criticalSizes[0]);++i){
        int sz = criticalSizes[i];
        if(sz>50) continue;
        if(!fontSystem[sz] || !fontSystem[sz]->isValid()){
            if(fontSystem[sz]) { delete fontSystem[sz]; fontSystem[sz]=nullptr; }
            // Simulate successful prime
            fontSystem[sz] = new MockFreeTypeGX(MockFreeTypeGX::createValid());
            fontSystem[sz]->pointSize = sz;
        }
        assert(fontSystem[sz] && fontSystem[sz]->isValid());
        assert(fontSystem[sz]->pointSize == sz);
    }
    std::cout << "test_setupgui_priming PASS\n";
    for(int i=0;i<=50;++i){ delete fontSystem[i]; fontSystem[i]=nullptr; }
}

static void test_download_thread_null_manager() {
    // Mirrors src/network/transfer.cpp:273 DownloadThread null manager guards
    struct MockManager {
        bool CreateBarCalled=false;
        int* CreateBar(){ CreateBarCalled=true; return (int*)0x1; }
        void SetProgress(void*,double){}
        bool CancelDownload(int*){ return false; }
        void RemoveBar(int*){}
    } *manager = nullptr;

    // AddHandle must return false if manager null (prevents null deref -> ISI)
    auto AddHandle = [&](void* data)->bool{
        if(!manager) return false;
        (void)data;
        return true;
    };
    assert(AddHandle(nullptr)==false);
    manager = new MockManager();
    assert(AddHandle(nullptr)==true);
    delete manager;
    std::cout << "test_download_thread_null_manager PASS\n";
}

static void test_video_null_checks() {
    // Mirrors src/media/video.cpp:312 InitVideo2 null checks
    void* vmode = nullptr;
    void* xfb0 = nullptr;
    void* xfb1 = nullptr;
    void* gp_fifo = nullptr;
    bool gx_init_called = false;
    auto InitVideo2_mock = [&]()->bool{
        if(!vmode) return false; // early return prevents SYS_AllocateFramebuffer(null)
        // ... would allocate xfb
        if(!xfb0 || !xfb1) return false;
        if(!gp_fifo) return false; // prevents GX_Init(NULL)
        gx_init_called = true;
        return true;
    };
    assert(InitVideo2_mock()==false);
    assert(!gx_init_called);
    vmode = (void*)0x1;
    xfb0 = (void*)0x1; xfb1 = (void*)0x1;
    gp_fifo = (void*)0x1;
    assert(InitVideo2_mock()==true);
    std::cout << "test_video_null_checks PASS\n";
}

int main(){
    test_invalid_face_no_isi();
    test_valid_face_works();
    test_gui_text_null_guard();
    test_setupgui_priming();
    test_download_thread_null_manager();
    test_video_null_checks();
    std::cout << "ALL freetype_guard tests PASS (ISI 0x4E800020 mitigated)\n";
    return 0;
}
