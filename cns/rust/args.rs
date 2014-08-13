
use getopts::{optopt,optflag,getopts,usage};

#[deriving(Eq, Show, PartialEq)]
enum Err {
    // Now we don't need that phoney None variant.
    ParseErr,
}

#[deriving(Show)]
struct Args {
    pub constantFilename: String
}

static BRIEF: &'static str = "Spiking neural network simulator";

pub fn parse_args(args: Vec<String>) -> Option<Args> {
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
        return None;
    }
    let constF = match matches.opt_str("c") {
        Some(c) => c,
        None => { println!("Need constants filename!"); return None; }
    };

    Some(Args { constantFilename : constF })
}

