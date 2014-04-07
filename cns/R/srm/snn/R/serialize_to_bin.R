#!/usr/bin/RScript
saveMatrixList <- function(baseName, mtxList) {
    idxName <- paste(baseName, ".idx", sep="")
    idxCon <- file(idxName, 'wb')
    on.exit(close(idxCon))

    dataName <- paste(baseName, ".bin", sep="")
    con <- file(dataName, 'wb')
    on.exit(close(con))

    writeBin(0L, idxCon)

    for (m in mtxList) {
        writeBin(dim(m), con)
        writeBin(typeof(m), con)
        writeBin(c(m), con)
        flush(con)

        offset <- as.integer(seek(con))
#        cat('offset', offset)
        writeBin(offset, idxCon)
    }

    flush(idxCon)
}

loadMatrix <- function(baseName = "data", index) {
    idxName <- paste(baseName, ".idx", sep="")
    idxCon <- file(idxName, 'rb')
    on.exit(close(idxCon))

    dataName <- paste(baseName, ".bin", sep="")
    con <- file(dataName, 'rb')
    on.exit(close(con))

    seek(idxCon, (index-1)*4)
    offset <- readBin(idxCon, 'integer')

    seek(con, offset)
    d <- readBin(con, 'integer', 2)
    type <- readBin(con, 'character', 1)
    close(idxCon)
    return(structure(readBin(con, type, prod(d)), dim=d))
}
