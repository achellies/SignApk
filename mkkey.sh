#!/bin/sh
AUTH='/C=US/ST=California/L=Mountain View/O=Android/OU=Android/CN=Android/emailAddress=android@android.com'

openssl genrsa -3 -out apkpack.pem 2048

openssl req -new -x509 -key apkpack.pem -out apkpack.x509.pem -days 10000 -subj "$AUTH"

openssl pkcs8 -in apkpack.pem -topk8 -outform DER -out apkpack.pk8
