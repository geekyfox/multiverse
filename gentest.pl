#!/usr/bin/perl

if ($#ARGV != 1) {
	print "Usage: gentest.pl <source> <target>\n";
	exit 1;
}

open IN, "<$ARGV[0]" or die "Unable to open $ARGV[0] for reading";
open OUT, ">$ARGV[1]" or die "Unable to open $ARGV[1] for writing";

my $inside = 0;
my @tests = ();

while (<IN>) {
	my $line = $_;
	if ($line =~ /^TEST /) {
		die "Nested tests are not allowed" if $inside;
		my ($testname) = ($line =~ m/TEST[ ]+([a-zA-Z0-9_]+)\(\)[ ]*\{/);
		print OUT "TEST $testname() { ENTER();\n";
		push(@tests, $testname);
		$inside = 1;
	} elsif ($line =~ /^}/) {
		if ($inside) {
			print OUT "SUCCESS(); ";
			$inside = 0;
		}
		print OUT "}\n";
	} else {
		print OUT $line;
	}
}

if ($#tests >= 0) {
	my ($casename) = ($ARGV[1] =~ /[a-z]+\/([a-z]+)\.c/);
	print OUT "TESTCASE perform_$casename\_testsuite() { ENTER(); \n"; 
	print OUT "\t$_();\n" foreach (@tests);
	print OUT "SUCCESS() ;}\n\n\n";
}

close OUT;

