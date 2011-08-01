#!/usr/bin/perl

sub regtest {
	die "Nested tests are not allowed" if $inside;
	my ($name, $extra) = @_;
	print OUT "TEST $name() { ENTER(); $extra\n";
	push(@tests, $name);
	$inside = 1;
}

if ($#ARGV != 1) {
	print "Usage: gentest.pl <source> <target>\n";
	exit 1;
}

open IN, "<$ARGV[0]" or die "Unable to open $ARGV[0] for reading";
open OUT, ">$ARGV[1]" or die "Unable to open $ARGV[1] for writing";

my ($suitename) = ($ARGV[0] =~ m/([a-z]+)\.c/);

$inside = 0;
$final = "";
@tests = ();

while (<IN>) {
	my $line = $_;
	if ($line =~ /^TEST /) {
		my ($testname) = ($line =~ m/TEST[ ]+([a-zA-Z0-9_]+)\(\)[ ]*\{/);
		regtest($testname, "");
	} elsif ($line =~ /^TESTREQ [0-9]+ {$/) {
		my ($testid) = ($line =~ m/TESTREQ ([0-9]+)/);
		regtest("$suitename\_REQ$testid", "BEFORE(REQ$testid);");
		$final = "AFTER; ";
	} elsif ($line =~ /^TESTBADREQ [0-9]+ {$/) {
		my ($testid) = ($line =~ m/TESTBADREQ ([0-9]+)/);
		regtest("$suitename\_BADREQ$testid",
		        "BEFOREBAD(BADREQ$testid);");
		$final = "AFTERBAD; ";
	} elsif ($line =~ /^}/) {
		if ($inside) {
			print OUT "$final SUCCESS();";
			$final = "";
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

