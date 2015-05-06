gauss = function(x, xc, sigma) {
    exp( - ( (x-xc)*(x-xc) )/(2*sigma*sigma) )
}

dg = function(a, b, r, x, xc)  {
    (1+a)*gauss(x, xc, r) - a*gauss(x, xc, b*r)
}

a = 3
b = 3
r = 2

x = seq(0,100)
xc = 51

plot(dg(a,b,r,x,xc), type="l")