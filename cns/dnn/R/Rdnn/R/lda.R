

tr = function(m) {
    sum(diag(m))
}

KFD = function(K, only_scatter=FALSE, mu=0.001) {
    y = colnames(K)
    cls = unique(y)
    
    N = matrix(0, nrow = length(y), ncol = length(y))
    M = matrix(0, nrow = length(y), ncol = length(y))
    Ms = NULL
    for(lab in cls) {
        l_idx = which(y == lab)
        l = length(l_idx)
        
        Mi = rowSums(as.matrix(K[, l_idx]))/l
        Ki = K[,l_idx]
        eye = diag(l)
        eye_l = matrix(1/l, nrow=l, ncol=l)
        N = N + Ki %*% (eye - eye_l) %*% t(Ki)
        Ms = cbind(Ms, Mi)
    }
    
    M0 = rowSums(Ms)/length(cls)
    for(li in 1:length(cls)) {
        M = M + (Ms[, li] - M0) %*% t(Ms[, li] - M0)
    }
    
    A = solve(N+mu*diag(length(y))) %*% M
    if(only_scatter) {
        return(list(M=M, N=N, A=A))
    }
    Ae = eigen(A)
    return(list(y=Ae$vectors, M=M, N=N, A=A))
}

