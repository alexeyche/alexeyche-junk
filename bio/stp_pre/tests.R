plotl = function(...) plot(..., type="l")

U = 0.01
D = 150.0
F = 15.0

dt = 1.0

f = 0
d = 1.0


T = 100.0
Tsize = T/dt

#x = rep(0, Tsize)
#x[1] = 1.0
x = rpois(Tsize, 0.2)

vd = c()
vf = c()
vp = c()
for (ti in 1:Tsize) {
    p = (f * (1.0 - U) + U) * d
    df = - f/F + U * (1.0 - f) * x[ti]
    dd = (1.0 - d)/D - p * x[ti]
    
    
    d = d + dt * dd
    f = f + dt * df
    
    vd = c(vd, d)
    vf = c(vf, f)
    vp = c(vp, p)
    
    cat(ti, "\n")
}

par(mfrow=c(4,1))
plotl(x)
plotl(vp)
plotl(vf)
plotl(vd)

