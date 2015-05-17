

require(Rdnn)
setwd(path.expand("~/dnn/r_package/r_scripts"))

datasets_dir = path.expand("~/dnn/datasets")


source("edf/read_edf.R")
source("edf/utilities.R")

d01 = read.edf(sprintf("%s/bci2000/S001/S001R01.edf", datasets_dir))
d02 = read.edf(sprintf("%s/bci2000/S001/S001R02.edf", datasets_dir))
d03 = read.edf(sprintf("%s/bci2000/S001/S001R03.edf", datasets_dir))