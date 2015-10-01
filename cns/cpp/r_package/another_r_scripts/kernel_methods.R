
normalize = function(K) {
    D = diag(1/sqrt(diag(K)))
    Kn = D %*% K %*% D
    colnames(Kn) <- colnames(K) 
    rownames(Kn) <- rownames(K) 
    return(Kn)
}

centering = function(K) {
    ell = nrow(K)
    D = colMeans(K)/ell
    E = mean(D)/ell
    J = rep(1, ell) %*% t(D)
    K = K - J - t(J) - E * matrix(rep(1, ell*ell), nrow=ell, ncol=ell)
}



get.labs = function(K) {
    sapply(
        strsplit(colnames(K), split="[.]"), 
        function(s) tail(s, 1)
    )
}

simple_metric = function(K) {
    labs = get.labs(K)
    acc = 0
    for(li in 1:nrow(K)) {
        for(lj in 1:ncol(K)) {
            mult = 0
            if(labs[li] == labs[lj]) {
                mult = 1.0   
            } else {
                mult = -1.0
            }
            acc = acc + mult * K[li, lj] #(K[li, li]^2 - 2 * K[li, lj] + K[lj, lj]^2)
        }
    }
    return(acc)
}

