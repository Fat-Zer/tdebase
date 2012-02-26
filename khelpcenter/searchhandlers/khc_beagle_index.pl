#!/usr/bin/perl
# vim:sw=4:et

use warnings;

sub getKDEDocDir() 
{
    my $prefix = `tde-config --prefix`;
    chomp $prefix;

    $prefix = "/opt/kde" if (not defined($prefix));
    return "$prefix/share/doc";
}

sub addRoot() 
{
    my $kdedocdir = &getKDEDocDir;

    open (IN, "-|") || exec "beagle-config", "indexing", "ListRoots";

    my $kdedoc_found = 0;
    while(<IN>) {
        if (/^$kdedocdir/o) {
            $kdedoc_found = 1;
            last;
        }
    }
    close(IN);

    if (not $kdedoc_found) {
        `beagle-config indexing AddRoot $kdedocdir`;
        `beagle-config indexing AddRoot $kdedocdir-bundle`;
    }
}

sub createExistsFile($$)
{
    my ($idir, $ident) = @_;

    open(OUT, ">", "$idir/$idir");
    close(OUT);
}

my $idir = $ARGV[0];
my $ident = $ARGV[1];

if (addRoot) {
    createExistsFile($idir, $ident);
}
