require(R.matlab)
require(waved)
require(Rdnn)


xt = RProto$new("/home/alexeyche/dnn/ts/riken_14chan_3LRR.pb")$read()$values[[1]]
xt = xt[1:1000]

normalize = function(x, min_val, max_val) {
    1 - (max_val-x)/(max_val-min_val) 
}
norm = function(x) {
    x/sqrt(sum(x^2))
}

bd = "/home/alexeyche/prog/alexeyche-junk/cns/cpp/build/bb"
gbd = function(ff) {
    RProto$new(sprintf("%s/%s",bd,ff))$read()[[1]]
}

M = 64
L = 100

set.seed(1)
filters = matrix(rnorm(M*L), nrow=M, ncol=L)
fi = t(sapply(1:M, function(i) norm(filters[i,])))

# fi = gbd("filter.pb")

i = 1 
s_thr = 0.1
max_ep = 10

spikes = list()


#for(i in 1:(length(xt)-L)) {
for(ep in 1:max_ep) {
    grad_m = matrix(0, nrow=nrow(fi), ncol=ncol(fi))
    
    delta_sum = 0
    deltas = c()
    for(i in 1:900) {
        #if((ep==5)) stop("123")
        x_start = xt[i:(i+L-1)]
        x = x_start
        
#         x_p = gbd(sprintf("%d_x%d.pb", ep, i-1))
#         if (sum(x-x_p) > 0.00001) {
#             stop("Bad val")    
#         }
        
        #     
        s_v = NULL
        si_v = NULL
        for(j in 1:50) {
            ip = c(x %*% t(fi))
            si = which(ip == max(ip))
            s = ip[si]
            res = x - s * fi[si,]    
            #cat("s: ", s, ", si: ", si, ", |res|: ", sqrt(sum(res^2)), "\n")
            if(s>=s_thr) {
                #cat("Spike!")
                spikes[[length(spikes)+1]] = list(t=i,si=si,s=s)
            }
            x = res
            s_v = c(s_v, s)
            si_v = c(si_v, si)
        }
#         si_vp = gbd(sprintf("%s_winners_id%d.pb", ep, i-1))
#         if( sum(si_vp[1:10] - si_v[1:10]+1)>0.0001) {
#             stop("Bad val")
#         }
#         s_vp = gbd(sprintf("%s_s%d.pb", ep, i-1))
#         if( sum(s_vp - s_v)>0.0001) {
#             stop("Bad val")
#         }
        x_d = colSums(s_v*fi[si_v,])
#         x_dp = gbd(sprintf("%d_xdes%d.pb",ep, i-1))
#         if( sum(x_dp - x_d)>0.0001) {
#             stop("Bad val")
#         }
        delta = x_start - x_d
#         delta_p = gbd(sprintf("%d_deltas%d.pb",ep, i-1))
#         if( sum(delta_p - delta)>0.0001) {
#             stop("Bad val")
#         }
        delta_sum = delta_sum + sum(delta^2)    
        deltas = c(deltas, delta)
        for(s_i in 1:length(si_v)) {
            s = s_v[s_i]
            si = si_v[s_i]
            grad = 1*s*delta
            grad_m[si, ] = grad_m[si, ] + grad
        }
    }
    cat("delta; ", delta_sum, "\n")
#     grad_p = gbd(sprintf("%d_dfilter.pb", ep))
#     if( sum(grad_p - grad_m)>0.0001) {
#         stop("Bad val")
#     }
    
    fi = fi + 0.1*grad_m    
    fi = t(sapply(1:M, function(i) norm(fi[i,])))
}

#gr_pl(grad_m)
#gr_pl()
# plot(x_start, ylim=c(min(x,x_start), max(x,x_start)), type="l")
# lines(x_d,col="red")
# x = c()
# y = c()
# cex = c()
# for(sp in spikes) {
#     x = c(x, sp$t)
#     y=c(y, sp$si)
#     cex = c(cex, sp$s)
#     
# }
# 
# xyplot(y ~ x, list(x = x, y = y), cex=cex*10,  col = "black")