# Generator Benchmarks

#### Basic Minimax

Used:
- Minimax algorithm
- Bitboards
- Memoization

| Width | Height | Time (ms) | Tests |
|-------|--------|-----------|-------|
| 1     | 1      | 0         | 10    |
| 2     | 2      | 0         | 10    |
| 3     | 3      | 0         | 10    |
| 4     | 4      | 3         | 10    |
| 5     | 5      | 1073      | 10    |
| 6     | 6      | To slow   | 0     |

*For 6x6 expecting about 1/2h of compute*

New optimizations:
- memoization on tree array instead of hashmap

| Width | Height | Time (ms) | Tests |
|-------|--------|-----------|-------|
| 1     | 1      | 0         | 10    |
| 2     | 2      | 0         | 10    |
| 3     | 3      | 0         | 10    |
| 4     | 4      | 2         | 10    |
| 5     | 5      | 954       | 10    |
| 6     | 6      | To slow   | 0     |
