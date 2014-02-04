
spike_distance_vp = function(sp1, sp2, cost) {
    scr = matrix(0, nrow=(length(sp1)+1), ncol=(length(sp2)+1))

    scr[,1] = 0:length(sp1)
    scr[1,] = 0:length(sp2)

    for(i in 2:nrow(scr)) {
        for(j in 2:ncol(scr)) {
            scr[i,j] = min(c(scr[i-1,j]+1, scr[i, j-1]+1, scr[i-1,j-1]+cost*abs(sp1[i-1]-sp2[j-1])))
        }
    }
    return(scr[length(sp1)+1, length(sp2)+1])
}


