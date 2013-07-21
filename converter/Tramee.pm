#!/usr/bin/perl -w
package Tramee;




sub load
{
	my ($f) = @_;

	$_ = <$f>;
	chomp;
	s/^\s+//g;
	s/\s+$//g;
	my @fields = split/\s+/;

	sub slash_to_empty { "@_" eq "/" ? "" : "@_" }
	return () if $fields[1] eq '/';

	{
		gender => $fields[0],
		name   => $fields[1],
		note   => [ split("；", &slash_to_empty($fields[2])) ],
		childs => [ map &load($f), (1 .. $fields[3]) ],
	}
}




1;

