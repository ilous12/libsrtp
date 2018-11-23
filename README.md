<a name="introduction-to-libsrtp"></a>
# Introduction to libSRTP

./test/rtp_decoder -o 44 -f udp -s AES_CM_128_HMAC_SHA1_80 -b wpsNWUUp4Hbpk5m+J0valvXP8KSBQURKeX8SZtXq < ./srtp_crypto.pcap -d test.amr | text2pcap -t "%M:%S." -u 36880,20014 - - > ./srtp_decrypted.pcap


./rtp_decoder -o 44 -f udp -s AES_CM_128_HMAC_SHA1_80 -b wpsNWUUp4Hbpk5m+J0valvXP8KSBQURKeX8SZtXq < ./rtp.pcap -d test7.amr


./test/rtp_decoder -o 44 -f udp -s AES_CM_128_HMAC_SHA1_80 -b g+VXwk3/TEYWAZIsfxCF7NpePB7+cn9DKCQKsQqF < ./srtp_crypto.pcap -d test.amr