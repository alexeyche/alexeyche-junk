require(rjson, quietly=TRUE)

args <- commandArgs(trailingOnly = FALSE)
we_are_in_r_studio = length(grep("RStudio", args)) > 0
arg_i = grep("--args", args)
if(length(arg_i) == 0) {
    f = "/home/alexeyche/prog/sim_spear/eval_clustering_p_stat_structure/28/1_proc_output.json"
} else {
    f = args[arg_i+1]
}

target_rate = 10.0
###################################

dist_xy = function(x_y1, x_y2) {
    sqrt((x_y1[1] - x_y2[1])^2 + (x_y1[2] - x_y2[2])^2)
}

sse_bss_criterion = function(points, ulabs, centroids, global_centroid) {
    dist_to_c = NULL
    for(lab in ulabs) {
        labi = which(lab == ulabs)
        d = sapply(1:nrow(points), function(i) dist_xy(points[i,], centroids[labi, ]) )
        dist_to_c = cbind(dist_to_c, d)
    }
    sse = 0
    bss = 0
    for(lab in ulabs) {
        labi = which(lab == ulabs)
        ci = which(labs == lab)
        d = dist_to_c[ci,labi]
        sse = sse + sum(d^2)
        
        bss = bss + length(d)*dist_xy(centroids[labi,], global_centroid)^2
    }
    return(sse-bss)
}

calinski_harabasz_criterion = function(points, ulabs, labs, centroids, global_centroid) {
    cluster_lengths = table(labs)
    ss_b = 0
    for(lab in ulabs) {
        labi = which(lab == ulabs)        
        ss_b = ss_b + cluster_lengths[labi]*dist_xy(centroids[labi,], global_centroid)^2
    } 
    ss_w = 0
    for(lab in ulabs) {
        labi = which(lab == ulabs)
        cpoints = points[which(lab == labs),]
        d = sapply(1:nrow(cpoints), function(i) dist_xy(cpoints[i,], centroids[labi, ]) )
        ss_w = ss_w + sum(d^2)
    }
    k = length(ulabs)
    return( (ss_b/ss_w) )
}

#########################

calculate_criterion = function(proc_out_json) {    
    data = fromJSON(file = proc_out_json)  
    dist = do.call(rbind, data$distance_matrix)
    mean_rate = data$mean_rate
    labs = data$labels
    
    ulabs = unique(labs)
    
    if(all(dist == 0)) {
        return(99999)
    }
    
    fit = cmdscale(dist, 2, eig=TRUE)
    x <- fit$points[,1]
    y <- fit$points[,2]
    points = cbind(x,y)
    
    global_centroid =  matrix(c(mean(x), mean(y)), nrow=1, ncol=2)
    centroids = NULL
    for(lab in ulabs) {
        ci = which(labs == lab)
        xc = x[ci]
        yc = y[ci]
        centroids = rbind(centroids, c(mean(xc), mean(yc)))  
    }
    if(!we_are_in_r_studio) {
        png(sprintf("%s_eval_dist_matrix.png", data$epoch),width=1024, height=768)
    }
    val = -100*calinski_harabasz_criterion(points, ulabs, labs, centroids, global_centroid)
    if(mean_rate<target_rate) {
        val = val*exp(- ((mean_rate - target_rate)^2)/10.0)
    } else {
        val = val*exp(- ((mean_rate - target_rate)^2)/100.0)
    }
    suppressWarnings({
        plot(x, y, xlab="Coordinate 1", ylab="Coordinate 2", main=sprintf("Metric MDS: %s", val),    type="n")
        lab_cols = rainbow(length(ulabs))
        text(x, y, labels = labs, cex=.7, col=lab_cols[sapply(labs, function(l) which(l == ulabs))])
        points(centroids, lwd=10, pch=3, col=lab_cols)
        points(global_centroid, lwd=10, pch=3, col="black")
    })        
    if(!we_are_in_r_studio) {
        invisible(dev.off())
    }
    return(val)    
}
##############
if(!we_are_in_r_studio) {
    cat(calculate_criterion(f),"\n")
}
