; Verify # reports the previous command status
echo false status should be 1
false
: #
echo true status should be 0
true
? #
echo missing file status should be 4
./tests/status-missing.bin
: #
