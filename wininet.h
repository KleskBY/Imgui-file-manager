#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <wininet.h>

struct WURL 
{
	std::string host, path;
	INTERNET_PORT port;
	DWORD dwFlags;
};

static  WURL parse_url(std::string url)
{
	WURL wurl = {};
	size_t index = url.find("://");
	if (index != std::string::npos)
	{
		const std::string& protocol = url.substr(0, index);
		if (protocol.compare("https") == 0)
		{
			wurl.dwFlags |= INTERNET_FLAG_SECURE;
			wurl.port = 443;
		}
		else
		{
			if ((wurl.dwFlags & INTERNET_FLAG_SECURE) == INTERNET_FLAG_SECURE) wurl.dwFlags ^= INTERNET_FLAG_SECURE;
			wurl.port = 80;
		}
		url = url.substr(index + 3);
		index = url.find("/");
		if (index == std::string::npos)
		{
			wurl.host = url;
			wurl.path = "/";
		}
		else
		{
			wurl.host = url.substr(0, index);
			wurl.path = url.substr(index);
		}
		index = wurl.host.find(":");
		if (index != std::string::npos)
		{
			wurl.port = atoi(wurl.host.substr(index + 1).c_str());
			wurl.host = wurl.host.substr(0, index);
		}
	}
	return wurl;
}

static std::vector<uint8_t> HttpRequest(const std::string& url, std::string method = "GET", std::string data = "1")
{
	const WURL& wurl = parse_url(url);
	std::vector<uint8_t> answer = {};
	HINTERNET hSession = InternetOpenA("OTE6uCb_E6y4uu_DDoSep", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
	if (hSession)
	{
		HINTERNET hConnect = InternetConnectA(hSession, wurl.host.c_str(), wurl.port, nullptr, nullptr, INTERNET_SERVICE_HTTP,0,0);
		if (hConnect)
		{
			HINTERNET hRequest = HttpOpenRequestA(hConnect, method.c_str(), wurl.path.c_str(), HTTP_VERSIONA, nullptr, nullptr, wurl.dwFlags, 0);
			if (hRequest)
			{
				bool bResults = HttpSendRequestA(hRequest, "Content-Type: application/x-www-form-urlencoded", 47, (LPVOID)data.c_str(), strlen(data.c_str()));
				if (bResults)
				{
					char szBuffer[8192] = { 0 };
					DWORD dwBytesRead = 0;
					do
					{
						bResults = InternetReadFile(hRequest, szBuffer, sizeof(szBuffer), &dwBytesRead);
						if (!bResults) break;
						answer.insert(answer.end(), szBuffer, szBuffer + dwBytesRead);
						ZeroMemory(szBuffer, dwBytesRead);
					} 
					while (dwBytesRead);
				}
				InternetCloseHandle(hRequest);
			}
			InternetCloseHandle(hConnect);
		}
		InternetCloseHandle(hSession);
	}
	
	return answer;
}

static std::string GetRequest(std::string url)
{
	std::string answer = "";
	int update = 0;
	while (!answer.size() && update < 10)
	{
		update = update + 1;
		std::vector<uint8_t> InternetRequest = HttpRequest(url, "GET");
		answer = std::string(InternetRequest.begin(), InternetRequest.end());
	}
	return answer;
}

static std::string PostRequest(std::string url, std::string body)
{
	std::string answer = "";
	int update = 0;
	while (!answer.size() && update < 5)
	{
		update = update + 1;
		std::vector<uint8_t> InternetRequest = HttpRequest(url, "POST", body.c_str());
		answer = std::string(InternetRequest.begin(), InternetRequest.end());
	}
	return answer;
}



/*
#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <winhttp.h>

struct WURL
{
	std::wstring host, path;
	INTERNET_PORT port;
	DWORD dwFlags;
};

static inline WURL parse_url(std::wstring url)
{
	WURL wurl = {};
	size_t index = url.find(L"://");
	if (index != std::string::npos)
	{
		const std::wstring& protocol = url.substr(0, index);
		if (protocol.compare(L"https") == 0)
		{
			wurl.dwFlags |= WINHTTP_FLAG_SECURE;
			wurl.port = 443;
		}
		else
		{
			if ((wurl.dwFlags & WINHTTP_FLAG_SECURE) == WINHTTP_FLAG_SECURE) wurl.dwFlags ^= WINHTTP_FLAG_SECURE;
			wurl.port = 80;
		}
		url = url.substr(index + 3);
		index = url.find(L"/");
		if (index == std::string::npos)
		{
			wurl.host = url;
			wurl.path = L"/";
		}
		else
		{
			wurl.host = url.substr(0, index);
			wurl.path = url.substr(index);
		}
		index = wurl.host.find(L":");
		if (index != std::string::npos)
		{
			wurl.port = _wtoi(wurl.host.substr(index + 1).c_str());
			wurl.host = wurl.host.substr(0, index);
		}
	}
	return wurl;
}

static inline std::vector<uint8_t> HttpRequest(const std::wstring& url, std::wstring method = L"GET", LPSTR data = WINHTTP_NO_REQUEST_DATA)
{
	const WURL& wurl = parse_url(url);
	std::vector<uint8_t> answer = {};
	HINTERNET hSession = WinHttpOpen(L"OTE6uCb_E6y4uu_DDoSep", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (hSession)
	{
		HINTERNET hConnect = WinHttpConnect(hSession, wurl.host.c_str(), wurl.port, 0);
		if (hConnect)
		{
			HINTERNET hRequest = WinHttpOpenRequest(hConnect, method.c_str(), wurl.path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, wurl.dwFlags);
			if (hRequest)
			{
				if (bool bResults = WinHttpSendRequest(hRequest, L"Content-Type: application/x-www-form-urlencoded", 0, data, strlen(data), strlen(data), 0))
				{
					if (bResults = WinHttpReceiveResponse(hRequest, nullptr))
					{
						char szBuffer[8192] = { 0 };
						DWORD dwBytesRead = 0;
						do
						{
							bResults = WinHttpReadData(hRequest, szBuffer, sizeof(szBuffer), &dwBytesRead);
							if (!bResults) break;
							answer.insert(answer.end(), szBuffer, szBuffer + dwBytesRead);
							ZeroMemory(szBuffer, dwBytesRead);
						}
						while (dwBytesRead);
					}
				}
			}
			WinHttpCloseHandle(hRequest);
		}
		WinHttpCloseHandle(hConnect);
	}
	WinHttpCloseHandle(hSession);
	return answer;
}


*/