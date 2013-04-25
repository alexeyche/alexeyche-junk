#!/usr/bin/RScript

require(RSQLite)

db_name <- "/home/alexeyche/ml.db"


load_matrices <- function(names, iter) {
    df_all <- dbGetQuery(con,paste("SELECT name, colNum, rowNum, value FROM matrices WHERE iter =",iter))
    for(name in names) {
        f <- df_all[df_all$name == name,2:4]        
        mat <- sparseMatrix(i = f$rowNum,
                        j = f$colNum,
                        x = f$value, index1=FALSE)
        assign(name, mat, envir = .GlobalEnv)
    }          
}

get_names <- function() {
    dbGetQuery(con,"SELECT DISTINCT name FROM matrices")$name
}
last_iter <- function() {
    dbGetQuery(con,"SELECT max(iter) as iter FROM matrices")$iter
}
while(TRUE) {
    con <- dbConnect(SQLite(), dbname=db_name, flags=SQLITE_RO)
    result <- try(load_matrices(get_names(), last_iter()), silent =TRUE)
    if (class(result) != "try-error") {
        hist(hid_m@x)
        Sys.sleep(2)
    }
    Sys.sleep(0.5)
}
#load_matrices(get_names(),0)
