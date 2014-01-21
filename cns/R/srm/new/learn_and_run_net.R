
run_net <- function(layers, run_options, open_plots = FALSE, model_descr=NULL) {
  input_neurons = layers[[1]]
  net_neurons = layers[2:length(layers)]
  patterns = input_neurons$patterns
  lengths = sapply(patterns, function(p) length(p$data))
  stopifnot(all(lengths == lengths[1]))
  
  id_m = input_neurons$ids
  id_n = c(sapply(layers[2:length(layers)], function(n) n$ids))  
  
  net = list()
  net[id_m] = patterns[[1]]$data
  net[id_n] = -Inf
  run_options$target_set$class = patterns[[id_patt]]$class
  
  loss = NULL
  for(ep in 1:run_options$epochs) {
    net_all = list()
    for(id_patt in 1:length(patterns)) {
      net[id_m] = patterns[[id_patt]]$data
      net[id_n] = -Inf
      run_options$target_set$label = patterns[[id_patt]]$label
      
      c(net, net_neurons, stat, mean_grad) := run_srm(net_neurons, net, run_options)
      
      cat("epoch: ", ep, ", pattern # ", id_patt,"\n")
          
      neurons = net_neurons[[1]]
      W = get_weights_matrix(net_neurons)
      not_fired = all(sapply(net[id_n], function(sp) length(sp) == 1))
      
      pic_filename = sprintf("%s/R/run_ep%s_patt%s_label%s.png", dir, ep, id_patt, patterns[[id_patt]]$label)
      png(pic_filename, width=1024, height=480)
      if(!not_fired) 
        p1 = plot_rastl(net[id_n], sprintf("epoch %d, pattern %d, class %d", ep, id_patt, patterns[[id_patt]]$label))

      p2 = levelplot(W, col.regions=colorRampPalette(c("black", "white")))
      p3 = levelplot(mean_grad, col.regions=colorRampPalette(c("black", "white")))
      if(!is.null(loss))
        p4 = xyplot(y~x, list(x=1:ep,y=loss), type="l")
      
      if(!not_fired)
        print(p1, position=c(0, 0.5, 0.5, 1), more=TRUE)
      if(!is.null(loss))
        print(p4, position=c(0, 0, 0.5, 0.5), more=TRUE)
      print(p2, position=c(0.5, 0, 1, 0.5), more=TRUE)
      print(p3, position=c(0.5, 0.5, 1, 1))
      
      dev.off()
      if(open_plots) 
        system(sprintf("eog -w %s 1>/dev/null 2>/dev/null",pic_filename), ignore.stdout=TRUE, ignore.stderr=TRUE, wait=FALSE)
      net_all[[id_patt]] = list(data=net, label=patterns[[id_patt]]$label)
      
    }
    if(! is.null(run_options$test_function)) {
      mode_acc = run_options$mode
      test_net_all = list()
      for(id_patt in 1:length(run_options$test_patterns)) {
        for(trial in 1:run_options$trials) {
          glob_id = trial+(id_patt-1)*run_options$trials
          
          net[id_m] = run_options$test_patterns[[id_patt]]$data
          net[id_n] = -Inf
          run_options$target_set$class = run_options$test_patterns[[id_patt]]$label
          run_options$mode = "run"          
          
          c(net, net_neurons, stat, mean_grad) := run_srm(net_neurons, net, run_options)
          test_net_all[[glob_id]] = list(data=net, label=run_options$test_patterns[[id_patt]]$label)
        }
      }
      run_options$mode = mode_acc
      loss <- c(loss, run_options$test_function(net_all, test_net_all))
    }
  }

}