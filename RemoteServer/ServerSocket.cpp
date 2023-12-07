#include "pch.h"
#include "ServerSocket.h"

//单例模式 --类的定义
CServerSocket* CServerSocket::m_instance = NULL;
CServerSocket::CHelper CServerSocket::m_helper;
