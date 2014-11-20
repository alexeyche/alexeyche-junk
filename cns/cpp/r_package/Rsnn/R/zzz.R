#.onLoad <- function(pkgname, libname){
#    # load the module and store it in our namespace
#    unlockBinding( "r_snn_mod" , .NAMESPACE )
#    assign( "r_snn_mod",  Module( "r_snn_mod" ), .NAMESPACE )
#    lockBinding( "r_snn_mod", .NAMESPACE )
#}

.onLoad <- function(pkgname, libname) {
    assign("RSim", Module("snnMod")$RSim, envir=parent.env(environment()))
    assign("RProto", Module("snnMod")$RProto, envir=parent.env(environment()))
    assign("RConstants", Module("snnMod")$RConstants, envir=parent.env(environment()))
    setMethod( "show", RSim, function(object) {
        object$print()    
    } )
    setMethod( "show", RConstants, function(object) {
        object$print()
    } )
    setMethod( "show", RProto, function(object) {
        object$print()
    } )

}

#loadModule("snnMod", TRUE)
#mod <- Module("snnMod")

