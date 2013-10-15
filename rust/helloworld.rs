#!/usr/local/bin/rust run


use std::vec;
use std::task;
use std::io;

fn main() {
    let value = vec::from_fn(5, |x| x + 1);      // (1)
    let (server_chan, server_port) = stream();   // (2)
    let (client_chan, client_port) = stream();   // (3)

    do task::spawn {
        let val: ~[uint] = server_chan.recv();   // (4)
        let res = val.map(|v| {v+1});
        client_port.send(res)                    // (5)
    }

    server_port.send(value);                     // (6)
    io::println(fmt!("Result: %?", 
    client_chan.recv()));                        // (7) 
}
