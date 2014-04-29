setwd("~/prog/alexeyche-junk/cns/R/srm/cprog")



library(snn)

rundir="/home/alexeyche/prog/sim/runs"
names = c("n100_full", "n100_full.1", "n100_full.2","n100_full.3")


rates = NULL
maxep = 33
for(runname in names) {
    workdir=sprintf("%s/%s", rundir, runname)
    evaldir=sprintf("%s/eval", workdir)
    
    
    
    for(ep in 1:maxep) {
        if(file.exists(sprintf("%s/%s/eval_output.bin", evaldir, ep))) {
            eval_output = sprintf("%s/%s/eval_output", evaldir, ep)
            r = loadMatrix(eval_output, 1)
            
            rates = rbind(rates, c(ep, min(r)))
        }
    }
}
ln =length(names)

rates = rates[order(rates[,1]),]

m = NULL
dev = NULL
for(ep in 1:maxep) {
    ind = which(rates[,1] == ep)
    m = c(m, mean(rates[ind,2]))
    dev = c(dev, sd(rates[ind,2]))
}

x = 1:13*2-1
CI.up = m+dev
CI.dn = m-dev
x=1:maxep
plot(m~x, cex=1.5,ylim=c(0,0.2),yaxp=c(0,0.20,10), xlab='Эпоха',ylab='Рейтинг ошибки', main='',col='blue',pch=16, , las=1)
arrows(x,CI.dn,x,CI.up,code=3,length=0.2,angle=90,col='red')
abline(h=0.12, col="black")
#legend("bottomleft",paste(names,": S.E=",data$se),ncol=6,text.width=1)