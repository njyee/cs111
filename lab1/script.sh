# simple command

ls

echo foo



# simple command exec

#exec echo bar



# simple command I/O redirection

ls -a > a.txt

cat < a.txt

cat < a.txt > b.txt



# subshell command

(echo hello world)



# subshell I/O redirection

(cat) <b.txt

(cat <b.txt) >c.txt

(cat) <c.txt >d.txt



# sequence command

echo a; echo b

echo c
echo d



# pipe command

cat < d.txt | echo

cat d.txt | tr a-z A-Z | sort -u

ls -a |
cat



# if command

if echo 1
then echo 2
fi

# while command
# until command
