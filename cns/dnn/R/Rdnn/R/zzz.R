
.onLoad <- function(pkgname, libname) {
    assign("TProto", Module("dnnMod")$TProto, envir=parent.env(environment()))
    setMethod( "show", TProto, function(object) {
        object$print()
    } )
}


proto.read = function(f) {
    f = path.expand(f)
    return(Module("dnnMod")$TProto$new()$read(f))
}


proto.write = function(l, f) {
    f = path.expand(f)
    return(Module("dnnMod")$TProto$new()$write(l, f))
}