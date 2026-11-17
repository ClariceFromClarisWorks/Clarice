cd /mnt/Audit
./grep.clx -F x record > grep-new.out
echo GREP:$?
status runtime
./sed.clx s/x/y/g record > sed-new.out
echo SED:$?
status runtime
echo PORTS_DONE
