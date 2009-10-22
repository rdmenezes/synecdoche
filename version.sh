#!/bin/sh

revision=`env LANG=C svnversion "$2" 2>/dev/null`
if test $revision && test "x$revision" != "xexported"; then
    revision="\"$revision\"";
else
    revision=0;
fi

# Create new version string
new_revision="const char* SYNEC_SVN_VERSION = $revision;";

# Read and compare the old and the new version string.
# If they are different replace the old one by the new string
# else keep the old one to prevent an extra rebuild.
old_revision=`cat "$1" 2> /dev/null`;

if test "$new_revision" != "$old_revision"; then
    echo "$new_revision" > "$1"
fi;
