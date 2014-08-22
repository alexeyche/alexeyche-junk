function G = sameconv(A, B);
%  G = sameconv(A, B);
%   
%  2D filters B with A, producing an output the same size as A,
%  but zero-padding occurs at top
%  Handles appropriate flipping of B so convolution is correct;

[am, an] = size(A);
[bm, bn] = size(B);
G = conv2([zeros(bm-1,an); A], flipud(fliplr(B)), 'valid');