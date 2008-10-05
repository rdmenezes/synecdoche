@echo off
rem This file is part of Synecdoche.
rem http://synecdoche.googlecode.com/
rem Copyright (C) 2005 University of California
rem
rem Synecdoche is free software: you can redistribute it and/or modify
rem it under the terms of the GNU Lesser General Public License as published
rem by the Free Software Foundation, either version 3 of the License, or
rem (at your option) any later version.
rem
rem Synecdoche is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
rem See the GNU Lesser General Public License for more details.
rem
rem You should have received a copy of the GNU Lesser General Public
rem License with Synecdoche.  If not, see <http://www.gnu.org/licenses/>.

FOR /F "usebackq delims==" %%I IN ('%1') DO set PROJECTROOTDIR=%%~I\..
FOR /F "usebackq delims==" %%J IN ('%2') DO set OUTPUTDIR=%%~J
FOR /F "usebackq delims==" %%K IN ('%3') DO set PLATFORMNAME=%%~K
FOR /F "usebackq delims==" %%L IN ('%4') DO set CONFIGNAME=%%~L

if not exist %OUTPUTDIR%\dbghelp.dll (
    echo Copying dbghelp to the output directory...
    copy "%PROJECTROOTDIR%\win_build\installerv2\redist\Windows\%PLATFORMNAME%\dbghelp.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\dbghelp95.dll (
    echo Copying dbghelp95 to the output directory...
    copy "%PROJECTROOTDIR%\win_build\installerv2\redist\Windows\%PLATFORMNAME%\dbghelp95.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\symsrv.dll (
    echo Copying symsrv to the output directory...
    copy "%PROJECTROOTDIR%\win_build\installerv2\redist\Windows\%PLATFORMNAME%\symsrv.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\symsrv.yes (
    echo Copying symsrv.yes to the output directory...
    copy "%PROJECTROOTDIR%\win_build\installerv2\redist\Windows\%PLATFORMNAME%\symsrv.yes" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\srcsrv.dll (
    echo Copying srcsrv.dll to the output directory...
    copy "%PROJECTROOTDIR%\win_build\installerv2\redist\Windows\%PLATFORMNAME%\srcsrv.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\libcurld.dll (
    echo Copying libcurld to the output directory...
    copy "%PROJECTROOTDIR%\curl\mswin\%PLATFORMNAME%\%CONFIGNAME%\bin\libcurld.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\libcurl.dll (
    echo Copying libcurl to the output directory...
    copy "%PROJECTROOTDIR%\curl\mswin\%PLATFORMNAME%\%CONFIGNAME%\bin\libcurl.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\ca-bundle.crt (
    echo Copying ca-bundle.crt to the output directory...
    copy "%PROJECTROOTDIR%\curl\ca-bundle.crt" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\libeay32.dll (
    echo Copying libeay32 to the output directory...
    copy "%PROJECTROOTDIR%\openssl\mswin\%PLATFORMNAME%\%CONFIGNAME%\bin\libeay32.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\ssleay32.dll (
    echo Copying ssleay32 to the output directory...
    copy "%PROJECTROOTDIR%\openssl\mswin\%PLATFORMNAME%\%CONFIGNAME%\bin\ssleay32.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\zlib1d.dll (
    echo Copying zlib1d to the output directory...
    copy "%PROJECTROOTDIR%\zlib\mswin\%PLATFORMNAME%\%CONFIGNAME%\bin\zlib1d.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\zlib1.dll (
    echo Copying zlib1 to the output directory...
    copy "%PROJECTROOTDIR%\zlib\mswin\%PLATFORMNAME%\%CONFIGNAME%\bin\zlib1.dll" "%OUTPUTDIR%"
)

EXIT /B 0
