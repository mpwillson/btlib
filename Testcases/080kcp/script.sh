#!/bin/sh
../../bt <<EOF
c test_db
e ../defdata.bt
dr test
e ../defdata.bt
q
EOF
../../kcp test_db new_db
echo "Files should be identical"
../../bt <<EOF
o new_db
f
lk
o test_db
f
lk
q
EOF
echo "Changing test_db..."
../../bt <<EOF
o test_db
e ../remdata.bt
q
EOF
../../kcp test_db new_db
echo "Files should now differ"
../../bt <<EOF
o new_db
f
lk
o test_db
f
lk
q
EOF
rm new_db
