$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

Parameters for our shapelet codes (except shapelet_lex.exe)

shapelet_XXX [TRAIN SET] [number of instance] [number of class] [maxLength] [minLength] [step size from minLength to maxLength] 

Here maxLength is the maximum length of a shapelet you want to search for. This has to be smaller than the smallest instance in the dataset.
minLength is the minimum length of a shapelet. It has to be larger than 1.

For the cricket dataset an example run would be the following.

shapelet_best Cricket_TRAIN 9 2 300 10 1

For other .exe files distributed with this package the same set of parameters are required in the same order.

$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

The code for the 2009 shapelet paper to which we compare and gain speedup is shapelet_lex.exe

Parameters for this code is the following.

shapelet_lex [Train Set] 1 [maxLength] [minLength] [stepSize] 1

For example for the Cricket dataset it can be
 
shapelet_lex Cricket_TRAIN 1 300 10 1 1