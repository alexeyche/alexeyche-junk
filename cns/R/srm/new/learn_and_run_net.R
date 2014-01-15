
run_net <- function(layers, patterns, run_options, open_plots = FALSE, model_descr=NULL) {
  lengths = sapply(patterns, function(p) length(p$data))
  stopifnot(all(lengths == lengths[1]))
  
  M = lengths[1]
  
  N = sum(sapply(layers, function(l) l$len))
  id_m = seq(1, M)
  id_n = seq(M+1, M+N)
  
  all_n = M
  
  null_pattern.N = list()
  
  for(i in 1:N) {
    null_pattern.N[[i]] <- -Inf
  }
  
  net = list()
  for(ep in 1:run_options$epochs) {
    for(id_patt in 1:length(patterns)) {
      net[id_m] = patterns[[id_patt]]$data
      net[id_n] = null_pattern.N
      run_options$target_set$class = patterns[[id_patt]]$class
      
      c(net, layers, stat, mean_grad) := run_srm(layers, net, run_options)
      
      cat("epoch: ", ep, ", pattern # ", id_patt,"\n")
          
      neurons = layers[[1]]
      W = get_weights_matrix(layers)
      not_fired = all(sapply(net[id_n], function(sp) length(sp) == 1))
      
      pic_filename = sprintf("%s/run_ep%s_patt%s.png", dir, ep, id_patt)
      png(pic_filename, width=1024, height=480)
      if(!not_fired) 
        p1 = plot_rastl(net[id_n], sprintf("epoch %d, pattern %d", ep, id_patt))

      p2 = levelplot(W, col.regions=colorRampPalette(c("black", "white")))
      p3 = levelplot(mean_grad, col.regions=colorRampPalette(c("black", "white")))
      
      if(!not_fired)
        print(p1, position=c(0, 0, 0.5, 1), more=TRUE)
      print(p2, position=c(0.5, 0, 1, 0.5), more=TRUE)
      print(p3, position=c(0.5, 0.5, 1, 1))
      dev.off()
      if(open_plots) 
        system(sprintf("eog -w %s 1>/dev/null 2>/dev/null",pic_filename), ignore.stdout=TRUE, ignore.stderr=TRUE, wait=FALSE)
      
    }
  }
  W = get_weights_matrix(layers)
  if(run_mode == "learn") {
    saveMatrixList(model_file, list(W))
  }
}