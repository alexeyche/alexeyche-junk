#pragma once

#include <dnn/io/stream.h>
#include <dnn/util/pretty_print.h>

namespace dnn {




/*@GENERATE_PROTO@*/
struct TimeSeriesInfo : public Serializable<Protos::TimeSeriesInfo> {

	void serial_process() {
		begin() << "labels_ids: " 	   << labels_ids 	  << ", " \
		        << "unique_labels: "   << unique_labels   << ", " \
		        << "labels_timeline: " << labels_timeline << Self::end;
	}
	
	void addLabelAtPos(const string &lab, size_t pos) {
		size_t lab_id;
		auto ulab_ptr = std::find(unique_labels.begin(), unique_labels.end(), lab);
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

/*@GENERATE_PROTO@*/
struct TimeSeriesData : public Serializable<Protos::TimeSeriesData> {
	void serial_process() {
		begin() << "values: " << values << Self::end;
	}

	vector<double> values;
};

struct TimeSeriesInterface {
	getValueAtIndexDelegate getValueAt;
};

struct TimeSeries : public SerializableBase {
	const string name() const {
		return "TimeSeries";
	}
	typedef TimeSeriesInterface interface;

	TimeSeries() {}
	TimeSeries(const string &filename, const string &format) {
		readFromFile(filename, format);
	}

	
	void readFromFile(const string &filename, const string &format) {
		ifstream f(filename);
		if(!f.is_open()) {
			throw dnnException()<< "Can't open file " << filename << "\n";
		}
		if(format == "ucr-ts") {			
			string line;
			while (std::getline(f, line)) {
				string lab;
				convertUcrTimeSeriesLine(line, data.values, lab);
				info.addLabelAtPos(lab, data.values.size());
			}
		} else 
		if(format == "protobin") {
			SerializableBase* o = Stream(f, Stream::Binary).readObject();
			TimeSeries* inp_ts = dynamic_cast<TimeSeries*>(o);
			if(!inp_ts) {
				throw dnnException() << "Failed to cast " << o->name() << " to TimeSeries\n";
			}
			(*this) = *inp_ts;
			Factory::inst().deleteLast();
		} else {
			throw dnnException() << "TimeSeries: unknown format " << format << "\n";
		}
	}
	size_t length() {
		return data.values.size();
	}

	const double& getValueAt(const size_t &index) {		
		return data.values[index];	
	}
	
	template <typename T>
	void provideInterface(TimeSeriesInterface &i) {
		i.getValueAt = MakeDelegate(static_cast<T*>(this), &T::getValueAt);
	}
	
	static const double& getValueAtDefault(const size_t &index) {
		throw dnnException()<< "Calling inapropriate default function method\n";
	}
	static void provideDefaultInterface(TimeSeriesInterface &i) {
		i.getValueAt = &TimeSeries::getValueAtDefault;
	}

	void serial_process() {
		begin() << "info: " << info << ", " << "data: " << data << Self::end;
	}
	static void convertUcrTimeSeriesLine(const string &line, vector<double> &ts_data, string &lab) {
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

	TimeSeriesInfo info;
	TimeSeriesData data;
};

}