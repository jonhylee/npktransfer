#ifndef __REGEXT_UTIL_H__
#define __REGEXT_UTIL_H__

#include <string>
#include "deelx.h"

/**
 * 基于正则表达式判断字串是否合法
 *
 * colVal: 列值
 * regExp: 正则表达式
 * isTrue: 是否校验通过
 */
void ValidRegexp(const char* colVal, const char* regExp, bool &isTrue)
{
	if(!isTrue) return;
	CRegexpT<char> regexp(regExp);
	MatchResult result = regexp.MatchExact(colVal);
	if(result.IsMatched())
	{
		isTrue = true;
	}
	else
	{
		isTrue = false;
	}
}

#endif /* __REGEXT_UTIL_H__ */
