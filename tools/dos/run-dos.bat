tools\dos\dosbox-x\dosbox-x -conf tools\dos\dosbox-x.conf -fastlaunch -exit %1.exe
IF %ERRORLEVEL% NEQ 0 (Exit /b 1)
