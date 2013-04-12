#!/usr/bin/RScript

# data
data <- data.all
numruns <- 100
c(num.vis, num.hid) := dim(model$W)
betas <- c(seq(0,0.5,by=1/1000),seq(0.5,0.9,by=1/10000),seq(0.9,1,by=1/100000))


p_m <- colMeans(data)

log_base_rate <- log(p_m) - log(1-p_m)

#visbias_base <- t(log_base_rate)
visbias_base <- matrix(0, nrow=1,ncol=num.vis)

hidbias <- rep.row(model$hid_bias, numruns)
visbias <- rep.row(model$vis_bias, numruns)

logww <- matrix(0, nrow=numruns)
negdata <- rep.row( sigmoid(visbias_base), numruns) # only bias
negdata <- sample_bernoulli(negdata)

logww <- logww - (negdata %*% t(visbias_base) + num.hid * log(1+exp(1)))

Wh <- negdata %*% model$W + rep.row(model$hid_bias, numruns)
Bv_base <- negdata %*% t(visbias_base)
Bv <- negdata %*% t(model$vis_bias)

bb <- betas[4]


logww <- logww + (1-bb) * Bv_base + bb * Bv + rowSums(log(1+exp(bb*Wh)))

poshidprobs <- sigmoid(exp(bb*Wh))
poshidstates <- sample_bernoulli(poshidprobs)

negdata <- sigmoid( (1-bb)* rep.row(visbias_base, numruns) - 
                       bb * (poshidstates %*% t(model$W) + rep.row(model$vis_bias,numruns)) 
                  )

Wh <- negdata %*% model$W + rep.row(model$hid_bias, numruns)
Bv_base <- negdata %*% t(visbias_base)
Bv <- negdata %*% t(model$vis_bias)

logww  <-  logww - ( (1-bb)*Bv_base + bb*Bv + rowSums(log(1+exp(bb*Wh))) )
