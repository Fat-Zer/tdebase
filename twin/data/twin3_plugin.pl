#!/usr/bin/perl
foreach (<>) {
    if(/^PluginLib=twin_(.*)$/) {
        print "PluginLib=twin3_$1\n"; 
        next;
    }
    print $_;
}
