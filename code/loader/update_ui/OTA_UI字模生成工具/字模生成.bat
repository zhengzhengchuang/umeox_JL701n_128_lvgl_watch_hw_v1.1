@echo off
echo fontsize:12 16x16(����) 8x16(����)
echo fontsize:24 32x32(����) 16x32(����)
echo fontsize:48 64x64(����) 32x64(����)
set fontsize=24
ota_text_matrix.exe -intxt text.txt -fontsize %fontsize% -outfile text_matrix.h -format IMC_OSD1
echo ��ģ�������.
pause