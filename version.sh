#!/bin/sh

revision=`svnversion 2>/dev/null`
if test $revision && test "x$revision" != "xexported"; then
    revision="\"$revision\"";
else
    revision=0;
fi
new_revision="extern const char* SYNEC_SVN_VERSION = $revision;";
old_revision=`cat "$1" 2> /dev/null`;

if test "$new_revision" != "$old_revision"; then
    echo "$new_revision" > "$1"
fi;
