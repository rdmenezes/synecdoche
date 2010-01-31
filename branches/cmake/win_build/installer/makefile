# This file is part of Synecdoche.
# http://synecdoche.googlecode.com/
# Copyright (C) 2009 David Barnard
#
# Synecdoche is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Synecdoche is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License with Synecdoche.  If not, see <http://www.gnu.org/licenses/>.

# Synecdoche Windows installer makefile

# Dot directive lists extensions for inference-rule matching
.SUFFIXES : .wxs .wixobj

# Command macros
candle=candle -nologo
light=light -nologo
lit=lit -nologo
copy=copy /y $** $@

# Defaults
platform=Win32
config=Release

# Paths
# All paths are relative to the win_build directory.
root=..
project=installer
loc=$(project)Localization
out=$(platform)^\$(config)
int=$(out)^\$(project)^\obj
source=$(out)^\$(project)^\SourceDir
deps=$(SYNEC_DEPENDS)


# Default target
all: "$(out)\synecdoche.msi"

# Bootstrapper is currently not built
#setup.exe: synecdoche.msi
#	$(burn)

# WiX linker (light) builds the final installer package
"$(out)\synecdoche.msi": SourceDir "$(int)\synecdoche.wixlib"
	$(light) -ext WixUIExtension -b "$(out)\$(project)\\" -out $@ "$(int)\synecdoche.wixlib"

# WiX library tool (lit) combines wixobj files into a single library
"$(int)\synecdoche.wixlib": "$(int)\synecdoche.wixobj" "$(int)\Components.wixobj"
	$(lit) -ext WixUtilExtension -out $@ $**

# WiX compiler (candle) batch mode inference-rule
{$(project)\}.wxs.wixobj::
	$(candle) -out "$(int)\\" $<


# Installation files
# ...
SourceDir: "$(source)" \
"$(source)\COPYING.txt" \
"$(source)\COPYING.LESSER.txt" \
"$(source)\COPYRIGHT.txt" \
"$(source)\NEWS.txt" \
"$(source)\License.rtf" \
"$(source)\synecd.exe" \
"$(source)\synecmgr.exe" \
"$(source)\syneccmd.exe" \
"$(source)\idlemon.exe" \
"$(source)\synecdoche.dll" \
"$(source)\synecdoche.scr" \
"$(source)\Microsoft_VC80_CRT_x86.msm" \
"$(source)\libcurl.dll" \
"$(source)\libeay32.dll" \
"$(source)\ssleay32.dll" \
"$(source)\zlib1.dll" \
"$(source)\ca-bundle.crt" \
"$(source)\all_projects_list.xml" \

$(source):
	-mkdir "$(source)"

# General files
# As a convenience for Windows users, we add a .txt suffix to the standard text files.

"$(source)\COPYING.txt": "$(root)\COPYING"
	$(copy)

"$(source)\COPYING.LESSER.txt": "$(root)\COPYING.LESSER"
	$(copy)

"$(source)\COPYRIGHT.txt": "$(root)\COPYRIGHT"
	$(copy)

"$(source)\NEWS.txt": "$(root)\NEWS"
	$(copy)

"$(source)\License.rtf": "$(root)\resources\License.rtf"
	$(copy)

# Project output files
# This is a little confusing. Files are copied from the build output directory to
# the installer source directory.

"$(source)\synecd.exe": "$(out)\synecd.exe"
	$(copy)

"$(source)\synecmgr.exe": "$(out)\synecmgr.exe"
	$(copy)

"$(source)\syneccmd.exe": "$(out)\syneccmd.exe"
	$(copy)

"$(source)\idlemon.exe": "$(out)\idlemon.exe"
	$(copy)

"$(source)\synecdoche.dll": "$(out)\synecdoche.dll"
	$(copy)

"$(source)\synecdoche.scr": "$(out)\synecdoche.scr"
	$(copy)

# Dependencies

"$(source)\Microsoft_VC80_CRT_x86.msm": "$(deps)\merge\Microsoft_VC80_CRT_x86.msm"
	$(copy)

"$(source)\libcurl.dll": "$(deps)\curl\mswin\$(platform)\$(config)\bin\libcurl.dll"
	$(copy)

"$(source)\libeay32.dll": "$(deps)\openssl\mswin\$(platform)\$(config)\bin\libeay32.dll"
	$(copy)

"$(source)\ssleay32.dll": "$(deps)\openssl\mswin\$(platform)\$(config)\bin\ssleay32.dll"
	$(copy)

"$(source)\zlib1.dll": "$(deps)\zlib\mswin\$(platform)\$(config)\bin\zlib1.dll"
	$(copy)

# The certificate file is a shared resource.
"$(source)\ca-bundle.crt": "$(root)\resources\ca-bundle.crt"
	$(copy)

"$(source)\all_projects_list.xml": "$(root)\resources\all_projects_list.xml"
	$(copy)




# Delete intermediate and output files
clean:
	del /f /q "$(source)"
	-rmdir /s /q "$(source)"
	del /f /q "$(int)"
	-rmdir /s /q "$(int)"
	-del /f /q "$(out)\synecdoche.msi"
	-del /f /q "$(out)\synecdoche.wixpdb"
