#![feature(macro_rules)]


use common::get_global_id;
use std::fmt;
use std::rand::distributions::{IndependentSample, Range};
use std::rand;

mod common;

#[deriving(Show)]
struct Synapse {
    id_pre: uint,
    x: f32,
    w: f32,
//    input_q: Queue<Spike>,
//    net_q: Queue<Spike>,
}


impl Synapse {
    fn new( id_pre : uint ) -> Synapse {
        Synapse { 
            id_pre : id_pre, 
            x : 0.0, 
            w : 0.0, 
//            input_q : Queue<Spike>::new(), 
//            net_q : Queue<Spike>::new() 
        }
    }
}

#[deriving(Show)]
struct BasicNeuron {
    id : uint,
    y : f32,
    syns: Vec<Synapse>,
    qr : f32
}


trait Neuron {
    fn new( id : uint ) -> Self;
    fn add_connection(&mut self, pre: &Self);
}

impl Neuron for BasicNeuron {
    fn add_connection(&mut self, pre: &BasicNeuron) {
        self.syns.push(Synapse::new(pre.id));
    }
    fn new( id : uint ) -> BasicNeuron {
        return BasicNeuron { 
            id : id, 
            y : 0.0, 
            syns : Vec::new(),
            qr : 0.0 
        };
    }
}



#[deriving(Show,Clone)]
struct Layer<T> {
    id : uint,
    neurons: Vec<Box<T>>
}

trait SimLayer  {
    fn new( id: uint, size: uint ) -> Self;
    fn add_random_connections(&mut self, dst: &Self, prob: f32);
}

impl<T:Neuron> Layer<T> {
    fn new( id: uint, size: uint ) -> Layer<T> {
        let mut l: Layer<T> = Layer { neurons : vec!{}, id : id };
        for i in range(0u, size) {
            let n: T = Neuron::new(get_global_id());
            l.neurons.push( box n );
        }
        return l;
    }
    fn add_random_connections(&mut self, dst: &Layer<T>, prob: f32) {
        let runif = Range::new(0f32, 1f32);
        let mut rng = rand::task_rng();

        for i in range(0u, self.neurons.len()) {
            for j in range(0u, dst.neurons.len()) {
                let coin = runif.ind_sample(&mut rng);
                if coin >= prob {
                    self.neurons[i].add_connection(&*dst.neurons[j]);
                }
            }
        }
    }

}


//struct Sim<'a> {
//    layers: Vec< Box<SimLayer> >
//}
//
//impl<'a> Sim<'a> {
//    pub fn new() -> Sim<'a> {
//        Sim { layers : Vec::new() }
//    }
//    pub fn add_layer(&mut self, l: Box<SimLayer<Neuron+'a>+'a>) {
//        self.layers.push(l);        
//    }
//}

fn main() {
    let mut v: Vec<Box<Layer<Box<Neuron>> > > = Vec::new();

    let mut l: Layer<BasicNeuron> = Layer::new(0, 100);
    let mut l2: Layer<BasicNeuron> = Layer::new(0, 100);
    l.add_random_connections(&l2, 0.5);
    
    v.push(box l);

    println!("{}", l);
}
