#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import subprocess
from pathlib import Path
import zipfile
import shutil

# --- YOUR EXACT PATHS ---
DOCX_FILE = Path("/home/super/Desktop/Power Flow/feeder34/ieee_34_node_test_feeder.docx")
IMAGE_DIR = DOCX_FILE.parent / "data" / "tables" / "photos"

# --- CREATE THE FOLDER ---
IMAGE_DIR.mkdir(parents=True, exist_ok=True)
print(f"Extracting photos from: {DOCX_FILE.name}...\n")

# --- STEP 1: EXTRACT IMAGES DIRECTLY FROM THE DOCX ZIP STRUCTURE ---
wmf_files = []

with zipfile.ZipFile(DOCX_FILE, 'r') as docx_zip:
    for item in docx_zip.namelist():
        # All images in a Word document live in the 'word/media/' folder
        if item.startswith('word/media/') and '.' in item:
            # Extract it temporarily
            extracted_file = docx_zip.extract(item, path=IMAGE_DIR)

            # Move it directly into our 'photos' folder (removes the word/media subfolders)
            final_path = IMAGE_DIR / Path(extracted_file).name
            shutil.move(extracted_file, final_path)
            print(f"🖼️ Found photo: {final_path.name}")

            # Keep track of WMFs so we can convert them to PNG
            if final_path.suffix.lower() == '.wmf':
                wmf_files.append(final_path)

# Clean up the empty 'word' folder that got created during unzipping
shutil.rmtree(IMAGE_DIR / "word", ignore_errors=True)

# --- STEP 2: CONVERT WMF TO PNG (So you can view and upload them!) ---
if wmf_files:
    print("\nConverting WMF photos to PNG so you can upload them...")
    for wmf in wmf_files:
        try:
            subprocess.run([
                "libreoffice",
                "--headless",
                "--convert-to", "png",
                str(wmf),
                "--outdir", str(IMAGE_DIR)
            ], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            print(f"✅ Converted to PNG: {wmf.name}")
        except Exception as e:
            print(f"❌ Failed to convert {wmf.name}: {e}")

print(f"\n🎉 All done! Your photos are waiting for you here:")
print(f"👉 {IMAGE_DIR}")