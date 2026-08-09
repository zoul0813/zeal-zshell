; H-06 startup argument parsing
echo startup valid quiet batch
zshell -q ./tests/nested.zs
? echo startup valid PASS

echo startup missing path
zshell -q
: echo startup missing path PASS

echo startup invalid option
zshell -x ./tests/nested.zs
: echo startup invalid option PASS
