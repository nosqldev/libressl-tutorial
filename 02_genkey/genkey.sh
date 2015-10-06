#!/bin/sh

# generate certificate and key for your testing

CAKEY=cakey
CACERT=cacert

../autogen/libressl/bin/openssl req -new -x509 -keyout $CAKEY.pem -out $CACERT.pem
../autogen/libressl/bin/openssl x509 -in $CACERT.pem -inform PEM -out $CACERT.der -outform DER
../autogen/libressl/bin/openssl pkey -in $CAKEY.pem -outform DER -out $CAKEY.der

mkdir -p ../autogen/key
mv $CAKEY.pem $CACERT.pem $CAKEY.der $CACERT.der ../autogen/key
