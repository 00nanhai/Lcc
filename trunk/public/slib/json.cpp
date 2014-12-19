#include "json.h"

namespace slib
{

const JValue JValue::null;

JValue::JValue(TYPE type) : m_eType(type)
{
	m_Value.vFloat = 0;
}

JValue::JValue(int val) : m_eType(E_INT)
{
	m_Value.vInt64 = val;
}

JValue::JValue(int64_t val) : m_eType(E_INT)
{
	m_Value.vInt64 = val;
}

JValue::JValue(bool val) : m_eType(E_BOOL)
{
	m_Value.vBool = val;
}

JValue::JValue(double val) : m_eType(E_FLOAT)
{
	m_Value.vFloat = val;
}

JValue::JValue(const char* val) : m_eType(E_STRING)
{
	m_Value.vString = NewString(val);
}

JValue::JValue(const string& val) : m_eType(E_STRING)
{
	m_Value.vString = NewString(val.c_str());
}

JValue::JValue(const JValue &other)
{
	CopyValue(other);
}

JValue::~JValue()
{
	Free();
}

void JValue::clear()
{
	Free();
}

bool JValue::isInt() const
{
	return (E_INT == m_eType);
}

bool JValue::isNull() const
{
	return (E_NULL == m_eType);
}

bool JValue::isBool() const
{
	return (E_BOOL == m_eType);
}

bool JValue::isFloat() const
{
	return (E_FLOAT == m_eType);
}

bool JValue::isString() const
{
	return (E_STRING == m_eType);
}

bool JValue::isArray() const
{
	return (E_ARRAY == m_eType);
}

bool JValue::isObject() const
{
	return (E_OBJECT == m_eType);
}

JValue::operator const char*() const
{
	return asCString();
}

JValue::operator int() const
{
	return asInt();
}

JValue::operator int64_t() const
{
	return asInt64();
}

JValue::operator double() const
{
	return asFloat();
}

JValue::operator string() const
{
	return asCString();
}

JValue::operator bool() const
{
	return asBool();
}

char* JValue::NewString(const char* cstr)
{
	char* str = NULL;
	if(NULL != cstr)
	{
		size_t len = (strlen(cstr) + 1)*sizeof(char);
		str = (char*)malloc(len);
		memcpy(str, cstr, len);
	}
	return str;
}

void JValue::CopyValue( const JValue& src )
{
	m_eType = src.m_eType;
	switch (m_eType)
	{
	case E_ARRAY:
		m_Value.vArray  = (NULL == src.m_Value.vArray)  ? NULL : new vector<JValue>(*(src.m_Value.vArray));
		break;
	case E_OBJECT:
		m_Value.vObject = (NULL == src.m_Value.vObject) ? NULL : new map<string, JValue>(*(src.m_Value.vObject));
		break;
	case E_STRING:
		m_Value.vString = (NULL == src.m_Value.vString) ? NULL : NewString(src.m_Value.vString);
		break;
	default:
		m_Value = src.m_Value;
		break;
	}
}

void JValue::Free()
{
	switch (m_eType)
	{
	case E_INT:
		{
			m_Value.vInt64 = 0;
		}
		break;
	case E_BOOL:
		{
			m_Value.vBool = false;
		}
		break;
	case E_FLOAT:
		{
			m_Value.vFloat = 0.0;
		}
		break;
	case E_STRING:
		{
			if(NULL != m_Value.vString)
			{
				free(m_Value.vString);
				m_Value.vString = NULL;
			}
		}
		break;
	case E_ARRAY:
		{
			if(NULL != m_Value.vArray)
			{
				delete m_Value.vArray;
				m_Value.vArray = NULL;
			}
		}
		break;
	case E_OBJECT:
		{
			if(NULL != m_Value.vObject)
			{
				delete m_Value.vObject;
				m_Value.vObject = NULL;
			}
		}
		break;
	default:
		break;
	}
	m_eType = E_NULL;
}

JValue& JValue::operator = (const JValue &other)
{
	if(this != &other)
	{
		Free();
		CopyValue(other);
	}
	return (*this);
}

JValue::TYPE JValue::type() const
{
	return m_eType;
}

int JValue::asInt() const
{
	return (int)asInt64();
}

int64_t	JValue::asInt64() const
{
	switch (m_eType)
	{
	case E_INT:
		return m_Value.vInt64;
		break;
	case E_BOOL:
		return m_Value.vBool ? 1 : 0;
		break;
	case E_FLOAT:
		return int(m_Value.vFloat);
		break;
	case E_STRING:
		return (int64_t)atoll(asCString());
		break;
	default:
		break;
	}
	return 0;
}

double JValue::asFloat() const
{
	switch (m_eType)
	{
	case E_INT:
		return double(m_Value.vInt64);
		break;
	case E_BOOL:
		return m_Value.vBool ? 1.0 : 0.0;
		break;
	case E_FLOAT:
		return m_Value.vFloat;
		break;
	case E_STRING:
		return atof(asCString());
		break;
	default:
		break;
	}
	return 0.0;
}

bool JValue::asBool() const
{
	switch (m_eType)
	{
	case E_BOOL:
		return m_Value.vBool;
		break;
	case E_INT:
		return (0 != m_Value.vInt64);
		break;
	case E_FLOAT:
		return (0.0 != m_Value.vFloat);
		break;
	case E_ARRAY:
		return (NULL == m_Value.vArray)  ? false : (m_Value.vArray->size() > 0);
		break;
	case E_OBJECT:
		return (NULL == m_Value.vObject) ? false : (m_Value.vObject->size() > 0);
		break;
	case E_STRING:
		return (NULL == m_Value.vString) ? false : (strlen(m_Value.vString) > 0);
		break;
	default:
		break;
	}
	return false;
}

string JValue::asString() const
{
	switch (m_eType)
	{
	case E_BOOL:
		return m_Value.vBool ? "true" : "false";
		break;
	case E_INT:
		{
			char buf[256];
			snprintf(buf, 256, "%lld", (long long int)m_Value.vInt64);
			return buf;
		}
		break;
	case E_FLOAT:
		{
			char buf[256];
			snprintf(buf, 256, "%lf", m_Value.vFloat);
			return buf;
		}
		break;
	case E_ARRAY:
		return "array";
		break;
	case E_OBJECT:
		return "object";
		break;
	case E_STRING:
		return (NULL == m_Value.vString) ? "" : m_Value.vString;
		break;
	default:
		break;
	}
	return "";
}

const char* JValue::asCString() const
{
	if(E_STRING == m_eType && NULL != m_Value.vString)
	{
		return m_Value.vString;
	}
	return "";
}

size_t JValue::size() const
{
	switch (m_eType)
	{
	case E_ARRAY:
		return (NULL == m_Value.vArray) ? 0 : m_Value.vArray->size();
		break;
	case E_OBJECT:
		return (NULL == m_Value.vObject) ? 0 : m_Value.vObject->size();
		break;
	default:
		break;
	}
	return 0;
}

JValue& JValue::operator[](int index)
{
	return (*this)[(size_t)(index < 0 ? 0 : index)];
}

const JValue& JValue::operator[](int index) const
{
	return (*this)[(size_t)(index < 0 ? 0 : index)];
}

JValue& JValue::operator[](size_t index)
{
	if(E_ARRAY != m_eType || NULL == m_Value.vArray)
	{
		Free();
		m_eType = E_ARRAY;
		m_Value.vArray = new vector<JValue>();
	}
	
	size_t sum = m_Value.vArray->size();
	if(sum <= index)
	{
		size_t fill = index - sum;
		for(size_t i = 0; i <= fill; i++)
		{
			m_Value.vArray->push_back(null);
		}
	}
	
	return m_Value.vArray->at(index);
}

const JValue& JValue::operator[](size_t index) const
{
	if(E_ARRAY == m_eType && NULL != m_Value.vArray)
	{
		if(index < m_Value.vArray->size())
		{
			return m_Value.vArray->at(index);
		}
	}
	return null;
}

JValue& JValue::operator[](const string& key)
{
	return (*this)[key.c_str()];
}

const JValue& JValue::operator[](const string& key) const
{
	return (*this)[key.c_str()];
}

JValue& JValue::operator[](const char* key)
{
	map<string, JValue>::iterator it;
	if(E_OBJECT != m_eType || NULL == m_Value.vObject)
	{
		Free();
		m_eType = E_OBJECT;
		m_Value.vObject = new map<string, JValue>();
	}
	else
	{
		it = m_Value.vObject->find(key);
		if(it != m_Value.vObject->end())
		{
			return it->second;
		}
	}
	it = m_Value.vObject->insert(m_Value.vObject->end(), make_pair(key, null));
	return it->second;
}

const JValue& JValue::operator[](const char* key) const
{
	if(E_OBJECT == m_eType && NULL != m_Value.vObject)
	{
		map<string, JValue>::const_iterator it = m_Value.vObject->find(key);
		if (it != m_Value.vObject->end())
		{
			return it->second;
		}
	}
	return null;
}

bool JValue::has( int index ) const
{
	if(index >= 0)
	{
		return has((size_t)index);
	}
	return false;
}

bool JValue::has( size_t index ) const
{
	if(E_ARRAY == m_eType && NULL != m_Value.vArray)
	{
		if(index < size())
		{
			return true;
		}
	}
	return false;
}

bool JValue::has( const char* key ) const
{
	if(E_OBJECT == m_eType && NULL != m_Value.vObject)
	{
		if(m_Value.vObject->end() != m_Value.vObject->find(key))
		{
			return true;
		}
	}
	return false;
}

JValue& JValue::at( int index )
{
	return (*this)[index];
}

JValue& JValue::at( size_t index )
{
	return (*this)[index];
}

JValue& JValue::at( const char* key )
{
	return (*this)[key];
}

bool JValue::erase( int index )
{
	if(index >= 0)
	{
		return erase((size_t)index);
	}
	return false;
}

bool JValue::erase( size_t index )
{
	if(E_ARRAY == m_eType && NULL != m_Value.vArray)
	{
		if(index < m_Value.vArray->size())
		{
			m_Value.vArray->erase(m_Value.vArray->begin() + index);
			return true;
		}
	}
	return false;
}

bool JValue::erase(const char* key)
{
	if(E_OBJECT == m_eType && NULL != m_Value.vObject)
	{
		if(m_Value.vObject->end() != m_Value.vObject->find(key))
		{
			m_Value.vObject->erase(key);
			return !has(key);
		}
	}
	return false;
}

bool JValue::names(vector<string>& members) const
{
	if(E_OBJECT == m_eType && NULL != m_Value.vObject)
	{
		members.reserve(m_Value.vObject->size());
		map<string, JValue>::iterator itbeg = m_Value.vObject->begin();
		map<string, JValue>::iterator itend = m_Value.vObject->end();
		for(; itbeg != itend; itbeg++) 
		{
			members.push_back((itbeg->first).c_str());
		}
		return true;
	}
	return false;
}

string JValue::write() const
{
	string strDoc;
	return write(strDoc);
}

const char* JValue::write( string& strDoc ) const
{
	strDoc.clear();
	JWriter::FastWrite((*this), strDoc);
	return strDoc.c_str();
}

bool JValue::read(const string& strdoc, string* pstrerr)
{
	return read(strdoc.c_str(), pstrerr);
}

bool JValue::read(const char* pdoc, string* pstrerr)
{
	JReader reader;
	bool bret = reader.parse(pdoc, *this);
	if(!bret)
	{
		if(NULL != pstrerr)
		{
			reader.error(*pstrerr);
		}
	}
	return bret;
}

bool JValue::append( JValue& jv )
{
	if((E_OBJECT == m_eType || E_NULL == m_eType) && E_OBJECT == jv.type())
	{
		vector<string> members;
		jv.names(members);
		for(size_t i = 0; i < members.size(); i++)
		{
			(*this)[members[i]] = jv[members[i]];
		}
		return true;
	}
	else if((E_ARRAY == m_eType || E_NULL == m_eType )&& E_ARRAY == jv.type())
	{
		size_t count = this->size();
		for(size_t i = 0; i < jv.size(); i++)
		{
			(*this)[count] = jv[i];
			count++;
		}
		return true;
	}

	return false;
}

std::string JValue::swrite() const
{
	string strDoc;
	return swrite(strDoc);
}

const char* JValue::swrite( string& strDoc ) const
{
	strDoc.clear();
	JWriter jw;
	strDoc = jw.StyleWrite(*this);
	return strDoc.c_str();
}

// Class Reader
// //////////////////////////////////////////////////////////////////
bool JReader::parse(const char* pdoc, JValue &root)
{
	m_pBeg = pdoc;
	m_pEnd = m_pBeg + strlen(pdoc);
	m_pCur = m_pBeg;
	m_pErr = m_pBeg;
	m_strErr = "null";
	return readValue(root);
}

bool JReader::readValue(JValue& jval)
{
	Token token;
	readToken(token);
	switch (token.type)
	{
	case Token::E_True:
		jval = true;
		break;
	case Token::E_False:
		jval = false;
		break;
	case Token::E_Null:
		jval = JValue();
		break;
	case Token::E_Number:
		return decodeNumber(token, jval);
		break;
	case Token::E_ArrayBegin:
		return readArray(jval);
		break;
	case Token::E_ObjectBegin:
		return readObject(jval);
		break;
	case Token::E_String:
		{
			string strval;
			bool bok = decodeString(token, strval);
			if(bok)
			{
				jval = strval.c_str();
			}
			return bok;
		}
		break;
	default:
		return addError("Syntax error: value, object or array expected.", token.pbeg);
		break;
	}
	return true;
}

bool JReader::readToken(Token &token)
{
	skipSpaces();
	token.pbeg = m_pCur;
	switch(GetNextChar())
	{
	case '{':
		token.type = Token::E_ObjectBegin;
		break;
	case '}':
		token.type = Token::E_ObjectEnd;
		break;
	case '[':
		token.type = Token::E_ArrayBegin;
		break;
	case ']':
		token.type = Token::E_ArrayEnd;
		break;
	case ',':
		token.type = Token::E_ArraySeparator;
		break;
	case ':':
		token.type = Token::E_MemberSeparator;
		break;
	case 0:
		token.type = Token::E_End;
		break;
	case '"':
		token.type = readString() ? Token::E_String : Token::E_Error;
		break;
	case '/':
	case '#':
	case ';':
		{
			skipComment();
			return readToken(token);
		}
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '-':
		{
			token.type = Token::E_Number;
			readNumber();
		}
		break;
	case 't':
		token.type = match("rue", 3) ? Token::E_True : Token::E_Error;
		break;
	case 'f':
		token.type = match("alse", 4) ? Token::E_False : Token::E_Error;
		break;
	case 'n':
		token.type = match("ull", 3) ? Token::E_Null : Token::E_Error;
		break;
	default:
		token.type = Token::E_Error;
		break;
	}
	token.pend = m_pCur;
	return true;
}

void JReader::skipSpaces()
{
	while(m_pCur != m_pEnd)
	{
		char c = *m_pCur;
		if(c == ' ' || c == '\t' || c == '\r' || c == '\n')
		{
			m_pCur++;
		}
		else
		{
			break;
		}
	}
}

bool JReader::match(const char* pattern, int patternLength)
{
	if(m_pEnd - m_pCur < patternLength)
	{
		return false;
	}
	int index = patternLength;
	while(index--)
	{
		if(m_pCur[index] != pattern[index])
		{
			return false;
		}
	}
	m_pCur += patternLength;
	return true;
}

void JReader::skipComment()
{
	char c = GetNextChar();
	if(c == '*')
	{
		while (m_pCur != m_pEnd)
		{
			char c = GetNextChar();
			if (c == '*'  &&  *m_pCur == '/')
			{
				break;
			}
		}
	}
	else
	{
		while (m_pCur != m_pEnd)
		{
			char c = GetNextChar();
			if ( c == '\r'  ||  c == '\n')
			{
				break;
			}
		}
	}
}

void JReader::readNumber()
{
	while (m_pCur != m_pEnd)
	{
		char c = *m_pCur;
		if((c >= '0' && c <= '9') || ( c == '.' || c == 'e' || c == 'E' || c == '+' || c == '-'))
		{
			++m_pCur;
		}
		else
		{
			break;
		}
	}
}

bool JReader::readString()
{
	char c = 0;
	while (m_pCur != m_pEnd)
	{
		c = GetNextChar();
		if ('\\' == c)
		{
			GetNextChar();
		}
		else if ('"' == c)
		{
			break;
		}
	}
	return ('"' == c);
}

bool JReader::readObject(JValue& jval)
{
	string name;
	Token tokenName;
	jval = JValue(JValue::E_OBJECT);
	while(readToken(tokenName))
	{
		if(Token::E_ObjectEnd == tokenName.type)
		{//empty
			return true;
		}

		if(Token::E_String != tokenName.type)
		{
			break;
		}

		if(!decodeString(tokenName, name))
		{
			return false;
		}

		Token colon;
		readToken(colon);
		if(Token::E_MemberSeparator != colon.type)
		{
			return addError("Missing ':' after object member name", colon.pbeg);
		}

		if(!readValue(jval[name.c_str()])) 
		{// error already set
			return false;
		}

		Token comma;
		readToken(comma);
		if(Token::E_ObjectEnd == comma.type)
		{
			return true;
		}

		if(Token::E_ArraySeparator != comma.type)
		{
			return addError("Missing ',' or '}' in object declaration", comma.pbeg);
		}
	}
	return addError("Missing '}' or object member name", tokenName.pbeg);
}


bool JReader::readArray(JValue& jval)
{
	jval = JValue(JValue::E_ARRAY);
	skipSpaces();
	if(']' == *m_pCur) // empty array
	{
		Token endArray;
		readToken(endArray);
		return true;
	}

	size_t index = 0;
	while(true)
	{
		if(!readValue(jval[index++]))
		{//error already set
			return false;
		}

		Token token;
		readToken(token);
		if(Token::E_ArrayEnd == token.type)
		{
			break;
		}
		if(Token::E_ArraySeparator != token.type)
		{
			return addError("Missing ',' or ']' in array declaration", token.pbeg);
		}
	}
	return true;
}

bool JReader::decodeNumber(Token &token, JValue& jval)
{
	int64_t val = 0;
	bool isNeg = false;
	const char* pcur = token.pbeg;
	if('-' == *pcur)
	{
		pcur++;
		isNeg = true;
	}
	for(const char* p = pcur; p != token.pend; p++)
	{
		char c = *p;
		if('.' == c || 'e' == c || 'E' == c)
		{
			return decodeDouble(token, jval);
		}
		else if(c < '0'  ||  c > '9')
		{
			return addError("'" + string(token.pbeg, token.pend) + "' is not a number.", token.pbeg);
		}
		else
		{
			val = val * 10 + (c - '0');
		}
	}
	jval = isNeg ? -val : val;
	return true;
}

bool JReader::decodeDouble(Token &token, JValue& jval)
{
	const size_t szbuf = 512;
	size_t len = size_t(token.pend - token.pbeg);
	if(len <= szbuf)
	{
		char buf[szbuf];
		memcpy(buf, token.pbeg, len);
		buf[len] = 0;
		double val = 0;
		if(1 == sscanf(buf, "%lf", &val))
		{
			jval = val;
			return true;
		} 
	}
	return addError("'" + string(token.pbeg, token.pend) + "' is too large or not a number.", token.pbeg);
}

bool JReader::decodeString(Token &token, string &strdec)
{
	strdec = "";
	const char* pcur = token.pbeg + 1;
	const char* pend = token.pend - 1;	
	strdec.reserve(size_t(token.pend - token.pbeg));
	while(pcur != pend)
	{
		char c = *pcur++;
		if('\\' == c)
		{
			if(pcur != pend)
			{
				char escape = *pcur++;
				switch(escape)
				{
				case '"':	strdec += '"';		break;
				case '\\':	strdec += '\\';		break;
				case 'b':	strdec += '\b';		break;
				case 'f':	strdec += '\f';		break;
				case 'n':	strdec += '\n';		break;
				case 'r':	strdec += '\r';		break;
				case 't':	strdec += '\t';		break;
				case '/':	strdec += '/';		break;
				case 'u':	strdec += "\\u";	break;
				default:
					return addError("Bad escape sequence in string", pcur);
					break;
				}
			}
			else
			{
				return addError("Empty escape sequence in string", pcur);
			}
		}
		else if('"' == c)
		{
			break;
		}
		else
		{
			strdec += c;
		}
	}
	return true;
}

bool JReader::addError( const string &message, const char* ploc)
{
	m_pErr	 = ploc;
	m_strErr = message;
	return false;
}

char JReader::GetNextChar()
{
	return (m_pCur == m_pEnd) ? 0 : *m_pCur++;
}

void JReader::error(string& strmsg) const
{
	strmsg = "";
	int row = 1;
	const char* pcur  = m_pBeg;
	const char* plast = m_pBeg;
	while(pcur < m_pErr && pcur <= m_pEnd)
	{
		char c = *pcur++;
		if(c == '\r' || c == '\n')
		{
			if(c == '\r' && *pcur == '\n')
			{
				pcur++;
			}
			row++;
			plast = pcur;
		}
	}
	char msg[64];
	snprintf(msg,64, "Error: Line %d, Column %d, ", row, int(m_pErr - plast) + 1);
	strmsg += msg + m_strErr + "\n";
}

// Class Writer
// //////////////////////////////////////////////////////////////////
void JWriter::FastWrite(const JValue& jval, string& strDoc)
{
	strDoc = "";
	FastWriteValue(jval, strDoc);
	strDoc += "\n";
}

void JWriter::FastWriteValue(const JValue& jval, string& strDoc)
{
	switch (jval.type())
	{
	case JValue::E_NULL:
		strDoc += "null";
		break;
	case JValue::E_INT:
		strDoc += v2s(jval.asInt64());
		break;
	case JValue::E_BOOL:
		strDoc += jval.asBool() ? "true" : "false";
		break;
	case JValue::E_FLOAT:
		strDoc += v2s(jval.asFloat());
		break;
	case JValue::E_STRING:
		strDoc += v2s(jval.asCString());
		break;
	case JValue::E_ARRAY:
		{
			strDoc += "[";
			size_t usize = jval.size();
			for(size_t i = 0; i < usize; i++)
			{
				strDoc += (i > 0) ? "," : "";
				FastWriteValue(jval[i], strDoc);
			}
			strDoc += "]";
		}
		break;
	case JValue::E_OBJECT:
		{
			strDoc += "{";
			vector<string> members;
			jval.names(members);
			size_t usize = members.size();
			for(size_t i = 0; i < usize; i++)
			{
				const string& name = members[i];
				strDoc += (i > 0) ? "," : "";
				strDoc += v2s(name.c_str()) + ":";
				FastWriteValue(jval[name.c_str()], strDoc);
			}
			strDoc += "}";
		}
		break;
	}
}

const string&  JWriter::StyleWrite(const JValue &jval)
{
	m_strDoc = "";
	m_strTab = "";
	m_bAddChild = false;
	StyleWriteValue(jval);
	m_strDoc += "\n";
	return m_strDoc;
}

void JWriter::StyleWriteValue(const JValue& jval)
{
	switch(jval.type())
	{
	case JValue::E_NULL:
		PushValue("null");
		break;
	case JValue::E_INT:
		PushValue(v2s(jval.asInt64()));
		break;
	case JValue::E_BOOL:
		PushValue(jval.asBool() ? "true" : "false");
		break;
	case JValue::E_FLOAT:
		PushValue(v2s(jval.asFloat()));
		break;
	case JValue::E_STRING:
		PushValue(v2s(jval.asCString()));
		break;
	case JValue::E_ARRAY:
		StyleWriteArrayValue(jval);
		break;
	case JValue::E_OBJECT:
		{
			vector<string> members;
			jval.names(members);
			if(!members.empty())
			{
				m_strDoc += '\n' + m_strTab + "{";
				m_strTab += '\t';
				size_t usize = members.size();
				for(size_t i = 0; i < usize; i++)
				{
					const string& name = members[i];
					m_strDoc += (i > 0) ? "," : "";
					m_strDoc += '\n' + m_strTab + v2s(name.c_str()) + " : ";
					StyleWriteValue(jval[name]);
				}
				m_strTab.resize(m_strTab.size() - 1);
				m_strDoc += '\n' + m_strTab + "}";
			}
			else
			{
				PushValue("{}");
			}
		}
		break;
	}
}

void JWriter::StyleWriteArrayValue(const JValue& jval)
{
	size_t usize = jval.size();
	if(usize > 0)
	{
		bool isArrayMultiLine = isMultineArray(jval);
		if(isArrayMultiLine)
		{
			m_strDoc += '\n' + m_strTab + "[";
			m_strTab += '\t';
			bool hasChildValue = !m_childValues.empty();
			for(size_t i = 0; i < usize; i++)
			{
				m_strDoc += (i > 0) ? "," : "";
				if(hasChildValue)
				{
					m_strDoc += '\n' + m_strTab + m_childValues[i];
				}
				else
				{
					m_strDoc += '\n' + m_strTab;
					StyleWriteValue(jval[i]);
				}
			}
			m_strTab.resize(m_strTab.size() - 1);
			m_strDoc += '\n' + m_strTab + "]";
		}
		else
		{
			m_strDoc += "[ ";
			for(size_t i =0; i < usize; ++i )
			{
				m_strDoc += (i > 0) ? ", " : "";
				m_strDoc += m_childValues[i];
			}
			m_strDoc += " ]";
		}
	}
	else
	{
		PushValue("[]");
	}
}

bool JWriter::isMultineArray( const JValue& jval )
{
	m_childValues.clear();
	size_t usize = jval.size();
	bool isMultiLine = (usize >= 25);
	if(!isMultiLine)
	{
		for(size_t i = 0; i < usize; i++)
		{
			if(jval[i].size() > 0)
			{
				isMultiLine = true;
				break;
			}
		}
	}
	if(!isMultiLine)
	{
		m_bAddChild = true;
		m_childValues.reserve(usize);
		size_t lineLength = 4 + (usize - 1)*2; // '[ ' + ', '*n + ' ]'
		for(size_t i = 0; i < usize; i++)
		{
			StyleWriteValue(jval[i]);
			lineLength += m_childValues[i].length();
		}
		m_bAddChild = false;
		isMultiLine = lineLength >= 75;
	}
	return isMultiLine;
}

void JWriter::PushValue(const string& strval)
{
	if(!m_bAddChild)
	{
		m_strDoc += strval;
	}
	else
	{
		m_childValues.push_back(strval);
	}
}

string JWriter::v2s(int64_t val)
{
	char buf[32];
	snprintf(buf,32, "%lld", (long long int)val);
	return buf;
}

string JWriter::v2s(double val)
{
	char buf[512];
	snprintf(buf,512, "%g", val);
	return buf;
}

string JWriter::v2s(const char* pstr)
{
	if(NULL != strpbrk(pstr, "\"\\\b\f\n\r\t") )
	{
		string ret;
		ret.reserve(strlen(pstr)*2 + 3);
		ret += "\"";
		for(const char* c = pstr; 0 != *c; c++)
		{
			switch(*c)
			{
			case '\\':
				{
					c++;
					if('u' == *c)
					{
						ret += "\\u";
					}
					else
					{
						ret += "\\\\";
						c--;
					}
				}
				break;
			case '\"':	ret += "\\\"";	break;
			case '\b':	ret += "\\b";	break;
			case '\f':	ret += "\\f";	break;
			case '\n':	ret += "\\n";	break;
			case '\r':	ret += "\\r";	break;
			case '\t':	ret += "\\t";	break;
			default:	ret += *c;		break;
			}
		}
		ret += "\"";
		return ret;
	}
	else
	{
		return string("\"") + pstr + "\"";
	}
}

} // namespace slib

