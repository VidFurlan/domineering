#pragma once

struct DepthReplace {
	template<class State>
		bool operator()(const State& old, const State& neu) const {
			return old.depth > neu.depth;
		}
};

