status runtime
for i in 1 2 3; do
 /tmp/ftp -P 2121 get 192.168.1.112 binary.dat /tmp/ftp-get-$i
 echo get-$i:$?
 /tmp/ftp -P 2121 put 192.168.1.112 /tmp/ftp-get-$i uploaded-$i.dat
 echo put-$i:$?
 status runtime
done
/tmp/ftp -P 2121 get 192.168.1.112 empty.dat /tmp/ftp-empty
echo empty:$?
/tmp/ftp -P 2121 put 192.168.1.112 /tmp/ftp-empty uploaded-empty.dat
echo put-empty:$?
/tmp/ftp -P 2121 get 192.168.1.112 missing.dat /tmp/ftp-missing
echo missing:$?
/tmp/ftp -P 2121 get 192.168.1.112 binary.dat /tmp/ftp-get-1
echo exists:$?
status runtime
echo FTP_DONE
