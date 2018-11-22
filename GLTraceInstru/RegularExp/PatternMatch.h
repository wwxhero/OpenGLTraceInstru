#pragma once
#ifndef _PATTERNMATCH_H
#define _PATTERNMATCH_H

#define A_LETTER(c)\
	(c > 'A'-1 && c <'Z'+1)\
	|| (c > 'a'-1 && c < 'z'+1)
#define A_NUMBER(c)\
	(c > '0'-1 && c < '9'+1)
#define LEFT_PARENTHESIS(c)\
	('(' == c)
#define RIGHT_PARENTHESIS(c)\
	(')' == c)
#define COMMA(c)\
	(',' == c)
#define SPACE(c)\
	(' ' == c)
#define TAB(c)\
	('\t' == c)
#define LINEBR(p)\
	((*p == '\n')\
	|| (*p == '\r' && *(p+1) == '\n'))


#define FIX_MATCH_CONSTRU(p)\
		p, sizeof(p)-1

class PatternMatch
{
public:
	virtual bool Match(const char*& p, const char* end)
	{
		return false;
	}
	virtual void Range(const char* r[2])
	{
		r[0] = m_range[0];
		r[1] = m_range[1];
	}
protected:
	const char* m_range[2];
};

class FixMatch : public PatternMatch
{
public:
	FixMatch(const char* pattern, unsigned int len)
	{
		m_pattern = (char*)malloc(len);
		strncpy(m_pattern, pattern, len);
		m_len = len;
	}
	~FixMatch()
	{
		free(m_pattern);
	}
	virtual bool Match(const char*& p, const char* end) override
	{
		m_range[0] = p;
		const char*& s = p;
		const char*  s_e = end;
		const char*  d_e = m_pattern + m_len;
		bool match = true;
		for ( const char*  d = m_pattern
			; d < d_e && s < s_e && match
			; d ++, s ++ )
			match = ((*d) == (*s));
		if (!match)
			s --;
		m_range[1] = s;
		return match;
	}

private:
	char* m_pattern;
	unsigned int m_len;
};

class NumberMatch : public PatternMatch
{
public:
	virtual bool Match(const char*& p, const char* end) override
	{
		bool match = false;
		if (p < end)
			match = A_NUMBER(*p);
		if (match)
			p ++;
		return match;
	}
private:

};

class BlankMatch : public PatternMatch
{
public:
	BlankMatch()
	{

	}
	virtual bool Match(const char*& p, const char* end) override
	{
		if (p < end)
		{
			m_range[0] = p;
			bool match = (SPACE(*p)
						|| TAB(*p));
			if (match)
				p ++;
			m_range[1] = p;
			return match;
		}
		else
			return false;
	}
};

template<class TMatch>
class Star : public PatternMatch
{
public:
	Star(TMatch* m) : m_pMatch(m)
	{
	}
	virtual bool Match(const char*& p, const char* end) override
	{
		m_range[0] = p;
		while(m_pMatch->Match(p, end));
		m_range[1] = p;
		return true;
	}
private:
	TMatch* m_pMatch;
};

typedef Star<BlankMatch> BlankMatchStar;

template<class TMatch>
class Plus : public PatternMatch
{
public:
	Plus(TMatch* m) : m_pMatch(m)
	{
	}
	virtual bool Match(const char*& p, const char* end) override
	{
		m_range[0] = p;
		int i = 0;
		while (m_pMatch->Match(p, end))
			i ++;
		m_range[1] = p;
		return i > 0;
	}
private:
	TMatch* m_pMatch;
};

typedef Plus<BlankMatch> BlankMatchPlus;

class Or : public PatternMatch
{
public:
	Or(PatternMatch** m, unsigned int n) : m_arr(m)
										, m_n(n)
	{
	}
	virtual bool Match(const char*& p, const char* end) override
	{
		bool match = false;
		for (unsigned int i = 0; i < m_n && !match; i ++)
		{
			match = (m_arr[i]->Match(p, end));
			if (match)
				m_arr[i]->Range(m_range);
		}
		return match;
	}
private:
	PatternMatch** m_arr;
	unsigned int m_n;
};

class And : public PatternMatch
{
public:
	And(PatternMatch** m, unsigned int n) : m_arr(m)
										, m_n(n)
	{
	}
	virtual bool Match(const char*& p, const char* end) override
	{
		bool match = true;
		m_range[0] = p;
		for (unsigned int i = 0; i < m_n && match; i ++)
		{
			match = (m_arr[i]->Match(p, end));
		}
		m_range[1] = p;
		return match;
	}
private:
	PatternMatch** m_arr;
	unsigned int m_n;
};

class LetterMatch : public PatternMatch
{
public:
	LetterMatch()
	{

	}
	virtual bool Match(const char*& p, const char* end) override
	{
		m_range[0] = p;
		bool matched = A_LETTER(*p);
		while(A_LETTER(*p)
		   && p < end)
			p ++;
		m_range[1] = p;
		return matched;
	}
};

class NameMatch : public PatternMatch
{
public:
	NameMatch()
	{

	}
	virtual bool Match(const char*& p, const char* end) override
	{
		m_range[0] = p;
		bool matched = (A_LETTER(*p)
					|| *p == '_');
		while( (A_LETTER(*p)
				|| *p == '_'
				|| A_NUMBER(*p))
		   && p < end)
			p ++;
		m_range[1] = p;
		return matched;
	}
};

class TypeMatch : public PatternMatch
{
public:
	TypeMatch()
	{

	}
	virtual bool Match(const char*& p, const char* end) override
	{
		m_range[0] = p;
		bool match = true;

		FixMatch m1(FIX_MATCH_CONSTRU("const"));
		BlankMatch m2;
		Plus<BlankMatch> m3(&m2);
		//m1 -> m2: const<blank>
		match = (!m1.Match(p, end) || m3.Match(p, end));


		NameMatch m4;
		match = (match
			  && m4.Match(p, end));

		if (match)
		{
			m_range[1] = p;
			BlankMatch m0;
			Star<BlankMatch> m1(&m0);
			FixMatch m2(FIX_MATCH_CONSTRU("*"));
			FixMatch m3(FIX_MATCH_CONSTRU("&"));
			PatternMatch* g[] = {&m2, &m3};
			Or m4(g, 2);
			Plus<Or> m5(&m4);
			Star<BlankMatch> m6(&m0);
			PatternMatch* g2[] = {&m1, &m5, &m6};
			And m_and(g2, 3);
			Plus<And> andPlus(&m_and);
			if (andPlus.Match(p, end))
				m_range[1] = p;
			else
				p = m_range[1];

		}
		return match;
	}
};

#endif