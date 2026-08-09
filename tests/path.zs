; Invalid PATH update must preserve prior entries
set PATH=A:/,B:/
? set PATH=A:/,,C:/
? echo PATH INVALID UPDATE FAILED
: echo path invalid update PASS
set PATH
