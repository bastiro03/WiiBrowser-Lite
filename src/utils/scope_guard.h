#pragma once
#include <utility>
#include <functional>

namespace wbl {

// Lightweight ScopeGuard (C++20) — executes callable on destruction unless dismissed.
template<typename F>
class ScopeGuard {
public:
    explicit ScopeGuard(F&& f) : fn_(std::forward<F>(f)), active_(true) {}
    explicit ScopeGuard(const F& f) : fn_(f), active_(true) {}
    ~ScopeGuard() { if(active_) fn_(); }
    ScopeGuard(ScopeGuard&& o) noexcept : fn_(std::move(o.fn_)), active_(o.active_) { o.active_ = false; }
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    void dismiss() noexcept { active_ = false; }
    void execute() { if(active_) { active_ = false; fn_(); } }
private:
    F fn_;
    bool active_;
};

template<typename F>
ScopeGuard<F> make_guard(F&& f) { return ScopeGuard<F>(std::forward<F>(f)); }

// RAII wrappers for common Wii resources
struct FileCloser { void operator()(FILE* f) const { if(f) fclose(f); } };
struct FreeDeleter { void operator()(void* p) const { if(p) free(p); } };

} // namespace wbl
