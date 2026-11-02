#!/usr/bin/env python3
"""Source/resource invariants, not a substitute for compilation or hardware."""
import ast
import hashlib
import importlib.util
import pathlib
import re
import struct
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET

ROOT = pathlib.Path(__file__).resolve().parents[1]


def read(name):
    return (ROOT / name).read_text(encoding="utf-8", errors="replace")


class SourceChecks(unittest.TestCase):
    def test_asset_generator_and_development_package(self):
        sys.dont_write_bytecode = True
        def load(name, file):
            spec = importlib.util.spec_from_file_location(name, ROOT / file)
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            return module
        assets = load("asset_builder", "scripts/build_assets.py")
        packager = load("hbc_packager", "scripts/package.py")
        base = pathlib.Path("/tmp/opencode")
        base.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(prefix="wiibrowser-check-", dir=base) as directory:
            directory = pathlib.Path(directory)
            files = sorted((ROOT / "assets/images").rglob("*.png"))
            header = directory / "embedded_assets.h"
            assets.headers(header, files)
            generated = header.read_text()
            for file in files:
                self.assertIn(f"return {assets.symbol(file)}_size;", generated)
            assembly = directory / "asset.S"
            assets.embed(files[0], assembly)
            self.assertIn(".balign 32", assembly.read_text())
            self.assertIn(".incbin", assembly.read_text())
            self.assertIn(".long .L", assembly.read_text())
            # A data fixture, never an executed Wii binary.
            words = [0] * 57
            words[0], words[18], words[36], words[56] = 256, 0x80004000, 4, 0x80004000
            binary = directory / "fixture.dol"
            binary.write_bytes(struct.pack(">57I", *words).ljust(256, b"\0") + b"test")
            output = directory / "development.zip"
            packager.build(binary, output)
            import zipfile
            with zipfile.ZipFile(output) as package:
                names = set(package.namelist())
                self.assertIn("apps/wiibrowser/cacert.pem", names)
                self.assertIn("apps/wiibrowser/licenses/DejaVu.txt", names)
                self.assertFalse(any("wiibrowser-lite/" in name for name in names))
                metadata = ET.fromstring(package.read("apps/wiibrowser/meta.xml"))
                self.assertRegex(metadata.findtext("release_date"), r"^\d{14}$")

    def test_embedded_png_preflight_limits(self):
        sys.dont_write_bytecode = True
        spec = importlib.util.spec_from_file_location("png_builder", ROOT / "scripts/build_assets.py")
        assets = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(assets)
        for path in (ROOT / "assets/images").rglob("*.png"):
            original = path.read_bytes()
            data = assets.compiled_png(original)
            def pixels(stream):
                result, cursor = bytearray(), 8
                while cursor < len(stream):
                    length = struct.unpack_from(">I", stream, cursor)[0]
                    if stream[cursor + 4:cursor + 8] == b"IDAT":
                        result.extend(stream[cursor + 8:cursor + 8 + length])
                    cursor += length + 12
                return bytes(result)
            self.assertEqual(pixels(original), pixels(data), str(path))
            width, height = struct.unpack_from(">II", data, 16)
            self.assertLessEqual(width * height, 1024 * 1024, str(path))
            cursor = 8
            while cursor < len(data):
                length = struct.unpack_from(">I", data, cursor)[0]
                kind = data[cursor + 4:cursor + 8]
                self.assertNotIn(kind, (b"iCCP", b"zTXt", b"iTXt"), str(path))
                self.assertLessEqual(cursor + 12 + length, len(data), str(path))
                cursor += 12 + length
            self.assertEqual(cursor, len(data))

    def test_quarantined_features(self):
        config = read("src/core/config.h")
        for feature in ("UPDATER", "ARCHIVES", "UPLOADS"):
            self.assertRegex(config, rf"#define WBL_ENABLE_{feature} 0")
        self.assertIn("#undef WIIFLOW", config)
        self.assertNotIn("CURLFORM_FILE", read("src/network/httplib.cpp"))
        self.assertNotIn('remove(data->save.name)', read("src/network/transfer.cpp"))
        self.assertNotIn("ArchiveHandle archive", read("src/filesystem/fileop.cpp"))
        self.assertIn("$(error Forwarder quarantined", read("forwarder/Makefile"))

    def test_no_unsafe_release_inputs(self):
        http = read("src/network/httplib.cpp")
        self.assertNotIn("multipartform", http)
        self.assertNotIn("sd:/apps/wiibrowser/cacert.pem", http)
        self.assertIn("CURLOPT_SSL_VERIFYPEER, 1L", http)
        self.assertIn("CURLOPT_SSL_VERIFYHOST, 2L", http)
        self.assertNotIn("strrchr(urlCopy,", http)
        self.assertNotIn('fopen ("sd:/page.html"', read("src/ui/menu.cpp"))
        self.assertNotIn("fscanf", read("src/core/liste.cpp"))
        self.assertNotIn("textDynNum = TextLines.size()", read("external/libwiigui/gui_longtext.cpp"))

    def test_build_inputs(self):
        makefile = read("Makefile")
        for stale in ("libs/wii/libzip.a", "-lunrar", "-lsevenzip", "-llua", "scripts/embeddata.sh"):
            self.assertNotIn(stale, makefile)
        self.assertIn("-iquote", makefile)
        self.assertIn("pkg-config --libs --static freetype2", makefile)
        self.assertNotIn("-liconv", makefile)
        self.assertIn('#include "url_helper.h"', read("src/html_parser/html.cpp"))
        self.assertIn('bool CheckFile(const char *filepath);', read("src/filesystem/fileop.h"))
        self.assertNotIn("#include <urlcode.h>", read("src/network/urlcode.c"))
        self.assertNotIn("#include <utils/timer.h>", read("external/libwiigui/gui.h"))
        self.assertIn("%parse-param {void *yyparam}", read("src/html_parser/css_syntax.y"))
        for name in ("scripts/build_assets.py", "scripts/package.py", "scripts/clean.py", "tests/check_source.py"):
            ast.parse(read(name), filename=name)

    def test_package_metadata(self):
        for name in ("HBC/meta.xml", "HBC/meta.xml.in"):
            app = ET.fromstring(read(name))
            self.assertTrue(app.findtext("version"))
            if name.endswith(".xml"):
                self.assertRegex(app.findtext("release_date", ""), r"^\d{14}$")
            self.assertIsNone(app.find("ahb_access"))
        icon = (ROOT / "HBC/icon.png").read_bytes()
        self.assertEqual(struct.unpack_from(">II", icon, 16), (128, 48))
        self.assertIn("Autoupdate = 0", read("HBC/wiibrowser.cfg"))
        self.assertIn("UnzipFile = 0", read("HBC/wiibrowser.cfg"))

    def test_trust_and_font_resources(self):
        certificates = (ROOT / "HBC/cacert.pem").read_bytes()
        self.assertEqual(hashlib.sha256(certificates).hexdigest(), read("HBC/cacert.sha256").split()[0])
        self.assertGreater(certificates.count(b"-----BEGIN CERTIFICATE-----"), 50)
        for path in (ROOT / "assets/fonts").glob("*.ttf"):
            self.assertNotIn("Calibri".encode("utf-16-be"), path.read_bytes())
        self.assertTrue((ROOT / "assets/fonts/LICENSE-DejaVu.txt").is_file())


if __name__ == "__main__":
    unittest.main(argv=[sys.argv[0]], verbosity=2)
