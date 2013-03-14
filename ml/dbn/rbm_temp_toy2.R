#!/usr/bin/RScript



gen_data <- function(n, n_delay) {
    data_all <- NULL
    for(j in 1:(num_cases/6)) { 
      data <- matrix(0, nrow = 6, ncol=num_vis)  
      for(i in 1:(num_vis-4)) {
        data[i,i] <- 1  
        data[i,i+2] <- 1
        data[i,i+4] <- 1
      }
      data_all <- rbind(data_all, data)
    }
    data <- matrix(0, nrow = n_delay, ncol=num_vis)  
    for(i in 1:n_delay) {
        data[i,i] <- 1  
        data[i,i+2] <- 1
        data[i,i+4] <- 1
    }
    data_all <- rbind(data_all, data)
    return(data_all)
}


n_delay <- 3
num_vis <- 10
num_hid <- 15
num_cases <- 1200
batch_size <- 100


model <- list(W = array(0.01*rnorm(num_vis*num_hid),dim=c(num_hid,num_vis)), # visible units for row, hidden units for col
              W_uv = array(0.01*rnorm(num_vis*num_vis*n_delay),dim=c(num_vis,num_vis,n_delay)),
              W_uh = array(0.01*rnorm(num_vis*n_delay*num_hid),dim=c(num_vis,num_hid,n_delay)),
              vis_bias = array(0,dim = c(1,num_vis)), 
              hid_bias = array(0,dim = c(1,num_hid)),
              num_cases = num_cases,
              n_delay = n_delay,
              num_vis = num_vis,
              num_hid = num_hid)

train.params = list(e.w =  0.001, e.v =  0.001, e.h =  0.001, w_decay = 0.0002, e.w_uv =  0.001, e.w_uh =  0.001,
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 1500, cd.iter = 1)
for (v in 1:length(train.params)) assign(names(train.params)[v], train.params[[v]])

data_all <- gen_data(num_cases, n_delay)
batch_start <- sample((n_delay+1):(num_cases+n_delay))
num_batches <- num_cases/batch_size

W_inc <- W_uv_inc <- W_uh_inc <- vis_bias_inc <- hid_bias_inc <- 0

b_s <- batch_start[((b-1)*batch_size+1):((b-1)*batch_size+batch_size)]

data <- array(dim=c(num_cases, num_vis,n_delay+1))
        data[,,1] <- data_all[b_s,]
        for(t in 1:n_delay) {
            data[,,t+1] <- data_all[b_s-t,]
        }
