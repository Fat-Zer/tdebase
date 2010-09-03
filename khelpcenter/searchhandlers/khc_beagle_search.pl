#!/usr/bin/perl -w
# vim:sw=4:et

use warnings;
use strict;
use Getopt::Long;

sub isBeagleRunning()
{
    open(IN, "-|") || exec "beagle-ping";
    while(<IN>) {
        if (/^Daemon version:/) {
            close(IN);
            return 1;
        }
    }
    close(IN);
    return 0;
}

sub formatHTML($$)
{
    my ($query, $hits) = @_;

    print "<html>\n<body\n<ul>\n";

    foreach my $hit(@$hits) {
        print "<li>$hit</li>\n";
    }
    print "</ul>\n</body>\n</html>\n";
}

sub beagleQuery($$$)
{
    my ($words, $method, $maxnum) = @_;

    my @hits = ();

    open(IN, "-|") || exec "beagle-query", "--type", "DocbookEntry", "--type", "File", "--max-hits", $maxnum, @$words, "ext:docbook";
    while(<IN>) {
        chop;
        next if (/^Debug:/);

        my $uri = $_;
        $uri = $1 if ($uri =~ /^file:\/\/(.*)$/);

        print "uri: $uri\n";
        my $helpLink = &makeHelpLink($uri);

        push(@hits, $helpLink) if (!grep { /^$helpLink$/ } @hits);
    }
    close(IN);
    return @hits;
}

sub makeHelpLink($)
{
    # Try to figure out the name of the application from the path to its index.docbook file

    my ($path) = @_;
    my @pathcomponents = split '/', $path;

    my $appName = $pathcomponents[-2];
    my $appName2 = $pathcomponents[-3];

    if ($appName eq $appName2 or $appName2 eq "doc" 
        or (-d "/usr/share/locale/$appName2")) {
        return "<a href=\"help:/$appName\">$appName</a>";
    }
    return "<a href=\"help:/$appName2/$appName\">$appName ($appName2)</a>";
}

my $method = "and";
my $maxnum = 100;

GetOptions("method=s", \$method, "maxnum=i", \$maxnum);

my @hits = ("The Beagle daemon is not running, search is not available");

my @words = @ARGV;

if (isBeagleRunning()) {
    @hits = beagleQuery(\@words, $method, $maxnum);
}

@hits = ("There are no search results") if ($#hits < 0);

formatHTML(\@words, \@hits);
