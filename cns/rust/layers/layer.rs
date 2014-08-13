
pub trait NetLayer {
    fn new(size: uint, id : uint, glob_id: &mut uint) -> Self;
}

#[deriving(Show)]
struct InputNeuron {
    id : uint,
}

pub struct InputLayer {
    id : uint,
    neurons: Vec<InputNeuron>,
}
impl InputNeuron {
    fn new(id: uint) -> InputNeuron {
        InputNeuron { id : id }
    }
}
impl NetLayer for InputLayer{
    fn new(size: uint, id : uint, glob_id: &mut uint) -> InputLayer {
       let mut l = InputLayer { neurons : vec!{}, id : id };
       for i in range(0u, size) {
           l.neurons.push( InputNeuron::new(*glob_id) );
           *glob_id+=1;
       }
       return l;
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

impl Neuron {
    fn new(id: uint) -> Neuron {
        Neuron { x : vec!{}, y : 0.0, id : id, id_conns : vec!{}, w : vec!{} }
    }
}

#[deriving(Show)]
pub struct Layer {
    id : uint, 
    neurons : Vec<Neuron>,
}

impl NetLayer for Layer{
    fn new(size: uint, id : uint, glob_id: &mut uint) -> Layer {
       let mut l = Layer { neurons : vec!{}, id : id };
       for i in range(0u, size) {
           l.neurons.push( Neuron::new(*glob_id) );
           *glob_id+=1;
       }
       return l;
    }
//    pub fn connect_feed_forward(&mut self, l: &Layer, in_prob: f32, out_prob: f32) {
//    }
//    pub fn connect_reccurrent(&mut self, prob: f32) {
//    }
}


