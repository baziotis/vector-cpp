#include <iostream>

#include "Vec.h"

using namespace std;

void helper(Vec<int> b) {
	for(Vec<int>::iterator it = b.begin(); it != b.end(); ++it) {
		*it += 1;
	}

	for(size_t i = 0; i != b.size(); ++i) {
		cout << b[i] << endl;
	}
	cout << endl;
}

// TODO(stefanos): More tests.
void test1(void) {
	Vec<int> a;

	for(int i = 0; i != 10; ++i) {
		a.push_back(i);
	}
	helper(a);

	for(Vec<int>::const_iterator it = a.begin(); it != a.end(); ++it) {
		cout << *it << endl;
	}
}

int main(void) {
	test1();
}
