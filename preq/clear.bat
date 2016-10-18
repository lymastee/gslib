@echo off
del /f /q /s *.suo *.sdf *.opensdf *.ncb *.old *.user
del /f /q /s /a:h *.suo *.sdf *.opensdf *.ncb *.old *.user
rd /q /s debug
rd /q /s release
cd cximage
call clear.bat