require(rjson, quietly=TRUE)

args <- commandArgs(trailingOnly = FALSE)
we_are_in_r_studio = length(grep("RStudio", args)) > 0
arg_i = grep("--args", args)
if(we_are_in_r_studio) {
    method = "nn_nmi"
    f = "/home/alexeyche/prog/sim_spear/eval_clustering_p_stat_structure/4366/1_proc_output.json"
} else {
    usage = function() {
        cat("Options:\n\t--method=(clustering|NN_NMI)\n\t--stat=json_file_with_stat\n")
        q()
    }
    if(length(arg_i) == 0) usage() 
    i = 1
    method = f = NULL
    while(i <= length(args)) {
        if(args[i] == "--method") {
            method = args[i+1]
            i = i + 1
        } else
        if(args[i] == "--stat") {
            f = args[i+1]
            i = i + 1
        }
        i = i + 1
    }
    if((is.null( c(method, f)))||(is.na( c(method, f)))) usage()
}
data = NULL
if(file.exists(f)) data = fromJSON(file = f)  
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
rate_penalty = function(val) {
    rates = data$rates    
    target_rate_sum = sqrt(sum(rep(target_rate, length(data$rates))^2))
    rate_sum = sqrt(sum((rates^2)))
    if(mean(rates)>target_rate) {
        mod = 200*exp( - (rate_sum - target_rate_sum)^2/2000.0)
        if(mod>1) mod = 1
        val*mod
    } else {
        val*exp( - (rate_sum - target_rate_sum)^2/500.0)
    }
}

nn_nmi = function(data) {
    library(caret, quietly=TRUE)
    dist = do.call(rbind, data$distance_matrix)
    diag(dist) <- Inf
    
    labs = data$labels
    test_labs = data$test_labels
    ulabs = unique(c(labs, test_labs))
    
    tr_ids = 1:length(labs)
    conf_m = matrix(0, nrow=length(ulabs), ncol=length(ulabs))
    
    resp = ulabs
    pred = ulabs
    for(i in 1:length(test_labs)) {
        tr_i = which(dist[i+length(labs),tr_ids] == min(dist[i+length(labs),tr_ids]))
        tr_i = tr_i[1]
        act_class = which(ulabs == labs[i])
        pred_class = which(ulabs == labs[tr_i])
        
        resp = c(resp, act_class)
        pred = c(pred, pred_class)        
    }
    t = table(pred,resp)
    cf = confusionMatrix(t)
    print(cf)
    return(-cf$overall[1])
}

calculate_criterion = function(data, pl=TRUE) {    
    dist = do.call(rbind, data$distance_matrix)
    labs = data$labels
    
    ulabs = unique(labs)
    
    if(all(dist == 0)) {
        return(0.0)
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
    val = -10*calinski_harabasz_criterion(points, ulabs, labs, centroids, global_centroid)
    if(!is.null(data$rates)) val = rate_penalty(val)
    if(pl) {
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
    }
    return(val)    
}
##############
if(!we_are_in_r_studio) {
    if(method == "clustering") {
        cat(calculate_criterion(data),"\n")
    } else    
    if(method == "nn_nmi") {
        cat(nn_nmi(data),"\n")
    }
}

