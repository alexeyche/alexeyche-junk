#pragma once

#include <dnn/io/serialize.h>

namespace dnn {

void convertUcrTimeSeriesLine(const string &line, vector<double> &ts_data, string &lab) {
   vector<string> els = split(line, ' ');
   assert(els.size() > 0);

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
}



/*@GENERATE_PROTO@*/
struct TimeSeriesInfo : public Serializable<Protos::TimeSeriesInfo> {

	void serial_process() {
		begin() << "labels_ids: " 	   << labels_ids 	  << ", " \
		        << "unique_labels: "   << unique_labels   << ", " \
		        << "labels_timeline: " << labels_timeline << Self::end;
	}
	
	void addLabelAtPos(const string &lab, size_t pos) {
		size_t lab_id;
		auto ulab_ptr = unique_labels.find(lab);
		if(ulab_ptr == unique_labels.end()) {
			unique_labels.push_back(lab);
			lab_id = unique_labels.size()-1;
		} else {
			lab_id = ulab_ptr - unique_labels.begin();
		}
		labels_ids.push_back(lab_id);
		labels_timeline.push_back(pos);
	}

	vector<size_t> labels_ids;
	vector<string> unique_labels;
	vector<size_t> labels_timeline;
};


struct TimeSeries : public SerializableBase {
	const string name() const {
		return "TimeSeries";
	}
	void readFromFile(const string &filename, const string &format) {
		if(format == "ucr-ts") {
			ifstream f(filename);
			string line;
			while (std::getline(f, line)) {
				string lab;
				convertUcrTimeSeriesLine(line, ts, lab);
				info.addLabelAtPos(lab, ts.size());
			}
		}
	}
	void serial_process() {
		begin() << "info: " << info << ", " << "ts: " << ts << Self::end;
	}

	TimeSeriesInfo info;
	vector<double> ts;
};

}