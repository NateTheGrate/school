#!/usr/bin/bash

gcc -std=gnu99 -o otp_enc_d otp_enc_d.c -Wall
gcc -std=gnu99 -o otp_dec_d otp_dec_d.c -Wall

gcc -std=gnu99 -o otp_enc otp_enc.c -Wall
gcc -std=gnu99 -o otp_dec otp_dec.c -Wall

gcc -std=gnu99 -o keygen keygen.c -Wall

killall -q -u $USER otp_*
