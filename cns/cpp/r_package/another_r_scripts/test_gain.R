

#left_hist = c(6, 0, 5, 5)
#right_hist = c(4, 10, 5, 5)

left_hist = c(6, 0, 2, 1)
right_hist = c(4, 10, 8, 9)

N = sum(left_hist) + sum(right_hist)

de = 1.38629

calc_ent = function(h) {
    rat = h/sum(h)
    arr = rat*log(rat)
    E = 0
    for(v in arr) {
        if(is.nan(v)) next
        E = E - v
    }
    return(E)
}


de - sum(left_hist)*calc_ent(left_hist)/N - sum(right_hist)*calc_ent(right_hist)/N

