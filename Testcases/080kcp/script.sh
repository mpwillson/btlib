#!/bin/sh
../../bt <<EOF
c test_db
e ../defdata.bt
dr test
e ../defdata.bt
q
EOF
../../kcp test_db new_db
../../bt <<EOF
o new_db
f
ld
cr test
f
ld
q
EOF
echo "Files should not differ"
diff -s test_db new_db
echo "Changing new_db..."
../../bt <<EOF
o test_db
rd aaaa0057
rd aaaa0058
rd aaaa0060
rd aaaa0038
rd aaaa0001
cr test
rd aaaa0057
rd aaaa0058
rd aaaa0060
rd aaaa0038
rd aaaa0001
q
EOF
../../kcp test_db new_db
echo "Files should now differ"
# Make diff output for binary files same on Linux as on other systems
diff test_db new_db|sed -e "s/Binary file/File/"
rm new_db
