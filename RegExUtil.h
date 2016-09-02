#ifndef __REGEXT_UTIL_H__
#define __REGEXT_UTIL_H__

#include <string>
#include "deelx.h"

/**
 * ����������ʽ�ж��ִ��Ƿ�Ϸ�
 *
 * colVal: ��ֵ
 * regExp: ������ʽ
 * isTrue: �Ƿ�У��ͨ��
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
