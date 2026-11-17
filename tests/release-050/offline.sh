pwd > /tmp/offline-results
ls /bin >> /tmp/offline-results
ipconfig >> /tmp/offline-results 2>&1
echo network-status:$? >> /tmp/offline-results
awk 'BEGIN {print "OFFLINE_OK"}' >> /tmp/offline-results
appinstall install /mnt/Clarice/Utilities/wc flush-offline
echo OFFLINE_DONE
