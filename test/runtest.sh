#!/bin/bash

# Create Ouzo script
./indexes --type uint32 -maxkeys 1 > test.ouzo
# Feed it to Ouzo
../ouzo < test.ouzo

# Get results from Ouzo and format it for comparison
../ouzo "show uint32_test" | sed -e '1,/^\s*$/d' -e '/^\s*$/d' | perl -n -e 'chomp;($k,$d)=split(/:/);@d=split(/\s+/,$d);foreach $n (@d) { print "$k:$n\n";}'|sort > actual.out
# Format test script's output
grep "^(" test.ouzo | sed -e 's/(//' -e 's/)//' -e 's/;//' | perl -n -e 'chomp;($k,$d)=split(/:/);@d=split(/\s+/,$d);foreach $n (@d) { print "$k:$n\n";}'|sort -u > expected.out

# Compare the two
diff -u expected.out actual.out
