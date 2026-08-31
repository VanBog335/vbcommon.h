// vbcommon.h - by https://github.com/VanBog335. v0.2.0

#ifndef VBCOMMON_H
#define VBCOMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

typedef char s8;
typedef unsigned char u8;
typedef int16_t s16;
typedef uint16_t u16;
typedef int32_t s32;
typedef uint32_t u32;
typedef int64_t s64;
typedef uint64_t u64;
typedef void u0;
typedef bool u1;
typedef float f32;
typedef double f64;
typedef s32 fp32;
// i say no to fp64

#define FIXEDBITS 14
#define FIXEDONE (1<<FIXEDBITS)

#ifndef M_PI
	#define M_PI 3.14159265358979323846f
#endif

#define M_DEG2RAD (M_PI / 180.0f)
#define M_RAD2DEG (180.0f / M_PI)

#define da_append(xs, x) \
	do { \
		if ((xs)->count >= (xs)->cap) { \
			if ((xs)->cap == 0) (xs)->cap = 256; \
			else (xs)->cap <<= 1; \
			(xs)->items = realloc((xs)->items, (xs)->cap*sizeof(*(xs)->items)); \
		} \
		(xs)->items[(xs)->count++] = (x); \
	} while (0)
/*
typedef struct {
	yourType_t *items;
	size_t count;
	size_t cap;
} da_items_t;
*/

u0 vb_setSeed(uint64_t seed);
u8 vb_rand8();
u16 vb_rand16();
u32 vb_rand32();
u64 vb_rand64();
f64 vb_randfloat();
f32 vb_invSqrt(f32 x);
fp32 vb_fpSqrt(fp32 x);
f32 vb_sin(f32 angle);
f32 vb_cos(f32 angle);
f32 vb_lerp(f32 a, f32 b, f32 t);
fp32 vb_fpMul(fp32 a, fp32 b);
fp32 vb_fpDiv(fp32 a, fp32 b);
fp32 vb_double2fixed(f64 a);
f64 vb_fixed2double(fp32 a);
s8 *vb_fixed2str(s8 *buf, fp32 num, u8 decimals);
u32 vb_hash32(u0 *data, u64 len);
u64 vb_hash64(u0 *data, u64 len);

#endif // VBCOMMON_H


#ifdef VBCOMMON_IMPLEMENTATION
#ifndef VBCOMMON_IMPLEMENTATION_ONCE
#define VBCOMMON_IMPLEMENTATION_ONCE
// -------- Random -------- //
static u8  mseed8;
static u16 mseed16;
static u32 mseed32;
static u64 mseed64;

u0 vb_setSeed(uint64_t seed)
{
	mseed8 = seed;
	mseed16 = seed;
	mseed32 = seed;
	mseed64 = seed;
}

u8 vb_rand8()
{
	mseed8 ^= mseed8 << 3;
	mseed8 ^= mseed8 >> 5;
	mseed8 ^= mseed8 << 4;

	return mseed8;
}

u16 vb_rand16()
{
	mseed16 ^= mseed16 << 7;
	mseed16 ^= mseed16 >> 9;
	mseed16 ^= mseed16 << 8;

	return mseed16;
}

u32 vb_rand32()
{
	mseed32 ^= mseed32 << 14;
	mseed32 ^= mseed32 >> 13;
	mseed32 ^= mseed32 << 15;

	return mseed32;
}

u64 vb_rand64()
{
	mseed64 ^= mseed64 << 7;
	mseed64 ^= mseed64 >> 9;

	return mseed64;
}

f64 vb_randfloat()
{
	return (f64)vb_rand64() / UINT64_MAX;
}

// -------- End random -------- //
// -------- Math -------- //

// -------- Function from http://rrrola.wz.cz/inv_sqrt.html -------- //
f32 vb_invSqrt(f32 x)
{
	union { f32 f; u32 u; } y = {x};
	y.u = 0x5F1FFFF9ul - (y.u >> 1);
	return 0.703952253f * y.f * (2.38924456f - x * y.f * y.f);
}

fp32 vb_fpSqrt(fp32 x)
{
	if (x < 0) return -1;
	if (!x) return 0;

	u32 base = x, y = 0;

	for (u8 i = 0; i < sizeof(fp32)*8; i++) {
		y += base;
		if ( ((u64)y * (u64)y)>>FIXEDBITS > x ) {
			y -= base;
		}
		base >>= 1;
		if (!base) break;
	}

	return y;
}

// -------- Source from: https://www.coranac.com/2009/07/sines -------- //
#define ISIN_QN	10
#define QA		12
#define ISIN_B	19900
#define ISIN_C	3516

static int isin_s4(int x)
{
	int c, x2, y;

	c  = x << (30 - ISIN_QN);
	x -= 1 << ISIN_QN;

	x <<= (31 - ISIN_QN);
	x >>= (31 - ISIN_QN);
	x  *= x;
	x >>= (2 * ISIN_QN - 14);

	y = ISIN_B - (x * ISIN_C >> 14);
	y = (1 << QA) - (x * y >> 16);

	return (c >= 0) ? y : (-y);
}

f32 vb_sin(f32 angle)
{
	int raw = (int)(angle * M_RAD2DEG * 4096 / 360);
	return isin_s4(raw) / 4096.0f;
}

f32 vb_cos(f32 angle)
{
	int raw = (int)(angle * M_RAD2DEG * 4096 / 360);
	raw += (1 << ISIN_QN);
	return isin_s4(raw) / 4096.0f;
}

#undef ISIN_QN
#undef QA
#undef ISIN_B
#undef ISIN_C
// -------- Source from site end -------- //

f32 vb_lerp(f32 a, f32 b, f32 t)
{
	return a + (b - a) * t;
}
// -------- End math -------- //
// -------- Fixed point -------- //
fp32 vb_fpMul(fp32 a, fp32 b)
{
	return ((s64)a * (s64)b)>>FIXEDBITS;
}

fp32 vb_fpDiv(fp32 a, fp32 b)
{
	return ((s64)a<<FIXEDBITS) / (s64)b;
}

fp32 vb_double2fixed(f64 a)
{
	return a * FIXEDONE;
}

f64 vb_fixed2double(fp32 a)
{
	return a / (f64)FIXEDONE;
}

static const u32 scales[8] = {
	1, 10, 100, 1000, 10000, 100000, 1000000, 10000000
};

s8 *vb_fixed2str(s8 *buf, fp32 num, u8 decimals)
{
	s8 *oldbuf = buf;
	s8 numOfDecimals = decimals & 7;
	s32 scale = scales[numOfDecimals];

	u32 unum = (num >= 0) ? num : -num;
	if (num < 0)
		*buf++ = '-';

	u64 intpart = unum >> FIXEDBITS;
	u64 fracpart = unum & (FIXEDONE-1);
	fracpart = (fracpart*scale)>>FIXEDBITS;

	buf += sprintf(buf, "%u", intpart);

	if (scale != 1) {
		*buf++ = '.';
		sprintf(buf, "%0*u\0", numOfDecimals, fracpart);
	}

	return oldbuf;
}
// -------- End fixed point -------- //
// -------- Other -------- //

// fnv1a-32
u32 vb_hash32(u0 *data, u64 len)
{
	u8 *ptr = (u8 *)data;
	u32 hash = 0x811C9DC5;

	for (u64 i = 0; i < len; i++) {
		hash ^= ptr[i];
		hash *= 0x01000193;
	}

	return hash;
}

// fnv1a-64
u64 vb_hash64(u0 *data, u64 len)
{
	u8 *ptr = (u8 *)data;
	u64 hash = 0xCBF29CE484222325ULL;

	for (u64 i = 0; i < len; i++) {
		hash ^= ptr[i];
		hash *= 0x100000001B3ULL;
	}

	return hash;
}

#endif // VBCOMMON_IMPLEMENTATION_ONCE
#endif // VBCOMMON_IMPLEMENTATION
