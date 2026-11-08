echo alpha > /tmp/in.txt
cat /tmp/in.txt > /tmp/copy.txt
cat < /tmp/in.txt | cat | cat > /tmp/pipe.txt
echo beta >> /tmp/copy.txt
cat /missing 2> /tmp/err.txt > /tmp/out.txt
echo $? > /tmp/error-status.txt
cat /missing > /tmp/combined.txt 2>&1
cat /missing 2>&1 > /tmp/ordered.txt | cat > /tmp/order-err.txt
printf data >&-
echo $? > /tmp/closed-status.txt
echo okay > /tmp/after-closed.txt
cat <&- 2> /tmp/closed-input.txt
echo $? > /tmp/closed-input-status.txt
false | cat > /tmp/false-pipe.txt
echo $? > /tmp/last-status.txt
true | false
echo $? > /tmp/false-status.txt
echo IO_COMPLETE
