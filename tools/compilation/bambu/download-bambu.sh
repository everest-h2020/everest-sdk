#!/bin/bash
wget https://release.bambuhls.eu/bambu-2024.03.AppImage
chmod +x bambu-*.AppImage
ln -sf $PWD/bambu-*.AppImage ./bambu
ln -sf $PWD/bambu-*.AppImage ./mlir-opt-12
ln -sf $PWD/bambu-*.AppImage ./mlir-translate-12
