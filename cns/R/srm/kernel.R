#!/usr/bin/Rscript
require(abind)

gaussian_kernel = function(s, sigma) {
  sum((1/sqrt(2*pi*sigma^2))*exp(-(s^2)/(2*sigma^2)))
}

get_finger_print = function(spikes, T0, Tmax, window, sigma) {
  K = matrix(0, nrow=length(spikes), ncol=(Tmax-T0)/window)
  scale_factor = 1
  num_spikes = 0
  for(sp_i in 1:length(spikes)) {
    if(length(spikes[[sp_i]]) == 0) next
    if((length(spikes[[sp_i]]) == 1)&&(spikes[[sp_i]][1] == -Inf)) next
    for(t0_i in 1:((Tmax-T0)/window)) {
      K[sp_i, t0_i] = integrate( Vectorize(function(t) gaussian_kernel(t-spikes[[sp_i]], sigma)), (t0_i-1)*window, (t0_i-1)*window + window)$value 
    }    
    scale_factor = scale_factor + 1
    num_spikes = num_spikes + length(spikes[[sp_i]])
  }
  return(K)
}

get_mean_activity = function(net_all, ro) {
  x = lapply(net_all, function(x) get_finger_print(x$data, ro$T0, ro$Tmax, ro$fp_window, ro$fp_kernel_size))
  x = simplify2array(x)
  xm = apply(x, c(1,2), mean)
  dd = sapply(1:dim(x)[3], function(i) sum((xm - x[,,i])^2))  
  return(list(mean_act=xm, deviation_range=c(min(dd), max(dd))) )
}

iter = 0
reward_func = function(curr_act, Kmean_classes, mean_error=0) {
  if( !is.null(Kmean_classes)) {
#      K = kernelPass_autoCorr(curr_act, list(sigma = ro$fp_kernel_size, window =  ro$fp_window_size, T0 = ro$T0, Tmax = ro$Tmax, quad = 256))
      K = kernelWindow_spikes(curr_act, list(sigma = ro$fp_kernel_size, window =  ro$fp_window_size, T0 = ro$T0, Tmax = ro$Tmax, quad = 256))
     
      png(sprintf("/home/alexeyche/my/sim/runs/pics/curr_act_%s_%s.png", iter, K$label), width=1024, height=480)
      iter <<- iter + 1
      p = levelplot(K$data, col.regions=colorRampPalette(c("black", "white")))
      print(p)
      dev.off()

#      diag(K$data) = 0
      Krange = sum(K$data^2)
      class_rates = NULL
      for(cl in names(Kmean_classes)) {
        Kmean_range = sum(Kmean_classes[[cl]]^2)
        hb = max(max(K$data), max(Kmean_classes[[cl]]))
        lb = min(min(K$data), min(Kmean_classes[[cl]]))
#        class_rates = c(class_rates, sum( (K$data-Kmean_classes[[cl]])^2/(hb^2)) )
        class_rates = c(class_rates, 1-sqrt(mean( (K$data-Kmean_classes[[cl]])^2 ))/(hb-lb) )
#        class_rates = c(class_rates, 1-mean( (K$data-Kmean_classes[[cl]])^2 )/(mean(K$data)*mean(Kmean_classes[[cl]])) )       
      }
      id_cl = which(names(Kmean_classes) == as.character(K$label))
      cl_exp = exp(class_rates)
      softmax_rate = cl_exp[id_cl]/sum(cl_exp)
      vv = cl_exp/sum(cl_exp)
      vv_srt = sort(vv, decreasing=TRUE)
      
      cat(K$label, "  <=>  ")
      for(id_cl in 1:length(vv_srt)) {
          id_cl_true = which(vv == vv_srt[id_cl])
          cat(names(Kmean_classes)[id_cl_true], ":", vv[id_cl_true]," ", sep="")
      }
      cat(" => ", softmax_rate-mean(vv), "\n", sep="")
      return(softmax_rate-max(vv))
  }
  return(0.0)
}
test_kernel_functions = function() {
  sigma=10
  window=1
  kernel_options = list(sigma = sigma, window = window, T0 = 0, Tmax = duration, quad = 256)
  net1$label=1
  net2$label=1
  ans = kernelPass_crossNeurons(net1, net2, kernel_options)
  
  ans1 = kernelWindow_spikes(net1, kernel_options)
  ans2 = kernelWindow_spikes(net2, kernel_options)
  a = ans1$data*ans2$data
  ans_same = rowSums(a)
  err = sum(ans_same - ans$data)
  
  
  K = kernelPass_corr(net1, net2, kernel_options)$data
  levelplot(K, col.regions=colorRampPalette(c("black", "white")))  
}

get_mean_activity_classes = function(net_all, ro) {
  kernel_options = list(sigma = ro$fp_kernel_size, window =  ro$fp_window_size, T0 = ro$T0, Tmax = ro$Tmax, quad = 256)
  Kclasses = list()  
  for(sp in net_all) {
    #K = kernelPass_autoCorr(sp, kernel_options)
    K = kernelWindow_spikes(sp, kernel_options)
    png(sprintf("/home/alexeyche/my/sim/runs/pics/mean_%s.png",K$label), width=1024, height=480)
    p = levelplot(K$data, col.regions=colorRampPalette(c("black", "white")))
    print(p)
    dev.off()
    
#    diag(K$data) = 0
    if(! K$label %in% names(Kclasses)) { 
      Kclasses[[as.character(K$label)]] = array(K$data, dim= c(nrow(K$data), ncol(K$data), 1))
    } else {
      Kclasses[[as.character(K$label)]] = abind(Kclasses[[as.character(K$label)]], K$data)  
    }
  }
  Kmean_classes = list()
  for(cl in names(Kclasses)) {
    Kmean_classes[[cl]] = apply(Kclasses[[cl]], c(1,2), mean)
  }
  return(Kmean_classes)
}
