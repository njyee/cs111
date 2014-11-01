sh script.sh > out1.txt
mv a.txt A.txt
mv b.txt B.txt
mv c.txt C.txt
mv d.txt D.txt
mv e.txt E.txt
touch a.txt
touch b.txt
touch c.txt
touch d.txt
touch e.txt
./profsh script.sh > out2.txt
diff out1.txt out2.txt
diff a.txt A.txt
diff b.txt B.txt
diff c.txt C.txt
diff d.txt D.txt
diff e.txt E.txt
