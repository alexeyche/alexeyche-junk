
w=0.04

ws = 0.5

x_w = c(1,1/2,1/4,1/8,0)
y_w = c(0.04, 0.04/4, 0.04/16, 0.04/32,0)
plot(x_w, y_w)

yy = c()
xx = c()
kk = c()
bb = c()
for(i in length(x_w):2) {
    dx = x_w[i] -x_w[i-1]
    dy = y_w[i] -y_w[i-1]
    k = dy/dx
    b = -(k * x_w[i-1]) + y_w[i-1]
    
    #x dx/dy-x_w[i-1](dx/dy) + y_x[i-1]) = y
    cat(k,"*x+",b,"\n",sep="")
    x = seq(x_w[i],x_w[i-1],length.out=10)
    yy=c(yy, k*x+b)
    xx=c(xx,x)
    kk=c(kk, k)
    bb=c(bb,b)
}
plot(xx,yy,type="l", xlim=c(0,1))

#plot(x_w, y_w, type="l")

#ww = seq(0,ws, by=0.1)

#plot(ww, ww*16.66-16.62, xlim=c(0.9,1.1), ylim=c(0.03,0.05), type="l")

rate_calc = function(w) 0.04*w^4/(w^4+ws^4)
