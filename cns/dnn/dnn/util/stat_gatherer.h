#pragma once

#include <dnn/base/base.h>
#include <dnn/util/serial/proto_serial.h>
#include <dnn/util/ptr.h>
#include <dnn/protos/stat_gatherer.pb.h>

#include <limits>

namespace NDnn {

	struct TStatistics: public IProtoSerial<NDnnProto::TStatistics> { 
		TStatistics() {}

		TStatistics(const TString& name, ui32 from, ui32 to)
			: Name(name)
			, From(from)
			, To(to)
		{}

		void SerialProcess(TProtoSerial& serial) override {
            serial(Values);
            serial(From);
            serial(To);
            serial(Name);
        }

        TVector<double> Values;
        ui32 From;
        ui32 To;
        TString Name;
	};

	class TStatCollector {
	public:
		TStatCollector(const TString& name, std::function<double()> cb, ui32 from = 0, ui32 to = std::numeric_limits<ui32>::max())
			: Callback(cb)
			, Result(name, from, to) 
		{}
		
		TStatCollector(const TStatCollector& other) {
			(*this) = other;	
		}

		TStatCollector& operator=(const TStatCollector& other) {
			if (this != &other) {
				Result = other.Result;
				Callback = other.Callback;
			}
			return *this;
		}

		void Collect() {
			Result.Values.push_back(Callback());
		}
		
		const ui32& GetFrom() const {
			return Result.From;
		}

		const ui32& GetTo() const {
			return Result.To;
		}

		TStatistics& GetMutStatistics() {
			return Result;
		}

	private:
		TStatistics Result;
		std::function<double()> Callback;
	};

	class TStatGatherer {
	public:
		TStatGatherer();

		TStatGatherer(const TStatGatherer& other);

		TStatGatherer& operator=(const TStatGatherer& other);

		void ListenStat(const TString& name, std::function<double()> cb, ui32 from, ui32 to);

		void Collect(const TTime& t);

		void SaveStat(const TString& fname);

		void Init();

		ui32 Size() const;
	private:
		TVector<TPtr<TStatCollector>> ActiveStats;
		TVector<TStatCollector> Stats;
	};


} // namespace NDnn