

require(Rsnn)

pr = RProto$new("/home/alexeyche/prog/newsim/runs/7d31572604619944e7138bbdd6f86c89_0002/1_p_stat.pb")$read()
sp = RProto$new("/home/alexeyche/prog/newsim/runs/7d31572604619944e7138bbdd6f86c89_0002/1_output_spikes.pb")$read()


from = 0
feats = vector("list", length(sp$end_of_patterns))
for(pi in 1:length(sp$end_of_patterns)) {
    to = sp$end_of_patterns[pi]-100
    for(ni in 1:length(pr)) {
        feats[[pi]] = c(feats[[pi]], pr[[ni]][["p"]][from:to])
    }
    from = to + 101
}

feats = do.call(rbind, feats)

require(irlba)

irlba(feats, aug=c("harm"))