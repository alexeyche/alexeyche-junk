
use getopts::{optopt,optflag,getopts,OptGroup,usage,short_usage};
use std::os;

#[deriving(Eq, Show, PartialEq)]
enum Err {
    // Now we don't need that phoney None variant.
    ParseErr,
}

struct Args {
    constantFilename: &'static str
}

static BRIEF: &'static str = "Spiking neural network simulator";

pub fn parseArgs(args: Vec<String>) -> Option<Args> {
    let program = args.get(0).clone();

    let opts = [
        optopt("c", "", "set constants file name", ""),
        optflag("h", "help", "print this help menu")
    ];
    let matches = match getopts(args.tail(), opts) {
        Ok(m) => { m }
        Err(f) => { fail!(f.to_err_msg()) }
    };

    if matches.opt_present("h") {
        println!("{}", usage(BRIEF, opts));
        return(None);
    }
    let constF = match  matches.opt_str("c") {
        Some(c) => c.as_slice(),
        None => { return(None) }            
    };
    Some(Args { constantFilename : constF })
}

