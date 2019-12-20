
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

arcsinh <- function(x) {
  return(log(1.0 + sqrt(x * x + 1.0)))
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

generate_data <- function(num_points, type, q_probs) {
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
    sd = 1.0
    x = 15.0 + rnorm(num_points) * sd
    df = data.frame(
      x = seq(min(x), max(x), length.out=length(q_probs)-2)
    )
    df$pdf = dnorm(df$x, sd=sd)
    df$cdf = pnorm(df$x, sd=sd)
    return(list(x, df))
  }
  if (type == 2) {
    lam = 1.0
    x = rexp(num_points, lam)
    min_x = min(x)
    max_x = max(x)
    df = data.frame(
      x = seq(min_x, max_x, length.out=length(q_probs)-2)
    )
    df$pdf = dexp(df$x, lam)
    df$cdf = pexp(df$x, lam)
    return(list(x, df))
  }
  if (type == 3) {
    mean = 0.0
    mean_shift = 4.0
    x = rnorm(num_points, mean + mean_shift*rbinom(num_points, 1, 0.3))
    df = data.frame(
      x = seq(min(x), max(x), length.out=length(q_probs)-2)
    )
    df$pdf = 0.7*dnorm(df$x, mean, 1) + 0.3*dnorm(df$x, mean + mean_shift, 1)
    df$cdf = 0.7*pnorm(df$x, mean, 1) + 0.3*pnorm(df$x, mean + mean_shift, 1)
    
    return(list(x, df))
  }
  if (type == 4) {
    means = c(95.0, 100.0, 105.5)
    sds = c(1.0, 2.0, 5.0)
    
    bsize = length(means)-1
    size = length(means)
    p = 0.3
    
    choices = rbinom(num_points, bsize, p)
    
    x = rowSums(sapply(1:size, function(id) {
      return(rnorm(num_points, means[id], sds[id]) * (choices == (id-1)))
    }))
    
    x_q = seq(min(x), max(x), length.out=length(q_probs)-2)
    
    stat = lapply(1:size, function(id) {
      pid = dbinom(id-1, bsize, p)
      cat(id, " ", pid, "\n")
      return(data.frame(
        pdf=pid * dnorm(x_q, means[id], sds[id]),
        cdf=pid * pnorm(x_q, means[id], sds[id])
      ))
    })
    
    df = Reduce(function(l, r) data.frame(pdf=l$pdf + r$pdf, cdf=l$cdf + r$cdf), stat)
    df$x = x_q
    return(list(x, df))
  }
  stop("unknown type")
}


DISTR = "/Users/aleksei/distr"
CPP_IMPL = paste(DISTR, "moment-sketch-query/sketch", sep="/")
JAVA_IMPL = paste(
  DISTR,
  "m-sketch-hackday/javamsketch/msolver/target/msolver-1.0-SNAPSHOT-jar-with-dependencies.jar",
  sep="/"
)

sketch <- function(min, max, power_sums, log_min, log_max, log_power_sums, q_probs, solution) {
  if (solution == 0) {
    stats_str = c(
      toString(min), toString(max), 
      format(power_sums, digits=22, nsmall=20)
    )
    stats_line = trimws(paste(stats_str, " ", collapse=""))
    cat("Running command:\n")
    cat(stats_line)
    o = system(
      CPP_IMPL,
      input=c(stats_line, sapply(q_probs, toString)),
      ignore.stderr=FALSE,
      ignore.stdout=FALSE,
      intern=TRUE
    )
    
  } else if (solution == 1) {
    stats_str = c(
      toString(min), toString(max), 
      sapply(power_sums, toString)
    )
    log_stats_str = c(
      toString(log_min), toString(log_max), 
      sapply(log_power_sums, toString)
    )
    stats_line = trimws(paste(stats_str, " ", collapse=""))
    log_stats_line = trimws(paste(log_stats_str, " ", collapse=""))
    q_line = sapply(q_probs, toString)
    cat("Running command:\n")
    cat(stats_line, "\n")
    cat(log_stats_line, "\n")
    o = system(
      paste("java -jar ", JAVA_IMPL),
      input=c(stats_line, log_stats_line, q_line),
      ignore.stderr=FALSE,
      ignore.stdout=FALSE,
      intern=TRUE
    )
  } else {
    stop("unknown solution")
  }
  
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



Sys.setenv(
  LEARNING_RATE="0.1",
  MAX_ITER="5000",
  TOLERANCE="1e-10",
  PROGRESS_ITERATION="1000",
  BETA1="0.9",
  BETA2="0.99",
  EPSILON="1e-05",
  SOLVER="2"
)

num_points = 100000
num_powers = 15
q = seq(0.0, 1.0, 0.001)


c(x, df) := generate_data(num_points, 1, q)

# x <- arcsinh(x)
# df$x <- arcsinh(df$x)

# Ex = mean(x)
# x = x - Ex
# 
# df$x <- arcsinh(df$x)
# df$x = df$x - Ex

# x <- log(x)


x_pos = x[x > 0.0]

power_sums = sapply(0:(num_powers-1), function(p) sum(x^p))
log_power_sums = sapply(0:(num_powers-1), function(p) sum(log(x_pos)^p))
min_x = min(x)
max_x = max(x)
log_min_x = min(log(x_pos))
log_max_x = max(log(x_pos))

df_sk = sketch(
  min_x,
  max_x,
  power_sums,
  log_min_x,
  log_max_x,
  log_power_sums,
  q,
  solution=0
)



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
