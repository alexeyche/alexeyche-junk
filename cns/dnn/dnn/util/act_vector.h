#pragma once

#include <dnn/base/base.h>

#include <unordered_set>
#include <iostream>

namespace NDnn {

	template<typename T>
	class TActVector : private TVector<T> {
	public:
		T& operator[] (const ui32 &i) {
			SetActive(i);
			return TVector<T>::operator[](i);
		}

		void SetActive(const ui32 &i) {
			if(ActIndices.find(i) == ActIndices.end()) {
				ActIndices.insert(i);
			}
		}

		T& operator[](const std::unordered_set<ui32>::iterator &i) {
			return TVector<T>::operator[](*i);
		}
		T& Get(const ui32 &i) {
			return TVector<T>::operator[](i);
		}
		const T& Get(const ui32 &i) const {
			return TVector<T>::operator[](i);
		}
		void push_back(const T &v) {
			TVector<T>::push_back(v);
		}
		typename TVector<T>::iterator begin() {
			return TVector<T>::begin();
		}
		typename TVector<T>::iterator end() {
			return TVector<T>::end();
		}

		typename TVector<T>::const_iterator begin() const {
			return TVector<T>::cbegin();
		}

		typename TVector<T>::const_iterator end() const {
			return TVector<T>::cend();
		}

		std::unordered_set<ui32>::iterator abegin() {
			return ActIndices.begin();
		}
		std::unordered_set<ui32>::iterator aend() {
			return ActIndices.end();
		}

		void SetInactive(std::unordered_set<ui32>::iterator &i) {
			i = ActIndices.erase(i);
		}

		ui32 size() const {
			return TVector<T>::size();
		}
		void resize(ui32 s) {
			TVector<T>::resize(s);
		}
		void emplace_back(T&& v) {
			TVector<T>::emplace_back(std::forward<T>(v));
		}

	    friend std::ostream& operator<<(std::ostream& str, const TActVector& self) {
	        for(ui32 i=0; i<self.size(); ++i) {
	        	std::cout << i << ":";
	        	if(self.ActIndices.find(i) == self.ActIndices.end()) {
	        		std::cout << "n";
	        	} else {
	        		std::cout << "a";
	        	}
	        	std::cout << ", ";
	        }
	    	std::cout << "\n";
	        return str;
	    }
	private:
		std::unordered_set<ui32> ActIndices;
	};


} // namespace NDnn