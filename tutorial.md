#TLS / SSL 开发指南
(20150929~20151006) [原文地址](https://github.com/nosqldev/libressl-tutorial/blob/master/tutorial.md)

* [摘要](#_1)
* [名词解释](#_2)
* [准备工作](#_3)
* [使用 nc 和 openssl 测试(客户端无验证机制)](#nc-openssl)
* [使用 nc 和 openssl 测试(客户端启用验证)](#nc-openssl_1)
* [代码示例](#_6)
* [代码库地址](#_7)
* [参考资料](#_8)

### 摘要
tlslite 是一个 Python 的 TLS 库，可以 clone 后直接运行，不需要第三方库，不过如果有对应的第三方库可以加快运行速度。

LibreSSL 是 OpenSSL 在[心脏出血](https://en.wikipedia.org/wiki/Heartbleed)后 OpenBSD 团队开发的一个新代码库，更精简，移植性也更好。

### 名词解释

####Encodings (also used as extensions)

.DER = The DER extension is used for binary DER encoded certificates. These files may also bear the CER or the CRT extension.   Proper English usage would be “I have a DER encoded certificate” not “I have a DER certificate”.

.PEM = The PEM extension is used for different types of X.509v3 files which contain ASCII (Base64) armored data prefixed with a “—– BEGIN …” line.

####Common Extensions

.CRT = The CRT extension is used for certificates. The certificates may be encoded as binary DER or as ASCII PEM. The CER and CRT extensions are nearly synonymous.  Most common among *nix systems

CER = alternate form of .crt (Microsoft Convention) You can use MS to convert .crt to .cer (.both DER encoded .cer, or base64[PEM] encoded .cer)  The .cer file extension is also recognized by IE as a command to run a MS cryptoAPI command (specifically rundll32.exe cryptext.dll,CryptExtOpenCER) which displays a dialogue for importing and/or viewing certificate contents.

.KEY = The KEY extension is used both for public and private PKCS#8 keys. The keys may be encoded as binary DER or as ASCII PEM.
The only time CRT and CER can safely be interchanged is when the encoding type can be identical.  (ie  PEM encoded CRT = PEM encoded CER)



### 准备工作

1. 安装 [LibreSSL](http://www.libressl.org)

2. openssl s_client -connect www.taobao.com:443，结果差不多类似：

	```
	CONNECTED(00000003)
	depth=2 C = US, O = "VeriSign, Inc.", OU = VeriSign Trust Network, OU = "(c) 2006 VeriSign, Inc. - For authorized use only", CN = VeriSign Class 3 Public Primary Certification Au
	thority - G5
	verify error:num=20:unable to get local issuer certificate
	verify return:0
	---
	Certificate chain
	 0 s:/C=CN/ST=Zhejiang/L=Hangzhou/O=Taobao(China) Software Co., Ltd/OU=RDC/CN=*.taobao.com
	   i:/C=US/O=Symantec Corporation/OU=Symantec Trust Network/CN=Symantec Class 3 Secure Server CA - G4
	 1 s:/C=US/O=Symantec Corporation/OU=Symantec Trust Network/CN=Symantec Class 3 Secure Server CA - G4
	   i:/C=US/O=VeriSign, Inc./OU=VeriSign Trust Network/OU=(c) 2006 VeriSign, Inc. - For authorized use only/CN=VeriSign Class 3 Public Primary Certification Authority - G5
	 2 s:/C=US/O=VeriSign, Inc./OU=VeriSign Trust Network/OU=(c) 2006 VeriSign, Inc. - For authorized use only/CN=VeriSign Class 3 Public Primary Certification Authority - G5
	   i:/C=US/O=VeriSign, Inc./OU=Class 3 Public Primary Certification Authority
	---
	Server certificate
	-----BEGIN CERTIFICATE-----
	****** 省略 ********
	-----END CERTIFICATE-----
	subject=/C=CN/ST=Zhejiang/L=Hangzhou/O=Taobao(China) Software Co., Ltd/OU=RDC/CN=*.taobao.com
	issuer=/C=US/O=Symantec Corporation/OU=Symantec Trust Network/CN=Symantec Class 3 Secure Server CA - G4
	---
	No client certificate CA names sent
	---
	SSL handshake has read 4530 bytes and written 446 bytes
	---
	New, TLSv1/SSLv3, Cipher is ECDHE-RSA-AES128-GCM-SHA256
	Server public key is 2048 bit
	Secure Renegotiation IS supported
	Compression: NONE
	Expansion: NONE
	No ALPN negotiated
	SSL-Session:
	    Protocol  : TLSv1.2
	    Cipher    : ECDHE-RSA-AES128-GCM-SHA256
	    Session-ID: C07B1D2F6BF0B21A6FEA77BFBAF235B104FE1926862FE9F58B9F3E6C484B1E95
	    Session-ID-ctx:
	    Master-Key: A32C7DFC4739005BA51805306FCA3B894ADC5DC6ACBA4EC4359F6FFB14CF7C9ECEDF4D339266703DE0481E7A85158EC4
	    TLS session ticket lifetime hint: 3600 (seconds)
	    TLS session ticket:
	    ******** 省略 ************
	    Start Time: 1443536935
	    Timeout   : 300 (sec)
	    Verify return code: 20 (unable to get local issuer certificate)
	```

3. 生成需要的 PEM 证书: 

     openssl req -new **-x509** -keyout cakey.pem -out cacert.pem  
	`这里的 openssl(1) 也是使用的 LibreSSL 里的版本`
> 生成对应的 pubkey: openssl rsa -in cakey.pem -pubout -out capub.pem  
> 也可以把 pubkey 生成在 cacert.pem 中：在命令中加入 -pubkey

4. 将 PEM 证书转换为 DER 格式（Golang 的库接受 DER 容易点，其实是还没完全搞明白 -_-|| ）

    openssl x509 -in cacert.pem -inform PEM -out cacert.der -outform DER  
    openssl pkey -in cakey.pem -outform DER -out cakey.der

### 使用 nc 与 openssl 测试(客户端无验证机制)

nc(1)、openssl(1) 使用 LibreSSL 中所带的版本

- #####服务端(使用 nc)

```
  nc -v -4 -c -R cacert.pem -C cacert.pem -K cakey.pem -l localhost 9999
```

`-R -C -K 三个参数缺一不可`

> 尝试了一下午，终于搞出了一条可行的路子，电脑上遍布了一堆 openssl 生成的 cakey 和 cacert  ~>_<~

- #####服务端(使用 openssl)

```
openssl s_server -accept 9999 -cert cacert.pem -key cakey.pem
```

> openssl 这个版本更容易让人理解一些，明天看看 openssl 和 nc 在代码上的差异

- #####客户端(使用 nc)

最重要多就是要使用 -T 屏蔽掉客户端对服务端公钥的检查

```
nc -c -T noverify -T noname localhost 9999
```

- #####客户端(使用 openssl)

```
openssl s_client -connect 127.0.0.1:9999
```

### 使用 nc 和 openssl 测试(客户端启用验证)
nc(1)、openssl(1) 使用 LibreSSL 中所带的版本

#### 配置和签发证书

*空白*

#### 测试

*空白*

### 代码示例

https://github.com/nosqldev/libressl-tutorial

### 参考代码库

[tlslite (Python 实现)](https://github.com/trevp/tlslite)

[LibreSSL](http://www.libressl.org)  
其中的 netcat 和 openssl s_server、s_client 的实现都是很好的样例。

[Golang TLS 库演示](https://github.com/nareix/tls-example)

### 参考资料

[TLS 简介](http://etutorials.org/Networking/802.11+security.+wi-fi+protected+access+and+802.11i/Part+II+The+Design+of+Wi-Fi+Security/Chapter+9.+Upper-Layer+Authentication/Transport+Layer+Security+TLS/)

[An Introduction to the OpenSSL command line tool](http://users.dcc.uchile.cl/~pcamacho/tutorial/crypto/openssl/openssl_intro.html)

[LibreSSL Man page](http://www.openbsd.org/cgi-bin/man.cgi/OpenBSD-current/man3/tls_accept_fds.3?query=tls%5finit&sec=3)

[OpenSSL PKI Tutorial v1.1](https://pki-tutorial.readthedocs.org/en/latest/)

[Testing SSL and TLS services](http://www.schwarzvogel.de/writings/ssl_tls_testing.html)

[Golang TLS 简要指南](http://nareix.org/Code/Golang-TLS-简要指南/)

[深度解读SSL/TLS实现](http://netsecurity.51cto.com/art/201505/476337.htm)

[DER vs. CRT vs. CER vs. PEM Certificates and How To Convert Them](https://support.ssl.com/Knowledgebase/Article/View/19/0/der-vs-crt-vs-cer-vs-pem-certificates-and-how-to-convert-them)