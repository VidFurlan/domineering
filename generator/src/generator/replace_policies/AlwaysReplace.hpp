#pragma once

struct AlwaysReplace {
	template<class State>
		bool operator()(const State& old, const State& neu) const {
			return true;
		}
};
