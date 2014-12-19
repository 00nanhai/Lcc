/*
 * XXTea 加密算法。
 * 
 * author: Soli
 * date  : 2012-03-06
 */
#include <arpa/inet.h>
#include <string.h>
#include "xxtea.h"

namespace slib
{

/*
 * 以组元为单位，每个组元32bit。这里用uint32_t来实现。
 *
 * v - 是要加密的组元的起始地址。
 * n - 是要加密的组元个数，正数是加密，负数是解密。
 * k - 是密钥的起始地址，长度为4个组元，即 4 x 32 = 128 bit。
 *
 * 加密的结果会直接写回到v中。
 *
 * */

#define DELTA 0x9e3779b9
//#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))
//加括号，消除歧义
#define MX ( ( ((z>>5) ^ (y<<2)) + ((y>>3) ^ (z<<4)) )   ^    ( (sum^y) + (key[(p&3)^e] ^ z) ) )

static void btea(uint32_t *v, int n, uint32_t const key[4])
{
	uint32_t y, z, sum;
	int p;
	unsigned rounds, e;
	if (n > 1) {          /* Coding Part */
		rounds = 6 + 52/n;
		sum = 0;
		z = v[n-1];
		do {
			sum += DELTA;
			e = (sum >> 2) & 3;
			for (p=0; p<n-1; p++) {
				y = v[p+1];
				z = v[p] += MX;
			}
			y = v[0];
			z = v[n-1] += MX;
		} while (--rounds);
	} else if (n < -1) {  /* Decoding Part */
		n = -n;
		rounds = 6 + 52/n;
		sum = rounds*DELTA;
		y = v[0];
		do {
			e = (sum >> 2) & 3;
			for (p=n-1; p>0; p--) {
				z = v[p-1];
				y = v[p] -= MX;
			}
			z = v[n-1];
			y = v[0] -= MX;
		} while ((sum -= DELTA) != 0);
	}
}

XXTea::XXTea()
{
}

XXTea::~XXTea()
{
}

void XXTea::setKey(const std::string &key)
{
	m_key = key;
}

bool XXTea::encrypt(const std::string &plain, std::string &secret)
{
	secret.assign(plain);

	// 4字节补齐。
	uint32_t align = 4 - secret.size() % 4;
	secret.append(align, '=');

	// 把补齐的字节数追加到数据尾部。
	align = htonl(align);
	secret.append((char*)&align, 4);

	return _doCrypt((char *)secret.data(), secret.size(), true);
}

bool XXTea::decrypt(const std::string &secret, std::string &plain)
{
	// 因为有4自己补齐，还有追加的字节数，所以合法的密文至少有8字节。
	if(secret.size() < 8 || secret.size() % 4 != 0)
		return false;

	plain.assign(secret);

	if(!_doCrypt((char*) plain.data(), plain.size(), false))
		return false;

	uint32_t align = 0;
	memcpy(&align, plain.data() + plain.size() - 4, 4);
	align = ntohl(align);

	if(align > 4)
		return false;

	plain.erase(plain.size() - 4 -align);

	return true;
}

bool XXTea::_doCrypt(char *data, size_t len, bool encrypt)
{
	uint32_t key[4];
	int n = (int)len;

	// v
	uint32_t *v = (uint32_t *)data;

	// n
	if(n % 4 != 0)
		return false;

	n = n / 4;
	if(!encrypt)
		n = -n;

	// key
	memset(key, 0, sizeof(key));
	memcpy(key, m_key.data(), sizeof(key) < m_key.size() ? sizeof(key) : m_key.size());

	// btea
	btea(v, n, key);
	return true;
}

} // namespace slib
 
