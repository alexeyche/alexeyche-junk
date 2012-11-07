#!/usr/bin/RScript

generate.data <- function(n=1000, m=5, sig.features=1:5, noise.level=0.10) {
  # create bogus feature matrix
  features <- matrix(runif(n*m), nrow=n, ncol=m)
  rownames(features) <- sprintf("ex%04d",seq(n))
  colnames(features) <- sprintf("feat%04d",seq(m))

  # generate random model parameters
  intercept <- rnorm(1)
  coefs <- rnorm(length(sig.features))
  names(coefs) <- colnames(features)[sig.features]

  # create response data
  response <- features[,sig.features] %*% coefs
              + intercept 
              + rnorm(n, sd=noise.level)

  # return generated data
  list(n=n, m=m,
       sig.features=sig.features,
       noise.level=noise.level,
       features=features,
       params=list(intercept=intercept, coefs=coefs),
       response=response)
}

## return a data.frame containing all non-zero fit
## coefficients along with the true values
compare.coefs <- function(data, fit) {
  coefs <- data$params$coefs
  intercept <- data$params$intercept
  merge(
    data.frame(
      feature.name=c('(Intercept)', names(coefs)),
      param=c(`(Intercept)`=intercept, coefs)),
    subset(
      data.frame(
        feature.name=rownames(coef(fit)),
        coef=coef(fit)[,1]),
      coef!=0),
    all=TRUE)
}

data <- generate.data(10,3,1:3)
