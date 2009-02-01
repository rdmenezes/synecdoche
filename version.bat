@echo off

rem Get the svn revision number
svnversion > %1.tmp
set /p _revision= < %1.tmp
if x%_revision%==xexported set _revision=0

rem Create new version string
echo extern const char* SYNEC_SVN_VERSION = "%_revision%"; > %1.tmp

rem Read and compare the old and the new version string.
rem If they are different replace the old one by the new string
rem else keep the old one to prevent an extra rebuild.
set /p _oldrevision= < %1
set /p _newrevision= < %1.tmp
set _oldrevision=%_oldrevision:"=%
set _newrevision=%_newrevision:"=%
if "x%_oldrevision%"=="x%_newrevision%" (
	del %1.tmp
) else (
	move %1.tmp %1
)

rem Remove the used environment variables.
set _revision=
set _newrevision=
set _oldrevision=
