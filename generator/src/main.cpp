#include <cstdint>
#include "generator/GeneratorMinimax.hpp"

int main() {
	GeneratorMinimax<uint32_t, 1, 1> generator1;
	GeneratorMinimax<uint32_t, 2, 2> generator2;
	GeneratorMinimax<uint32_t, 3, 3> generator3;
	GeneratorMinimax<uint32_t, 4, 4> generator4;
	GeneratorMinimax<uint32_t, 5, 5> generator5;
	GeneratorMinimax<uint64_t, 6, 6> generator6;

	generator1.benchmark();
	generator2.benchmark();
	generator3.benchmark();
	generator4.benchmark();
	generator5.benchmark();
	generator6.benchmark();

	return 0;
}
