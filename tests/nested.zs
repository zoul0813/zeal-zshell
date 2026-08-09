; Parent state must survive nested batch execution
echo nested parent before child
exec ./tests/nested-1.zs
echo nested parent resumed
echo nested PASS
