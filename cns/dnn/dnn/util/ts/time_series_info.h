#pragma once

#include <dnn/base/base.h>
#include <dnn/util/pretty_print.h>
#include <dnn/protos/time_series_info.pb.h>
#include <dnn/util/serial/proto_serial.h>
#include <dnn/util/maybe.h>

namespace NDnn {

    struct TLabelInfo: public IProtoSerial<NDnnProto::TLabelInfo> {
        TLabelInfo() {}

        TLabelInfo(TString name, ui32 duration)
            : Name(name)
            , Duration(duration)
        {}

        void SerialProcess(TProtoSerial& serial) override {
            serial(Name);
            serial(Duration);
        }

        bool operator == (const TLabelInfo& other) const {
            return (Name == other.Name) && (Duration == other.Duration);
        }

        TString Name;
        ui32 Duration;
    };

    struct TLabelPos: public IProtoSerial<NDnnProto::TLabelPos> {
        TLabelPos() {}

        TLabelPos(ui32 labelId, ui32 start)
            : LabelId(labelId)
            , Start(start)
        {}

        void SerialProcess(TProtoSerial& serial) override {
            serial(LabelId);
            serial(Start);
        }

        bool operator == (const TLabelPos& other) const {
            return (LabelId == other.LabelId) && (Start == other.Start);
        }


        ui32 LabelId;
        ui32 Start;
    };

    struct TTimeSeriesInfo: public IProtoSerial<NDnnProto::TTimeSeriesInfo> {

        void SerialProcess(TProtoSerial& serial) {
            serial(UniqueLabels);
            serial(LabelsStart);
            serial(Dt);
            serial(DimSize);
        }

        void AddLabelAtPos(const string &lab, ui32 pos, ui32 dur) {
            TMaybe<ui32> lab_id;
            auto ulabPtr = UniqueLabels.begin();
            for(; ulabPtr != UniqueLabels.end(); ++ulabPtr) {
                if(ulabPtr->Name == lab) {
                    ENSURE(ulabPtr->Duration == dur, "Trying to add label with same name but another duration");
                    lab_id = ulabPtr - UniqueLabels.begin();
                }
            }
            if(!lab_id) {
                UniqueLabels.push_back(TLabelInfo(lab, dur));
                lab_id = UniqueLabels.size()-1;
            }
            LabelsStart.push_back(TLabelPos(lab_id.GetRef(), pos));
        }

        void ChangeTimeDelta(double dt) {
            Dt = dt;
            for(auto &lt: UniqueLabels) {
                lt.Duration = ceil(lt.Duration/dt);
            }
            for(auto &lt: LabelsStart) {
                lt.Start = ceil(lt.Start/dt);
            }
        }

        bool operator == (const TTimeSeriesInfo &l) {
            if(LabelsStart != LabelsStart) return false;
            if(UniqueLabels != UniqueLabels) return false;
            return true;
        }
        bool operator != (const TTimeSeriesInfo &l) {
            return ! (*this == l);
        }

        TMaybe<ui32> GetClassIdFromPosition(double t) {
            if(
               (LabelsStart[__current_ahead_position-1].Start +
                UniqueLabels[ LabelsStart[__current_ahead_position-1].LabelId ].Duration)
               > t
            ) {
                return LabelsStart[__current_ahead_position-1].LabelId;
            } else {
                return Nothing<ui32>();
            }
        }

        TMaybe<ui32> GetClassId(double t) {
            while(__current_ahead_position <= LabelsStart.size()) {
                if(t <= LabelsStart[__current_ahead_position].Start) {
                    return GetClassIdFromPosition(t);
                }
                __current_ahead_position += 1;
            }
            return Nothing<ui32>();
        }

        double GetDuration() const {
            if (LabelsStart.size() == 0) {
                return 0.0;
            }
            const TLabelPos& last = LabelsStart.back();
            return last.Start + UniqueLabels.at(last.LabelId).Duration;
        }

        TVector<TLabelInfo> UniqueLabels;
        TVector<TLabelPos> LabelsStart;

        double Dt = 1.0;
        ui32 DimSize = 0;

        ui32 __current_ahead_position = 1;
    };


}