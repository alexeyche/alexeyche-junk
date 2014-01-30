
run_net <- function(input_neurons, layers, run_options, open_plots = FALSE, model_descr=NULL) {
  net_neurons = layers
  patterns = input_neurons$patterns
  lengths = sapply(patterns, function(p) length(p$data))
  stopifnot(all(lengths == lengths[1]))
  
  id_m = input_neurons$ids
  id_n = c(sapply(net_neurons$l, function(n) n$ids))  
  
  net = list()
  net[id_m] = patterns[[1]]$data
  net[id_n] = -Inf
  run_options$target_set$class = patterns[[id_patt]]$class
  
  loss = NULL  
  stable = NULL  
  for(ep in run_options$start_epoch:run_options$epochs) {
    mean_dev = NULL
    net_all = list()    
    for(id_patt in 1:length(patterns)) {
      net[id_m] = patterns[[id_patt]]$data
      net[id_n] = -Inf
      run_options$target_set$label = patterns[[id_patt]]$label
      
      c(net, net_neurons, stat, grad) := run_srm(net_neurons, net, run_options)
      mean_dev = c(mean_dev, reward_func(net[id_n], run_options))
      cat("epoch: ", ep, ", pattern # ", id_patt,"\n")
          
      neurons = net_neurons$l[[1]]
      W = get_weights_matrix(net_neurons$l)
      not_fired = all(sapply(net[id_n], function(sp) length(sp) == 1))
      
      pic_filename = sprintf("%s/R/run_ep%s_patt%s_label%s.png", dir, ep, id_patt, patterns[[id_patt]]$label)
      png(pic_filename, width=1024, height=480)
      if(!not_fired) 
        p1 = plot_rastl(net[id_n], sprintf("epoch %d, pattern %d, class %d", ep, id_patt, patterns[[id_patt]]$label))

      p2 = levelplot(W, col.regions=colorRampPalette(c("black", "white")))
      p3 = levelplot(list_to_matrix(grad), col.regions=colorRampPalette(c("black", "white")))
      if(!is.null(loss)) {
        dfrm = data.frame(x=1:length(loss), y=c(loss))
        p4 = xyplot(y~x, data=dfrm, type="l")
      }
      if(!is.null(stable)) {
        dfrm = data.frame(x=1:length(stable), y=c(stable))
        p5 = xyplot(y~x, data=dfrm, type="l")
      }
#      if(!is.null(mean_dev))
#        p4 = xyplot(y~x, list(x=1:id_patt,y=mean_dev), type="l")
      
      if(!not_fired)
        print(p1, position=c(0, 0.5, 0.5, 1), more=TRUE)
      if(!is.null(loss))
        print(p4, position=c(0, 0, 0.5, 0.25), more=TRUE)
      if(!is.null(stable)) {
        print(p5, position=c(0, 0.25, 0.5, 0.5), more=TRUE)
      }
#      if(!is.null(mean_dev))
#        print(p4, position=c(0, 0, 0.5, 0.5), more=TRUE)      
      print(p2, position=c(0.5, 0, 1, 0.5), more=TRUE)
      print(p3, position=c(0.5, 0.5, 1, 1))
      
      dev.off()
      if(open_plots) 
        system(sprintf("eog -w %s 1>/dev/null 2>/dev/null",pic_filename), ignore.stdout=TRUE, ignore.stderr=TRUE, wait=FALSE)
      net_all[[id_patt]] = list(data=net[id_n], label=patterns[[id_patt]]$label)
      
    }
    if(run_options$reward_learning)
      run_options$mean_activity_stat = get_mean_activity(net_all, run_options)
      
    model_file = sprintf("%s/R/%s_%dx%d_%d", dir, data, M, N, ep)
 
    W = get_weights_matrix(list(neurons))
    saveMatrixList(model_file, list(W))
    if((! is.null(run_options$test_function))&&(ep %% run_options$test_run_freq == 0)) {
      o_train = evalNet(patterns, run_options, constants, net_neurons$l)
      o_test = evalNet(run_options$test_patterns, run_options, constants, net_neurons$l)
      
      t_out <- run_options$test_function(o_train$spikes, o_test$spikes)
      
      if((length(stable)>0)&&(t_out$stable>(5*stable[length(stable)]))) {
        t_out$stable <- stable[length(stable)]
      }
      
      stable <- c(stable, t_out$stable)      
      loss <- c(loss, t_out$loss)
      system( sprintf("echo %s > %s/R/%d.log", t_out$loss, dir, ep))
#      mode_acc = run_options$mode
#      test_net_all = list()
#      cat("Running net on test data (N=", length(run_options$test_patterns),") with ", run_options$trials, " sampling trials\n", sep="")
#      for(id_patt in 1:length(run_options$test_patterns)) {
#        for(trial in 1:run_options$trials) {
#          glob_id = trial+(id_patt-1)*run_options$trials
#          
#          net[id_m] = run_options$test_patterns[[id_patt]]$data
#          net[id_n] = -Inf
#          run_options$target_set$class = run_options$test_patterns[[id_patt]]$label
#          run_options$mode = "run"          
#          
#          c(net, net_neurons, stat, mean_grad) := run_srm(net_neurons, net, run_options)
#          net = lapply(net[neurons$ids], function(sp) sp[sp != -Inf])
#
#          test_net_all[[glob_id]] = list(data=net, label=run_options$test_patterns[[id_patt]]$label)
#        }
#      }
#      net_all = list()
#      cat("Running net on train data (N=", length(patterns),") with ", run_options$trials, " sampling trials\n", sep="")
#      for(id_patt in 1:length(patterns)) {
#        for(trial in 1:run_options$trials) {
#          glob_id = trial+(id_patt-1)*run_options$trials
#          
#          net[id_m] = patterns[[id_patt]]$data
#          net[id_n] = -Inf
#          run_options$target_set$class = patterns[[id_patt]]$label
#          run_options$mode = "run"          
#          
#          c(net, net_neurons, stat, mean_grad) := run_srm(net_neurons, net, run_options)
#          net = lapply(net[neurons$ids], function(sp) sp[sp != -Inf])
#          
#          net_all[[glob_id]] = list(data=net, label=patterns[[id_patt]]$label)
#        }
#      }     
#      run_options$mode = mode_acc
#      loss <- c(loss, run_options$test_function(net_all, test_net_all))
      
      #system(sprintf("R --slave --args %s < eval.R > %s/R/%d.log", model_file, dir, ep), wait=FALSE) 
    }

  }

}
