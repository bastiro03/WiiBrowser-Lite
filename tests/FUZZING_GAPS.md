# Fuzzing Coverage Gaps

## Current Coverage (Good)
- ✅ URL parsing (`fuzz_parseline`)
- ✅ CSS parsing (`fuzz_css_parse`)
- ✅ HTML parsing (`fuzz_html_parse`)
- ✅ JS evaluation (`fuzz_js_eval`)

All pure functions, perfect for coverage-driven fuzzing.

## Missing Coverage (Found via Manual Testing)

### 1. HTTP Redirect Handling UAF Bug
**Bug**: DNS resolve list use-after-free in `getrequest()` redirect loop.

**Why fuzzing missed it**:
- Requires real curl handle lifecycle (cleanup + init)
- Requires multi-hop HTTPS redirects
- Requires DNS resolution timing interaction
- Not a parser - it's stateful network I/O

**What would catch it**:
```c
/* Hypothetical fuzz target */
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Parse fuzz input as redirect chain spec:
    // - Number of hops
    // - URL for each hop
    // - Status code for each response (301, 302, etc.)
    
    MockHTTPServer server;
    setup_redirect_chain(&server, data, size);
    
    CURL *handle = curl_easy_init();
    struct block result = getrequest(handle, server.initial_url, NULL);
    curl_easy_cleanup(handle);
    
    // ASan would catch the UAF when resolve list freed too early
}
```

**Problem**: Requires mocking curl's response handling, which is complex.

### 2. mbedTLS Context Corruption (Overlapping Connections)
**Bug**: `curl_easy_reset()` keeps connections alive, causing nfds=2.

**Why fuzzing missed it**:
- Requires actual network connections (can't mock file descriptors easily)
- Requires mbedTLS SSL state machine
- Bug only triggers with real TLS handshakes

**What would catch it**:
- Integration tests with real HTTPS test server
- Property-based testing: "After redirect hop, assert nfds == 1"

### 3. x-safari-* Redirect Loop
**Bug**: watchOS User-Agent triggers x-safari-https:// redirects on x.com.

**Why fuzzing missed it**:
- Requires specific server behavior (x.com's UA sniffing)
- Would need to fuzz User-Agent + Location header combinations
- Could be caught by fuzzing redirect chains with various URL schemes

**Possible fuzz target**:
```c
/* Fuzz redirect chain with various URL schemes */
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Generate redirect chains with:
    // - http:// https://
    // - x-safari-http:// x-safari-https://
    // - Loops (A → B → A)
    // - Max depth violations
    
    // Feed to redirect loop detection logic
    // (Could fuzz the logic from redirect_loop_test.c)
}
```

## Recommendations

### Short-term: Test What We Can
1. **Fuzz redirect loop logic** (from `redirect_loop_test.c`):
   - Extract loop detection into pure function
   - Fuzz with random redirect chains
   - Check for crashes, infinite loops, memory leaks

2. **Property-based tests for network layer**:
   - Mock curl responses at callback level
   - Generate random redirect chains
   - Assert invariants (nfds=1, no UAF, max hops enforced)

### Long-term: Network Fuzzing
1. **Mock HTTP server** for integration tests:
   - Serve random redirect chains
   - Fuzz status codes, headers, body content
   - Run under ASan/UBSan

2. **Stateful fuzzing** with AFL/libFuzzer custom mutators:
   - Track curl handle state
   - Generate redirect sequences
   - Detect crashes in full network stack

### Why This Is Hard
- **State explosion**: Network code has complex state (DNS cache, connections, SSL sessions)
- **External dependencies**: curl, mbedTLS, sockets - hard to mock
- **Timing sensitivity**: UAF bugs depend on cleanup timing
- **Integration scope**: Need full stack (app → curl → mbedTLS → sockets)

Parser fuzzing is easy because parsers are **stateless pure functions**.  
Network fuzzing is hard because it's **stateful integration testing**.

## Lessons Learned
- ✅ Fuzzing found 0 bugs in parsers (they're solid)
- ❌ Fuzzing found 0 bugs in network layer (not covered)
- ✅ Manual testing found 3 bugs (redirect loop, nfds=2, UAF)

**Takeaway**: Expand fuzzing to cover stateful network I/O, or accept that
manual/integration testing is needed for this layer.
