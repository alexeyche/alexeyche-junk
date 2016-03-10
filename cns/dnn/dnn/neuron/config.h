#pragma once


namespace NDnn {


	template <typename TSynapseType, typename TActivationFunctionType>
	struct TNeuronConfig {
		using TSynapse = TSynapseType;
		using TActivationFunction = TActivationFunctionType;
	};


} // namespace NDnn
