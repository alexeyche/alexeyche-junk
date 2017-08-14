
from sympy import *


# W0, W1 = symbols("W0 W1")
# x0 = symbols("x0")
# x = Matrix([x0])
# y = Matrix(symbols("y0, y1"))

# W = Matrix([[W0, W1]])

# L = sqrt(sum((y - Matrix(W.dot(x))).applyfunc(lambda x: x ** 2)))




W, s, a = symbols("W s a")

L = 0.5 * (s - W * a) ** 2

print L.diff(a)
print L.diff(W)


## dL  = W ** 2
## dy 