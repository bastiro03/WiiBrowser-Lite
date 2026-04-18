#!/bin/bash
# Verification script to confirm getsockname is properly disabled

echo "=== Checking HAVE_GETSOCKNAME in curl_config.h ==="
grep "HAVE_GETSOCKNAME" third_party/curl/lib/curl_config.h

echo ""
echo "=== Checking for getsockname error message in libcurl.a ==="
strings third_party/curl/lib/.libs/libcurl.a | grep -i "getsockname.*failed" || echo "NOT FOUND (expected)"

echo ""
echo "=== Checking for getsockname error message in wiibrowserlite.elf ==="
strings wiibrowserlite.elf | grep -i "getsockname.*failed" || echo "NOT FOUND (expected)"

echo ""
echo "=== Checking for getsockname symbol references in libcurl.a ==="
powerpc-eabi-nm third_party/curl/lib/.libs/libcurl.a 2>/dev/null | grep -i getsockname || echo "NO REFERENCES (expected)"

echo ""
echo "=== Build timestamps ==="
ls -l third_party/curl/lib/curl_config.h third_party/curl/lib/.libs/libcurl.a wiibrowserlite.elf wiibrowserlite.dol 2>/dev/null | awk '{print $6,$7,$8,$9}'
