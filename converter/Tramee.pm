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

	{
		gender => $fields[0],
		name   => &slash_to_empty($fields[1]),
		note   => [ split("；", &slash_to_empty($fields[2])) ],
		childs => [ map &load($f), (1 .. $fields[3]) ],
	}
}




1;

