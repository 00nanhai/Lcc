
#ifndef __SLIB_JSON_H__
#define __SLIB_JSON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <stdint.h>

namespace slib
{

using namespace std;

class JValue 
{
public:
	enum TYPE
	{
		E_NULL = 0,
		E_INT,
		E_BOOL,
		E_FLOAT,
		E_ARRAY,
		E_OBJECT,
		E_STRING
	};

public:
	JValue(TYPE type = E_NULL);
	JValue(int val);
	JValue(bool val);
	JValue(double val);
	JValue(int64_t val);
	JValue(const char* val);
	JValue(const string& val);
	JValue(const JValue& other);
	~JValue();

public:

	int				asInt()		const;
	bool			asBool()	const;
	double			asFloat()	const;
	int64_t			asInt64()	const;
	string			asString()	const;
	const char*		asCString()	const;

	TYPE			type() const;
	size_t			size() const;
	void			clear();

	JValue&			at(int index);
	JValue&			at(size_t index);
	JValue&			at(const char* key);

	bool			has(int index) const;
	bool			has(size_t index) const;
	bool			has(const char* key) const;

	bool			erase(int index);
	bool			erase(size_t index);
	bool			erase(const char* key);
	bool			names(vector<string>& members) const;
	bool			append(JValue& jv);
	
	bool			isInt()			const;
	bool			isNull()		const;
	bool			isBool()		const;
	bool			isFloat()		const;
	bool			isArray()		const;
	bool			isObject()		const;
	bool			isString()		const;

	operator		int()			const;
	operator		bool()			const;
	operator		double()		const;
	operator		int64_t()		const;
	operator		string()		const;
	operator		const char*()	const;

	JValue&			operator=(const JValue &other);

	JValue&			operator[](int index);
	const JValue&	operator[](int index) const;

	JValue&			operator[](size_t index);
	const JValue&	operator[](size_t index) const;

	JValue&			operator[](const char* key);
	const JValue&	operator[](const char* key) const;

	JValue&			operator[](const string& key);
	const JValue&	operator[](const string& key) const;

	friend bool	operator==(const JValue& jv, const char* psz) 
	{ 
		return (0 == strcmp(jv.asCString(), psz));
	}

	friend bool	operator==(const char* psz, const JValue& jv) 
	{ 
		return (0 == strcmp(jv.asCString(), psz));
	}

	friend bool	operator!=(const JValue& jv, const char* psz)
	{
		return (0 != strcmp(jv.asCString(), psz));
	}

	friend bool	operator!=(const char* psz, const JValue& jv) 
	{ 
		return (0 != strcmp(jv.asCString(), psz));
	}

private:
	void			Free();
	char*			NewString(const char* cstr);
	void			CopyValue(const JValue& src);

public:
	static const JValue	null;

private:
	union HOLD
	{
		bool					vBool;	
		double					vFloat;
		int64_t					vInt64;
		char*					vString;	
		vector<JValue>*			vArray;
		map<string, JValue>*	vObject;

	} m_Value;

	TYPE m_eType;

public:
	string			write() const;
	const char*		write(string& strDoc) const;

	string			swrite() const;
	const char*		swrite(string& strDoc) const;

	bool			read(const char* pdoc, string* pstrerr = NULL);
	bool			read(const string& strdoc, string* pstrerr = NULL);
};

class JReader
{
public:
	bool	parse(const char* pdoc, JValue &root);
	void	error(string& strmsg) const;

private:
	struct Token
	{
		enum TYPE
		{
			E_Error = 0,
			E_End,
			E_Null,
			E_True,
			E_False,
			E_Number,
			E_String,
			E_ArrayBegin,
			E_ArrayEnd,
			E_ObjectBegin,
			E_ObjectEnd,
			E_ArraySeparator,
			E_MemberSeparator			
		};
		TYPE			type;
		const char*		pbeg;
		const char*		pend;
	};

	void	skipSpaces();
	void	skipComment();

	bool	match(const char* pattern, int patternLength);

	bool	readToken(Token &token);
	bool	readValue(JValue& jval);
	bool	readArray(JValue& jval);
	void	readNumber();

	bool	readString();
	bool	readObject(JValue& jval);

	bool	decodeNumber(Token &token, JValue& jval);
	bool	decodeString(Token &token, string &decoded);
	bool	decodeDouble(Token &token, JValue& jval);

	char	GetNextChar();
	bool	addError(const string &message, const char*  ploc);

private:
	const char*		m_pBeg;
	const char*		m_pEnd;
	const char*		m_pCur;
	const char*		m_pErr;
	string			m_strErr;
};

class JWriter
{
public:
	static	void	FastWrite(const JValue& jval, string& strDoc);
	static	void	FastWriteValue(const JValue& jval, string& strDoc);

public:
	const string& StyleWrite(const JValue& jval);

private:
	void	PushValue( const string& strval);
	void	StyleWriteValue(const JValue& jval);
	void	StyleWriteArrayValue(const JValue& jval);
	bool	isMultineArray(const JValue& jval);

private:
	static	string	v2s(double val);
	static	string	v2s(int64_t val);
	static	string	v2s(const char* val);

private:
	string			m_strDoc;
	string			m_strTab;
	bool			m_bAddChild;
	vector<string>	m_childValues;
};

} // namespace slib

#endif // #ifndef __SLIB_JSON_H__
