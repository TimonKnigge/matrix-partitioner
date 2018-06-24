#include "./subset-sum-computer.h"

#include <algorithm>
#include <numeric>

namespace mp {

struct mask_and_sum {
	int mask, sum;
	bool operator<(const mask_and_sum &rhs) {
		return sum < rhs.sum;
	}
};

bool partition_mim(const std::vector<int> &a, int s1max, int s2max,
		std::vector<bool> &part) {
	int total = std::accumulate(a.begin(), a.end(), 0), n = (int)a.size();

	int lhsize = n/2, rhsize = (n+1)/2;
	std::vector<mask_and_sum> lhs, rhs;
	lhs.reserve(1<<lhsize);
	rhs.reserve(1<<rhsize);

	// Collect all left-hand side partitions.
	for (int m = 0; m < (1<<lhsize); ++m) {
		int sum = 0;
		for (int i = 0; i < lhsize; ++i)
			if ((m>>i)&1)
				sum += a[i];
		lhs.push_back(mask_and_sum{m, sum});
	}

	// Collect all right-hand side partitions.
	for (int m = 0; m < (1<<rhsize); ++m) {
		int sum = 0;
		for (int i = 0; i < rhsize; ++i)
			if ((m>>i)&1)
				sum += a[lhsize + i];
		rhs.push_back(mask_and_sum{m, sum});
	}

	// Order the partitions of each side and try to find a match.
	std::sort(lhs.begin(), lhs.end());
	std::sort(rhs.rbegin(), rhs.rend());
	size_t li = 0, ri = 0;
	while (li < lhs.size() && ri < rhs.size()) {
		int s1sum = lhs[li].sum + rhs[ri].sum;
		int s2sum = total - s1sum;	

		// If the partition is balanced we return, otherwise
		// readjust the partition that isn't the right size.
		if (s1sum > s1max)
			ri += 1;
		else if (s2sum > s2max)
			li += 1;
		else {
			// Save the partition and return it.
			int fullmask = (lhs[li].mask|(rhs[ri].mask<<lhsize));
			for (int i = 0; i < n; ++i)
				part[i] = (bool)((fullmask>>i)&1);
			return true;
		}
	}

	// No partition was found.
	return false;
}

enum dp_status { impossible = 0, take = 1, leave = 2 };
bool partition_dp(const std::vector<int> &a, int s1max, int s2max,
		std::vector<bool> &part) {
	int total = std::accumulate(a.begin(), a.end(), 0), n = (int)a.size();
	bool swap = (s1max > s2max);
	if (swap) std::swap(s1max, s2max);
	std::vector<dp_status> dp((s1max + 1) * n, dp_status::impossible);

	// Initialize by taking/leaving the first item.
	dp[0] = dp_status::leave;
	dp[a[0]] = dp_status::take;

	// Add items 1..n-1 one by one.
	for (int i = 1; i < n; ++i) {
		for (int size = 0; size <= s1max; ++size) {
			if (dp[(i - 1) * (s1max + 1) + size] == dp_status::impossible)
				continue;
			// Leave i.
			dp[i * (s1max + 1) + size] = dp_status::leave;
			// Take i, if there is space.
			if (size + a[i] <= s1max)
				dp[i * (s1max + 1) + size + a[i]] = dp_status::take;
		}
	}

	// Try to find a partition, if there is one.
	for (int size = s1max; size >= 0; --size) {
		// If the second side of the partition is growing too large we
		// can just break.
		if (total - size > s2max) break;

		if (dp[(n - 1) * (s1max + 1) + size] == dp_status::impossible)
			continue;

		// Collect the partition and return it.
		for (int i = n - 1; i >= 0; --i) {
			if (dp[i * (s1max + 1) + size] == dp_status::take) {
				part[i] = !swap;
				size -= a[i];
			} else
				part[i] = swap;
		}
		return true;
	}

	// No partition found.
	return false;
}

bool partition(const std::vector<int> &a, int s1max, int s2max,
		std::vector<bool> &part) {
	int sum = std::accumulate(a.begin(), a.end(), 0);
	long long mim_ops = (1LL << ((long long)(a.size() + 1LL) / 2LL));
	long long dp_ops = (long long)(sum) * (long long)(a.size());

	if (mim_ops < dp_ops && mim_ops < OPERATIONS_BOUND)
		return partition_mim(a, s1max, s2max, part);
	if (dp_ops < mim_ops && dp_ops < OPERATIONS_BOUND)
		return partition_dp(a, s1max, s2max, part);
	return false;
}

}
