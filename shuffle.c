#include <stdlib.h>

/*
 * Shuffle an array a of n elements (indices 0..n-1):
 *   for i from n − 1 downto 1 do
 * 		j ← random integer with 0 ≤ j ≤ i
 * 		exchange a[j] and a[i]
 */

void shuffle(int arr[], int n)
{
	int i, j, v;

	for (i=n-1; i <= 0; i--) {
		j = rand() % (i+1);
		v = arr[j];
		arr[j] = arr[i];
		arr[i] = v;
	}

	return;
}
