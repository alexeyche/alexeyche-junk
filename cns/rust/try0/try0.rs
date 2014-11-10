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

#[deriving(Clone)]
struct Layer<'a, T:'a> {
    id : uint,
    neurons: Vec<T>
}

trait SimLayer<'a, T:Neuron+'a> {
    fn new( id: uint, size: uint ) -> Self;
//    fn random_connection(&self, dst: &mut Self, prob: f32);
}

impl<'a, T: 'a+Neuron+fmt::Show> SimLayer<'a,T> for Layer<'a,T> {
    fn new( id: uint, size: uint ) -> Layer<'a, T> {
        let mut l: Layer<'a,T> = Layer { neurons : vec!{}, id : id };
        for i in range(0u, size) {
            let n = Neuron::new(get_global_id());
            l.neurons.push( n );
        }
        return l;
    }
    fn random_connection(&self, dst: &mut Layer<T>, prob: f32) {
        let runif = Range::new(0f32, 1f32);
        let mut rng = rand::task_rng();

        for i in range(0u, self.neurons.len()) {
            for j in range(0u, dst.neurons.len()) {
                let coin = runif.ind_sample(&mut rng);
                if coin >= prob {
                    dst.neurons[j].add_connection(&self.neurons[i]);
                }
            }
        }
    }
}


impl<'a,T:fmt::Show> fmt::Show for Layer<'a, T> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        for i in range(0u, self.neurons.len()) {
            try!(write!(f, "{}\n", self.neurons[i])); 
        }
        Ok(())    
    }
}



//#[deriving(Show)]
struct Sim<'a> {
    layers: Vec<Box<SimLayer<'a, Neuron+'a>+'a>>
}

impl<'a> Sim<'a> {
    pub fn new() -> Sim<'a> {
        Sim { layers : Vec::new() }
    }
    pub fn add_layer(&mut self, l: Box<SimLayer<Neuron+'a>+'a>) {
        self.layers.push(l);        
    }
}

//impl<'a> fmt::Show for Sim<'a> {
//    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
//        for i in range(0u, self.layers.len()) {
//            try!(write!(f, "{}\n", self.layers[i])); 
//        }
//        Ok(())
//    }
//}



fn main() {
    let mut l: Layer<BasicNeuron> = SimLayer::new(0, 100);
    let mut l2: Layer<BasicNeuron> = SimLayer::new(0, 100);

//    l2.random_connection(&mut l, 0.5);
    let s = Sim::new();
    s.add_layer(box l);
//    v.push(box l);

    println!("{}", l2);
}
