/*
 * Copyright (C) 2016 Marcelina Kościelnicka <mwk@0x04.net>
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef OLD_H
#define OLD_H

#include "hwtest.h"

struct hwtest_ctx {
	int cnum;
	bool colors;
	struct chipset_info chipset;
	unsigned short rand48[3];
};

struct hwtest_test {
	const char *name;
	int (*fun) (struct hwtest_ctx *ctx);
	const struct hwtest_group *group;
	int slow;
};

struct hwtest_group {
	const struct hwtest_test *tests;
	int testsnum;
	int (*prep) (struct hwtest_ctx *ctx);
};

namespace hwtest {
	class OldTest : public Test {
	private:
		int (*fun) (struct hwtest_ctx *);
	public:
		OldTest(TestOptions &opt, uint32_t seed, int (*fun) (struct hwtest_ctx *))
			: Test(opt, seed), fun(fun) {}
		int run() override {
			struct hwtest_ctx ctx;
			ctx.cnum = cnum;
			ctx.chipset = chipset;
			ctx.colors = opt.colors;
			ctx.rand48[0] = rnd();
			ctx.rand48[1] = rnd();
			ctx.rand48[2] = rnd();
			return fun(&ctx);
		}
	};

	class OldTestGroup : public Test {
	private:
		const struct hwtest_group *group;
	public:
		OldTestGroup(TestOptions &opt, uint32_t seed, const struct hwtest_group *group)
			: Test(opt, seed), group(group) {}
		int run() override {
			struct hwtest_ctx ctx;
			ctx.cnum = cnum;
			ctx.chipset = chipset;
			ctx.colors = opt.colors;
			ctx.rand48[0] = rnd();
			ctx.rand48[1] = rnd();
			ctx.rand48[2] = rnd();
			return group->prep(&ctx);
		}
		Subtests subtests() override {
			Subtests res;
			for (int i = 0; i < group->testsnum; i++) {
				auto &test = group->tests[i];
				res.push_back({
					test.name,
					test.group ? static_cast<Test*>(new OldTestGroup(opt, rnd(), test.group))
					: new OldTest(opt, rnd(), test.fun)
				});
			}
			return res;
		}
	};
}

#define HWTEST_TEST(a, slow) { #a, a, 0, slow }
#define HWTEST_GROUP(a) { #a, 0, &a##_group, 0 }
#define HWTEST_DEF_GROUP(a, ...) const struct hwtest_test a##_tests[] = { __VA_ARGS__ }; const struct hwtest_group a##_group = { a##_tests, ARRAY_SIZE(a##_tests), a##_prep };

#define TEST_BITSCAN(reg, all1, all0) do { \
	uint32_t _reg = reg; \
	uint32_t _all0 = all0; \
	uint32_t _all1 = all1; \
	uint32_t _tmp = nva_rd32(ctx->cnum, _reg); \
	nva_wr32(ctx->cnum, _reg, 0xffffffff); \
	uint32_t _rall1 = nva_rd32(ctx->cnum, _reg); \
	nva_wr32(ctx->cnum, _reg, 0); \
	uint32_t _rall0 = nva_rd32(ctx->cnum, _reg); \
	nva_wr32(ctx->cnum, _reg, _tmp); \
	if (_rall1 != _all1 || _rall0 != _all0) { \
		fprintf(stderr, "Bitscan mismatch for %06x: is %08x/%08x, expected %08x/%08x\n", _reg, _rall1, _rall0, _all1, _all0); \
		return HWTEST_RES_FAIL; \
	} \
} while (0)

#define TEST_READ(reg, exp, msg, ...) do { \
	uint32_t _reg = reg; \
	uint32_t _exp = exp; \
	uint32_t _real = nva_rd32(ctx->cnum, _reg); \
	if (_exp != _real) { \
		fprintf(stderr, "Read mismatch for %06x: is %08x, expected %08x - " msg "\n", _reg, _real, _exp, __VA_ARGS__); \
		return HWTEST_RES_FAIL; \
	} \
} while (0)

#define TEST_READ_MASK(reg, exp, mask, msg, ...) do { \
	uint32_t _reg = reg; \
	uint32_t _exp = exp; \
	uint32_t _mask = mask; \
	uint32_t _real = nva_rd32(ctx->cnum, _reg); \
	if (_exp != (_real & _mask)) { \
		fprintf(stderr, "Read mismatch for %06x: is %08x (& %08x), expected %08x - " msg "\n", _reg, _real, _mask, _exp, __VA_ARGS__); \
		return HWTEST_RES_FAIL; \
	} \
} while (0)

extern const struct hwtest_group nv10_tile_group;
extern const struct hwtest_group nv04_pgraph_group;
extern const struct hwtest_group nv50_ptherm_group;
extern const struct hwtest_group nv84_ptherm_group;
extern const struct hwtest_group pvcomp_isa_group;
extern const struct hwtest_group vp2_macro_group;
extern const struct hwtest_group mpeg_crypt_group;
extern const struct hwtest_group vp1_group;

#endif
