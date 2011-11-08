#!/usr/bin/perl
foreach (<>) {
    if(/^PluginLib=libtwin(.*)$/) {
        print "PluginLib=twin_$1\n"; 
        next;
    }
    print $_;
}
