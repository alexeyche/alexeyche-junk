
use getopts::{optopt,optflag,getopts,usage};


#[deriving(Show)]
struct Args {
    pub constant_filename: String
}

static BRIEF: &'static str = "Spiking neural network simulator";

pub fn parse_args(args: Vec<String>) -> Option<Args> {
    let opts = [
        optopt("c", "", "set constants file name", ""),
        optflag("h", "help", "print this help menu")
    ];
    let matches = match getopts(args.tail(), opts) {
        Ok(m) => { m }
        Err(f) => { panic!(f) }
    };

    if matches.opt_present("h") {
        println!("{}", usage(BRIEF, opts));
        return None;
    }
    let const_f = match matches.opt_str("c") {
        Some(c) => c,
        None => { println!("Need constants filename!"); return None; }
    };

    Some(Args { constant_filename : const_f })
}

