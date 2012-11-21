#!/usr/bin/env perl


sub AvgPosCountRate {
    my $avg_pos_count = @_[0];
    if($avg_pos_count <= 2) { 
        return 0;
    }    
    return(($avg_pos_count)/11);
}

sub DwellTimeUntilClickRate {
    my $param = @_[0];
    if ($param == -1) {
        return 1;
    }
    my $val = 1-1/exp(3*$param/3000);
    return($val);
}

sub SumDensBadQueryRate {
    my $param = @_[0];
    return($param/0.02355663); # max val
}

sub NumBackSerpRate {
    my $param = @_[0];
    return(1-1/exp(4*$param/15));
}

sub QuerySimilarityRate {
    my $param = @_[0];
    return(1-1/exp(10*$param/50));
}

sub ClickCountRate {
    my $param = @_[0];
    return(1-1/exp($param/50));
}

sub QueryWOClickRate {
    my $param = @_[0];
    return(1-1/exp($param/5));
}

sub test {
    printf(AvgPosCountRate(3)."\n");
    printf(DwellTimeUntilClickRate(3000)."\n");
    printf(SumDensBadQueryRate(0.003)."\n");
    printf(NumBackSerpRate(1)."\n");
    printf(QuerySimilarityRate(2)."\n");
    printf(ClickCountRate(10)."\n");
    printf(QueryWOClickRate(10)."\n");
}





