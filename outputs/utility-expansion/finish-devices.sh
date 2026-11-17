export PATH=/mnt/Audit/tools:/bin
cd /mnt/Audit/help
free -b > free-fixed
free -h > free-human-fixed
i=0
while [ "$i" -lt 6 ]; do
 cmp /dev/null /dev/null
 cksum /dev/null > /dev/null
 status runtime
 i=$((i+1))
done > device-repeat
printf '%s\n' READY_FOR_CANCEL
