#!/usr/bin/perl

use strict;
use warnings;

use Data::Dumper;
use DBI;

my $GENOME_SIZE = 16;
my $MIN_POPULATION = 20;
my $MAX_POPULATION = 1000;

my @values = qw(v00 v01 v02 v03 v04 v05 v06 v07 v08 v09 v0a v0b v0c v0d v0e v0f v10 v11 v12 v13 v14 v15 v16 v17 v18 v19 v1a v1b v1c v1d v1e);
my $dbh = DBI->connect('DBI:mysql:database=imgur;host=localhost', 'imgur', 'imgur') or die 'Cannot connect to mysql';
my $good = 0;
my $bad = 0;

sub getBit {
	return (rand(1) < 0.5);
}

sub query {
	my $q = shift;
	my $sth = $dbh->prepare($q) or die "Cannot prepare query '$q'";
	$sth->execute or die "Cannot execute query '$q'";
	return $sth;
}


sub getGene {
	my $gene;
	if(getBit()) {
		# vX >< Y
		my $item = $values[rand(@values)];
		my $cmp = getBit() ? '<' : '>';
		my $val = rand(1);
		$gene = "$item $cmp $val";
	} else {
		# vA +-* k >< vB
		my $item1 = $values[rand(@values)];
		my $k = rand(1);
		my $item2;
		my $cmp = getBit() ? '<' : '>';
		do {
			$item2 = $values[rand(@values)];
		} while ($item1 eq $item2);
		$gene = "$item1 * $k $cmp $item2";
	}
	return $gene;
}

sub getGenome {
	my @ar = ();
	for(my $i=0; $i<$GENOME_SIZE; $i++) {
		push (@ar, getGene());
	}
	return @ar;
}

sub fitness {
	my $q = "SELECT sum((case score when 1 then 100/$good else -800/$bad end)) FROM imgur WHERE ".join (" AND ", @_);
	my $sth = query $q;
	my @res = $sth->fetchrow_array;
	return $res[0];
}

sub newP {
	while(1) {
		my @genome = getGenome();
		my $fit = fitness(@genome);
		next unless $fit;
		return ([ @genome ], $fit);
	}
}

sub countItems {
	my $query = "SELECT score, count(*) FROM imgur ";
	$query .= "WHERE ".join(" AND ", @_) if @_;
	$query .= " GROUP BY score";
	my $sth = query $query;
	my $gd = 0;
	my $bd = 0;
	while(my @res = $sth->fetchrow_array) {
		if($res[0]) {
			$gd = $res[1];
		} else {
			$bd = $res[1];
		}
	}
	return ($gd, $bd);
}

sub tooOld {
	my $children = shift;
	if($children > 5) {
		$children = 5;
	}
	return rand($children) > 1;
}

my @res;

my %Pidx;
my @Pgen;
my %Pbrt;
my $i;
($good, $bad) = countItems();

for($i = 1; $i <= $MIN_POPULATION; $i++) {
	my @person = newP;
	print "Initial population: $i\n";
	$Pgen[$i-1] = $person[0];
	$Pidx{$i-1} = $person[1];
	$Pbrt{$i-1} = 0;
}

while(1) {
	my @list = sort {$Pidx{$b} <=> $Pidx{$a}} keys %Pidx;
	my ($a, $b);
	$i = 0;

	while(1) {
        	if(rand(1) < 0.1) {
			if(!$a) {
				$a = $list[$i] unless tooOld($Pbrt{$list[$i]});
			} else {
				if (!tooOld($Pbrt{$list[$i]})) {
					$b = $list[$i];
					last;
				}
			}
	        }
		$i++;
		if($i > $#list) {
			$a = undef;
			$b = undef;
			$i = 0;
		}
	}

	$Pbrt{$a}++;
	$Pbrt{$b}++;

	my @child;
	my $fit;
	while(1) {
		if(rand(1) < 0.8) {
			# normal breed #
			for($i=0; $i<$GENOME_SIZE; $i++) {
				$child[$i] = rand(3) < 2 ? $Pgen[$a]->[$i] : $Pgen[$b]->[$i];
			}
		} elsif(getBit()) {
			# crossover
			my $split = int rand($GENOME_SIZE-1);
			for($i=0; $i<=$split; $i++) {
				$child[$i] = $Pgen[$a]->[$i];
			}
			for(; $i<$GENOME_SIZE; $i++) {
				$child[$i] = $Pgen[$b]->[$i];
			}
		} elsif(getBit()) {
			# mutation
			my $mutate = int rand($GENOME_SIZE);
			@child = @{$Pgen[$a]};
			if(getBit()) {
				# whole gene
				$child[$mutate] = getGene();
			} else {
				# minor mutation
				my $gene = $child[$mutate];
				my $expr = '0\.[0-9]+';
				my $repl = rand(1);
				$gene =~ s/$expr/$repl/;
				$child[$mutate] = $gene;
			}
		} else {
			# immigrant
			@child = getGenome();
			if(getBit() && fitness(@child)) {
				# immigrant, murderer and rapist
				$Pgen[$b] = [ @child ];
				$Pidx{$b} = fitness(@child);
				$Pbrt{$b} = 0;
				next;
			}
		}
		$fit = fitness(@child);
		next unless $fit || getBit(); # give the same couple another chance
		last;
	}

	next unless $fit && $fit >= $Pidx{$list[$#list]};

	my $idx = (@list < $MAX_POPULATION) ? @list : $list[$#list];
	$Pidx{$idx} = $fit;
	$Pgen[$idx] = [ @child ];
	$Pbrt{$idx} = 0;

	if($fit > $Pidx{$list[0]}) {
		my ($gd, $bd) = countItems(@child);
		$gd = $gd * 100 / $good;
		$bd = $bd * 100 / $bad;
		print "New max: $gd% - $bd%\n";
		print join(" AND ", @child);
		print "\n";
	}
}

