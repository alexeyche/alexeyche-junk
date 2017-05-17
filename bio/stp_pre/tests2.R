plotl = function(...) plot(..., type="l")
tau_d = 10.0
tau_f = 100.0
U = 0.2

set.seed(1)

dt = 1.0


x = 1.0
u = U


vx = c()
vu = c()

T = 100.0
Tsize = T/dt

inp = rpois(Tsize, 0.2)


for (ti in 1:Tsize) {
    
    dx = (1.0 - x)/tau_d - u * x * inp[ti]
    du = (U - u)/tau_f + U * (1.0 - u) * inp[ti]
    
    x = x + dt * dx
    u = u + dt * du
    
    vx = c(vx, x)
    vu = c(vu, u)
}
par(mfrow=c(4,1))
plotl(inp)
plotl(vx)
plotl(vu)
plotl(vu*vx)

