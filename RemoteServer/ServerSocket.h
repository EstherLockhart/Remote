#pragma once

#include "pch.h"
#include "framework.h"

class CPacket {
public:
	WORD sHead;//包头 固定位FE FF
	DWORD nLength;//包长度(从命令到校验)
	WORD sCmd;//控制命令
	std::string strData;//包数据
	WORD sSum;//和校验
public:
	CPacket() : sHead(0), nLength(0), sCmd(0), sSum(0) {
	
	}
	CPacket(const CPacket& packet) {
		sHead = packet.sHead;
		nLength = packet.nLength;
		sCmd = packet.sCmd;
		strData = packet.strData;
		sSum = packet.sSum;
	}
	CPacket(const BYTE* pData, size_t& nSize) {
		size_t i = 0;
		for (i = 0; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize) {
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i);
		i += 4;
		if (nLength + i > nSize) {
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i);
		i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i);
		i += 2;

		WORD sum = 0;
		for (size_t i = 0; i < strData.size(); i++) {
			sum += (BYTE)strData[i] & 0xFF;
		}
		if (sum == sSum) {
			nSize = i;
			return;
		}
		nSize = 0;
	}
	~CPacket() {
		
	}
	CPacket& operator=(const CPacket& packet) {
		if (this != &packet) {
			sHead = packet.sHead;
			nLength = packet.nLength;
			sCmd = packet.sCmd;
			strData = packet.strData;
			sSum = packet.sSum;
		}
		return *this;
	}
};

//单例模式
class CServerSocket
{
//成员变量
private:
	static CServerSocket* m_instance;
	SOCKET m_server_socket;
	SOCKET m_client_socket;
	CPacket m_packet;
public:

//构造函数
private:
	CServerSocket() {
		m_client_socket = INVALID_SOCKET;
		if (false == InitSockEnv()) {
			MessageBox(NULL, _T("无法初始化套接字环境，请检查网络！"), _T("初始化错误！"), MB_OK | MB_ICONERROR);
			exit(0);
		}

		m_server_socket = socket(PF_INET, SOCK_STREAM, 0);
	}
	CServerSocket(const CServerSocket& server_socket) {
		m_server_socket = server_socket.m_server_socket;
		m_client_socket = server_socket.m_client_socket;
	}
	~CServerSocket() {
		closesocket(m_server_socket);
		WSACleanup();
	}

//重载
private:
	CServerSocket& operator=(const CServerSocket& server_socket) {
	
	}
public:

//成员函数
private:
	bool InitSockEnv() {
		WSADATA data;
		if (-1 == WSAStartup(MAKEWORD(1, 1), &data)) {
			return false;
		}
		return true;
	}
	
	static void ReleseInstance() {
		if (m_instance != NULL) {
			delete m_instance;
			m_instance = NULL;
		}
	}

public:
	static CServerSocket* InitInstance() {
		if (m_instance == NULL) {
			m_instance = new CServerSocket;
		}
		return m_instance;
	}
	bool InitSocket() {
		if (m_server_socket == -1)
			return false;

		sockaddr_in ser_addr;
		memset(&ser_addr, 0, sizeof(ser_addr));
		ser_addr.sin_family = AF_INET;
		ser_addr.sin_addr.S_un.S_addr = INADDR_ANY;
		ser_addr.sin_port = htons(6000);

		//bind
		if (-1 == bind(m_server_socket, (sockaddr*)&(ser_addr), sizeof(ser_addr)))
			return false;

		//listen
		if (-1 == listen(m_server_socket, 1))
			return false;

		return true;
	}
	bool AcceptClient() {
		sockaddr_in client_addr;
		int client_addr_len = sizeof(client_addr);
		m_client_socket = accept(m_server_socket, (sockaddr*)&client_addr, &client_addr_len);
		if (-1 == m_client_socket)
			return false;
		return true;
	}
	#define BUFFER_SIZE 4096
	int DealCommand() {
		if (m_client_socket == -1)
			return -1;
		//char buffer[1024] = {};
		char* buffer = new char[BUFFER_SIZE];
		size_t index = 0;
		while (1) {
			size_t len = recv(m_client_socket, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0) {
				return -1;
			}
			index += len;
			//TODO:处理
			m_packet = CPacket((BYTE*)buffer, index);
			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}
	bool Send(const char* pData, int nSize) {
		return send(m_client_socket, pData, nSize, 0) > 0;
	}

//成员类
private:
	class CHelper {
	public:
		CHelper() {
			CServerSocket::InitInstance();
		}
		~CHelper() {
			CServerSocket::ReleseInstance();
		}
	};
	static CHelper m_helper;
};

extern CServerSocket server;
