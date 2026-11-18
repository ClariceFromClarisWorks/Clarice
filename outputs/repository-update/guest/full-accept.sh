echo COMPLETE-UPDATE-BEGIN
clarice update --check --source http://192.168.1.112:8083/full/index.txt || exit 1
clarice update -y --source http://192.168.1.112:8083/full/index.txt || exit 2
echo CORE-HANDOFF-RETURNED
