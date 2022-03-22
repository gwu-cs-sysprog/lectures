#pragma once

static inline unsigned long long
cycles(void)
{
	unsigned long a, d, c;

	/*
	 * Assembly language inlined into C! When you need to execute
	 * an instruction that you don't have access to from C! In
	 * this case, rdtsc reads out a "time stamp counter" -- the
	 * number of CPU cycles since the system booted up!
	 */
	__asm__ __volatile__("rdtsc" : "=a" (a), "=d" (d), "=c" (c) : : );

	return ((unsigned long long)d << 32) | (unsigned long long)a;
}
