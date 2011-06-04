#!/usr/bin/perl

if ($#ARGV < 1) {
	print "Usage: gensuite.pl <target> (<source>)+\n";
	exit 1;
}

my @cases;

for ($i = 1; $i <= $#ARGV; $i++) {
	open IN, "<$ARGV[$i]" or die "Unable to read $ARGV[$i]";
	my @lines = grep /^TESTCASE ([a-zA-Z0-9_])+\(\)/, <IN>;
 	foreach (@lines) {
		push(@cases, ($_ =~ m/^TESTCASE ([a-zA-Z0-9_]+)\(\)/))
	}
	close IN;
}

open OUT, ">$ARGV[0]" or die "Unable to write $ARGV[0]";

print OUT "#include <test.h>\n\n";
print OUT "TESTCASE $_();\n" foreach (@cases);
print OUT "\nint main() {\n";
print OUT "\t$_();\n" foreach (@cases);
print OUT "\treturn 0;\n}\n";

close OUT

