sum_over <- function(func, N) {
  sum <- 0
  for(i in 1:N) {
    sum <- sum + func(i)
  }
  return(sum)
}