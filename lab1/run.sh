sh script.sh > out1.txt
cp a.txt A.txt
cp b.txt B.txt
cp c.txt C.txt
cp d.txt D.txt
./profsh script.sh > out2.txt
diff out1.txt out2.txt
diff a.txt A.txt
diff b.txt B.txt
diff c.txt C.txt
diff d.txt D.txt
