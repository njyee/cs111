sort read-command.c

sleep 1
sleep 1

ls -a | cat > f.txt

mv f.txt g.txt

ls -a > i.txt

diff g.txt i.txt

#mkdir test

#ps

grep command read-command.c

less main.c

who | cat | cat | cat > h.txt

whoami







# simple command

ls

echo foo

echo wordthatislongerthan8bytes



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

if echo 3; then echo 4; fi

if echo 5; then echo 6; fi > e.txt

# while command
# until command
