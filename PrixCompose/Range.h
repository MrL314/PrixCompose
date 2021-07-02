


#ifndef RANGE_H
#define RANGE_H

#include "Utility.h"

struct Range
{
	u32 start;
	u32 end; 

	

	Range() { start = 0; end = 0; };
	Range(u32 s, u32 e) {
		start = s;
		end = e;
	};

	Range(const Range& r)
	{
		start = r.start;
		end = r.end;
	};

	Range& operator=(const Range& r)
	{
		if (this != &r)
		{
			start = r.start;
			end = r.end;
		}
		return *this;
	};

	~Range() {};




	u32 getLen() { return end - start; };
	u32 setLen(u32 l) { end = start + l; return l; };


	bool operator==(const Range& r) { 
		return (start == r.start) && (end == r.end);
	};
	bool operator!=(const Range& r) { return !(*this == r); };

	bool Equals(const Range& r) { return (this == &r); };
	bool Includes(u32 v) {
		return (start <= v) && (v <= end);
	};

	bool Empty() { return start == end; };


};


#endif


