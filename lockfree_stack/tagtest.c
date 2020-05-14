#include "tag_ptr.h"
#include <iostream>
#include <cassert>

int main()
{
	int tag = 23;
	int val = 92;
	int *ptr = &val;

	tag_ptr<int> tagged;
	tagged.set_ptr(ptr);
	tagged.set_tag(tag);

	assert(tagged.get_ptr() == ptr);
	assert(tagged.get_tag() == tag);
	return 0;
}
