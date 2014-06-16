
use std::io::BufferedReader;
use std::io::File;
use regex::Regex;

#[deriving(Show)]
struct NetConfiguration {
    input_layers_size: Vec<uint>,
    net_layers_size: Vec<uint>,
    net_edge_prob: Vec<f32>,
    input_edge_prob: Vec<f32>,
    output_edge_prob: Vec<f32>,
    inhib_frac: Vec<f32>,
}


#[deriving(Show)]
struct Constants {
    net: NetConfiguration
}



pub fn parse_constants(const_filename : String) -> Option<Constants> {
    let path = Path::new(const_filename);
    let mut file = BufferedReader::new(File::open(&path));
    let re_group = match Regex::new(r"\[.*\]") {
        Ok(re) => re,
        Err(err) => fail!("{}", err),
    };
    
    for line in file.lines() {
        if re_group.is_match(line.clone().unwrap().as_slice()) {
            print!("match {}", line.unwrap());
        }
    }
    let input_layers_size = vec!{};
    let net_layers_size = vec!{};
    let net_edge_prob = vec!{};
    let input_edge_prob = vec!{};
    let output_edge_prob = vec!{};
    let inhib_frac = vec!{};

    Some(Constants { net : NetConfiguration { input_layers_size : input_layers_size, net_layers_size : net_layers_size, net_edge_prob : net_edge_prob, input_edge_prob : input_edge_prob, output_edge_prob : output_edge_prob, inhib_frac : inhib_frac } })
}

