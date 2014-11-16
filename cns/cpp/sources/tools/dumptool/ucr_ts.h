#pragma once

Protos::LabeledTimeSeries convertUcrTimeSeriesLine(const string &line) {
    vector<string> els = split(line, ' ');
    assert(els.size() > 0);
    
    string lab;
    vector<double> ts_data;
    for(size_t i=0; i<els.size(); i++) {
        trim(els[i]);
        if(!els[i].empty()) {
            if(lab.empty()) {
                std::ostringstream lab_format;
                lab_format << stoi(els[i]);
                lab = lab_format.str();
                continue;
            }
            ts_data.push_back(stof(els[i]));
        }
    }
    
    Protos::LabeledTimeSeries ts = doubleVectorToLabeledTimeSeries(lab, ts_data);
    return ts;
}
