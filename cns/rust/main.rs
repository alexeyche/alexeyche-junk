#![feature(macro_rules)]

extern crate regex;
extern crate getopts;
use std::os;

use sim::Sim;

mod layers {
    pub mod layer;
}
mod sim;
mod args;
mod constants;



fn main() {
    let a = os::args();
    let args = match args::parse_args(a) {
        Some(v) => v,
        None => return,
    };

    println!("{}", args.to_string());
    
    let c = match constants::parse_constants(args.constantFilename) {
        Some(v) => v,
        None => return,
    };
    println!("{}", c.to_string());

    let s = Sim::new(&c);
    println!("{}", s);
}
