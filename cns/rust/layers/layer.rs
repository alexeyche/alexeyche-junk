use std::rand;
use std::rand::distributions::{IndependentSample, Range};

pub trait NetNeuron {
    fn new(id: uint) -> Self;
    fn add_connection(&mut self, id : uint, w : f32) -> ();
    fn getId(&self) -> uint;
}

pub trait NetLayer {
    fn new(size: uint, id : uint, glob_id: &mut uint) -> Self;
//    fn connect_feed_forward(&mut self, l: &mut NetLayer, conn_prob: f32) -> ();
    fn getNeurons(&mut self) -> &mut Vec<Box<NetNeuron>>;
    
    fn connect_feed_forward(&mut self, l: &mut NetLayer, conn_prob: f32) {
        let runif = Range::new(0f32, 1f32);
        let mut rng = rand::task_rng();
        for pre in self.getNeurons().iter() {
            for post in l.getNeurons().mut_iter() {
                let coin = runif.ind_sample(&mut rng);
                if coin >= conn_prob {
                    println!("{} >= {}",  coin, conn_prob);
                    post.add_connection(pre.getId(), 0.0);
                }
            }
        }            
    }

}

#[deriving(Show)]
pub struct InputNeuron {
    id : uint,
}

impl NetNeuron for InputNeuron {
    fn new(id: uint) -> InputNeuron {
        InputNeuron { id : id }
    }
    fn add_connection(&mut self, id : uint, w : f32) -> () {
        println!("InputLayer is not supposed to get connections");
    }
    fn getId(&self) -> uint {
        return self.id;
    }
}

pub struct InputLayer {
    id : uint,
    neurons: Vec<Box<NetNeuron>>,
}


impl NetLayer for InputLayer {
    fn new(size: uint, id : uint, glob_id: &mut uint) -> InputLayer {
       let mut l = InputLayer { neurons : vec!{}, id : id };
       for i in range(0u, size) {
//           l.neurons.push( InputNeuron::new(*glob_id) );
           *glob_id+=1;
       }
       return l;
    }
    fn getNeurons(&mut self) -> &mut Vec<Box<NetNeuron>> {
        return &mut self.neurons;
    }
}


#[deriving(Show)]
struct Neuron {
    id : uint,
    x : Vec<f32>,
    y : f32,
    id_conns : Vec<uint>,
    w : Vec<uint>,
}

impl NetNeuron for Neuron {
    fn new(id: uint) -> Neuron {
        Neuron { x : vec!{}, y : 0.0, id : id, id_conns : vec!{}, w : vec!{} }
    }
    fn add_connection(&mut self, id : uint, w : f32) -> () {
        println!("===")
    }
    fn getId(&self) -> uint {
        return self.id;
    }

}

pub struct Layer {
    id : uint, 
    neurons : Vec<Box<NetNeuron>>,
}

impl NetLayer for Layer{
    fn new(size: uint, id : uint, glob_id: &mut uint) -> Layer {
       let mut l = Layer { neurons : vec!{}, id : id };
       for i in range(0u, size) {
//           l.neurons.push( box Neuron::new(*glob_id) );
           *glob_id+=1;
       }
       return l;
    }
    
    fn getNeurons(&mut self) -> &mut Vec<Box<NetNeuron>> {
        return &mut self.neurons;
    }
}

impl Layer {
    pub fn connect_reccurrent(&mut self, prob: f32) {
    }
}

