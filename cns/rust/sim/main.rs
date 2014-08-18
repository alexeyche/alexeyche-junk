#![feature(macro_rules)]

extern crate regex;
extern crate getopts;
use std::os;

use sim::Sim;
use args::parse_args;
use constants::parse_constants;

mod layers {
    pub mod layer;
    pub mod input_layer;
    pub mod net_layer;
}
mod sim;
mod args;
mod constants;

macro_rules! trysome(
    ($e:expr) => (
        match $e {
            Some(v) => v,
            None => fail!("Failed operation"),
        };
    );
)

fn main() {
    let a = os::args();
    let args = trysome!(parse_args(a));

    println!("{}", args.to_string());
    
    let c = trysome!(parse_constants(args.constantFilename));

    println!("{}", c.to_string());

    let s = Sim::new(&c);
//    println!("{}", s);
}
