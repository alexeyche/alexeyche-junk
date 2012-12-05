#!/usr/bin/env perl

use strict;
use Data::Dumper;
use List::Util qw(sum);
use IO::Handle '_IOLBF';

sub read_file_line {
  my $fh = shift;

  if ($fh) { 
    my $line = <$fh>;
    chomp $line;
    return [ split(/ /, $line) ];
  }
  return;
}


open(FREQ_EP, "<../episode_out.freq.sort");

my @top_episodes_arr;
my $count_l = 0;

while(<FREQ_EP>) {
    chomp($_);
    my @line = split('\t',$_);
    push @top_episodes_arr, $line[0];
    $count_l++;
    if($count_l == 100) {
        last;
    }
}
close(FREQ_EP);

my %top_episodes = map { $_ => 1 } @top_episodes_arr;

my $test_postfix = "";
my $feats;
my $episodes;
my $sid_file;
my $distr_for_q = 0;
if((@ARGV > 0)&&($ARGV[0] eq '-t')) {
  open($feats, "<parse_out.svm.test.scale.class");
  open($episodes, "<episode_out.test");
} elsif((@ARGV > 0)&&($ARGV[0] eq '-q')) {
    $distr_for_q=1;
    open($feats, "<parse_out.svm.test.scale");
    open($episodes, "<episode_out.test");
    open($sid_file, "<parse_out.svm.test");
} else {
    open($feats, "<parse_out.svm.scale");
    open($episodes, "<episode_out");
}

my $feat = read_file_line($feats);
my $episode_arr = read_file_line($episodes);
my $episode = @$episode_arr[0];
my $sid_line;
if($distr_for_q) {
    $sid_line = read_file_line($sid_file);    
}

my %patt_to_file;
my @others_patt;
while($feat and $episode) {
    if (! exists($top_episodes{$episode}) ) {
        push @others_patt, $episode;
        $episode = "others";
    }
    if(! exists($patt_to_file{$episode}) ) {
        my $episode_filename = $episode;
        $episode_filename =~ s/,/_/g;
        my $fh;
        if(!$distr_for_q) {
            open($fh, ">>./episodes/$episode_filename");
        } else {
            open($fh, ">>./episodes_quest/$episode_filename");
        }
        $patt_to_file{$episode} = $fh;
    }
    my $cur_fh = $patt_to_file{$episode};
    if(!$distr_for_q) {
        print $cur_fh join(" ", @$feat) ."\n";
    } else {
        print $cur_fh $$sid_line[0] . " ";
        print $cur_fh join(" ", @$feat[1..@$feat]) ."\n";
    }

    $feat = read_file_line($feats);
    $episode_arr = read_file_line($episodes);
    $episode = @$episode_arr[0];
    if($distr_for_q) {
        $sid_line = read_file_line($sid_file);    
    } 
}

if(@others_patt) {
    open(OTHERS,">others_patt");
    foreach my $el(@others_patt) {
    
    }
}

foreach my $ep (keys %patt_to_file) {
    my $fh = $patt_to_file{$ep};
    close($fh);
}
close($feats);
close($episodes);
if($distr_for_q) {
    close($sid_file);
}
