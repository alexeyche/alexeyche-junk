
use std::io::BufferedReader;
use std::io::File;
use regex::Regex;
use std::string::String;
use std::vec::Vec;
use std::collections::HashMap;
use std::fmt::Formatter;
use std::fmt::Result;
use std::fmt::Show;
use std::from_str::FromStr;

#[deriving(Show)]
enum LayerType {
    PoissonLayer,
}

impl FromStr for LayerType {
    #[inline]
    fn from_str(s: &str) -> Option<LayerType> {
        match s {
            "PoissonLayer" => Some(PoissonLayer),
            _ => None,
        }
    }

}    

#[deriving(Show)]
struct InputLayerConst {
    pub size: uint,
}
impl InputLayerConst {
    fn new() -> InputLayerConst {
        InputLayerConst { size: 0, }
    }
}

struct LayerConst {
    pub size: uint,
    pub net_edge_prob: f32,
    pub input_edge_prob: f32,
    pub output_edge_prob: f32,
    pub inhib_frac: f32,
    pub layer_type: LayerType,
}

impl LayerConst {
    fn new() -> LayerConst {
        LayerConst { size: 0, net_edge_prob : 0f32, input_edge_prob: 0f32, output_edge_prob: 0f32, inhib_frac: 0f32, layer_type: PoissonLayer, } 
    }
}
impl Show for LayerConst {
    fn fmt(&self, f: &mut Formatter) -> Result {
        try!(writeln!(f, ""));
        try!(writeln!(f, "\tsize : {} ", self.size));
        try!(writeln!(f, "\toutput_edge_prob : {} ", self.output_edge_prob));
        try!(writeln!(f, "\tinput_edge_prob : {} ", self.input_edge_prob));
        try!(writeln!(f, "\tinhib_frac : {} ", self.inhib_frac));
        writeln!(f, "\tlayer_type: {}", self.layer_type)
    }
}



#[deriving(Show)]
pub struct Constants {
    pub lc: Vec<LayerConst>,
    pub in_lc: Vec<InputLayerConst>,
}

impl Constants {
    pub fn new() -> Constants {
        Constants { 
            lc : vec!{}, in_lc : vec!{},
        }
    }
}


macro_rules! regex(
    ($inp:expr) => ( 
        match Regex::new($inp) {
            Ok(re) => re,
            Err(err) => fail!("{}", err),
        };
     );
)
macro_rules! parse( 
    ($inp:expr, $t:ident) => (
        match from_str::<$t>($inp) {
            Some(v) => v,
            None => fail!("Errors while parsing {}", $inp),
        }
    );
)

macro_rules! fill_lc(
    ($lc:expr, $vals:ident, $val_name:ident, $t:ident) => (
        for i in range(0, $lc.len()) {
            if $vals.len() > i {
                $lc.get_mut(i).$val_name = parse!($vals[i].as_slice(),$t);  
            } else {
                $lc.get_mut(i).$val_name =  $lc.get_mut($vals.len()-1).$val_name;
            }
        }
    );
)    

type ConstParsed = HashMap<String, HashMap<String,Vec<String>>>;

#[allow(unused_variable)]
fn fill_constants(c: &mut Constants, cp: &ConstParsed) {
    for (section_name, section_data) in cp.iter() {
        match section_name.as_slice() {
            "layers" => {
                let sizes = section_data.get(&String::from_str("size")); // marker values
                c.lc = range(0, sizes.len()).map(|i| LayerConst::new()).collect();
                
                for (val_name, vals) in section_data.iter() {
                    match val_name.as_slice() {
                        "size"              => fill_lc!(c.lc, vals, size, uint),
                        "net_edge_prob"     => fill_lc!(c.lc, vals, net_edge_prob, f32),
                        "input_edge_prob"   => fill_lc!(c.lc, vals, input_edge_prob, f32),
                        "output_edge_prob"  => fill_lc!(c.lc, vals, output_edge_prob, f32),
                        "inhib_frac"        => fill_lc!(c.lc, vals, inhib_frac, f32),
                        "layer_type"        => fill_lc!(c.lc, vals, layer_type, LayerType),
                        _ => return fail!("Unknown value {} in section name: {}", val_name, section_name),
                    }
                }
            },
            "inputs" => {
                let sizes = section_data.get(&String::from_str("size")); // marker values
                c.in_lc = range(0, sizes.len()).map(|i| InputLayerConst::new()).collect();
                
                for (val_name, vals) in section_data.iter() {
                    match val_name.as_slice() {
                        "size"              => fill_lc!(c.in_lc, vals, size, uint),
                        _ => return fail!("Unknown value {} in section name: {}", val_name, section_name),
                    }
                }
            },
            _ => fail!("Unknown section name: {}", section_name),

        }
    }
}


pub fn parse_constants(const_filename : String) -> Option<Constants> {
    let path = Path::new(const_filename);
    let mut file = BufferedReader::new(File::open(&path));
    let re_group = regex!(r"^[\s]*\[(.*)\]");
    let re_name_val = regex!(r"^[\s]*([\S]+)[\s]*=[\s]*([^\n;]+)");
    let re_val = regex!(r"([\S]+)");
    
    let mut current_section = String::new();
    let mut m: ConstParsed = HashMap::new();

    for line in file.lines() {
        for cap in re_group.captures_iter(line.clone().unwrap().as_slice()) {
            current_section = String::from_str(cap.at(1));
        }
        if !current_section.is_empty() && !m.contains_key(&current_section) {
            m.insert(current_section.clone(), HashMap::new());
        }
        for cap in re_name_val.captures_iter(line.clone().unwrap().as_slice()) {
            let name_val = String::from_str(cap.at(1));
            if !m.get(&current_section).contains_key( &name_val ) {
                m.get_mut(&current_section).insert(name_val.clone(), vec!{});
            }
            for cap_val in re_val.captures_iter(cap.at(2)) {
                m.get_mut(&current_section).get_mut(&name_val).push( String::from_str(cap_val.at(1)) );
            }                
        }

    }
    let mut c = Constants::new();
    fill_constants(&mut c,&m);   
    Some(c)
}
