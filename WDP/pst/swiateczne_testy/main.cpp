#include <bits/stdc++.h>
#include <cassert>
#include <vector>

#include "../prev.h"
const long long mod = 1ll << 32;
int dehashuj(int klucz, int x)
{
	klucz = int((2137ll * klucz % mod * klucz % mod * klucz % mod + 420ll * klucz % mod * klucz % mod + 69ll * klucz % mod) % mod + (long long)INT_MIN);
	return x ^ klucz;
}

int main()
{

	bool zahashowane = 0;
	{
		int czy;
		assert(scanf("%d", &czy));
		zahashowane = czy;
	}
	std::vector<int> tab;
	int n, q, i, l, r;
	int ostatni_wynik = 0;
	assert(scanf("%d%d", &n, &q) == 2);
	for (i = 0; i < n; i++)
	{
		tab.push_back(0);
		assert(scanf("%d", &tab.back()) == 1);
		if (zahashowane)
			tab.back() = dehashuj(ostatni_wynik, tab.back());
	}
	init(tab);
	while (q--)
	{
		int rodzaj;
		assert(scanf("%d", &rodzaj) == 1);
		if (rodzaj == 0)
		{
			assert(scanf("%d", &i) == 1);
			if (zahashowane)
				i = dehashuj(ostatni_wynik, i);
			pushBack(i);
		}
		else
		{
			assert(scanf("%d%d%d", &i, &l, &r) == 3);
			if (zahashowane)
			{
				i = dehashuj(ostatni_wynik, i);
				l = dehashuj(ostatni_wynik, l);
				r = dehashuj(ostatni_wynik, r);
			}
			ostatni_wynik = prevInRange(i, l, r);
			printf("%d\n", ostatni_wynik);
		}
	}
