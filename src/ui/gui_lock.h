#ifndef WIIBROWSER_GUI_LOCK_H
#define WIIBROWSER_GUI_LOCK_H

extern "C" void HaltGui();
void ResumeGui();

// Rendering, widget lifetimes and shared FreeType faces use the same recursive
// LWP mutex. Nesting is per owner, never a global permission to bypass a lock.
class GuiScopedLock {
public:
    GuiScopedLock() { HaltGui(); }
    ~GuiScopedLock() { ResumeGui(); }
    GuiScopedLock(const GuiScopedLock &) = delete;
    GuiScopedLock &operator=(const GuiScopedLock &) = delete;
};
#endif
