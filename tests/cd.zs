; Failed cd must return an error to batch conditionals
cd A:/__zshell_missing__/
? echo CD FAILURE STATUS FAILED
: echo cd failure PASS
