
use std::io::BufferedReader;
use std::io::File;
use regex::Regex;
use std::string::String;

#[deriving(Show)]
struct LayerConst {
    size: uint,
    net_edge_prob: f32,
    input_edge_prob: f32,
    output_edge_prob: f32,
    inhib_frac: f32,
}




#[deriving(Show)]
struct Constants {
    lc: Vec<LayerConst>,
}

macro_rules! regex(
    ($inp:expr) => ( 
        match Regex::new($inp) {
            Ok(re) => re,
            Err(err) => fail!("{}", err),
        };
     );
)

pub fn fill_const_struct(c : &Constants, sect: &str, val: &str) {
    if((sect == "layers")&&(val == "net_edge_prob")) {
        c.lc.push(LayerConst { net_edge_prob :
    }
}

pub fn parse_constants(const_filename : String) -> Option<Constants> {
    let path = Path::new(const_filename);
    let mut file = BufferedReader::new(File::open(&path));
    let re_group = regex!(r"^[\s]*\[(.*)\]");
    let re_name_val = regex!(r"^[\s]*([\S]+)[\s]*=[\s]*([^\n;]+)");
    let re_val = regex!(r"([\S]+)");
    
    let mut sections: Vec<String> = vec!{};
    
    let lc = vec!{};
    let mut c = Constants { lc : lc };

    for line in file.lines() {
        for cap in re_group.captures_iter(line.clone().unwrap().as_slice()) {
            println!("section: {}", cap.at(1));
            sections.push(String::from_str(cap.at(1)));
        }
        let section_current = match sections.last() {
            Some(val) => val.as_slice(),
            None => continue,
        };        
        
        for cap in re_name_val.captures_iter(line.clone().unwrap().as_slice()) {
            println!("valname: \"{}\" val: \"{}\"", cap.at(1), cap.at(2));
            for cap_val in re_val.captures_iter(cap.at(2)) {
                println!("\tmatch val {}", cap_val.at(1));
                fill_const_struct(&c, section_current, cap_val.at(1)); 
            }                
        }

        println!("section current: \"{}\"", section_current);
    }

    Some(c)
}

