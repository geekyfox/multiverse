#!/usr/bin/perl

if ($#ARGV < 0) {
	print "Usage: stylus.pl (<source>)+\n";
	exit 1;
}

my @cases;

for ($i = 0; $i <= $#ARGV; $i++) {
	open IN, "<$ARGV[$i]" or die "Unable to read $ARGV[$i]";
	my ($loc, $inside, $mark) = 0, 0, 0;
	my $fname;
	while (<IN>) {
		$loc++;
		my $line = $_;
		if ($line =~ /^TEST [a-zA-Z0-9_]+\(/) {
			die "Nested function @ $ARGV[$i]:$loc" if ($inside); 
			my ($name) = $line =~ m/^TEST ([a-zA-Z0-9_]+)\(/;
			$inside = 1;
			$mark = $loc;
			$fname = $name;
		}
		elsif ($line =~ /^TESTREQ [0-9]+ {/) {
			die "Nested function @ $ARGV[$i]:$loc" if ($inside);
			my ($name) = $line =~ m/^TESTREQ ([0-9]+)/;
			$inside = 1;
			$mark = $loc;
			$fname = "TESTREQ #$name";
		}
		elsif ($line =~ /^TESTBADREQ [0-9]+ {/) {
			die "Nested function @ $ARGV[$i]:$loc" if ($inside);
			my ($name) = $line =~ m/^TESTBADREQ ([0-9]+)/;
			$inside = 1;
			$mark = $loc;
			$fname = "TESTBADREQ #$name";
		}
		elsif ($line =~ /^(inline )*(static )*[a-z_\*]+ [a-zA-Z0-9_]+\(/) {
			die "Nested function @ $ARGV[$i]:$loc" if ($inside); 
			my ($inline, $stat, $name) = $line =~
				m/^(inline )*(static )*[a-z_\*]+ ([a-zA-Z0-9_]+)\(/;
			$inside = 1;
			$mark = $loc;
			$fname = $name;
		}
		elsif ($line =~ m/^}/) {
			die "Unmatched '}' @ $ARGV[$i]:$loc" if (not $inside);
			$inside = 0;
			$size = $loc - $mark;
			if ($size > 25) {
				my $limit;
				if ($line =~ /style:[0-9]+/) {
					($limit) = $line =~ m/style\:([0-9]+)/;
					print "$size ($limit)\t$fname\t$ARGV[$i]:$mark\n"
						if $size > $limit;
				} else {
					print "$size\t$fname\t$ARGV[$i]:$mark\n";
				}
			}
		}
	} 
	close IN
}

