# vim: noet ts=4 sw=4 sts=0 ft=perl

my @olds;

(
	enter => sub {
		@olds = ($ud::cc, $ud::ccflags);
		$ud::cc = "./lpp elm-ui";
		$ud::ccflags .= " " . `pkg-config --cflags elementary`;
		$ud::ldflags .= " " . `pkg-config --libs   elementary`;
	},
	leave => sub {
		($ud::cc, $ud::ccflags) = @olds;
	},
)

