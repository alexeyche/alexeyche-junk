

convNum = function(v, def) {
    if(v == "") return(def)
    return(as.numeric(v))
}
convStr = function(v, def) {
    if(v == "") return(def)
    return(v)
}

Rdnn.tempdir = function() {
    d = sprintf("%s/Rdnn_%s", convStr(Sys.getenv('TMP'), "/var/tmp"), paste(sample(c(letters, LETTERS), 20), sep="", collapse=""))
    if(!file.exists(d)) {
        dir.create(d)
    }
    return(d)
}

open_pic = function(f) {
    system(sprintf("%s %s", PIC_TOOL, f), wait=FALSE, ignore.stderr=TRUE)
}

convBool = function(v, def) {
    if(v=="") return(def)
    if(v %in% c("yes", "1", "True", "true")) {
        return(TRUE)
    }
    if(v %in% c("no", "0", "False", "false")) {
        return(FALSE)
    }
    stop(sprintf("Failed to cast %s to bool", v))
}


