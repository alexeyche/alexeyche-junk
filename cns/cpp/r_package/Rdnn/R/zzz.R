
.onLoad <- function(pkgname, libname) {
    assign("RSim", Module("dnnMod")$RSim, envir=parent.env(environment()))
    assign("RMatchingPursuit", Module("dnnMod")$RMatchingPursuit, envir=parent.env(environment()))
    assign("RProto", Module("dnnMod")$RProto, envir=parent.env(environment()))
    assign("RConstants", Module("dnnMod")$RConstants, envir=parent.env(environment()))
    assign("RGammatoneFB", Module("dnnMod")$RGammatoneFB, envir=parent.env(environment()))
    setMethod( "show", RSim, function(object) {
        object$print()    
    } )
    setMethod( "show", RConstants, function(object) {
        object$print()
    } )
    setMethod( "show", RProto, function(object) {
        object$print()
    } )
    setMethod( "show", RGammatoneFB, function(object) {
        object$print()    
    } )
    setMethod( "show", RMatchingPursuit, function(object) {
        object$print()
    } )

}

