#!/bin/sh

DIR=`pwd`

mkdir -p $DIR/../autogen
cd $DIR/../autogen/
git clone https://github.com/libressl-portable/portable.git

cd portable
./update.sh

echo 'All codes updated!'
echo 'You need to install LibreSSL by yourself'

./autogen.sh
./configure --prefix=$DIR/../autogen/libressl
make
make install
