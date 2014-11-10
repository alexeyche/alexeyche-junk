

use layers::Layer;
use layers::BasicNeuron;

#[deriving(Show)]
pub struct Sim<'a> {
    layers: Vec<&'a Layer> 
}


impl<'a> Sim<'a> {
    pub fn new () -> Sim<'a> { 
        let mut s: Sim = Sim { layers : Vec::new() };
//        let l: Layer = Layer<BasicNeuron>::new(0, 100);
//        let mut l2: Layer = Layer::new(0, 100);
//        l.connect_to(&mut l2, 0.5);

//        s.layers.push(l);
//        s.layers.push(l2);
        return s;
    }
}
