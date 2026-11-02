#!/usr/bin/env python3
"""Create a deterministic development HBC bundle, without publishing it."""
import argparse
import datetime
import hashlib
import html
import os
import pathlib
import re
import struct
import subprocess
import xml.etree.ElementTree as ET
import zipfile

ROOT = pathlib.Path(__file__).resolve().parents[1]


def build(binary, output):
    data = pathlib.Path(binary).read_bytes()
    if not 256 <= len(data) <= 24 * 1024 * 1024:
        raise ValueError("missing, empty or oversized development DOL")
    words = struct.unpack_from(">57I", data)
    sections = [(words[i], words[36 + i]) for i in range(7)]
    sections += [(words[7 + i], words[43 + i]) for i in range(11)]
    if not any(size for _, size in sections):
        raise ValueError("DOL has no sections")
    if any(size and (offset < 256 or offset > len(data) or size > len(data) - offset) for offset, size in sections):
        raise ValueError("DOL section exceeds file")
    config = (ROOT / "src/core/config.h").read_text()
    version = re.search(r'#define WBL_VERSION "([A-Za-z0-9.+-]+)"', config).group(1)
    epoch = int(os.environ.get("SOURCE_DATE_EPOCH") or subprocess.check_output(
        ["git", "log", "-1", "--format=%ct"], cwd=ROOT, text=True).strip())
    date = datetime.datetime.fromtimestamp(epoch, datetime.timezone.utc)
    metadata = (ROOT / "HBC/meta.xml.in").read_text().replace("@VERSION@", html.escape(version)).replace(
        "@RELEASE_DATE@", date.strftime("%Y%m%d%H%M%S"))
    app = ET.fromstring(metadata)
    if not re.fullmatch(r"\d{14}", app.findtext("release_date", "")):
        raise ValueError("invalid HBC release date")
    icon = (ROOT / "HBC/icon.png").read_bytes()
    if icon[:8] != b"\x89PNG\r\n\x1a\n" or struct.unpack_from(">II", icon, 16) != (128, 48):
        raise ValueError("HBC icon must be 128x48 PNG")
    certificates = (ROOT / "HBC/cacert.pem").read_bytes()
    checksum = (ROOT / "HBC/cacert.sha256").read_text().split()[0]
    if hashlib.sha256(certificates).hexdigest() != checksum:
        raise ValueError("trust bundle differs from its reviewed checksum")
    files = {
        "boot.dol": data, "meta.xml": metadata.encode(), "icon.png": icon,
        "wiibrowser.cfg": (ROOT / "HBC/wiibrowser.cfg").read_bytes(),
        "cacert.pem": certificates, "README.md": (ROOT / "README.md").read_bytes(),
        "RELEASE.md": (ROOT / "RELEASE.md").read_bytes(),
        "licenses/GPL-3.0.txt": (ROOT / "LICENSE").read_bytes(),
        "licenses/THIRD_PARTY.md": (ROOT / "THIRD_PARTY.md").read_bytes(),
        "licenses/DejaVu.txt": (ROOT / "assets/fonts/LICENSE-DejaVu.txt").read_bytes(),
    }
    destination = pathlib.Path(output)
    destination.parent.mkdir(parents=True, exist_ok=True)
    temporary = destination.with_suffix(destination.suffix + ".new")
    with zipfile.ZipFile(temporary, "w", zipfile.ZIP_DEFLATED) as archive:
        for name, content in sorted(files.items()):
            info = zipfile.ZipInfo("apps/wiibrowser/" + name, date.timetuple()[:6])
            info.external_attr = 0o100644 << 16
            info.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(info, content)
    temporary.replace(destination)
    print(f"Development bundle: {destination} (NOT release-cleared; see RELEASE.md)")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", required=True)
    parser.add_argument("--output", required=True)
    options = parser.parse_args()
    build(options.binary, options.output)
