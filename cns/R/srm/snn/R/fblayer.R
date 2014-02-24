

FBLayerClass = setRefClass("FBLayerClass", fields = c("obj"),
                           methods = list(
                           initialize = function(N, lowF, highF, constants) {
                            obj <<- new( FBLayer, N)
                            obj$a <<- ( 2*pi*seq(lowF, highF, length.out=N)*constants$tau ) ^2
                           },
                           setInput = function(I) {
                            .self$obj$I <- I
                           }
                           ))
