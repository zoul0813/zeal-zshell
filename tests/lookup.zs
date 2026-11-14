; Run from the parent of tests; restore your preferred PATH afterward.
set PATH=./tests/,A:/
longfilename1
? echo long .zs lookup PASS
: echo LONG .ZS LOOKUP FAILED
which longfilename1
? echo which long .zs PASS
: echo WHICH LONG .ZS FAILED
