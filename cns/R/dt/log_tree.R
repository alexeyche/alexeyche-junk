
t = ncol(spikes_bin)

sp_patts = lapply(1:ncol(spikes_bin), function(i) spikes_bin[,i])
u_sp_patts = unique(sp_patts)
u_sp_patts = u_sp_patts[sort(sapply(u_sp_patts, sum), index.return=TRUE)$ix]
m = length(u_sp_patts)

u_sp_patts_acc = vector("list",length(u_sp_patts))

for(i in 1:length(u_sp_patts)) {
    u_sp_patts_acc[[i]] = which(sapply(sp_patts, function(v) all(v == u_sp_patts[[i]]) ))
}


u_sp_patts_count = sapply(u_sp_patts_acc, length)
m = length(u_sp_patts_count)

Pm = sapply(u_sp_patts_count, function(Nm) Nm/t)


log_regr = Vectorize(function(x, beta) {
    val = beta[1] + beta[2:length(beta)]*x
    exp_val_minus = exp(-val)
    exp_val_plus = exp(val)
    
    left = (exp_val_minus)/(1+exp_val_minus)
    right = (exp_val_plus)/(1+exp_val_plus)
    return(c(left,right))
}, "x")



samp = sample(m)

left = samp[1:(m/2)]
right = samp[(1+m/2):m]

X_dim = 1

beta = c(0, runif(X_dim)-0.5)

