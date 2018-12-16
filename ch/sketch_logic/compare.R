
':=' <- function(lhs, rhs) {
  frame <- parent.frame()
  lhs <- as.list(substitute(lhs))
  if (length(lhs) > 1)
    lhs <- lhs[-1]
  if (length(lhs) == 1) {
    do.call(`=`, list(lhs[[1]], rhs), envir=frame)
    return(invisible(NULL)) 
  }
  if (is.function(rhs) || is(rhs, 'formula'))
    rhs <- list(rhs)
  if (length(lhs) > length(rhs))
    rhs <- c(rhs, rep(list(NULL), length(lhs) - length(rhs)))
  for (i in 1:length(lhs))
    do.call(`=`, list(lhs[[i]], rhs[[i]]), envir=frame)
  return(invisible(NULL)) 
}

sketch <- function(min, max, power_sums, q_probs) {
  stats_str = c(
    toString(min), toString(max), 
    sapply(power_sums, toString)
  )
  stats_line = trimws(paste(stats_str, " ", collapse=""))
  cat("Running command:\n")
  cat(stats_line)
  o = system(
    "/Users/aleksei/distr/alexeyche-junk/ch/sketch_logic/sketch",
    input=c(stats_line, sapply(q_probs, toString)),
    ignore.stderr=FALSE,
    ignore.stdout=FALSE,
    intern=TRUE
  )
  t = as.data.frame(
    matrix(
      sapply(unlist(strsplit(o, " ")), as.numeric),
      ncol=3,
      byrow=TRUE
    )
  )

  names(t) = c("x", "cdf", "pdf")
  return(t)
}

estimate <- function(x, q) {
  n = length(q)
  cdf_est = quantile(x, q)
  
  df = data.frame(
    x = cdf_est[2:(n-1)],
    cdf = q[2:(n-1)],
    pdf = diff(q,2)/diff(cdf_est,2)
  )
  return(df)
}

generate_data <- function(type, q_probs) {
  if (type == 0) {
    x = 1:num_points
    min_x = min(x)
    max_x = max(x)
    df = data.frame(
      x = seq(min_x, max_x, length.out=length(q_probs)-2)
    )
    df$pdf = dunif(df$x, min_x, max_x)
    df$cdf = punif(df$x, min_x, max_x)
    return(list(x, df))
  }
  if (type == 1) {
    sd = 0.1
    x = rnorm(num_points) * sd
    df = data.frame(
      x = seq(min(x), max(x), length.out=length(q_probs)-2)
    )
    df$pdf = dnorm(df$x, sd=sd)
    df$cdf = pnorm(df$x, sd=sd)
    return(list(x, df))
  }
  if (type == 2) {
    mean = 100.0
    mean_shift = 4.0
    x = rnorm(num_points, mean + mean_shift*rbinom(num_points, 1, 0.3))
    df = data.frame(
      x = seq(min(x), max(x), length.out=length(q_probs)-2)
    )
    df$pdf = 0.7*dnorm(df$x, mean, 1) + 0.3*dnorm(df$x, mean + mean_shift, 1)
    df$cdf = 0.7*pnorm(df$x, mean, 1) + 0.3*pnorm(df$x, mean + mean_shift, 1)
    return(list(x, df))
  }
  stop("unknown type")
}


Sys.setenv(
  LEARNING_RATE="0.001",
  MAX_ITER="20000",
  TOLERANCE="1e-18",
  GRID_SIZE="4048",
  PROGRESS_ITERATION="10000",
  BETA1="0.999",
  BETA2="0.9999",
  EPSILON="1e-03"
)

num_points = 500000
num_powers = 5
q = seq(0.0, 1.0, 0.005)


c(x, df) := generate_data(2, q)

power_sums = sapply(0:(num_powers-1), function(p) sum(x^p))

df_sk = sketch(min(x), max(x), power_sums, q)

# df_sk$pdf = predict(smooth.spline(df_sk$x, df_sk$pdf), df_sk$x)$y



library(ggplot2)
library(gridExtra)

# q0 = ggplot(df_sk, aes(x = x, y = cdf, color = "m-sketch")) + geom_line() +
#   geom_line(aes(x = df$x, y = df$cdf, color = "true")) +
#   scale_x_continuous("x") +
#   scale_y_continuous("CDF") +
#   scale_color_discrete("Method")


q1 = ggplot(df_sk, aes(x = x, y = pdf, color = "m-sketch")) + geom_line() +
  geom_line(aes(x = df$x, y = df$pdf, color = "true")) +
  scale_x_continuous("x") +
  scale_y_continuous("PDF") +
  scale_color_discrete("Method")

plot(q1)
# grid.arrange(q0, q1, ncol=1)
