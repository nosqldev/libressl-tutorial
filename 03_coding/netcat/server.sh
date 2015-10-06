#!/bin/sh

../../autogen/portable/apps/nc/nc -v -4 -c -R ../../autogen/key/cacert.pem -C ../../autogen/key/cacert.pem -K ../../autogen/key/cakey.pem -l localhost 9999
