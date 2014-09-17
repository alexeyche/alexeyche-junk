#.onLoad <- function(pkgname, libname){
#    # load the module and store it in our namespace
#    unlockBinding( "r_snn_mod" , .NAMESPACE )
#    assign( "r_snn_mod",  Module( "r_snn_mod" ), .NAMESPACE )
#    lockBinding( "r_snn_mod", .NAMESPACE )
#}
loadModule("snnMod", TRUE)
#mod <- Module("snnMod")

