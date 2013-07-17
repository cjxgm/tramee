# vim: ft=perl
@ud::targets = qw[ all clean rebuild test commit ];

$ud::cc = "gcc";
$ud::ld = "gcc";
$ud::ccflags = "-std=gnu11 -march=native -Ofast -Wall "
			 . "-Ilibcrude -Wno-main";
$ud::ldflags = "-Llibcrude -lcrude -lm";

(
	all => {
		require => [qw[ gen_db compile build ]],
	},

	clean => {
		pre => sub {
			unlink <lpp-elm-ui.*.db>;
			system "rm -rf build";
		},
	},

	rebuild => {
		require => [qw[ clean all ]],
	},

	test => {
		pre => sub {
			system "./build/tramee ./example/test.tramee";
		},
	},

	commit => {
		require => [qw[ clean ]],
		post => sub {
			system "git add .";
			system "git diff --cached";
			system "env LANG=C git commit -a";
		},
	},




	gen_db => {
		source => [qw[ lpp-elm-ui.func.db ]],
		dirty => sub {
			&ml::newer("lpp-elm-ui-db-gen", "@_");
		},
		file => sub {
			system "./lpp-elm-ui-db-gen";
		},
	},

	compile => {
		source => [qw[ src ]],
		pre => sub {
			mkdir "build";
		},
		dir_enter => sub {
			mkdir "build/@_";
		},
		file => sub {
			my ($file) = @_;
			return if $file !~ /^(.*)\.c$/;
			my $obj = "build/$1.o";

			system "env LANG=C $ud::cc $file -c -o $obj $ud::ccflags" and die;
		},
		dirty => sub {
			my ($file) = @_;
			return 0 if $file !~ /^(.*)\.c$/;
			my $obj = "build/$1.o";

			my $dirty = &ml::newer($file, $obj);
			return 1 if $dirty;

			my $deps = `gcc -MM $ud::ccflags $file`;
			$deps =~ s/\\\n/ /gm;
			$deps =~ s/^[^:]+:\s*//gm;
			for (split /\s+/, $deps) {
				return 1 if &ml::newer($_, $obj);
			}

			0;
		},
	},

	build => {
		source => [qw[ build ]],
		pre => sub {
			@ud::objs = ();
			$ud::dirty = 0;
		},
		file => sub {
			push @ud::objs, "@_";
		},
		dirty => sub {
			$ud::dirty |= &ml::newer("@_", "build/tramee");
			1;
		},
		post => sub {
			if ($ud::dirty) {
				system "env LANG=C $ud::ld -o build/tramee @ud::objs $ud::ldflags";
			}
			else { print "\e[0;30;42m skip \e[0m\n" }
		},
	},
)

