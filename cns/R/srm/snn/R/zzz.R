##
## zzz.R: Loading Rcpp 
##

## .onLoad <- function (lib, pkg) {
##     req <- get(paste("req", "uire", sep=""))	# we already Imports: methods, but there may
##     req("methods")  	        		# be a race condition lurking
##     loadRcppModules(direct=FALSE)

##     setMethod("show", "Rcpp_date", .show_date)
##     setGeneric("format", function(x,...) standardGeneric("format") )
##     setMethod("format", "Rcpp_date", .format_date )
## }


loadModule("snnMod", TRUE)

#newSRMLayer = function(N, id_shift=0) {
#    l = new(SRMLayer, N, id_shift)
#    setMethod( "show", l, function(object){
#         msg <- paste( "World object with message : ", object$greet() )
#     writeLines( msg )
#    } )   
#    return(l)
#}


### create a variable 'bdt' from out bdtMod Module
### this variable is used as a package-global instance
#delayedAssign( "bdt", local( {
#    x <- new( bdtDate )
#    x$setFromUTC()
#    x
#}) )
#
#.format_date <- function(x, ...) format(x$getDate(), ...)
#.show_date <- function(object) print(object$getDate())
#
### define an onLoad expression to set some methods
#evalqOnLoad({
#    setMethod("show", "Rcpp_bdtDate", .show_date)
#    setGeneric("format", function(x,...) standardGeneric("format") )
#    setMethod("format", "Rcpp_bdtDate", .format_date )
#})
