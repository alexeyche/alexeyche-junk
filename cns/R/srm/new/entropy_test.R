

require(entropy)

st1 <- c(17, 25, 60)
st2 <- c(17, 25, 40)

T0=0
Tmax=100

numB = 10
y1 = discretize(st1, numBins = numB, r = c(T0, Tmax))
y2 = discretize(st2, numBins = numB, r = c(T0, Tmax))
KL.empirical(y1, y2)

y12 = discretize2d(st1, st2, numBins1 = numB, numBins2 = numB, r1 = c(T0, Tmax), r2 = c(T0, Tmax))


H12 = entropy(y12)
H1 = entropy(rowSums(y12))
H2 = entropy(colSums(y12))

H1+H2-H12 # mutual entropy

