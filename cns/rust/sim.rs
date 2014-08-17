

use layers::net_layer::NetLayer;
use layers::input_layer::InputLayer;
use layers::layer::Layer;
use constants::Constants;

#[deriving(Show)]
struct GlobContext {
    pub last_global_id: uint,
}

pub struct Sim {
    pub globCtx: GlobContext, 
    pub layers: Vec<Box<NetLayer>>,
}



impl Sim {
    pub fn new(c: &Constants) -> Sim {
        let mut s = Sim { layers : vec!{}, globCtx: GlobContext { last_global_id : 0 } };

        let mut inp = vec!{};
        for layer_size in c.in_lc.iter().map(|in_lc| in_lc.size) {
            let layer_id = inp.len();
            let l: InputLayer = NetLayer::new(layer_size, layer_id);
            inp.push(box l);
        }
        
        for layer_size in c.lc.iter().map(|lc| lc.size) {
            let layer_id = s.layers.len();
            let l: Layer = NetLayer::new(layer_size, layer_id);
            s.layers.push(box l);
        }
        println!("{}", s.layers.to_str());
//        println!("{}", inp);
        return s;
    }
}
