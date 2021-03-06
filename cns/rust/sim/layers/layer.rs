use layers::net_layer::{NetNeuron, NetLayer, getGlobalId};
use std::fmt::Show;
use std::fmt::Formatter;
use std::fmt;
use std;

#[deriving(Show)]
struct Synapse {
    id_pre: uint,
    w: f32,
    x: f32,
}

#[deriving(Show)]
struct Neuron {
    id : uint,
    y : f32,
    syns: Vec<Synapse>,
}

impl NetNeuron for Neuron {
    fn new(id: uint) -> Neuron {
        Neuron { y : 0.0, id : id, syns: vec!{} }
    }
    fn add_connection(&mut self, id : uint, w : f32) -> () {
        println!("===")
    }
    fn getId(&self) -> uint {
        return self.id;
    }

}

#[deriving(Show)]
pub struct Layer {
    id : uint, 
    neurons : Vec<Box<NetNeuron>>,
}

impl NetLayer for Layer {
    fn new(size: uint, id : uint) -> Layer {
        let mut l = Layer { neurons : vec!{}, id : id };
        for i in range(0u, size) {
            let n: Neuron = NetNeuron::new(getGlobalId());
            l.neurons.push(box n);
        }
        return l;
    } 
    fn getNeurons(&mut self) -> &mut Vec<Box<NetNeuron>> {
        return &mut self.neurons;
    }
}

//impl Show for Box<NetNeuron> {
//    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
//                
//
//    }
//}


//impl NetLayer for Layer{
//    fn new(size: uint, id : uint, glob_id: &mut uint) -> Layer {
//        let mut l = Layer { neurons : vec!{}, id : id };
//        for i in range(0u, size) {
//            let n: Neuron = NetNeuron::new(*glob_id);
//            l.neurons.push(box n);
//            *glob_id+=1;
//        }
//        return l;
//    }
//    
//    fn getNeurons(&mut self) -> &mut Vec<Box<NetNeuron>> {
//        return &mut self.neurons;
//    }
//}

impl Layer {
    pub fn connect_reccurrent(&mut self, prob: f32) {
    }
}

