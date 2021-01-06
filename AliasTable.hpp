#include <queue>
#include <vector>

template <typename T, typename Compare>
using Heap = std::priority_queue<T, std::vector<T>, Compare>;

// Build an alias table over the given probability mass function to allow sampling from it in constant time.
// https://en.wikipedia.org/wiki/Alias_method
//   [in]  probabilities: Probability mass function (size = n)
//   [in]  n:             Number of values
//   [out] tableProbs:    Alias table probabilities (size = n)
//   [out] tableAliases:  Alias table aliases (size = n)
template <typename T = float, typename TIndex = unsigned int>
void BuildAliasTable(T const * probabilities, TIndex n, T * tableProbs, TIndex * tableAliases)
{
	// Vose's Alias Method
	// https://www.keithschwarz.com/darts-dice-coins/
	// Note: To generate a good alias table (i.e. w/ a smaller probability to perform an alias lookup at runtime),
	// we are using min and max heaps for the worklists to take from the max of the large worklist and give to the
	// min of the small worklist.

	const auto WorklistEntryLess    = [&](TIndex lhs, TIndex rhs) { return tableProbs[lhs] < tableProbs[rhs]; };
	const auto WorklistEntryGreater = [&](TIndex lhs, TIndex rhs) { return tableProbs[lhs] > tableProbs[rhs]; };

	using WorklistMinHeap = Heap<TIndex, decltype(WorklistEntryGreater)>;
	using WorklistMaxHeap = Heap<TIndex, decltype(WorklistEntryLess)>;

	WorklistMinHeap small(WorklistEntryGreater);
	WorklistMaxHeap large(WorklistEntryLess);

	for(TIndex i = 0; i < n; ++i)
	{
		tableAliases[i] = ~TIndex(0);
		tableProbs[i]   = probabilities[i] * n;
		
		if(tableProbs[i] < T(1))	small.push(i);
		else						large.push(i);
	}

	while(!small.empty() && !large.empty())
	{
		const TIndex s = small.top(); small.pop();
		const TIndex l = large.top(); large.pop();

		tableAliases[s] = l;
		tableProbs[l] = (tableProbs[l] + tableProbs[s]) - 1; // More numerically stable than: tableProbs[l] - (TValue(1) - tableProbs[s])

		if(tableProbs[l] < T(1))	small.push(l);
		else						large.push(l);
	}

	while(!small.empty())
	{
		tableProbs[small.top()] = T(1);
		small.pop();
	}

	while(!large.empty())
	{
		tableProbs[large.top()] = T(1);
		large.pop();
	}
}

// Sample the probability mass function represented by the alias table (built with BuildAliasTable)
//   [in] urand01:      Uniform random number in [0, 1)
//   [in] tableProbs:   Alias table probabilities (size = n)
//   [in] tableAliases: Alias table aliases (size = n)
//   [in] n:            Number of values
// Returns the sample index.
template <typename T = float, typename TIndex = unsigned int>
TIndex SampleAliasTable(T urand01, T const * tableProbs, TIndex const * tableAliases, TIndex n)
{
	const T x = urand01;

	const T nx = n * x;
	const TIndex i = static_cast<int>(nx) < n ? static_cast<int>(nx) : n - 1;
	const T y = nx - i;

	const T Ui = tableProbs[i];
	if(y < Ui)
	{
		return i;
	}
	else
	{
		return tableAliases[i];
	}

	// Alternative version: Marsaglia et. al. 'square histogram' method
	// Replace computation of y and test if(y < Ui) by
	//		const T Vi = (Ui + i) / n; 
	//		if(x < Vi)
}
