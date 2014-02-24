

SIMClass = setRefClass("SIMClass", fields = c("obj"),
                            methods = list(
                            initialize = function(layers=NULL) {
                                obj <<- new(SIM)
                                if(!is.null(layers)) {
                                    for( l in layers ) {
                                       .self$addLayer(l) 
                                    }
                                }
                            },
                            sim = function(sim_opt, constants, net) {
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
                            }))

