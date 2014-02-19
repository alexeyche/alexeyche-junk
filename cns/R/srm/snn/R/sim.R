

SIMClass = setRefClass("SIMClass", fields = c("obj"),
                            methods = list(
                            initialize = function(layers=NULL) {
                                obj <<- new(SIM)
                                if(!is.null(layers)) {
                                    for( l in layers ) {
                                        .self$obj$addLayer(l$obj)
                                    }
                                }
                            },
                            sim = function(sim_opt, constants, net) {
                                .self$obj$sim(sim_opt, constants, net)
                            }, 
                            addLayer = function(o) {
                                .self$obj$addLayer(o)
                            }))

