#!/usr/bin/env perl


sub AvgPosCountRate {
    my $param = @_[0];
    if($param <= 2) { 
        return 0;
    }    
    my $val = 1-1/exp(($param-2)/10);
    return($val);
}

sub DwellTimeUntilClickRate {
    my $param = @_[0];
    if ($param == -1) {
        return 1;
    }
    my $val = 1-1/exp(3*$param/200);
    return($val);
}

sub SumDensBadQueryRate {
    my $param = @_[0];
    return($param/0.02355663); # max val
}

sub NumBackSerpRate {
    my $param = @_[0];
    return(1-1/exp($param/2));
}

sub QuerySimilarityRate {
    my $param = @_[0];
    return(1-1/exp($param/4));
}

sub ClickCountRate {
    my $param = @_[0];
    return(1-1/exp($param/25));
}

sub QueryWOClickRate {
    my $param = @_[0];
    return(1-1/exp($param/5));
}

sub test {
    printf(AvgPosCountRate(4)."\n");
    printf(DwellTimeUntilClickRate(3000)."\n");
    printf(SumDensBadQueryRate(0.003)."\n");
    printf(NumBackSerpRate(1)."\n");
    printf(QuerySimilarityRate(2)."\n");
    printf(ClickCountRate(10)."\n");
    printf(QueryWOClickRate(10)."\n");
}

1;
