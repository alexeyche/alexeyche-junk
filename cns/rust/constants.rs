
use std::io::BufferedReader;
use std::io::File;
use regex::Regex;
use std::string::String;
use std::vec::Vec;
use std::collections::HashMap;

#[deriving(Show)]
struct LayerConst {
    size: uint,
    net_edge_prob: f32,
    input_edge_prob: f32,
    output_edge_prob: f32,
    inhib_frac: f32,
}
impl LayerConst {
    fn new() -> LayerConst {
        LayerConst { size: 0, net_edge_prob : 0f32, input_edge_prob: 0f32, output_edge_prob: 0f32, inhib_frac: 0f32 }
    }
}

#[deriving(Show)]
struct Constants {
    lc: Vec<LayerConst>,
}

impl Constants {
    pub fn new() -> Constants {
        Constants { 
            lc : vec!{},
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


type ConstParsed = HashMap<String, HashMap<String,Vec<String>>>;

fn fill_constants(c: &mut Constants, cp: &ConstParsed) {
    for (section_name, section_data) in cp.iter() {
        match section_name.as_slice() {
            "layers" => {
                println!("in layers");
            },
            _ => fail!("Unknown section name: {}", section_name),
        }

        println!("section name {}", section_name);
        println!("section data {}", section_data);
    }
}


pub fn parse_constants(const_filename : String) -> Option<Constants> {
    let path = Path::new(const_filename);
    let mut file = BufferedReader::new(File::open(&path));
    let re_group = regex!(r"^[\s]*\[(.*)\]");
    let re_name_val = regex!(r"^[\s]*([\S]+)[\s]*=[\s]*([^\n;]+)");
    let re_val = regex!(r"([\S]+)");
    
    let mut current_section = String::new();
    
    let mut c = Constants::new();
    let mut m: ConstParsed = HashMap::new();

    for line in file.lines() {
        for cap in re_group.captures_iter(line.clone().unwrap().as_slice()) {
//            println!("section: {}", cap.at(1));
            current_section = String::from_str(cap.at(1));
        }
        if !current_section.is_empty() && !m.contains_key(&current_section) {
            m.insert(current_section.clone(), HashMap::new());
        }
    
        for cap in re_name_val.captures_iter(line.clone().unwrap().as_slice()) {
//            println!("valname: \"{}\" val: \"{}\"", cap.at(1), cap.at(2));
            let name_val = String::from_str(cap.at(1));
            if !m.get(&current_section).contains_key( &name_val ) {
                m.get_mut(&current_section).insert(name_val.clone(), vec!{});
            }
            for cap_val in re_val.captures_iter(cap.at(2)) {
                m.get_mut(&current_section).get_mut(&name_val).push( String::from_str(cap_val.at(1)) );
//                println!("\tmatch val {}", cap_val.at(1));
            }                
        }

//        println!("section current: \"{}\"", current_section);
    }
//    println!("{}", c);
    fill_constants(&mut c,&m);   
    Some(c)
}


//#[deriving(Show)]
//struct Constants {
//    size:                   Vec<uint>,
//    net_edge_prob:          Vec<f32>,
//    input_edge_prob:        Vec<f32>,
//    output_edge_prob:       Vec<f32>,
//    inhib_frac:             Vec<f32>,
//}
//
//impl Constants {
//    pub fn new() -> Constants {
//        Constants { 
//                        size:              vec!{}, 
//                        net_edge_prob :    vec!{}, 
//                        input_edge_prob:   vec!{}, 
//                        output_edge_prob:  vec!{}, 
//                        inhib_frac:        vec!{}, 
//        }
//    }
//}
//
//
//macro_rules! parse( 
//    ($inp:expr, $t:ident) => (
//        match from_str::<$t>($inp) {
//            Some(v) => v,
//            None => fail!("Errors while parsing {}", $inp),
//        }
//    );
//)
//
//pub fn fill_const_struct(c: &mut Constants, sect: &str, field: &str, val: &str) {
//    match (sect, field) {
//        ("layers", "size")              => c.size.push( parse!(val,uint) ),
//        ("layers", "net_edge_prob")     => c.net_edge_prob.push( parse!(val,f32) ),
//        ("layers", "input_edge_prob")   => c.input_edge_prob.push( parse!(val,f32) ),
//        ("layers", "output_edge_prob")  => c.output_edge_prob.push( parse!(val,f32) ),
//        ("layers", "inhib_frac")        => c.inhib_frac.push( parse!(val,f32) ),
//        (_, _)                          => println!("Unknown section and fields {} {}", sect, field),
//    }
//}
//
//pub fn parse_constants(const_filename : String) -> Option<Constants> {
//    let path = Path::new(const_filename);
//    let mut file = BufferedReader::new(File::open(&path));
//    let re_group = regex!(r"^[\s]*\[(.*)\]");
//    let re_name_val = regex!(r"^[\s]*([\S]+)[\s]*=[\s]*([^\n;]+)");
//    let re_val = regex!(r"([\S]+)");
//    
//    let mut current_section = String::new();
//    
//    let mut c = Constants::new();
//
//    for line in file.lines() {
//        for cap in re_group.captures_iter(line.clone().unwrap().as_slice()) {
////            println!("section: {}", cap.at(1));
//            current_section = String::from_str(cap.at(1));
//        }
//    
//        for cap in re_name_val.captures_iter(line.clone().unwrap().as_slice()) {
////            println!("valname: \"{}\" val: \"{}\"", cap.at(1), cap.at(2));
//            for cap_val in re_val.captures_iter(cap.at(2)) {
////                println!("\tmatch val {}", cap_val.at(1));
//                fill_const_struct(&mut c, current_section.as_slice(), cap.at(1), cap_val.at(1));
//            }                
//        }
//
////        println!("section current: \"{}\"", current_section);
//    }
////    println!("{}", c);
//    Some(c)
//}
//
