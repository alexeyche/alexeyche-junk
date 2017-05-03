
from sympy import *


# x0, x1, W0, W1, b0, b1 = symbols('x0 x1 W0 W1 b0, b1')
x0, x1, W00, W11, W10, W01, b0, b1 = symbols('x0 x1 W00 W11 W10 W01 b0, b1')
x, y, W, b = symbols('x y W b')

p = Function("p")

e = 0.5 * x * y - p(x) * p(y) * W - b * p(x)

# e = - 0.5 * (W0 * p(x0) * p(x1) + W1 * p(x1) * p(x0)) - (b0 * p(x0) + b1 * p(x1))

# e = - 0.5 * (
# 	W00 * p(x0) * p(x0) + 
# 	W01 * p(x0) * p(x1) +
# 	W10 * p(x1) * p(x0) +
# 	W11 * p(x1) * p(x1)
# ) - (
# 	b0 * p(x0) + 
# 	b1 * p(x1)
# )


# e = p(x) * p(y) - p(x) * p(y) * W - b * p(x)

# print factor(e.diff(W00))

print factor(e.diff(x))

#print diff(e, W)