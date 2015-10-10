
require(rmongodb,quietly=TRUE)
require(base64enc, quietly=TRUE)

decode_command = "python -c \"import zlib; import sys; import numpy as np; print '\\n'.join([ str(np.fromstring(zlib.decompress(l.decode('base64')))[0]) for l in sys.stdin.readlines() if l.strip() ])\""
decodeFromDb = function(v) {
    as.numeric(system(sprintf("echo -n \"%s\" | %s", v, decode_command), intern=TRUE))
}

mongo <- mongo.create()
p = list()

coll = "spearmint.unsupervised_clustering.jobs"
d =  mongo.find.all(mongo, coll)

for(dv in d) {
    pars = names(dv$params)
    for(par in names(dv$params)) {
        if(!(par  %in% names(p))) {
            p[[par]] = NULL
        }
        p[[par]] = c(p[[par]], dv$params[[par]]$values$value)        
    }    
}
for(pn in names(p)) {
    p[[pn]] = decodeFromDb(paste(p[[pn]], collapse=''))
}

crit_name = "eval_clustering_p_stat"
p[["criterion"]] = unlist(sapply(d, function(x) if(crit_name %in% names(x$values)) { x$values[[crit_name]] } else { NA }))
p[["id"]] = unlist(sapply(d, function(x) x$id))
pc = p$criterion
plot(p[["id"]], pc, type="l")
