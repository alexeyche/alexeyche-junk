
from sympy import *

t, x, U, D, F = symbols('t x U D F')

s = symbols('s')

f_i = Function("f")
f = f_i(t, U, F, D)
d_i = Function("d")
d = d_i(t, U, F, D)


new_f = f - f/F + U * (1.0 - f) * x
new_d = d + (1.0 - d)/D - f * d * x

def deriv(e, on):
	f_s = "Derivative(f(t, U, F, D), {})".format(on)
 	d_s = "Derivative(d(t, U, F, D), {})".format(on)
 	return str(e.diff(on)) \
	.replace(f_s, "(df/d{})".format(on)) \
	.replace(d_s, "(dd/d{})".format(on)) \
	.replace("f(t, U, F, D)", "f")\
	.replace("d(t, U, F, D)", "d")

print deriv(new_d, D)
