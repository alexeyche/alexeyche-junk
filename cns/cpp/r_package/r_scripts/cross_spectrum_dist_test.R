# 
# 
# rate1 = 10 # Hz
# rate2 = 10 # Hz
# 
# T = 10000
# 
# s1 = rpois(T, rate1/1000)
# s2 = rpois(T, rate2/1000)

sales.spc <- spectrum(ts.union(BJsales, BJsales.lead),
                      kernel("modified.daniell", c(5,7)))

plot(sales.spc, plot.type = "coherency")

s1 = BJsales
s2 = BJsales.lead

fs1 = fft(s1)


