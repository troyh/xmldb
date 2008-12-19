#!/bin/bash

# INDEX_TYPE="int64"
MAXKEYS="1000"

OUZO_PROG="../ouzo"

function runtest {
	INDEX_TYPE=$1

	echo -n "Running $INDEX_TYPE test...";
	
	# Create Ouzo script
	./indexes --type $INDEX_TYPE -maxkeys $MAXKEYS > test.ouzo
	# Feed it to Ouzo
	$OUZO_PROG < test.ouzo
	
	# Get results from Ouzo and format it for comparison
	# $OUZO_PROG -e "show $INDEX_TYPE""_test" | sed -e '1,/^\s*$/d' -e '/^\s*$/d' | perl -n -e 'chomp;($k,$d)=split(/:/);@d=split(/\s+/,$d);foreach $n (@d) { print "$k:$n\n";}'|sort > actual.out
	cat test.ouzo | egrep "^\(" | sed -e "s/^(/get $INDEX_TYPE""_test /" -e 's/:.*$/;/' | sort -u | $OUZO_PROG | perl -n -e 'chomp;($k,$d)=split(/:/);@d=split(/\s+/,$d);foreach $n (@d) { print "$k:$n\n";}' | sort --field-separator=: --key=1,2n > actual.out
	
	# Format test script's output
	grep "^(" test.ouzo | sed -e 's/(//' -e 's/)//' -e 's/;//' | perl -n -e 'chomp;($k,$d)=split(/:/);@d=split(/\s+/,$d);foreach $n (@d) { print "$k:$n\n";}'|sort -u | sort --field-separator=: --key=1,2n > expected.out
	
	# Compare the two
	if ! diff -u actual.out expected.out > runtest.diff; then
		MISSING=`egrep -c "^-" runtest.diff`
		EXTRA=`egrep -c "^\+" runtest.diff`
		echo "$INDEX_TYPE test failed: $MISSING missing, $EXTRA extra keys";
	else
		echo "OK";
		rm -f runtest.diff actual.out expected.out *.index test.ouzo
	fi
}

if [ "x$1" == "x" ]; then
	TYPES="string int8 int16 int32 int64 uint8 uint16 uint32 uint64 double char8 date time";
else
	TYPES=$1
fi

for T in $TYPES; do
	runtest $T
done
