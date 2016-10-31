
#include <cstdio>
#include <iostream>
#include <stdint.h>

#include <tuple>

template <size_t N, typename T = char>
class BitSet {
public:
	BitSet()
		: size_(sizeof(T) * N * 8) 
	{
		for (size_t dIdx=0; dIdx < N; ++dIdx) {
			data_[dIdx] = 0;
		}
	}

	const size_t& size() const {
		return size_;
	}

	void debug_print() const {
		for (size_t bIdx=0; bIdx < size(); ++bIdx) {
			std::cout << get(bIdx) << "|";
		}
		std::cout << "\n";
	}

	bool get(size_t bIdx) const {
		if (bIdx >= size()) {
			throw std::runtime_error("Out of array");
		}
		const auto& address = get_address(bIdx);
		return data_[std::get<0>(address)] & std::get<1>(address);
	}

	void set(size_t bIdx) {
		if (bIdx >= size()) {
			throw std::runtime_error("Out of array");
		}
		const auto& address = get_address(bIdx);
		data_[std::get<0>(address)] |= std::get<1>(address);
	}

private:
	std::tuple<size_t, T> get_address(size_t bIdx) const {
		size_t denominator = 8 * sizeof(T);
		size_t data_id = bIdx / denominator;
		size_t byte_id = bIdx % denominator;
		T shift = 1;
		shift <<= byte_id;
		return std::make_tuple(data_id, shift);
	}

private:
	T data_[N];
	size_t size_;
};


int main(const int argc, const char** argv) {
	BitSet<24, char> bitset;

	bitset.set(14);
	std::cout << bitset.get(14) << "\n";
	bitset.debug_print();
	const size_t test_indices[] = {
		26, 155, 95, 9, 3, 183, 154, 130, 102, 103, 105, 19, 
		161, 190, 61, 162, 183, 11, 3, 88, 18, 155, 169, 73, 
		59, 58, 0, 105, 191, 140, 191, 91, 3, 4, 139, 176, 176, 
		180, 168, 16, 143, 96, 77, 38, 178, 189, 118, 109, 138, 
		69, 110, 102, 66, 85, 32, 190, 76, 66, 86, 40, 35, 104, 
		101, 89, 124, 27, 125, 46, 134, 96, 93, 161, 178, 83, 114, 
		128, 8, 55, 108, 167, 11, 184, 74, 164, 169, 101, 140, 31, 
		120, 167, 190, 85, 158, 118, 19, 63, 175, 155, 80, 58
	};
	
	for (const auto& test_idx: test_indices) {
		bitset.set(test_idx);
		bool ans = bitset.get(test_idx);
		if (!ans) {
			throw std::runtime_error("Failed test");
 		}
	}
	bitset.debug_print();

}

