

use layers::SimLayer;
use layers::Layer;

#[deriving(Show)]
pub struct Sim {
    layers: Vec<Layer> 
}


impl Sim {
    pub fn new () -> Sim { 
        let mut s: Sim = Sim { layers : Vec::new() };
        let l: Layer = SimLayer::new(0, 100);
        let mut l2: Layer = SimLayer::new(0, 100);
        l.connect_to(&mut l2, 0.5);

        s.layers.push(l);
        s.layers.push(l2);
        return s;
    }
}
