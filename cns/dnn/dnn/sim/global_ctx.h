#pragma once

#include "reward_control.h"

#include <dnn/base/base.h>

#include <dnn/util/ptr.h>

namespace NDnn {

	class TGlobalCtx {
	public:
		static TGlobalCtx& Inst();

		void Init(TRewardControl& rewardControl) {
			RewardControl.Set(rewardControl);
		}
		
		const double& GetReward() const {
			return RewardControl->GetReward();
		}
		
		double GetRewardDelta() const {
			return RewardControl->GetRewardDelta();
		}
		
		void PropagateReward(double r) {
			assert(RewardControl.IsSet());
			RewardControl->GatherReward(r);
		}
	private:
		TPtr<TRewardControl> RewardControl;
	};



} // namespace NDnn