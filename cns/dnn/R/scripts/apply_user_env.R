
require(Rdnn)
require(rjson)

user.json = fromJSON(readConst(user.json.file()))
user.env = user.json[[ Sys.getenv("USER") ]]
if(!is.null(user.env)) {
    user.env.def = list()
    for(e in names(user.env)) {
        if(Sys.getenv(e) == "") {
            user.env.def[[e]] = user.env[[e]]
        }
    }
    if(length(user.env.def)>0) {
        do.call(Sys.setenv, user.env.def)    
    }
}
