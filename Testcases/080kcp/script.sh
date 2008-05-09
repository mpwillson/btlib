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
s super
s block 0
o test_db
s super
s block 0
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
s super
s block 0
o test_db
s super
s block 0
q
EOF
rm new_db
