/*
 * XXTea 加密算法。
 * 
 * author: Soli
 * date  : 2012-03-06
 */

#ifndef __SLIB_XXTEA_H__
#define __SLIB_XXTEA_H__

#include <string>
#include <stdint.h>

namespace slib
{

class XXTea
{
public:
	XXTea();
	virtual ~XXTea();

	void setKey(const std::string &key);
	bool encrypt(const std::string &plain, std::string &secret);
	bool decrypt(const std::string &secret, std::string &plain);

protected:
	virtual bool _doCrypt(char *data, size_t len, bool encrypt);

protected:
	std::string m_key;
};

} // namespace slib
#endif // #ifndef __SLIB_XXTEA_H__
