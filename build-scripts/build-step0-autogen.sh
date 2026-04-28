#!/bin/sh
git apply "./android/patches/server_protocol.def.patch"
./autogen.sh
