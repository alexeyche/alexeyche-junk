

NetClass = setRefClass("NetClass", fields = list(net="list", timeline="vector", labels="vector", M="vector", Tmax="vector", duration="vector", n_ids="vector"), 
                                    methods = list(
                                    initialize = function(patterns, neurons_list, duration) {
                                        timeline <<- numeric(0)
                                        Tcur = 0
                                        n_ids <<- sapply(neurons_list, function(x) x$ids())
                                        M <<- length(patterns[[1]]$data)
                                        N = sum(sapply(neurons_list, function(x) x$len()))
                                        net <<- blank_net(M+N)
                                        for(p in patterns) {
                                            if(length(p$data) != M) {
                                                cat("Non homogenious patterns discovered. Error\n")
                                                q()
                                            }
                                            invisible(sapply(1:length(p$data), function(id) {
                                                sp = p$data[[id]] + Tcur
                                                net[[id]] <<- c(net[[id]], sp)
                                            }))
                                            Tcur = Tcur + duration
                                            timeline <<- c(timeline, Tcur)
                                            labels <<- c(labels, p$label)
                                        }
                                        Tmax <<- Tcur
                                        duration <<- duration
                                    },
                                    getResponces = function() {
                                        return( decomposePatterns(.self$net[.self$n_ids], .self$timeline, .self$labels) )
                                    },
                                    replicate = function(Tmax) {
                                        repl_net = blank_net(.self$M)
                                        repl_timeline = NULL
                                        repl_labels = NULL
                                        for(T0 in seq(0, Tmax-.self$Tmax, by=.self$Tmax)) {
                                            for(i in 1:.self$M) {
                                                repl_net[[i]] = c(repl_net[[i]], .self$net[[i]]+T0 )
                                            }
                                            repl_labels = c(repl_labels, .self$labels) 
                                            repl_timeline = c(repl_timeline, .self$timeline+T0)
                                        }
                                        .self$net = repl_net
                                        .self$labels = repl_labels
                                        .self$timeline = repl_timeline
                                        .self$Tmax = max(.self$timeline)
                                    }
                                    ))




SIMClass = setRefClass("SIMClass", fields = c("obj", "n_ids"),
                            methods = list(
                            initialize = function(layers=NULL) {
                                obj <<- new(SIM)
                                .self$n_ids <<- numeric(0)
                                if(!is.null(layers)) {
                                    for( l in layers ) {
                                       .self$addLayer(l) 
                                    }
                                }
                            },
                            sim = function(sim_opt, constants, net) {
                                net$net[.self$n_ids] = blank_net(length(.self$n_ids))
                                .self$obj$sim(sim_opt, constants, net)
                            }, 
                            addLayer = function(l) {
                                if (class(l) == "SRMLayerClass") {
                                    .self$obj$addSRMLayer(l$obj)
                                } else
                                if (class(l) == "FBLayerClass") {
                                    .self$obj$addFBLayer(l$obj)
                                } else {
                                    cat("Unknown layer class", class(l), "\n")
                                    q()
                                }
                                .self$n_ids = c(.self$n_ids, l$ids())
                            }))

